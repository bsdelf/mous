#include "TagLibParser.h"
#include <plugin/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "TagLib Parser",
    "Read/Editing audio meta data.",
    1
};

MOUS_DEF_PLUGIN(PluginType::TagParser, &info, TagLibParser);

