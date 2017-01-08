#include <util/PluginHelper.h>
#include "CoreAudioRenderer.h"

static const PluginInfo info = {
    "Yanhui Shen",
    "Core Audio Renderer",
    "Core Audio Renderer for macOS and iOS.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, CoreAudioRenderer);
