#pragma once

#include <util/PluginDef.h>

#define MOUS_EXPORT_PLUGIN(type, name, desc, version)\
extern "C" {\
    PluginType MousGetPluginType() {\
        return type;\
    }\
    const PluginInfo* MousGetPluginInfo() {\
        static const PluginInfo info {\
            name, desc, version\
        };\
        return &info;\
    }\
}