#pragma once

#include <vector>
#include <util/Option.h>
#include <util/PluginDef.h>

namespace mous {

struct PluginOption
{
    PluginType pluginType = PluginType::None;
    const PluginInfo* pluginInfo = nullptr;
    std::vector<const BaseOption*> options;

    PluginOption() = default;

    PluginOption(PluginType type, const PluginInfo* info):
        pluginType(type),
        pluginInfo(info)
    {
    }

    PluginOption(PluginType type, const PluginInfo* info, std::vector<const BaseOption*>&& options):
        pluginType(type),
        pluginInfo(info),
        options(options)
    {
    }
};

}
