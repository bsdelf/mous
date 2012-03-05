#ifndef IPLAYLISTVIEW_H
#define IPLAYLISTVIEW_H

namespace mous {
    struct MediaItem;
    class MediaLoader;
}

class IPlayListView
{
public:
    virtual ~IPlayListView() { }

    virtual void setMediaLoader(const mous::MediaLoader* loader) = 0;

    virtual const mous::MediaItem* getNextItem() = 0;
    virtual const mous::MediaItem* getPreviousItem() = 0;
    virtual size_t getItemCount() const = 0;
};

#endif // IPLAYLISTVIEW_H
