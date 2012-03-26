#ifndef MOUS_IRENDERER_H
#define MOUS_IRENDERER_H

#include <inttypes.h>
#include <vector>
#include <string>
#include <common/ErrorCode.h>
#include <common/Option.h>

namespace mous {

class IRenderer
{
public:
    virtual ~IRenderer() { }

    virtual EmErrorCode Open() = 0;
    virtual void Close() = 0;

    virtual EmErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample) = 0;
    virtual EmErrorCode Write(const char* buf, uint32_t len) = 0;

    // 0(muted) to 100(max)
    virtual int GetVolumeLevel() const = 0;
    virtual void SetVolumeLevel(int level) = 0;

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const { return false; };
};

}

#endif
