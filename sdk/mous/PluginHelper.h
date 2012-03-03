#ifndef MOUS_PLUGINHELPER_H
#define MOUS_PLUGINHELPER_H

#include <inttypes.h>

/**
 * Plugin common definition.
 */
namespace mous {

namespace PluginType {
enum e
{
    None = 0,
    Decoder,
    Encoder,
    Renderer,
    MediaPack,
    TagParser,
    Filter
};
}
typedef PluginType::e EmPluginType;

struct PluginInfo
{
    const char* author;
    const char* name;
    const char* description;
    const int32_t version;
};

const char* const StrGetPluginType = "GetPluginType";
const char* const StrGetPluginInfo = "GetPluginInfo";
const char* const StrCreatePlugin = "CreatePlugin";
const char* const StrReleasePlugin = "ReleasePlugin";

}

/**
 * Simple yet helpful macro for declare a plugin.
 */
#define MOUS_DEF_PLUGIN(type, pInfo, Derived)\
extern "C" {\
    EmPluginType GetPluginType() {  \
        return type;                \
    }                               \
    \
    const PluginInfo* GetPluginInfo() { \
        return pInfo;                   \
    }                                   \
    \
    void* CreatePlugin() {  \
        return new Derived; \
    }                       \
    \
    void ReleasePlugin(void* p) {                   \
        if (p != NULL) {                            \
            Derived* dp = static_cast<Derived*>(p); \
            delete dp;                              \
        }                                           \
    }                                               \
    \
} struct __end__

#endif
