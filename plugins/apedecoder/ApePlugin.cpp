#include "ApeDecoder.h"
#include <mous/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "MAC Decoder",
    "Decoder for APE audio.",
    1
};

MOUS_DEF_PLUGIN(MousDecoder, &info, IDecoder, ApeDecoder);
