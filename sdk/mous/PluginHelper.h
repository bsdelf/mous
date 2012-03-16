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

const char* const StrGetPluginType = "MousGetPluginType";
const char* const StrGetPluginInfo = "MousGetPluginInfo";
const char* const StrCreatePlugin = "MousCreatePlugin";
const char* const StrReleasePlugin = "MousReleasePlugin";

}

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
