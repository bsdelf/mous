#include "FaadDecoder.h"
#include "FaadHelper.h"
#include <iostream>
using namespace std;

FaadDecoder::FaadDecoder():
    m_File(NULL),
    m_Infile(NULL),
    m_NeAACDecHandle(NULL)
{

}

FaadDecoder::~FaadDecoder()
{
}

vector<string> FaadDecoder::GetFileSuffix() const
{
    vector<string> list;
    list.push_back("m4a");
    list.push_back("aac");
    list.push_back("mp4");
    return list;
}

EmErrorCode FaadDecoder::Open(const string& url)
{
    /**
     * Check for mp4 file
     */
    m_IsMp4File = false;

    FILE* file = fopen(url.c_str(), "rb");
    if (file == NULL)
        return ErrorCode::DecoderFailedToOpen;

    unsigned char header[8];
    size_t rcount = fread(header, 1, 8, file);
    fclose(file);
    if (rcount != 8)
        return ErrorCode::DecoderFailedToOpen;

    if (header[4] == 'f' && header[5] == 't' && 
            header[6] == 'y' && header[7] == 'p') {
        m_IsMp4File = true;
    }

    return m_IsMp4File ? OpenMp4(url) : OpenAac(url);
}

EmErrorCode FaadDecoder::OpenMp4(const string& url)
{
    NeAACDecConfigurationPtr config;
    mp4AudioSpecificConfig mp4Asc;

    /**
     * For gapless decoding
     */
    m_UseAacLength = 1;

    m_File = fopen(url.c_str(), "rb");
    if (m_File == NULL)
        return ErrorCode::DecoderFailedToOpen;

    m_Mp4Callback.read = &ReadCallback;
    m_Mp4Callback.seek = &SeekCallback;
    m_Mp4Callback.user_data = m_File;

    m_NeAACDecHandle = NeAACDecOpen();
    config = NeAACDecGetCurrentConfiguration(m_NeAACDecHandle);
    config->outputFormat = FAAD_FMT_16BIT;//outputFormat;
    config->downMatrix = 0;//downMatrix;
    NeAACDecSetConfiguration(m_NeAACDecHandle, config);

    m_Infile = mp4ff_open_read(&m_Mp4Callback);
    if (m_Infile == NULL)
        return ErrorCode::DecoderFailedToOpen;

    m_Track = GetAACTrack(m_Infile);
    if (m_Track < 0) {
        Close();
        return ErrorCode::DecoderFailedToInit;
    }

    unsigned char* confBuf = NULL;
    unsigned int confBufSize = 0;
    mp4ff_get_decoder_config(m_Infile, m_Track, &confBuf , &confBufSize);

    unsigned long sampleRate;
    unsigned char channels;
    if (NeAACDecInit2(m_NeAACDecHandle, confBuf, confBufSize, &sampleRate, &channels) < 0) {
        Close();
        return ErrorCode::DecoderFailedToInit;
    }
    m_SampleRate = sampleRate;
    m_Channels = channels;
    m_BitsPerSample = 16;

    m_TimeScale = mp4ff_time_scale(m_Infile, m_Track);
    m_FrameSize = 1024;
    m_UseAacLength = 0;

    if (confBuf != NULL) {
        if (NeAACDecAudioSpecificConfig(confBuf, confBufSize, &mp4Asc) >= 0) {
            if (mp4Asc.frameLengthFlag == 1)
                m_FrameSize = 960;
            if (mp4Asc.sbr_present_flag == 1)
                m_FrameSize *= 2;
        }
        free(confBuf);
    }

    m_SampleCount = mp4ff_num_samples(m_Infile, m_Track);
    m_SampleIndex = 0;

    float f = 1024.0;
    if (mp4Asc.sbr_present_flag == 1) {
        f *= 2.0;
    }
    m_Duration = (float)m_SampleCount*(float)(f-1.0)/(float)mp4Asc.samplingFrequency * 1000;

    return ErrorCode::Ok;
}

EmErrorCode FaadDecoder::OpenAac(const string& url)
{
    return ErrorCode::DecoderFailedToOpen;
}

void FaadDecoder::Close()
{
    if (m_NeAACDecHandle != NULL) {
        NeAACDecClose(m_NeAACDecHandle);
        m_NeAACDecHandle = NULL;
    }

    if (m_Infile != NULL) {
        mp4ff_close(m_Infile);
        m_Infile = NULL;
    }

    if (m_File != NULL) {
        fclose(m_File);
        m_File = NULL;
    }
}

bool FaadDecoder::IsFormatVaild() const
{
    return false;
}

EmErrorCode FaadDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    return m_IsMp4File ? 
        DecodeMp4Unit(data, used, unitCount) : DecodeAacUnit(data, used, unitCount);
}

EmErrorCode FaadDecoder::DecodeMp4Unit(char* data, uint32_t& used, uint32_t& unitCount)
{
    unsigned char* buffer = NULL;
    unsigned int bufferSize = 0;

    m_BitRate = mp4ff_get_avg_bitrate(m_Infile, m_Track);

    long duration = mp4ff_get_sample_duration(m_Infile, m_Track, m_SampleIndex);
    int rc = mp4ff_read_sample(m_Infile, m_Track, m_SampleIndex, &buffer,  &bufferSize);

    if (rc == 0) {
        Close();
        return ErrorCode::DecoderFailedToRead;
    }

    NeAACDecFrameInfo frameInfo;
    void* sampleBuf = NeAACDecDecode(m_NeAACDecHandle, &frameInfo, buffer, bufferSize);
    if (buffer != NULL)
        free(buffer);

    unsigned int sampleCount;
    unsigned int delay = 0;

    // gapless
    {
        if (m_SampleIndex == 0)
            duration = 0;

        if (m_UseAacLength || (m_TimeScale != m_SampleRate)) {
            sampleCount = frameInfo.samples;
        } else {
            sampleCount = (unsigned int)(duration * frameInfo.channels);
            if (sampleCount > frameInfo.samples)
                sampleCount = frameInfo.samples;

            if (!m_UseAacLength && 
                    (m_SampleIndex != 0) &&
                    (m_SampleIndex < m_SampleCount/2) &&
                    (sampleCount != frameInfo.samples)) {
                m_UseAacLength = 1;
                sampleCount = frameInfo.samples;
            }
        }

        if ((m_SampleIndex == 0) &&
                (sampleCount < (unsigned int)m_FrameSize*frameInfo.channels) && 
                (frameInfo.samples > sampleCount)) {
            delay = frameInfo.samples - sampleCount;
        }

        if ((frameInfo.error == 0) && (sampleCount > 0)) {
            audio_file aufile = {
                FAAD_FMT_16BIT,
                NULL,
                0, 
                0,
                16, // bits/sample
                2,  // channels
                0,
                0
            };
            aufile.outputBuf = data;
            aufile.samplerate = m_SampleRate;
            aufile.channelMask = aacChannelConfig2wavexChannelMask(&frameInfo);
            used = write_audio_file(&aufile, sampleBuf, sampleCount, delay);
        }
    }
    unitCount = 1;
    m_SampleIndex += 1;

    return ErrorCode::Ok;
}

EmErrorCode FaadDecoder::DecodeAacUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    return ErrorCode::Ok;
}

EmErrorCode FaadDecoder::SetUnitIndex(uint64_t index)
{
    m_SampleIndex = index;
    return ErrorCode::Ok;
}

uint32_t FaadDecoder::GetMaxBytesPerUnit() const
{
    return 10240;//m_BlocksPerRead * m_BlockAlign;
}

uint64_t FaadDecoder::GetUnitIndex() const
{
    return m_SampleIndex;
}

uint64_t FaadDecoder::GetUnitCount() const
{
    return m_SampleCount;
}

EmAudioMode FaadDecoder::GetAudioMode() const
{
    return AudioMode::Stereo;
}

int32_t FaadDecoder::GetChannels() const
{
    return m_Channels;
}

int32_t FaadDecoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t FaadDecoder::GetSampleRate() const
{
    return m_SampleRate;
}

int32_t FaadDecoder::GetBitRate() const
{
    return m_BitRate / 1000;
}

uint64_t FaadDecoder::GetDuration() const
{
    return m_Duration;
}

int FaadDecoder::GetAACTrack(mp4ff_t* infile)
{
    /* find AAC track */
    int i, rc;
    int numTracks = mp4ff_total_tracks(infile);

    for (i = 0; i < numTracks; i++) {
        unsigned char *buff = NULL;
        unsigned int buff_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config(infile, i, &buff, &buff_size);

        if (buff) {
            rc = NeAACDecAudioSpecificConfig(buff, buff_size, &mp4ASC);
            free(buff);

            if (rc < 0)
                continue;

            return i;
        }
    }

    /* can't decoder this */
    return -1;
}

uint32_t FaadDecoder::ReadCallback(void* userData, void* buffer, uint32_t length)
{
    return fread(buffer, 1, length, (FILE*)userData);
}

uint32_t FaadDecoder::SeekCallback(void* userData, uint64_t pos)
{
    return fseek((FILE*)userData, pos, SEEK_SET);
}

#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000

long FaadDecoder::aacChannelConfig2wavexChannelMask(NeAACDecFrameInfo *hInfo)
{
    if (hInfo->channels == 6 && hInfo->num_lfe_channels) {
        return SPEAKER_FRONT_LEFT + SPEAKER_FRONT_RIGHT +
            SPEAKER_FRONT_CENTER + SPEAKER_LOW_FREQUENCY +
            SPEAKER_BACK_LEFT + SPEAKER_BACK_RIGHT;
    } else {
        return 0;
    }
}
