#include "AoRenderer.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "AO Renderer",
    "Portable audio output library.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, AoRenderer);
