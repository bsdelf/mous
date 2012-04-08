#ifndef MOUS_MEDIATAG_H
#define MOUS_MEDIATAG_H

#include <string>
#include <inttypes.h>

namespace mous {

struct MediaTag
{
    std::string title;
    std::string artist;
    std::string album;
    std::string comment;
    std::string genre;
    int32_t year;
    int32_t track;

    MediaTag():
        year(-1),
        track(-1)
    {
    }
};

}

#endif
