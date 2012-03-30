#ifndef ALSARENDERER_H
#define ALSARENDERER_H

#include <plugin/IRenderer.h>
using namespace std;
using namespace mous;

class AlsaRenderer: public IRenderer
{
public:
    AlsaRenderer();
    virtual ~AlsaRenderer();

    virtual EmErrorCode Open();
    virtual void Close();

    virtual EmErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample);
    virtual EmErrorCode Write(const char* buf, uint32_t len);

    virtual int GetVolumeLevel() const;
    virtual void SetVolumeLevel(int level);

    virtual bool GetOptions(std::vector<const BaseOption*>& list) const;

};

#endif
