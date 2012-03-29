#ifndef MOUS_AUDIOMODE_H
#define MOUS_AUDIOMODE_H

namespace mous {

namespace AudioMode {
enum e
{
    None,
    Mono,
    Stereo,
    JointStero,
    DualChannel
};
}
typedef AudioMode::e EmAudioMode;

inline const char* ToString(EmAudioMode mode)
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

#endif
