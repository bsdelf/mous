#include <util/PluginHelper.h>
#include "CoreAudioOutput.h"

static const PluginInfo info = {
    "Yanhui Shen",
    "Core Audio Output",
    "Core Audio Output for macOS and iOS.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Output, &info, CoreAudioOutput);
