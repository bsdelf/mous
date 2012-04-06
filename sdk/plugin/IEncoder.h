#ifndef MOUS_IENCODER_H
#define MOUS_IENCODER_H

#include <string>
#include <vector>
#include <inttypes.h>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/MediaTag.h>
#include <util/Option.h>

namespace mous {

class IEncoder
{
public:
    virtual ~IEncoder() { }

    virtual const char* GetFileSuffix() const = 0;

    virtual EmErrorCode OpenOutput(const std::string& path) = 0;
    virtual void CloseOutput() = 0;

    virtual EmErrorCode Encode(char* buf, uint32_t len) = 0;
    virtual EmErrorCode FlushRest() = 0;

    virtual void SetChannels(int32_t channels) = 0;
    virtual void SetSampleRate(int32_t sampleRate) = 0;
    virtual void SetBitsPerSample(int32_t bitsPerSample) = 0;

    // reimplement this to support tagging
    virtual void WriteTag(const MediaTag& tag) const
    {
    }

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const 
    { 
        list.clear(); 
        return false; 
    };
};

}

#endif
