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

    virtual int GetVolumeLevel() const;
    virtual void SetVolumeLevel(int level);

    virtual bool GetOptions(std::vector<ConstOptionPair>& list) const;

private:
    std::string m_PrevPath;
    int m_Fd;
    bool m_IsOpened;
    int32_t m_Channels;
    int32_t m_SampleRate;
    int32_t m_BitsPerSample;

    StringOption m_OptDevicePath;
};

#endif
