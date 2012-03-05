#ifndef IPLAYLISTVIEW_H
#define IPLAYLISTVIEW_H

namespace mous {
    struct MediaItem;
}

class IPlayListView
{
public:
    virtual ~IPlayListView() { }

    virtual const mous::MediaItem* getNextItem() = 0;
    virtual const mous::MediaItem* getPreviousItem() = 0;
    virtual const size_t getItemCount() = 0;
};

#endif // IPLAYLISTVIEW_H
