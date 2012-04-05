#include "FaacEncoder.h"
#include <stdlib.h>
#include <string.h>

FaacEncoder::FaacEncoder():
    m_Mp4File(MP4_INVALID_FILE_HANDLE),
    m_Mp4Track(0),
    m_EncHandle(NULL),
    m_SampleRate(0),
    m_Channels(0),
    m_InputSamples(0),
    m_MaxOutputBytes(0),
    m_BitsPerSample(0),
    m_InputBuffer(NULL),
    m_InputBufferSize(0),
    m_InputBufferUsed(0),
    m_OutputBuffer(NULL),
    m_OutputBufferSize(0),
    m_OutputBufferUsed(0)
{
    m_Tns.desc = "TNS";
    m_Tns.detail = "Use Temporal Noise Shaping";
    m_Tns.defaultChoice = false;
    m_Tns.userChoice = false;

    m_MidSide.desc = "Mid/Side";
    m_MidSide.detail = "Allow mid/side coding";
    m_MidSide.defaultChoice = true;
    m_MidSide.userChoice = true;

    m_Optimize.desc = "Optimize";
    m_Optimize.detail = "Optimize MP4 container layout after encoding.";
    m_Optimize.defaultChoice = true;
    m_Optimize.userChoice = true;
}

FaacEncoder::~FaacEncoder()
{
    CloseOutput();
}

const char* FaacEncoder::GetFileSuffix() const
{
    return "m4a";
}

EmErrorCode FaacEncoder::OpenOutput(const std::string& path)
{
    m_FileName = path;

    // init faac & buffer
    m_EncHandle = faacEncOpen(m_SampleRate, m_Channels, &m_InputSamples, &m_MaxOutputBytes);
    if (m_EncHandle == NULL)
        return ErrorCode::EncoderFailedToInit;

    m_InputBufferSize = m_InputSamples * (m_BitsPerSample/8);
    m_InputBuffer = new char[m_InputBufferSize];
    m_InputBufferUsed = 0;

    m_OutputBufferSize = m_MaxOutputBytes;   
    m_OutputBuffer = new char[m_OutputBufferSize];
    m_OutputBufferUsed = 0;

    printf("input buf:%d\n", (int)m_InputBufferSize);
    printf("output buf:%d\n", (int)m_OutputBufferSize);

    // init mp4 file
    m_Mp4File = MP4Create(path.c_str(), MP4_DETAILS_ERROR, 0);
    if (m_Mp4File == MP4_INVALID_FILE_HANDLE)
        return ErrorCode::EncoderFailedToOpen;
    MP4SetTimeScale(m_Mp4File, 90000);
    m_Mp4Track = MP4AddAudioTrack(m_Mp4File, m_SampleRate, MP4_INVALID_DURATION, MP4_MPEG4_AUDIO_TYPE);
    MP4SetAudioProfileLevel(m_Mp4File, 0x0F);

    unsigned char *ASC = NULL;
    unsigned long ASCLength = 0;
    faacEncGetDecoderSpecificInfo(m_EncHandle, &ASC, &ASCLength);
    MP4SetTrackESConfiguration(m_Mp4File, m_Mp4Track, ASC, ASCLength);
    free(ASC);

    // set faac conf 
    faacEncConfigurationPtr conf = faacEncGetCurrentConfiguration(m_EncHandle);
    conf->aacObjectType = LOW;
    conf->mpegVersion = MPEG4;
    conf->useTns = m_Tns.userChoice ? 1 : 0;
    conf->inputFormat = FAAC_INPUT_16BIT;
    conf->outputFormat = 0; // raw stream
    faacEncSetConfiguration(m_EncHandle, conf);

    // set version tag
    char* faac_id_string;
    char* faac_copyright_string;
    char* version_string;
    faacEncGetVersion(&faac_id_string, &faac_copyright_string);
    int version_len = strlen(faac_id_string) + 6;
    version_string = (char*)malloc(version_len);
    strcpy(version_string, "FAAC ");
    strcpy(version_string + 5, faac_id_string);
    version_string[version_len-1] = '\n';

    const MP4Tags* tag = MP4TagsAlloc();
    MP4TagsFetch(tag, m_Mp4File);
    MP4TagsSetEncodingTool(tag, version_string);
    MP4TagsStore(tag, m_Mp4File);
    MP4TagsFree(tag);

    free(version_string);

    return ErrorCode::Ok;
}

void FaacEncoder::CloseOutput()
{
    if (m_Mp4File != MP4_INVALID_FILE_HANDLE) {
        MP4Close(m_Mp4File);
        m_Mp4File = MP4_INVALID_FILE_HANDLE;
    }

    if (m_EncHandle != NULL) {
        faacEncClose(m_EncHandle);
        m_EncHandle = NULL;
    }

    if (m_OutputBuffer != NULL) {
        delete[] m_OutputBuffer;
        m_OutputBuffer = NULL;
    }

    if (m_InputBuffer != NULL) {
        delete[] m_InputBuffer;
        m_InputBuffer = NULL;
    }

    m_FileName.clear();
}

EmErrorCode FaacEncoder::Encode(char* buf, uint32_t len)
{
    int used = 0;
    int rest = len;
    while (rest > 0) {
        int need = m_InputBufferSize - m_InputBufferUsed;
        if (need > rest) {
            // eat up
            memcpy(m_InputBuffer+m_InputBufferUsed, buf+used, rest);
            m_InputBufferUsed += rest;
            break;
        } else {
            // flush out
            memcpy(m_InputBuffer+m_InputBufferUsed, buf+used, need);

            int ret = faacEncEncode(m_EncHandle, 
                    (int32_t*)m_InputBuffer, m_InputSamples,
                    (unsigned char*)m_OutputBuffer, m_OutputBufferSize);
            if (ret >= 0) {
                //u_int64_t samples_left = total_samples - encoded_samples + delay_samples;
                MP4Duration dur = MP4_INVALID_DURATION;//samples_left > frameSize ? frameSize : samples_left;
                MP4Duration ofs = 0;//encoded_samples > 0 ? 0 : delay_samples;
                MP4WriteSample(m_Mp4File, m_Mp4Track, 
                        (const uint8_t *)m_OutputBuffer, ret, 
                        dur, ofs, true);
            } else {
                printf("Failed!\n");
                return ErrorCode::EncoderFailedToEncode;
            }

            m_InputBufferUsed = 0;
            used += need;
            rest -= need;
        }
    }
    return ErrorCode::Ok;
}

EmErrorCode FaacEncoder::FlushRest()
{
    if (m_Optimize.userChoice)
        MP4Optimize(m_FileName.c_str(), NULL, 0);
    return ErrorCode::Ok;
}

void FaacEncoder::SetChannels(int32_t channels)
{
    m_Channels = channels;
}

void FaacEncoder::SetSampleRate(int32_t sampleRate)
{
    m_SampleRate = sampleRate;
}

void FaacEncoder::SetBitsPerSample(int32_t bitsPerSample)
{
    m_BitsPerSample = bitsPerSample;
}

bool FaacEncoder::GetOptions(std::vector<const BaseOption*>& list) const
{
    list.resize(2);
    list[0] = &m_Tns;
    list[1] = &m_Optimize;
    return true;
}
