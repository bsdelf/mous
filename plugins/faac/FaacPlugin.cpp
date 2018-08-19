#include "FaacEncoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "faac",
    "FAAC Encoder",
    1
};

MOUS_DEF_PLUGIN(PluginType::Encoder, &info, FaacEncoder);
