#ifndef MP3DECODER_H
#define MP3DECODER_H

#include <mous/IDecoder.h>
#include "mpg123.h"
using namespace std;
using namespace mous;

class Mpg123Decoder: public IDecoder
{
public:
    Mpg123Decoder();
    virtual ~Mpg123Decoder();

    virtual void GetFileSuffix(std::vector<std::string>& list) const;

    virtual ErrorCode Open(const std::string& url);
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
    mpg123_handle* m_pHandle;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_UnitIndex;
    uint64_t m_UnitCount;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    uint64_t m_Duration;
};

#endif
