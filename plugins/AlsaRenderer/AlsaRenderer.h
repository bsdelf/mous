#ifndef ALSARENDERER_H
#define ALSARENDERER_H

#include <alsa/asoundlib.h>
#include <plugin/IRenderer.h>
#include <string>
using namespace std;
using namespace mous;

template <typename T>
struct Tuple
{
    T val;
    T max;
    T min;
};

class AlsaRenderer: public IRenderer
{
public:
    AlsaRenderer();
    virtual ~AlsaRenderer();

    virtual EmErrorCode Open();
    virtual void Close();

    virtual EmErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample);
    virtual EmErrorCode Write(const char* buf, uint32_t len);

    virtual int VolumeLevel() const;
    virtual void SetVolumeLevel(int level);

    virtual bool Options(std::vector<const BaseOption*>& list) const;

private:
    bool SetupHwParams();
    void SetupSwParams();

private:
    string m_DeviceName;

    snd_pcm_t* m_PcmHandle;

    int m_Dir;
    int m_Resample;
    snd_pcm_access_t m_Access;
    snd_pcm_format_t m_Format;

    int m_FrameLength;
    int m_BitsPerSample;
    Tuple<unsigned int> m_Channels;
    Tuple<unsigned int> m_SampleRate;
    Tuple<unsigned int> m_BufferTime;
    Tuple<unsigned int> m_PeriodTime;
    Tuple<snd_pcm_uframes_t> m_BufferSize;
    Tuple<snd_pcm_uframes_t> m_PeriodSize;
};

#endif
