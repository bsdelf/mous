#ifndef MOUS_MEDIALOADER_H
#define MOUS_MEDIALOADER_H

#include <string>
#include <mous/ErrorCode.h>
#include "PlayList.h"

namespace mous {

class MediaLoader
{
public:
    MediaLoader();
    ~MediaLoader();

    EmErrorCode LoadMedia(const std::string& path, PlayList& list);

private:
    EmErrorCode TryUnpack();
    EmErrorCode TryParseTag();
};

}

#endif
