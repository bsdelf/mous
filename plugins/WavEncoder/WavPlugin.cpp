#include "WavEncoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Wav Encoder",
    "PCM -> Wav",
    1
};

MOUS_DEF_PLUGIN(PluginType::Encoder, &info, WavEncoder);

