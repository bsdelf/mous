#include "Mp3Decoder.h"
#include <mous/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "mpg123 decoder",
    "decoder for mp3",
    1
};

MOUS_DEF_PLUGIN(MousDecoder, &info, IDecoder, Mp3Decoder);
