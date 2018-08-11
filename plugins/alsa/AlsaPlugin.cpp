#include "AlsaOutput.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "ALSA Output",
    "The Advanced Linux Sound Architecture.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Output, &info, AlsaOutput);
