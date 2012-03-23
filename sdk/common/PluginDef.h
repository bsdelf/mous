#ifndef MOUS_PLUGINDEF_H
#define MOUS_PLUGINDEF_H

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
    Filter,
    EventWatcher
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
const char* const StrCreateObject = "MousCreateObject";
const char* const StrFreeObject = "MousFreeObject";

}

#endif
