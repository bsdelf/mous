#ifndef IPLAYLISTVIEW_H
#define IPLAYLISTVIEW_H

namespace mous {
    struct MediaItem;
    class IMediaLoader;
}

class IPlayListView
{
public:
    virtual ~IPlayListView() { }

    virtual void setMediaLoader(const mous::IMediaLoader* loader) = 0;

    virtual const mous::MediaItem* getNextItem() const = 0;
    virtual const mous::MediaItem* getPreviousItem() const = 0;
    virtual size_t getItemCount() const = 0;
};

#endif // IPLAYLISTVIEW_H
