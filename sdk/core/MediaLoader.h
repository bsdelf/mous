#pragma once

using namespace std;

#include <vector>
#include <deque>
#include <string>
#include <memory>

#include <util/ErrorCode.h>
#include <util/MediaItem.h>
#include <core/Plugin.h>

namespace mous {

class MediaLoaderPrivate;

class MediaLoader
{
    friend MediaLoaderPrivate;

public:
    MediaLoader();
    ~MediaLoader();

    void RegisterMediaPackPlugin(const Plugin* pAgent);
    void RegisterMediaPackPlugin(std::vector<const Plugin*>& agents);

    void RegisterTagParserPlugin(const Plugin* pAgent);
    void RegisterTagParserPlugin(std::vector<const Plugin*>& agents);

    void UnregisterPlugin(const Plugin* pAgent);
    void UnregisterPlugin(vector<const Plugin*>& agents);
    void UnregisterAll();

    std::vector<std::string> SupportedSuffixes() const;
    EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const;

private:
    void AddMediaPack(const Plugin* pAgent);
    void RemoveMediaPack(const Plugin* pAgent);
    void AddTagParser(const Plugin* pAgent);
    void RemoveTagParser(const Plugin* pAgent);
 
    EmErrorCode TryUnpack(const std::string& path, std::deque<MediaItem>& list) const;
    EmErrorCode TryParseTag(std::deque<MediaItem>& list) const;

private:
    std::unique_ptr<MediaLoaderPrivate> d;
};

}
