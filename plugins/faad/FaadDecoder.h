#ifndef FAADDECODER_H
#define FAADDECODER_H

#include <plugin/IDecoder.h>
#include <cstdlib>
#include <cstdio>
#include <neaacdec.h>
#define HAVE_STDINT_H
#include <mp4ff.h>
using namespace mous;
using namespace std;

class FaadDecoder: public IDecoder
{
public:
    FaadDecoder() = default;
    virtual ~FaadDecoder();

    virtual vector<string> FileSuffix() const;

    virtual ErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual ErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual ErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t MaxBytesPerUnit() const;
    virtual uint64_t UnitIndex() const;
    virtual uint64_t UnitCount() const;

    virtual enum AudioMode AudioMode() const;
    virtual int32_t Channels() const;
    virtual int32_t BitsPerSample() const;
    virtual int32_t SampleRate() const;
    virtual int32_t BitRate() const;
    virtual uint64_t Duration() const;

private:
    ErrorCode OpenMp4(const string& url);
    ErrorCode OpenAac(const string& url);

    ErrorCode DecodeMp4Unit(char* data, uint32_t& used, uint32_t& unitCount);
    ErrorCode DecodeAacUnit(char* data, uint32_t& used, uint32_t& unitCount);

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
    FILE* m_File = nullptr;

    bool m_IsMp4File;

    mp4ff_t* m_Infile = nullptr;
    NeAACDecHandle m_NeAACDecHandle = nullptr;

    int m_Track;

    mp4ff_callback_t m_Mp4Callback;
    unsigned int m_UseAacLength;

    int64_t m_TimeScale;
    int32_t m_FrameSize;

    uint64_t m_SampleIndex;
    uint64_t m_SampleCount;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    int32_t m_BitRate;
    uint64_t m_Duration;
};

#endif
