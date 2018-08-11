#pragma once

#include <inttypes.h>

/**
 * Plugin common definition.
 */
namespace mous {

enum class PluginType: uint8_t
{
    None = 0,
    Decoder,
    Encoder,
    Output,
    SheetParser,
    TagParser
};

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

inline const char* ToString(PluginType type)
{
    switch (type) {
        case PluginType::None:
            return "None";

        case PluginType::Decoder:
            return "Decoder";

        case PluginType::Encoder:
            return "Encoder";

        case PluginType::Output:
            return "Output";

        case PluginType::SheetParser:
            return "SheetParser";

        case PluginType::TagParser:
            return "TagParser";

        default:
            return "Unknown";
    }
    return "";
}

}
