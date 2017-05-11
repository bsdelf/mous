#pragma once

#include <vector>
#include <deque>
#include <string>
#include <memory>

#include <util/ErrorCode.h>
#include <util/MediaItem.h>
#include <core/Plugin.h>

namespace mous {

class MediaLoader
{
    class Impl;

public:
    MediaLoader();
    ~MediaLoader();

    void RegisterMediaPackPlugin(const Plugin* pAgent);
    void RegisterMediaPackPlugin(std::vector<const Plugin*>& agents);

    void RegisterTagParserPlugin(const Plugin* pAgent);
    void RegisterTagParserPlugin(std::vector<const Plugin*>& agents);

    void UnregisterPlugin(const Plugin* pAgent);
    void UnregisterPlugin(std::vector<const Plugin*>& agents);
    void UnregisterAll();

    std::vector<std::string> SupportedSuffixes() const;
    ErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const;

private:
    std::unique_ptr<Impl> impl;
};

}
