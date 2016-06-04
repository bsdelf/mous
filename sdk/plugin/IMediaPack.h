#pragma once

#include <unordered_map>
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

    virtual std::vector<std::string> FileSuffix() const = 0;

    virtual void DumpMedia(const std::string& path, std::deque<MediaItem>& list,
        const std::unordered_map<std::string, IMediaPack*>* pMap) const = 0;

    virtual void DumpStream(const std::string& stream, std::deque<MediaItem>& list,
        const std::unordered_map<std::string, IMediaPack*>* pMap) const = 0;

    // reimplement this to provide options
    virtual bool Options(std::vector<const BaseOption*>& list) const { return false; };

public:
    typedef std::unordered_map<std::string, IMediaPack*>::iterator MediaPackMapIter;
};

}
