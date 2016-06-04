#pragma once

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

    virtual const char* FileSuffix() const = 0;

    virtual EmErrorCode OpenOutput(const std::string& path) = 0;
    virtual void CloseOutput() = 0;

    virtual EmErrorCode Encode(char* dat, uint32_t len) = 0;
    virtual EmErrorCode FlushRest() = 0;

    // these will be called before OpenOutput()
    virtual void SetChannels(int32_t channels) = 0;
    virtual void SetSampleRate(int32_t sampleRate) = 0;
    virtual void SetBitsPerSample(int32_t bitsPerSample) = 0;

    // reimplement this to support tagging
    // called before OpenOutput()
    // you can write tag after open but before close
    virtual void SetMediaTag(const MediaTag* tag)
    {
    }

    // reimplement this to provide options
    virtual std::vector<const BaseOption*> Options() const 
    { 
        return std::vector<const BaseOption*>();
    };
};

}
