#include "WavDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Wav Decoder",
    "WAV -> PCM",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, WavDecoder);

