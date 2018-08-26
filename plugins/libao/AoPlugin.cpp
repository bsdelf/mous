#include "AoOutput.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "ao",
    "Portable Audio Output",
    1
};

MOUS_DEF_PLUGIN(PluginType::Output, &info, AoOutput);
