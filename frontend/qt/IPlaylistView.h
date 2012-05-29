#ifndef IPLAYLISTVIEW_H
#define IPLAYLISTVIEW_H

namespace mous {
    struct MediaItem;
    class IMediaLoader;
}

class IPlaylistView
{
public:
    virtual ~IPlaylistView() { }

    virtual void SetMediaLoader(const mous::IMediaLoader* loader) = 0;

    virtual const mous::MediaItem* PrevItem() const = 0;
    virtual const mous::MediaItem* NextItem() const = 0;
    virtual int ItemCount() const = 0;
};

#endif // IPLAYLISTVIEW_H
