#include "FaacEncoder.h"
#include <stdlib.h>
#include <string.h>
#include <scx/Conv.hpp>

FaacEncoder::FaacEncoder():
    m_Mp4File(MP4_INVALID_FILE_HANDLE),
    m_Mp4Track(0),
    m_TotalSamples(0),
    m_EncodedSamples(0),
    m_FrameSize(0),
    m_DelaySamples(0),
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
    m_OutputBufferUsed(0),
    m_MediaTag(NULL)
{
    m_OptQuality.desc = "Quantizer quality";
    m_OptQuality.min = 10;
    m_OptQuality.max = 500;
    m_OptQuality.defaultVal = 100;
    m_OptQuality.userVal = 100;

    m_OptBitRate.desc = "Bit Rate";
    m_OptBitRate.min = 8;
    m_OptBitRate.max = 480;
    m_OptBitRate.defaultVal = 160;
    m_OptBitRate.userVal = 160;

    m_OptVbrOrAbr.desc = "Encode with";
    m_OptVbrOrAbr.groups.resize(2);
    m_OptVbrOrAbr.groups[0].first = "VBR";
    m_OptVbrOrAbr.groups[0].second.push_back(&m_OptQuality);
    m_OptVbrOrAbr.groups[1].first = "ABR";
    m_OptVbrOrAbr.groups[1].second.push_back(&m_OptBitRate);
    m_OptVbrOrAbr.defaultUse = 0;
    m_OptVbrOrAbr.userUse = 0;

    m_OptTns.desc = "TNS";
    m_OptTns.detail = "Use Temporal Noise Shaping";
    m_OptTns.defaultChoice = false;
    m_OptTns.userChoice = false;

    m_OptMidSide.desc = "Mid/Side";
    m_OptMidSide.detail = "Allow mid/side coding";
    m_OptMidSide.defaultChoice = true;
    m_OptMidSide.userChoice = true;

    m_OptOptimize.desc = "Optimize";
    m_OptOptimize.detail = "Optimize MP4 container layout after encoding.";
    m_OptOptimize.defaultChoice = true;
    m_OptOptimize.userChoice = true;
}

FaacEncoder::~FaacEncoder()
{
    CloseOutput();
}

const char* FaacEncoder::FileSuffix() const
{
    return "m4a";
}

EmErrorCode FaacEncoder::OpenOutput(const std::string& path)
{
    if (m_BitsPerSample != 16)
        return ErrorCode::EncoderFailedToOpen;

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

    //m_FloatBuffer = new float[m_InputSamples*sizeof(float)];

    printf("input buf:%d\n", (int)m_InputBufferSize);
    printf("output buf:%d\n", (int)m_OutputBufferSize);

    // set faac conf 
    faacEncConfigurationPtr conf = faacEncGetCurrentConfiguration(m_EncHandle);
    conf->aacObjectType = LOW;
    conf->mpegVersion = MPEG4;
    if (m_OptVbrOrAbr.userUse == 0)
        conf->quantqual = m_OptQuality.userVal;
    else
        conf->bitRate = (m_OptBitRate.userVal * 1000) / m_Channels;
    conf->allowMidside = m_OptMidSide.userChoice ? 1 : 0;
    conf->useTns = m_OptTns.userChoice ? 1 : 0;
    conf->bandWidth = 0; // disable cutoff
    conf->shortctl = SHORTCTL_NORMAL;
    conf->inputFormat = FAAC_INPUT_16BIT;
    conf->outputFormat = 0; // raw stream
    faacEncSetConfiguration(m_EncHandle, conf);

    // init mp4 file
    //MP4LogSetLevel(MP4_LOG_ERROR);
    m_Mp4File = MP4Create(path.c_str());
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
    
    WriteToolVersion();

    m_FrameSize = m_InputSamples / m_Channels;
    m_DelaySamples = m_FrameSize;

        
    return ErrorCode::Ok;
}

void FaacEncoder::CloseOutput()
{
    if (m_Mp4File != MP4_INVALID_FILE_HANDLE) {
        MP4Close(m_Mp4File);
        m_Mp4File = MP4_INVALID_FILE_HANDLE;

        UpdateMediaTag();

        if (m_OptOptimize.userChoice && !m_FileName.empty())
            MP4Optimize(m_FileName.c_str(), NULL);
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
            used = rest;
            rest = 0;
        } else {
            // write out
            memcpy(m_InputBuffer+m_InputBufferUsed, buf+used, need);

            //WavReadFloat32();
            int bytes = faacEncEncode(m_EncHandle, 
                    (int32_t*)m_InputBuffer, m_InputSamples,
                    (unsigned char*)m_OutputBuffer, m_OutputBufferSize);
            if (bytes > 0) {
                m_TotalSamples += m_InputSamples / m_Channels;
                uint64_t samples_left = m_TotalSamples - m_EncodedSamples + m_DelaySamples;
                MP4Duration dur = samples_left > m_FrameSize ? m_FrameSize : samples_left;
                MP4Duration ofs = m_EncodedSamples > 0 ? 0 : m_DelaySamples;
                MP4WriteSample(m_Mp4File, m_Mp4Track, 
                        (const uint8_t *)m_OutputBuffer, bytes, 
                        dur, ofs, true);
                m_EncodedSamples += dur;
            } else if (bytes == 0){
                printf("bytes == 0\n");
            } else {
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
    // this maybe unnecessary
    int need = m_InputBufferSize - m_InputBufferUsed;
    if (need > 0 && need != m_InputBufferSize) {
        printf("need pad: %d\n", need);
        char* empty = (char*)calloc(need, sizeof(char));
        Encode(empty, need);
        free(empty);
    }

    // flush
    int bytes = faacEncEncode(m_EncHandle, 
            (int32_t*)m_InputBuffer, 0,
            (unsigned char*)m_OutputBuffer, m_OutputBufferSize);
    if (bytes > 0) {
        printf("flushed: %d\n", bytes);
        m_TotalSamples += m_InputSamples / m_Channels;
        uint64_t samples_left = m_TotalSamples - m_EncodedSamples + m_DelaySamples;
        MP4Duration dur = samples_left > m_FrameSize ? m_FrameSize : samples_left;
        MP4Duration ofs = m_EncodedSamples > 0 ? 0 : m_DelaySamples;
        MP4WriteSample(m_Mp4File, m_Mp4Track, 
                (const uint8_t *)m_OutputBuffer, bytes, 
                dur, ofs, true);
        m_EncodedSamples += dur;
    } else if (bytes < 0) {
        return ErrorCode::EncoderFailedToFlush;
    }

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

void FaacEncoder::SetMediaTag(const MediaTag* tag)
{
    m_MediaTag = tag;
}

bool FaacEncoder::Options(std::vector<const BaseOption*>& list) const
{
    list.resize(4);
    list[0] = &m_OptVbrOrAbr;
    list[1] = &m_OptTns;
    list[2] = &m_OptMidSide;
    list[3] = &m_OptOptimize;
    return true;
}

/*
size_t FaacEncoder::WavReadFloat32()
{
    size_t i = 0;
    unsigned char* bufi = (unsigned char*)m_InputBuffer;

    while (i < m_InputSamples) {
        switch (m_BitsPerSample/8) {
            case 1:
            {
                m_FloatBuffer[i] = ((float)bufi[0] - 128) * (float)256;
                bufi += 1;
            }
                break;

            case 2:
            {
                int s = ((int16_t*)bufi)[0];
                m_FloatBuffer[i] = (float)s;
                bufi += 2;
            }
                break;

            case 3:
            {
                int s = bufi[0] | (bufi[1] << 8) | (bufi[2] << 16);
                if (s & 0x800000)
                    s |= 0xff000000;
                m_FloatBuffer[i] = (float)s / 256;
                bufi += 3;
            }
                break;

            default:
                return 0;
        }
        ++i;
    }
    return i;
}
*/

void FaacEncoder::WriteToolVersion()
{
    // set version tag
    char* faac_id_string;
    char* faac_copyright_string;
    char* version_string;
    faacEncGetVersion(&faac_id_string, &faac_copyright_string);
    int version_len = strlen(faac_id_string) + 6;
    version_string = new char[version_len];
    strcpy(version_string, "FAAC ");
    strcpy(version_string + 5, faac_id_string);
    version_string[version_len-1] = '\0';

    MP4SetMetadataTool(m_Mp4File, version_string);

    /*
    const MP4Tags* tag = MP4TagsAlloc();
    MP4TagsFetch(tag, m_Mp4File);
    MP4TagsSetEncodingTool(tag, version_string);
    MP4TagsSetArtist(tag, "hello");
    MP4TagsStore(tag, m_Mp4File);
    MP4TagsFree(tag);
    */

    delete[] version_string;
}

void FaacEncoder::UpdateMediaTag()
{
    if (m_FileName.empty() || m_MediaTag == NULL)
        return;

    MP4FileHandle file = MP4Modify(m_FileName.c_str());
    if (file == MP4_INVALID_FILE_HANDLE)
        return;
    const MP4Tags* tag = MP4TagsAlloc();
    MP4TagsFetch(tag, file);

    MP4TagsSetName(tag, m_MediaTag->title.c_str());
    MP4TagsSetArtist(tag, m_MediaTag->artist.c_str());
    MP4TagsSetAlbumArtist(tag, m_MediaTag->artist.c_str());
    MP4TagsSetAlbum(tag, m_MediaTag->album.c_str());
    MP4TagsSetComments(tag, m_MediaTag->comment.c_str());
    MP4TagsSetGenre(tag, m_MediaTag->genre.c_str());
    MP4TagsSetReleaseDate(tag, scx::NumToStr(m_MediaTag->year).c_str());
    MP4TagTrack track = { m_MediaTag->track, 0 };
    MP4TagsSetTrack(tag, &track);

    MP4TagsStore(tag, file);
    MP4TagsFree(tag);
    MP4Close(file);
}
