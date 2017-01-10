#ifndef LAMEENCODER_H
#define LAMEENCODER_H

#include <plugin/IEncoder.h>
#include <lame/lame.h>
#include <stdio.h>
using namespace std;
using namespace mous;

class LameEncoder: public IEncoder
{
public:
    LameEncoder();
    virtual ~LameEncoder();

    virtual const char* FileSuffix() const;

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode Flush();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual void SetMediaTag(const MediaTag* tag);

    virtual std::vector<const BaseOption*> Options() const;

private:
    RangedIntOption m_Quality;
    EnumedIntOption m_BitRate;
    BooleanOption m_ReplayGain;

    lame_global_flags* m_gfp = nullptr;
    FILE* m_OutputFile = nullptr;

    int m_BitsPerSample = 0;

    vector<unsigned char> m_Buffer;

    const MediaTag* m_MediaTag = nullptr;
};

#endif
