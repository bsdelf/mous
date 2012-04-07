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

    virtual const mous::MediaItem* GetNextItem() const = 0;
    virtual const mous::MediaItem* GetPreviousItem() const = 0;
    virtual size_t GetItemCount() const = 0;
};

#endif // IPLAYLISTVIEW_H
