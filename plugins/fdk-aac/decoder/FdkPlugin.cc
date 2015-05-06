#include "FdkDecoder.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "FDK Decoder",
    "Decoder for mp4 audio.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Decoder, &info, FdkDecoder);
