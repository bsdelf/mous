#ifndef OGGDECODER_H
#define OGGDECODER_H

#include <plugin/IDecoder.h>
using namespace mous;

#include <vector>
using namespace std;

#include <wavpack/wavpack.h>

class WvDecoder: public IDecoder
{
public:
    WvDecoder();
    virtual ~WvDecoder();

    virtual std::vector<std::string> FileSuffix() const;

    virtual EmErrorCode Open(const std::string& url);
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
    WavpackContext* m_Ctx;
    vector<int32_t> m_Buf;

    int m_BytesPerSample;

    uint64_t m_UnitIndex;
    uint64_t m_UnitCount;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    int32_t m_BitRate;
    uint64_t m_Duration;
};

#endif
