#ifndef MOUS_PLUGINOPTION_H
#define MOUS_PLUGINOPTION_H

#include <vector>
#include <common/Option.h>
#include <common/PluginDef.h>

namespace mous {

struct PluginOption
{
    EmPluginType pluginType;
    const PluginInfo* pluginInfo;
    std::vector<ConstOptionPair> options;
};

}

#endif
