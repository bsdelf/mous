#include "MediaLoader.h"
#include <iostream>
#include <scx/FileHelper.hpp>
#include <scx/Conv.hpp>
#include <util/MediaItem.h>
#include <plugin/IMediaPack.h>
#include <plugin/ITagParser.h>
#include <core/IPluginAgent.h>
using namespace std;
using namespace scx;
using namespace mous;

IMediaLoader* IMediaLoader::Create()
{
    return new MediaLoader;
}

void IMediaLoader::Free(IMediaLoader* ptr)
{
    if (ptr != NULL)
        delete ptr;
}

MediaLoader::MediaLoader()
{

}

MediaLoader::~MediaLoader()
{

}

void MediaLoader::RegisterMediaPackPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::MediaPack)
        AddMediaPack(pAgent);
}

void MediaLoader::RegisterMediaPackPlugin(vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterMediaPackPlugin(agents[i]);
    }
}

void MediaLoader::RegisterTagParserPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::TagParser)
        AddTagParser(pAgent);
}

void MediaLoader::RegisterTagParserPlugin(vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterTagParserPlugin(agents[i]);
    }
}

void MediaLoader::UnregisterPlugin(const IPluginAgent* pAgent)
{
    switch (pAgent->Type()) {
    case PluginType::MediaPack:
        RemoveMediaPack(pAgent);
        break;
    
    case PluginType::TagParser:
        RemoveTagParser(pAgent);
        break;

    default:
        break;
    }
}

void MediaLoader::UnregisterPlugin(vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        UnregisterPlugin(agents[i]);
    }
}

void MediaLoader::UnregisterAll()
{
    while (!m_AgentMap.empty()) {
        AgentMapIter iter = m_AgentMap.begin();
        UnregisterPlugin(iter->first);
    }
}

void MediaLoader::AddMediaPack(const IPluginAgent* pAgent)
{
    // Register agent.
    IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pPack));

    // Register MediaPack.
    vector<string> list(pPack->FileSuffix());
    for (size_t i = 0; i < list.size(); ++i) {
        string suffix = ToLower(list[i]);
        MediaPackMapIter iter = m_MediaPackMap.find(suffix);
        if (iter == m_MediaPackMap.end()) {
            m_MediaPackMap.insert(MediaPackMapPair(suffix, pPack));
        }
    }
}

void MediaLoader::RemoveMediaPack(const IPluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister MediaPack.
        IMediaPack* pPack = (IMediaPack*)iter->second;
        vector<string> list(pPack->FileSuffix());
        for (size_t i = 0; i < list.size(); ++i) {
            string suffix = ToLower(list[i]);
            MediaPackMapIter iter = m_MediaPackMap.find(suffix);
            if (iter != m_MediaPackMap.end()) {
                m_MediaPackMap.erase(iter);
            }
        }

        // Unregister IPluginAgent.
        pAgent->FreeObject(pPack);
        m_AgentMap.erase(iter);
    }
}

void MediaLoader::AddTagParser(const IPluginAgent* pAgent)
{
    // Register IPluginAgent.
    ITagParser* pParser = (ITagParser*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pParser));

    // Register TagParser.
    vector<string> list(pParser->FileSuffix());
    for (size_t i = 0; i < list.size(); ++i) {
        string suffix = ToLower(list[i]);
        TagParserMapIter iter = m_TagParserMap.find(suffix);
        if (iter == m_TagParserMap.end()) {
            m_TagParserMap.insert(TagParserMapPair(suffix, pParser));
        }
    }
}

void MediaLoader::RemoveTagParser(const IPluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister TagParser.
        ITagParser* pParser = (ITagParser*)iter->second;
        vector<string> list(pParser->FileSuffix());
        for (size_t i = 0; i < list.size(); ++i) {
            string suffix = ToLower(list[i]);
            TagParserMapIter iter = m_TagParserMap.find(suffix);
            if (iter != m_TagParserMap.end()) {
                m_TagParserMap.erase(iter);
            }
        }

        // Unregister IPluginAgent.
        pAgent->FreeObject(pParser);
        m_AgentMap.erase(iter);
    }
}

EmErrorCode MediaLoader::LoadMedia(const string& path, deque<MediaItem>& list) const
{
    list.clear();

    TryUnpack(path, list);
    TryParseTag(list);

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryUnpack(const string& path, deque<MediaItem>& list) const
{
    // Find MediaPack.
    string suffix = ToLower(FileHelper::FileSuffix(path));
    MediaPackMapConstIter iter = m_MediaPackMap.find(suffix);

    MediaItem tmpItem;
    if (iter == m_MediaPackMap.end()) {
        // General Media
        tmpItem.url = path;
        tmpItem.hasRange = false;
        list.push_back(tmpItem);
    } else {
        // MediaPack
        IMediaPack* pack = iter->second;
        pack->DumpMedia(path, list, &m_MediaPackMap);
    }

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryParseTag(deque<MediaItem>& list) const
{
    for (size_t i = 0; i < list.size(); ++i) {
        // Find TagParser.
        MediaItem& item = list[i];
        string suffix = ToLower(FileHelper::FileSuffix(item.url));
        TagParserMapConstIter iter = m_TagParserMap.find(suffix);
        if (iter == m_TagParserMap.end()) {
            iter = m_TagParserMap.find("*");
            if (iter == m_TagParserMap.end())
                continue;
        }

        // Parse & Fill.
        ITagParser* parser = iter->second;
        parser->Open(item.url);
        if (parser->HasTag()) {
            if (item.tag.title.empty())
                item.tag.title = parser->Title();
            if (item.tag.artist.empty())
                item.tag.artist = parser->Artist();
            if (item.tag.album.empty())
                item.tag.album = parser->Album();
            if (item.tag.comment.empty())
                item.tag.comment = parser->Comment();
            if (item.tag.genre.empty())
                item.tag.genre = parser->Genre();
            if (item.tag.year < 0)
                item.tag.year = parser->Year();
            if (item.tag.track < 0)
                item.tag.track = parser->Track();
        } else {
            cout << "WARN: no tag!!" << endl;
        }

        if (parser->HasProperties()) {
            if (item.duration < 0)
                item.duration = parser->Duration();
        } else {
            cout << "FATAL: no properties!!" << endl;
        }

        parser->Close();
    }

    return ErrorCode::Ok;
}
