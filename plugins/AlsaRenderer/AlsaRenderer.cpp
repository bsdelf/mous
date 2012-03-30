#include "AlsaRenderer.h"
AlsaRenderer::AlsaRenderer()
{
}

AlsaRenderer::~AlsaRenderer()
{
}

EmErrorCode AlsaRenderer::Open()
{
}

void AlsaRenderer::Close()
{
}

EmErrorCode AlsaRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
}

EmErrorCode AlsaRenderer::Write(const char* buf, uint32_t len)
{
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
