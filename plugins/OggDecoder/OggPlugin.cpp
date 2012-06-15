#include "OggDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Ogg Decoder",
    "Decoder for Ogg Vorbis stream.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, OggDecoder);
