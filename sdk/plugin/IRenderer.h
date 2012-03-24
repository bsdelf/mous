#ifndef MOUS_IRENDERER_H
#define MOUS_IRENDERER_H

#include <inttypes.h>
#include <string>
#include <common/ErrorCode.h>
#include <common/Option.h>

namespace mous {

class IRenderer: public IOptionProvider
{
public:
    virtual ~IRenderer() { }

    virtual EmErrorCode OpenDevice(const std::string& path) = 0;
    virtual void CloseDevice() = 0;

    virtual EmErrorCode SetupDevice(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample) = 0;
    virtual EmErrorCode WriteDevice(const char* buf, uint32_t len) = 0;

    // 0(muted) to 100(max)
    virtual int GetVolumeLevel() const = 0;
    virtual void SetVolumeLevel(int level) = 0;
};

}

#endif
