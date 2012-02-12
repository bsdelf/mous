#ifndef APEDECODER_H
#define APEDECODER_H

#include <mous/IDecoder.h>
#include <mac/All.h>
#include <mac/NoWindows.h>
#include <mac/APEDecompress.h>
using namespace mous;
using namespace std;

class ApeDecoder: public IDecoder
{
public:
    ApeDecoder();
    virtual ~ApeDecoder();

    virtual void GetFileSuffix(vector<string>& list) const;

    virtual ErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual ErrorCode ReadUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual ErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t GetMaxBytesPerUnit() const;
    virtual uint64_t GetUnitIndex() const;
    virtual uint64_t GetUnitCount() const;

    virtual AudioMode GetAudioMode() const;
    virtual uint32_t GetChannels() const;
    virtual uint32_t GetBitsPerSample() const;
    virtual uint32_t GetSampleRate() const;
    virtual uint64_t GetDuration() const;

private:
    IAPEDecompress* m_pDecompress;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_BlockIndex;
    uint64_t m_BlockCount;

    uint32_t m_BlockAlign;
    uint32_t m_BlocksPerFrame;

    uint32_t m_Channels;
    uint32_t m_BitsPerSample;
    uint32_t m_SampleRate;
    uint64_t m_Duration;
};

#endif
