#include "CuePack.h"
#include <plugin/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Cue Pack",
    "Cue sheet parser.",
    1
};

MOUS_DEF_PLUGIN(PluginType::MediaPack, &info, CuePack);
