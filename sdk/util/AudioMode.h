#pragma once

#include <inttypes.h>

namespace mous {

enum class AudioMode: uint32_t
{
    None,
    Mono,
    Stereo,
    JointStero,
    DualChannel
};

inline const char* ToString(AudioMode mode)
{
    switch (mode) {
        case AudioMode::None:
            return "None";

        case AudioMode::Mono:
            return "Mono";

        case AudioMode::Stereo:
            return "Stereo";

        case AudioMode::JointStero:
            return "JointStero";

        case AudioMode::DualChannel:
            return "DualChannel";
    }

    return "";
}

}
