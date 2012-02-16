#ifndef MACDECODER_H
#define MACDECODER_H

#include <mous/IDecoder.h>
#include <mac/All.h>
#include <mac/NoWindows.h>
#include <mac/APEDecompress.h>
using namespace std;
using namespace mous;

class MacDecoder: public IDecoder
{
public:
    MacDecoder();
    virtual ~MacDecoder();

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
    virtual int32_t GetChannels() const;
    virtual int32_t GetBitsPerSample() const;
    virtual int32_t GetSampleRate() const;
    virtual int32_t GetBitRate() const;
    virtual uint64_t GetDuration() const;

private:
    IAPEDecompress* m_pDecompress;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_BlockIndex;
    uint64_t m_BlockCount;

    uint32_t m_BlockAlign;
    uint32_t m_BlocksPerFrame;
    uint32_t m_BlocksPerRead;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    uint64_t m_Duration;
};

#endif
