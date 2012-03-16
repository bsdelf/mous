#ifndef MOUS_PLUGINHELPER_H
#define MOUS_PLUGINHELPER_H

#include <inttypes.h>
#include <common/PluginDef.h>

/**
 * Simple yet helpful macro for declare a plugin.
 */
#define MOUS_DEF_PLUGIN(type, pInfo, Derived)\
extern "C" {\
    EmPluginType MousGetPluginType() {  \
        return type;                    \
    }                                   \
    \
    const PluginInfo* MousGetPluginInfo() { \
        return pInfo;                       \
    }                                       \
    \
    void* MousCreatePlugin() {  \
        return new Derived; \
    }                       \
    \
    void MousReleasePlugin(void* p) {               \
        if (p != NULL) {                            \
            Derived* dp = static_cast<Derived*>(p); \
            delete dp;                              \
        }                                           \
    }                                               \
    \
} struct __MOUS_MACRO_END__

#endif
