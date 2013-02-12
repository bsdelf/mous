#include "WvDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "WavPack Decoder",
    "Decoder for WavPack.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, WvDecoder);
