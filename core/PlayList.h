#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

namespace mous {

namespace PlayListMode {

enum PlayListMode
{
    Normal,
    Repeat,
    Random,
    RandomRepeat,
    SingleRepeat
};

}

class PlayList
{
public:
    PlayList();
    ~PlayList();

    void DumpCover();
};

}

#endif
