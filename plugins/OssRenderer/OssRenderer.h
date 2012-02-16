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
    virtual ErrorCode SetupDevice(int32_t channels, int32_t sampleRate, int32_t bitsPerSample);
    virtual ErrorCode WriteDevice(const char* buf, uint32_t len);

private:
    int m_Fd;
    bool m_IsOpened;
};

#endif
