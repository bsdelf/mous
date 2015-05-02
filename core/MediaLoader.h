#ifndef MOUS_MEDIALOADER_H
#define MOUS_MEDIALOADER_H

#include <unordered_map>
#include <core/IMediaLoader.h>
using namespace std;

namespace mous {

struct MediaItem;
class IPluginAgent;
class IMediaPack;
class ITagParser;

class MediaLoader: public IMediaLoader
{
public:
    MediaLoader() = default;
    ~MediaLoader() = default;

    void RegisterMediaPackPlugin(const IPluginAgent* pAgent);
    void RegisterMediaPackPlugin(std::vector<const IPluginAgent*>& agents);

    void RegisterTagParserPlugin(const IPluginAgent* pAgent);
    void RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents);

    void UnregisterPlugin(const IPluginAgent* pAgent);
    void UnregisterPlugin(vector<const IPluginAgent*>& agents);
    void UnregisterAll();

    std::vector<std::string> SupportedSuffixes() const;
    EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const;

private:
    void AddMediaPack(const IPluginAgent* pAgent);
    void RemoveMediaPack(const IPluginAgent* pAgent);
    void AddTagParser(const IPluginAgent* pAgent);
    void RemoveTagParser(const IPluginAgent* pAgent);
 
    EmErrorCode TryUnpack(const std::string& path, std::deque<MediaItem>& list) const;
    EmErrorCode TryParseTag(std::deque<MediaItem>& list) const;

private:
    std::unordered_map<const IPluginAgent*, void*> m_AgentMap;
    std::unordered_map<std::string, IMediaPack*> m_MediaPackMap;
    std::unordered_map<std::string, ITagParser*> m_TagParserMap;
};

}

#endif
