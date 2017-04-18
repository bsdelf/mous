#pragma once

#include <vector>
#include <deque>
#include <string>

#include <util/ErrorCode.h>
#include <util/MediaItem.h>

namespace mous {

struct MediaItem;
class Plugin;

class IMediaLoader
{
public:
    static IMediaLoader* Create();
    static void Free(IMediaLoader*);

public:
    virtual ~IMediaLoader() { }

    virtual void RegisterMediaPackPlugin(const Plugin* pAgent) = 0;
    virtual void RegisterMediaPackPlugin(std::vector<const Plugin*>& agents) = 0;

    virtual void RegisterTagParserPlugin(const Plugin* pAgent) = 0;
    virtual void RegisterTagParserPlugin(std::vector<const Plugin*>& agents) = 0;

    virtual void UnregisterPlugin(const Plugin* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const Plugin*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual std::vector<std::string> SupportedSuffixes() const = 0;
    virtual EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const = 0;
};

}
