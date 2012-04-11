#include "MacDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "MAC Decoder",
    "Ported from MAC4.11 SDK.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, MacDecoder);
