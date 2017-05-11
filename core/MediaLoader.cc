#include <core/MediaLoader.h>
#include <core/Plugin.h>

#include "MediaLoaderImpl.h"

namespace mous {

MediaLoader::MediaLoader()
    : impl(std::make_unique<Impl>())
{
}

MediaLoader::~MediaLoader()
{
}

void MediaLoader::RegisterMediaPackPlugin(const Plugin* pAgent)
{
    return impl->RegisterMediaPackPlugin(pAgent);
}

void MediaLoader::RegisterMediaPackPlugin(std::vector<const Plugin*>& agents)
{
    return impl->RegisterMediaPackPlugin(agents);
}

void MediaLoader::RegisterTagParserPlugin(const Plugin* pAgent)
{
    return impl->RegisterTagParserPlugin(pAgent);
}

void MediaLoader::RegisterTagParserPlugin(std::vector<const Plugin*>& agents)
{
    return impl->RegisterTagParserPlugin(agents);
}

void MediaLoader::UnregisterPlugin(const Plugin* pAgent)
{
    return impl->UnregisterPlugin(pAgent);
}

void MediaLoader::UnregisterPlugin(std::vector<const Plugin*>& agents)
{
    return impl->UnregisterPlugin(agents);
}

void MediaLoader::UnregisterAll()
{
    return impl->UnregisterAll();
}

std::vector<std::string> MediaLoader::SupportedSuffixes() const
{
    return impl->SupportedSuffixes();
}

ErrorCode MediaLoader::LoadMedia(const std::string& path, std::deque<MediaItem>& list) const
{
    return impl->LoadMedia(path, list);
}

}
