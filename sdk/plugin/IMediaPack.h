#ifndef MOUS_IMEDIAPACK_H
#define MOUS_IMEDIAPACK_H

#include <map>
#include <vector>
#include <deque>
#include <string>
#include <util/Option.h>

namespace mous {

struct MediaItem;

class IMediaPack
{
public:
    virtual ~IMediaPack() { }

    virtual std::vector<std::string> GetFileSuffix() const = 0;

    virtual void DumpMedia(const std::string& path, std::deque<MediaItem*>& list,
        const std::map<std::string, IMediaPack*>* pMap) const = 0;

    virtual void DumpStream(const std::string& stream, std::deque<MediaItem*>& list,
        const std::map<std::string, IMediaPack*>* pMap) const = 0;

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const { return false; };

public:
    typedef std::map<std::string, IMediaPack*>::iterator MediaPackMapIter;
};

}

#endif
