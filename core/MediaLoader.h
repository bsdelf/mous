#ifndef MOUS_MEDIALOADER_H
#define MOUS_MEDIALOADER_H

#include <map>
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
    MediaLoader();
    ~MediaLoader();

    void RegisterMediaPackPlugin(const IPluginAgent* pAgent);
    void RegisterMediaPackPlugin(std::vector<const IPluginAgent*>& agents);

    void RegisterTagParserPlugin(const IPluginAgent* pAgent);
    void RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents);

    void UnregisterPlugin(const IPluginAgent* pAgent);
    void UnregisterPlugin(vector<const IPluginAgent*>& agents);
    
    void UnregisterAll();

    EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem*>& list) const;

private:
    void AddMediaPack(const IPluginAgent* pAgent);
    void RemoveMediaPack(const IPluginAgent* pAgent);
    void AddTagParser(const IPluginAgent* pAgent);
    void RemoveTagParser(const IPluginAgent* pAgent);
 
    EmErrorCode TryUnpack(const std::string& path, std::deque<MediaItem*>& list) const;
    EmErrorCode TryParseTag(std::deque<MediaItem*>& list) const;

private:
    std::map<const IPluginAgent*, void*> m_AgentMap;
    typedef std::pair<const IPluginAgent*, void*> AgentMapPair;
    typedef std::map<const IPluginAgent*, void*>::iterator AgentMapIter;

    std::map<std::string, IMediaPack*> m_MediaPackMap;
    typedef std::pair<std::string, IMediaPack*> MediaPackMapPair;
    typedef std::map<std::string, IMediaPack*>::iterator MediaPackMapIter;
    typedef std::map<std::string, IMediaPack*>::const_iterator MediaPackMapConstIter;

    std::map<std::string, ITagParser*> m_TagParserMap;
    typedef std::pair<std::string, ITagParser*> TagParserMapPair;
    typedef std::map<std::string, ITagParser*>::iterator TagParserMapIter;
    typedef std::map<std::string, ITagParser*>::const_iterator TagParserMapConstIter;
};

}

#endif
