#include "AlsaRenderer.h"

AlsaRenderer::AlsaRenderer():
    m_DeviceName("default"),
    m_PcmHandle(NULL),
    m_HwParams(NULL),
    m_SwParams(NULL),
    m_Dir(0),
    m_Resample(0)
{
}

AlsaRenderer::~AlsaRenderer()
{
}

EmErrorCode AlsaRenderer::Open()
{
    int ret = ::snd_pcm_open(&m_PcmHandle, m_DeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC);
    if (ret != 0)
        return ErrorCode::RendererFailedToOpen;

    return ErrorCode::Ok;
}

void AlsaRenderer::Close()
{
    ::snd_pcm_drain(m_PcmHandle);
    ::snd_pcm_close(m_PcmHandle);
}

EmErrorCode AlsaRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
}

EmErrorCode AlsaRenderer::Write(const char* buf, uint32_t len)
{
    int writen = ::snd_pcm_writei(m_PcmHandle, buf, 10);
}

int AlsaRenderer::GetVolumeLevel() const
{
}

void AlsaRenderer::SetVolumeLevel(int level)
{
}

bool AlsaRenderer::GetOptions(vector<const BaseOption*>& list) const
{
}
