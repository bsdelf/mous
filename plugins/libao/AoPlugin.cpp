#include "AoOutput.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "AO Output",
    "Portable audio output library.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Output, &info, AoOutput);
