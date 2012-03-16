#ifndef FAADDECODER_H
#define FAADDECODER_H

#include <plugin/IDecoder.h>
#include <cstdlib>
#include <cstdio>
#include <neaacdec.h>
#include <mp4ff.h>
using namespace mous;
using namespace std;

class FaadDecoder: public IDecoder
{
public:
    FaadDecoder();
    virtual ~FaadDecoder();

    virtual vector<string> GetFileSuffix() const;

    virtual EmErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual EmErrorCode ReadUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual EmErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t GetMaxBytesPerUnit() const;
    virtual uint64_t GetUnitIndex() const;
    virtual uint64_t GetUnitCount() const;

    virtual EmAudioMode GetAudioMode() const;
    virtual int32_t GetChannels() const;
    virtual int32_t GetBitsPerSample() const;
    virtual int32_t GetSampleRate() const;
    virtual int32_t GetBitRate() const;
    virtual uint64_t GetDuration() const;

private:
    EmErrorCode OpenMp4(const string& url);
    EmErrorCode OpenAac(const string& url);

    EmErrorCode ReadUnitMp4(char* data, uint32_t& used, uint32_t& unitCount);
    EmErrorCode ReadUnitAac(char* data, uint32_t& used, uint32_t& unitCount);

private:
    struct AudioFile
    {
        int outputFormat;
        char* output;
        unsigned int fileType;
        unsigned long samplerate;
        unsigned int bits_per_sample;
        unsigned int channels;
        unsigned long total_samples;
        long channelMask;
    };

private:
    static int GetAACTrack(mp4ff_t* infile);

    static uint32_t ReadCallback(void* userData, void* buffer, uint32_t length);
    static uint32_t SeekCallback(void* userData, uint64_t pos);

    static long aacChannelConfig2wavexChannelMask(NeAACDecFrameInfo *hInfo);

private:
    FILE* m_File;

    bool m_IsMp4File;

    mp4ff_t* m_Infile;
    NeAACDecHandle m_pDecoder;
    NeAACDecFrameInfo m_FrameInfo;

    int m_Track;

    mp4ff_callback_t m_Mp4Callback;
    unsigned int m_UseAacLength;

    int64_t m_TimeScale;
    int32_t m_FrameSize;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_SampleIndex;
    uint64_t m_SampleCount;

    uint32_t m_BlockAlign;
    uint32_t m_BlocksPerFrame;
    uint32_t m_BlocksPerRead;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    int32_t m_BitRate;
    uint64_t m_Duration;
};

#endif
