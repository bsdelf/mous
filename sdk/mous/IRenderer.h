#ifndef MOUS_IRENDERER_H
#define MOUS_IRENDERER_H

#include <inttypes.h>
#include <string>
#include <mous/ErrorCode.h>

namespace mous {

class IRenderer
{
public:
    virtual ~IRenderer() { }

    virtual EmErrorCode OpenDevice(const std::string& path) = 0;
    virtual void CloseDevice() = 0;
    virtual EmErrorCode SetupDevice(int32_t channels, int32_t sampleRate, int32_t bitsPerSample) = 0;
    virtual EmErrorCode WriteDevice(const char* buf, uint32_t len) = 0;
};

}

#endif
