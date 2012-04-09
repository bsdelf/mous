#include "AoRenderer.h"
#include <stdlib.h>

AoRenderer::AoRenderer():
    m_Driver(-1),
    m_Device(NULL)
{
    ao_initialize();
    m_Driver = ao_default_driver_id();
}

AoRenderer::~AoRenderer()
{
}

EmErrorCode AoRenderer::Open()
{
    return ErrorCode::Ok;
}

void AoRenderer::Close()
{
}

EmErrorCode AoRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    ao_sample_format format;
    memset(&format, 0, sizeof(format));
    format.channels = channels;
    format.bits = bitsPerSample;
    format.rate = sampleRate;
    format.byte_format = AO_FMT_LITTLE;

    m_Device = ao_open_live(m_Driver, &format, NULL);
    if (m_Device == NULL)
        return ErrorCode::RendererFailedToSetup;

    return ErrorCode::Ok;
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


