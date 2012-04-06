#ifndef MOUS_MEDIAITEM_H
#define MOUS_MEDIAITEM_H

#include <util/MediaTag.h>

namespace mous {

struct MediaItem
{
    std::string url;
    int32_t duration;

    bool hasRange;
    uint64_t msBeg;
    uint64_t msEnd;

    MediaTag tag;

    void* userData;

    MediaItem():
        duration(-1),
        userData(NULL)
    {

    }
};

}
#endif
