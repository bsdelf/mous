#include "FaadDecoder.h"
#include <plugin/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "FAAD Decoder",
    "Decoder for mp4 audio.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, FaadDecoder);
