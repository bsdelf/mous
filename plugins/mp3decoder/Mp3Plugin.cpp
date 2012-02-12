#include "Mp3Decoder.h"
#include <mous/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "MPG123 Decoder",
    "Decoder for MPG layer 1/2/3 audio.",
    1
};

MOUS_DEF_PLUGIN(MousDecoder, &info, IDecoder, Mp3Decoder);
