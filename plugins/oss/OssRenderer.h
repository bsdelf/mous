#ifndef OSS_RENDERER_H
#define OSS_RENDERER_H

#include <plugin/IRenderer.h>
using namespace mous;

class OssRenderer: public IRenderer
{
public:
    OssRenderer();

public:
    virtual ~OssRenderer();

    virtual EmErrorCode Open();
    virtual void Close();

    virtual EmErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample);
    virtual EmErrorCode Write(const char* buf, uint32_t len);

    virtual int VolumeLevel() const;
    virtual void SetVolumeLevel(int level);

    virtual std::vector<const BaseOption*> Options() const;

private:
    std::string m_PrevPath;
    int m_Fd = -1;
    bool m_IsOpened = false;
    int32_t m_Channels = -1;
    int32_t m_SampleRate = -1;
    int32_t m_BitsPerSample = -1;
    StringOption m_OptDevicePath;
};

#endif
