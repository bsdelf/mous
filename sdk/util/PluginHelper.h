#pragma once

#include <util/PluginDef.h>

/**
 * Simple yet helpful macro for declare a plugin.
 */
#define MOUS_DEF_PLUGIN(type, p_info, obj_t)\
extern "C" {\
    PluginType MousGetPluginType() {    \
        return type;                    \
    }                                   \
    \
    const PluginInfo* MousGetPluginInfo() { \
        return p_info;                      \
    }                                       \
    \
    void* MousCreateObject() {  \
        return new obj_t;       \
    }                           \
    \
    void MousFreeObject(void* p) {              \
        if (p != nullptr) {                     \
            obj_t* op = static_cast<obj_t*>(p); \
            delete op;                          \
        }                                       \
    }                                           \
    \
} struct __MOUS_MACRO_END__
