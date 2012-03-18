#ifndef MOUS_PLUGINHELPER_H
#define MOUS_PLUGINHELPER_H

#include <inttypes.h>
#include <common/PluginDef.h>

/**
 * Simple yet helpful macro for declare a plugin.
 */
#define MOUS_DEF_PLUGIN(type, p_info, obj_t)\
extern "C" {\
    EmPluginType MousGetPluginType() {  \
        return type;                    \
    }                                   \
    \
    const PluginInfo* MousGetPluginInfo() { \
        return p_info;                      \
    }                                       \
    \
    void* MousCreatObject() {  \
        return new obj_t; \
    }                       \
    \
    void MousFreeObject(void* p) {              \
        if (p != NULL) {                        \
            obj_t* dp = static_cast<obj_t*>(p); \
            delete dp;                          \
        }                                       \
    }                                           \
    \
} struct __MOUS_MACRO_END__

#endif
