#ifndef MOUS_IDECODER_H
#define MOUS_IDECODER_H

#include <string>
#include <vector>
#include "ErrorCode.h"

namespace mous {

enum AudioMode {
    MousMono,
    MousStereo,
    MousJointStero,
    MousDualChannel
};

class IDecoder 
{
public:
    virtual ~IDecoder() { }

    virtual void GetFileSuffix(std::vector<std::string>& list) const = 0;

    virtual ErrorCode Open(const std::string& url) = 0;
    virtual void Close() = 0;

    virtual bool IsFormatVaild() const = 0;

    virtual ErrorCode ReadUnit() = 0;
    virtual ErrorCode SetUnitIndex(uint32_t index) = 0;
    virtual uint32_t GetUnitIndex() const = 0;
    virtual uint32_t GetUnitCount() const = 0;
    virtual uint32_t GetMsPerUnit() const = 0;

    virtual AudioMode GetAudioMode() const = 0;
    virtual uint32_t GetBitRate() const = 0;
    virtual uint32_t GetSampleRate() const = 0;
    virtual uint64_t GetDuration() const = 0;
};

}

#endif
