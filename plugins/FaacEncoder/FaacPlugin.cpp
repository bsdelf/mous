#include "FaacEncoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "FAAC Encoder",
    "MPEG-2 and MPEG-4 AAC audio encoder.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Encoder, &info, FaacEncoder);
