#ifndef OSS_RENDERER_H
#define OSS_RENDERER_H

#include <mous/IRenderer.h>
using namespace mous;

class OssRenderer: public IRenderer
{
public:
    OssRenderer();

public:
    virtual ~OssRenderer();

    virtual ErrorCode OpenDevice(const std::string& path);
    virtual void CloseDevice();
    virtual ErrorCode SetupDevice(uint32_t channels, uint32_t sampleRate, uint32_t bitsPerSample);
    virtual ErrorCode WriteDevice(const char* buf, uint32_t len);

private:
    int m_Fd;
    bool m_IsOpened;
};

#endif
