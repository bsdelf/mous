#include "FaadDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "faad",
    "FAAD Decoder",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, FaadDecoder);
