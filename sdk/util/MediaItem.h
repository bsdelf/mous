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
        userData(nullptr)
    {

    }

    template<typename buf_t> void operator>>(buf_t& buf) const
    {
        buf << url << duration << hasRange << msBeg << msEnd;
        tag >> buf;
    }

    template<typename buf_t> void operator<<(buf_t& buf)
    {
        buf >> url >> duration >> hasRange >> msBeg >> msEnd;
        tag << buf;
    }
};

}
#endif
