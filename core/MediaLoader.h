#pragma once

#include <unordered_map>
#include <core/IMediaLoader.h>
using namespace std;

namespace mous {

struct MediaItem;
class Plugin;
class IMediaPack;
class ITagParser;

class MediaLoader: public IMediaLoader
{
public:
    MediaLoader() = default;
    ~MediaLoader() = default;

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
    std::unordered_map<const Plugin*, void*> m_AgentMap;
    std::unordered_map<std::string, IMediaPack*> m_MediaPackMap;
    std::unordered_map<std::string, ITagParser*> m_TagParserMap;
};

}
