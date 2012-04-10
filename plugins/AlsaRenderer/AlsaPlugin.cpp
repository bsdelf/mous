#include "AlsaRenderer.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "ALSA Renderer",
    "The Advanced Linux Sound Architecture.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, AlsaRenderer);
