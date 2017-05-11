#ifndef OGGENCODER_H
#define OGGENCODER_H

#include <plugin/IEncoder.h>
using namespace mous;
using namespace std;

#include <stdio.h>
#include <vorbis/vorbisenc.h>

class OggEncoder: public IEncoder
{
public:
    OggEncoder();
    virtual ~OggEncoder();

    virtual const char* FileSuffix() const;

    virtual ErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual ErrorCode Encode(char* buf, uint32_t len);
    virtual ErrorCode Flush();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual void SetMediaTag(const MediaTag* tag);

    virtual std::vector<const BaseOption*> Options() const;

private:
    RangedFloatOption m_Quality;
    EnumedIntOption m_BitRate;
    GroupedOption m_VbrOrAbr;

    FILE* m_OutputFile;

    int m_Channels;
    int m_SampleRate;
    int m_BitsPerSample;

    vorbis_info m_VobisInfo;

    const MediaTag* m_MediaTag;
};

#endif
