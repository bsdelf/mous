#ifndef MOUS_MEDIALOADER_H
#define MOUS_MEDIALOADER_H

#include <map>
#include <deque>
#include <string>
#include <mous/ErrorCode.h>
#include "PluginAgent.h"

namespace mous {

struct MediaItem;
class IMediaPack;
class ITagParser;

class MediaLoader
{
public:
    MediaLoader();
    ~MediaLoader();

    void RegisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterAll();

    EmErrorCode LoadMedia(const std::string& path, std::deque<MediaItem*>& list) const;

private:
    void AddMediaPack(const PluginAgent* pAgent);
    void RemoveMediaPack(const PluginAgent* pAgent);
    void AddTagParser(const PluginAgent* pAgent);
    void RemoveTagParser(const PluginAgent* pAgent);
 
    EmErrorCode TryUnpack(const std::string& path, std::deque<MediaItem*>& list) const;
    EmErrorCode TryParseTag(std::deque<MediaItem*>& list) const;

private:
    std::map<const PluginAgent*, void*> m_AgentMap;
    typedef std::pair<const PluginAgent*, void*> AgentMapPair;
    typedef std::map<const PluginAgent*, void*>::iterator AgentMapIter;

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
