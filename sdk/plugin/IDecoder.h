#ifndef MOUS_IDECODER_H
#define MOUS_IDECODER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <common/AudioMode.h>
#include <common/ErrorCode.h>
#include <common/Option.h>

namespace mous {

class IDecoder
{
public:
    virtual ~IDecoder() { }

    virtual std::vector<std::string> GetFileSuffix() const = 0;

    virtual EmErrorCode Open(const std::string& url) = 0;
    virtual void Close() = 0;

    virtual bool IsFormatVaild() const = 0;

    virtual EmErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount) = 0;
    virtual EmErrorCode SetUnitIndex(uint64_t index) = 0;
    virtual uint32_t GetMaxBytesPerUnit() const = 0;
    virtual uint64_t GetUnitIndex() const = 0;
    virtual uint64_t GetUnitCount() const = 0;

    virtual EmAudioMode GetAudioMode() const = 0;
    virtual int32_t GetChannels() const = 0;
    virtual int32_t GetBitsPerSample() const = 0;
    virtual int32_t GetSampleRate() const = 0;
    virtual int32_t GetBitRate() const = 0;
    virtual uint64_t GetDuration() const = 0;

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<ConstOptionPair>& list) const { return false; };
};

}

#endif
