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

}

#endif
