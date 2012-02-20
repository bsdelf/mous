#ifndef MOUS_IMEDIAPACK_H
#define MOUS_IMEDIAPACK_H

#include <vector>
#include <deque>
#include <string>

namespace mous {

struct MediaItem;

class IMediaPack
{
public:
    virtual ~IMediaPack()
    {

    }

    virtual void GetFileSuffix(std::vector<std::string>& list) const = 0;

    virtual void DumpMedias(const std::string& path, std::deque<MediaItem*>& list) = 0;
};

}

#endif
