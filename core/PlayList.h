#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <mous/MediaItem.h>

namespace mous {

namespace PlayMode {
enum e
{
    Normal,
    Repeat,
    Random,
    RandomRepeat,
    SingleRepeat
};
}
typedef PlayMode::e EmPlayMode;

class PlayList
{
public:
    PlayList();
    ~PlayList();

    void SetPlayMode(EmPlayMode mode);
    EmPlayMode GetPlayMode() const;

    void DumpCover();

private:
    EmPlayMode m_PlayMode;
};

}

#endif
