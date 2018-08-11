#include "OssOutput.h"
#include <util/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "OSS Output",
    "OSS is used on BSD system.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Output, &info, OssOutput);
