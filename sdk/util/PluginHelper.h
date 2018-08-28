#pragma once

#include <util/PluginDef.h>

#define MOUS_EXPORT_PLUGIN(type, name, desc, version)\
extern "C" {\
    const PluginInfo* MousGetPluginInfo() {\
        static const PluginInfo info {\
            type, name, desc, version\
        };\
        return &info;\
    }\
}