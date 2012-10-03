#include "WvDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    .author = "Yanhui Shen",
    .name = "WavPack Decoder",
    .desc = "Decoder for WavPack.",
    .version = 1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, WvDecoder);
