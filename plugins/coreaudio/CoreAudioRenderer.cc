#include "CoreAudioRenderer.h"
#include <iostream>
using namespace std;
#include <unistd.h>

#include "cmus/op.h"
#include "cmus/mixer.h"

const char* program_name = "CoreAudioRenderer";

CoreAudioRenderer::CoreAudioRenderer()
{
    op_pcm_ops.init(); 
    op_mixer_ops.init();
}

CoreAudioRenderer::~CoreAudioRenderer()
{
    Close();
    op_pcm_ops.exit();
    op_mixer_ops.exit();
}

ErrorCode CoreAudioRenderer::Open()
{
    int maxVol = 0;
    op_mixer_ops.open(&maxVol);
    return ErrorCode::Ok;
}

void CoreAudioRenderer::Close()
{
    op_mixer_ops.close();
}

ErrorCode CoreAudioRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    op_pcm_ops.drop();
    op_pcm_ops.close();

    const sample_format_t sf = sf_bits(bitsPerSample) | sf_rate(sampleRate) | sf_channels(channels) | sf_signed(1);
    op_pcm_ops.open(sf, nullptr);

    return ErrorCode::Ok;
}

ErrorCode CoreAudioRenderer::Write(const char* buf, uint32_t len)
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
    int l, r;
    op_mixer_ops.get_volume(&l, &r);
    return (l + r) / 2;
}

void CoreAudioRenderer::SetVolumeLevel(int avg)
{
    op_mixer_ops.set_volume(avg, avg);
}

std::vector<const BaseOption*> CoreAudioRenderer::Options() const
{
    return {};
}
