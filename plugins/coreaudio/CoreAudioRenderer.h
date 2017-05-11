#pragma once

#include <plugin/IRenderer.h>
using namespace mous;

class CoreAudioRenderer: public IRenderer
{
public:
    CoreAudioRenderer();

public:
    virtual ~CoreAudioRenderer();

    virtual ErrorCode Open();
    virtual void Close();

    virtual ErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample);
    virtual ErrorCode Write(const char* buf, uint32_t len);

    virtual int VolumeLevel() const;
    virtual void SetVolumeLevel(int level);

    virtual std::vector<const BaseOption*> Options() const;
};

