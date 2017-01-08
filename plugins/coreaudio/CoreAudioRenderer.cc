#include "CoreAudioRenderer.h"
#include <iostream>
using namespace std;
#include <unistd.h>

#include "cmus/op.h"

const char* program_name = "CoreAudioRenderer";

CoreAudioRenderer::CoreAudioRenderer()
{
    op_pcm_ops.init(); 
}

CoreAudioRenderer::~CoreAudioRenderer()
{
    Close();
    op_pcm_ops.exit();
}

EmErrorCode CoreAudioRenderer::Open()
{
    return ErrorCode::Ok;
}

void CoreAudioRenderer::Close()
{
}

EmErrorCode CoreAudioRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    op_pcm_ops.drop();
    op_pcm_ops.close();

    const sample_format_t sf = sf_bits(bitsPerSample) | sf_rate(sampleRate) | sf_channels(channels) | sf_signed(1);
    op_pcm_ops.open(sf, nullptr);

    return ErrorCode::Ok;
}

EmErrorCode CoreAudioRenderer::Write(const char* buf, uint32_t len)
{
    op_pcm_ops.write(buf, static_cast<int>(len));
    while (1) {
        const int space = op_pcm_ops.buffer_space();
        if (space >= len) {
            break;
        }
        usleep(10*1000);
    }
    return ErrorCode::Ok;
}

int CoreAudioRenderer::VolumeLevel() const
{
    return 0;
}

void CoreAudioRenderer::SetVolumeLevel(int avg)
{
}

std::vector<const BaseOption*> CoreAudioRenderer::Options() const
{
    std::vector<const BaseOption*> list(0);
    return list;
}
