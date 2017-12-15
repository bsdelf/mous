#ifndef IPLAYLISTVIEW_H
#define IPLAYLISTVIEW_H

namespace mous {
    struct MediaItem;
    class MediaLoader;
}

#include "PlaylistClipboard.h"

class IPlaylistView
{
public:
    virtual ~IPlaylistView() { }

    virtual void SetMediaLoader(const mous::MediaLoader* loader) = 0;
    virtual void SetClipboard(PlaylistClipboard<mous::MediaItem>* clipboard) = 0;

    virtual const mous::MediaItem* PrevItem() const = 0;
    virtual const mous::MediaItem* NextItem() const = 0;
    virtual int ItemCount() const = 0;
    virtual const char* PlayMode() const = 0;
    virtual void SetPlayMode(int mode) = 0;
    virtual void Save(const char* filename) const = 0;
    virtual void Load(const char* filename) = 0;

    virtual void OnMediaItemUpdated(const mous::MediaItem& item) = 0;
};

#endif // IPLAYLISTVIEW_H
