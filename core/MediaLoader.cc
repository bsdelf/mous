#include "MediaLoader.h"

#include <scx/FileHelper.hpp>
#include <scx/Conv.hpp>
using namespace scx;

#include "util/MediaItem.h"
#include "plugin/IMediaPack.h"
#include "plugin/ITagParser.h"
#include "core/IPluginAgent.h"
using namespace mous;

//#include <iostream>
//using namespace std;

IMediaLoader* IMediaLoader::Create()
{
    return new MediaLoader;
}

void IMediaLoader::Free(IMediaLoader* ptr)
{
    if (ptr != nullptr)
        delete ptr;
}

void MediaLoader::RegisterMediaPackPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::MediaPack)
        AddMediaPack(pAgent);
}

void MediaLoader::RegisterMediaPackPlugin(vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        RegisterMediaPackPlugin(agent);
    }
}

void MediaLoader::RegisterTagParserPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::TagParser)
        AddTagParser(pAgent);
}

void MediaLoader::RegisterTagParserPlugin(vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        RegisterTagParserPlugin(agent);
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
    for (auto agent: agents) {
        UnregisterPlugin(agent);
    }
}

void MediaLoader::UnregisterAll()
{
    while (!m_AgentMap.empty()) {
        auto iter = m_AgentMap.begin();
        UnregisterPlugin(iter->first);
    }
}

void MediaLoader::AddMediaPack(const IPluginAgent* pAgent)
{
    // Register agent.
    IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
    m_AgentMap.emplace(pAgent, pPack);

    // Register MediaPack.
    for (const string& item: pPack->FileSuffix()) {
        const string& suffix = ToLower(item);
        auto iter = m_MediaPackMap.find(suffix);
        if (iter == m_MediaPackMap.end()) {
            m_MediaPackMap.emplace(suffix, pPack);
        }
    }
}

void MediaLoader::RemoveMediaPack(const IPluginAgent* pAgent)
{
    auto iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister MediaPack.
        IMediaPack* pPack = (IMediaPack*)iter->second;
        for (const string& item: pPack->FileSuffix()) {
            const string& suffix = ToLower(item);
            auto iter = m_MediaPackMap.find(suffix);
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
    m_AgentMap.emplace(pAgent, pParser);

    // Register TagParser.
    for (const string& item: pParser->FileSuffix()) {
        const string& suffix = ToLower(item);
        auto iter = m_TagParserMap.find(suffix);
        if (iter == m_TagParserMap.end()) {
            m_TagParserMap.emplace(suffix, pParser);
        }
    }
}

void MediaLoader::RemoveTagParser(const IPluginAgent* pAgent)
{
    auto iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister TagParser.
        ITagParser* pParser = (ITagParser*)iter->second;
        for (const string& item: pParser->FileSuffix()) {
            const string& suffix = ToLower(item);
            auto iter = m_TagParserMap.find(suffix);
            if (iter != m_TagParserMap.end()) {
                m_TagParserMap.erase(iter);
            }
        }

        // Unregister IPluginAgent.
        pAgent->FreeObject(pParser);
        m_AgentMap.erase(iter);
    }
}

vector<string> MediaLoader::SupportedSuffixes() const
{
    vector<string> list;
    list.reserve(m_MediaPackMap.size());
    for (const auto& entry: m_MediaPackMap) {
        list.push_back(entry.first);
    }
    return list;
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
    const string& suffix = ToLower(FileHelper::FileSuffix(path));
    auto iter = m_MediaPackMap.find(suffix);

    if (iter == m_MediaPackMap.end()) {
        // General Media
        list.emplace_back(path);
    } else {
        // MediaPack
        IMediaPack* pack = iter->second;
        pack->DumpMedia(path, list, &m_MediaPackMap);
    }

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryParseTag(deque<MediaItem>& list) const
{
    for (auto& item: list) {
        // Find TagParser.
        const string& suffix = ToLower(FileHelper::FileSuffix(item.url));
        auto iter = m_TagParserMap.find(suffix);
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
            //cout << "WARN: no tag!!" << endl;
        }

        if (parser->HasAudioProperty()) {
            if (item.duration < 0)
                item.duration = parser->Duration();
        } else {
            item.duration = 0;
            //cout << "FATAL: no properties!!" << endl;
        }

        parser->Close();
    }

    return ErrorCode::Ok;
}
