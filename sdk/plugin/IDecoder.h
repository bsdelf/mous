#ifndef MOUS_IDECODER_H
#define MOUS_IDECODER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/Option.h>

namespace mous {

class IDecoder
{
public:
    virtual ~IDecoder() { }

    virtual std::vector<std::string> FileSuffix() const = 0;

    virtual EmErrorCode Open(const std::string& url) = 0;
    virtual void Close() = 0;

    virtual bool IsFormatVaild() const = 0;

    virtual EmErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount) = 0;
    virtual EmErrorCode SetUnitIndex(uint64_t index) = 0;
    virtual uint32_t MaxBytesPerUnit() const = 0;
    virtual uint64_t UnitIndex() const = 0;
    virtual uint64_t UnitCount() const = 0;

    virtual EmAudioMode AudioMode() const = 0;
    virtual int32_t Channels() const = 0;
    virtual int32_t BitsPerSample() const = 0;
    virtual int32_t SampleRate() const = 0;
    virtual int32_t BitRate() const = 0;
    virtual uint64_t Duration() const = 0;

    // reimplement this to provide options
    virtual std::vector<const BaseOption*> Options() const
    {
        return std::vector<const BaseOption*>();
    };
};

}

#endif
