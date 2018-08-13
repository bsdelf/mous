#include "CoreAudioOutput.h"
#include <iostream>
using namespace std;
#include <unistd.h>

extern "C" {
#include "cmus/op.h"
#include "cmus/mixer.h"
}

const char* program_name = "CoreAudioOutput";

CoreAudioOutput::CoreAudioOutput()
{
    op_pcm_ops.init(); 
    op_mixer_ops.init();
}

CoreAudioOutput::~CoreAudioOutput()
{
    Close();
    op_pcm_ops.exit();
    op_mixer_ops.exit();
}

ErrorCode CoreAudioOutput::Open()
{
    int maxVol = 0;
    op_mixer_ops.open(&maxVol);
    closed = false;
    return ErrorCode::Ok;
}

void CoreAudioOutput::Close()
{
    if (!closed) {
        op_mixer_ops.close();
        closed = true;
    }
}

ErrorCode CoreAudioOutput::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    op_pcm_ops.drop();
    op_pcm_ops.close();

    const sample_format_t sf = sf_bits(bitsPerSample) | sf_rate(sampleRate) | sf_channels(channels) | sf_signed(1);
    op_pcm_ops.open(sf, nullptr);

    return ErrorCode::Ok;
}

ErrorCode CoreAudioOutput::Write(const char* buf, uint32_t len)
{
    op_pcm_ops.write(buf, static_cast<int>(len));
    op_pcm_wait(len);
    return ErrorCode::Ok;
}

int CoreAudioOutput::VolumeLevel() const
{
    int l, r;
    op_mixer_ops.get_volume(&l, &r);
    return (l + r) / 2;
}

void CoreAudioOutput::SetVolumeLevel(int avg)
{
    op_mixer_ops.set_volume(avg, avg);
}

std::vector<const BaseOption*> CoreAudioOutput::Options() const
{
    return {};
}
