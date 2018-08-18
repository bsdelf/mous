#pragma once

#include <vector>
#include <deque>
#include <string>
#include <memory>

#include <util/ErrorCode.h>
#include <util/MediaItem.h>
#include <util/Plugin.h>

namespace mous {

class MediaLoader
{
    class Impl;

public:
    MediaLoader();
    ~MediaLoader();

    void LoadSheetParserPlugin(const std::shared_ptr<Plugin>& plugin);
    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin);
    void UnloadPlugin(const std::string& path);
    void UnloadPlugin();

    std::vector<std::string> SupportedSuffixes() const;
    ErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const;

private:
    std::unique_ptr<Impl> impl;
};

}
