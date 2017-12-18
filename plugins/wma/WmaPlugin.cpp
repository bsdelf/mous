#include "WmaDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "WMA Decoder",
    "Decoder for WMA.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, WmaDecoder);
