#pragma once

#include <inttypes.h>
#include <scx/Conv.h>

/**
 * Plugin common definitions.
 */
namespace mous {

enum class PluginType: uint32_t {
    None = 0,
    Decoder = 1u,
    Encoder = 1u << 1,
    Output = 1u << 2,
    SheetParser = 1u << 3,
    TagParser = 1u << 4
};

inline const char* ToString(PluginType type) {
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

inline auto operator & (PluginType lhs, PluginType rhs) {
    return static_cast<PluginType>(scx::ToUnderlying(lhs) & scx::ToUnderlying(rhs));
}

inline auto operator | (PluginType lhs, PluginType rhs) {
    return static_cast<PluginType>(scx::ToUnderlying(lhs) | scx::ToUnderlying(rhs));
}

inline auto& operator |= (PluginType& lhs, PluginType rhs) {
    lhs = static_cast<PluginType>(scx::ToUnderlying(lhs) | scx::ToUnderlying(rhs));
    return lhs;
}

struct PluginInfo {
    const char* name;
    const char* desc;
    const uint32_t version;
};

const char* const StrGetPluginType = "MousGetPluginType";
const char* const StrGetPluginInfo = "MousGetPluginInfo";

}
