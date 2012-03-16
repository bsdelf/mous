#include "OssRenderer.h"
#include <plugin/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "OSS Renderer",
    "OSS is used on BSD system.",
    1
};

MOUS_DEF_PLUGIN(PluginType::Renderer, &info, OssRenderer);
