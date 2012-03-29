#include "MacDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "MAC Decoder",
    "Decoder for APE audio.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, MacDecoder);
