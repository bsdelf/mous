#ifndef MOUS_IRENDERER_H
#define MOUS_IRENDERER_H

#include <string>
#include <mous/ErrorCode.h>

namespace mous {

class IRenderer
{
public:
    virtual ~IRenderer() { }

    virtual ErrorCode OpenDevice(const std::string& path) = 0;
    virtual void CloseDevice() = 0;
    virtual ErrorCode SetupDevice(uint32_t channels, uint32_t sampleRate, uint32_t bitsPerSample) = 0;
    virtual ErrorCode WriteDevice(const char* buf, uint32_t len) = 0;
};

}

#endif
