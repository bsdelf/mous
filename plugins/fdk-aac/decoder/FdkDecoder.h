#ifndef FAADDECODER_H
#define FAADDECODER_H

//#include <cstdlib>
//#include <cstdio>

#include <plugin/IDecoder.h>
using namespace mous;
using namespace std;

#include <fdk-aac/aacdecoder_lib.h>
#include <mp4v2/mp4v2.h>

class FdkDecoder: public IDecoder
{
public:
    FdkDecoder();
    virtual ~FdkDecoder();

    virtual vector<string> FileSuffix() const;

    virtual EmErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual EmErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual EmErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t MaxBytesPerUnit() const;
    virtual uint64_t UnitIndex() const;
    virtual uint64_t UnitCount() const;

    virtual EmAudioMode AudioMode() const;
    virtual int32_t Channels() const;
    virtual int32_t BitsPerSample() const;
    virtual int32_t SampleRate() const;
    virtual int32_t BitRate() const;
    virtual uint64_t Duration() const;

private:
    EmErrorCode OpenMP4(const string& url);
    EmErrorCode OpenAAC(const string& url);

    EmErrorCode DecodeMp4Unit(char* data, uint32_t& used, uint32_t& unitCount);
    EmErrorCode DecodeAacUnit(char* data, uint32_t& used, uint32_t& unitCount);

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
    bool m_ismp4 = false;

    // mp4v2
    MP4FileHandle m_mp4 = nullptr;
    int m_trackid = -1;

    // fdk-aac
    HANDLE_AACDECODER m_decoder = nullptr;

    // minor
    int32_t m_bitrate;
    uint64_t m_duration;

    // meta data
    int32_t m_channels;
    int64_t m_timescale;
    int32_t m_bits;

    // sample info
    uint64_t m_sampleid;
    uint64_t m_nsamples;
    vector<uint8_t> m_samplebuff;
};

#endif
