#include "OggEncoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Ogg Encoder",
    "Encoder for Ogg Vorbis stream.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Encoder, &info, OggEncoder);
