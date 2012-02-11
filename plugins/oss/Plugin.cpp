#include "OssRenderer.h"
#include <mous/PluginHelper.h>
using namespace mous;
using namespace std;

static const PluginInfo info = {
    "Yanhui Shen",
    "OSS Renderer",
    "OSS is used on BSD system.",
    1
};

MOUS_DEF_PLUGIN(MousRenderer, &info, IRenderer, OssRenderer);
