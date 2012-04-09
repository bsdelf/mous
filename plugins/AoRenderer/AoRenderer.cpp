#include "AoRenderer.h"
#include <string.h>

AoRenderer::AoRenderer():
    m_Driver(-1),
    m_Device(NULL)
{
    ao_initialize();

    m_Driver = ao_default_driver_id();

    int channels = 2;
    int rate = 44100;
    int bits = 16;
    Setup(channels, rate, bits);
}

AoRenderer::~AoRenderer()
{
    Close();

    ao_shutdown();
}

EmErrorCode AoRenderer::Open()
{
    m_Device = ao_open_live(m_Driver, &m_Format, NULL);
    return m_Device != NULL ? ErrorCode::Ok : ErrorCode::RendererFailedToOpen;
}

void AoRenderer::Close()
{
    if (m_Device != NULL) {
        ao_close(m_Device);
        m_Device = NULL;
    }
}

EmErrorCode AoRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    memset(&m_Format, 0, sizeof(m_Format));
    m_Format.channels = channels;
    m_Format.bits = bitsPerSample;
    m_Format.rate = sampleRate;
    m_Format.byte_format = AO_FMT_LITTLE;
    return Open() == ErrorCode::Ok ? ErrorCode::Ok : ErrorCode::RendererFailedToSetup;
}

EmErrorCode AoRenderer::Write(const char* buf, uint32_t len)
{
    int ret = ao_play(m_Device, const_cast<char*>(buf), (uint_32)len);
    return ret == 0 ? ErrorCode::Ok : ErrorCode::RendererFailedToWrite;
}

int AoRenderer::GetVolumeLevel() const
{
    return 0;
}

void AoRenderer::SetVolumeLevel(int level)
{
}

bool AoRenderer::GetOptions(std::vector<const BaseOption*>& list) const
{
    return false;
}


