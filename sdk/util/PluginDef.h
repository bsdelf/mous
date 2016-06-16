#pragma once

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
    const char* desc;
    const int32_t version;
};

const char* const StrGetPluginType = "MousGetPluginType";
const char* const StrGetPluginInfo = "MousGetPluginInfo";
const char* const StrCreateObject = "MousCreateObject";
const char* const StrFreeObject = "MousFreeObject";

inline const char* ToString(EmPluginType type) 
{
    switch (type) {
        case PluginType::None:
            return "None";

        case PluginType::Decoder:
            return "Decoder";

        case PluginType::Encoder:
            return "Encoder";

        case PluginType::Renderer:
            return "Renderer";

        case PluginType::MediaPack:
            return "MediaPack";

        case PluginType::TagParser:
            return "TagParser";

        case PluginType::Filter:
            return "Filter";

        case PluginType::EventWatcher:
            return "EventWatcher";

        default:
            return "Unknown";
    }
    return "";
}

}
