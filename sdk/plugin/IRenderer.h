#pragma once

#include <inttypes.h>
#include <vector>
#include <string>
#include <util/ErrorCode.h>
#include <util/Option.h>

namespace mous {

class IRenderer
{
public:
    virtual ~IRenderer() { }

    virtual EmErrorCode Open() = 0;
    virtual void Close() = 0;

    virtual EmErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample) = 0;
    virtual EmErrorCode Write(const char* dat, uint32_t len) = 0;

    // 0(muted) to 100(max)
    virtual int VolumeLevel() const = 0;
    virtual void SetVolumeLevel(int level) = 0;

    // reimplement this to provide options
    virtual std::vector<const BaseOption*> Options() const
    {
        return {};
    };
};

}
