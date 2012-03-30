#include "AlsaRenderer.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "ALSA Renderer",
    "ALSA output for Linux.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, AlsaRenderer);
