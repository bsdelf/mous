#pragma once

#include <vector>
#include <util/Option.h>
#include <util/PluginDef.h>

namespace mous {

struct PluginOption
{
    EmPluginType pluginType;
    const PluginInfo* pluginInfo;
    std::vector<const BaseOption*> options;
};

}
