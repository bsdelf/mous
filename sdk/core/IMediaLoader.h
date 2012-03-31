#ifndef MOUS_IMEDIALOADER_H
#define MOUS_IMEDIALOADER_H

#include <deque>
#include <string>
#include <util/ErrorCode.h>

namespace mous {

struct MediaItem;
class IPluginAgent;

class IMediaLoader
{
public:
    static IMediaLoader* Create();
    static void Free(IMediaLoader*);

public:
    virtual ~IMediaLoader() { }

    virtual void RegisterMediaPackPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterTagParserPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterAll() = 0;

    virtual EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem*>& list) const = 0;
};

}

#endif
