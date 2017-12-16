#include "sndio_output.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "",
    "",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, SndioOutput);
