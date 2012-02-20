#include "MediaLoader.h"
#include <scx/FileHelp.hpp>
#include <scx/Conv.hpp>
#include <mous/MediaItem.h>
#include <mous/IMediaPack.h>
#include <mous/ITagParser.h>
using namespace std;
using namespace scx;
using namespace mous;

MediaLoader::MediaLoader()
{

}

MediaLoader::~MediaLoader()
{

}

void MediaLoader::RegisterPluginAgent(const PluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
	case PluginType::MediaPack:
	    AddMediaPack(pAgent);
	    break;
	
	case PluginType::TagParser:
	    AddTagParser(pAgent);
	    break;

	default:
	    break;
    }
}

void MediaLoader::UnregisterPluginAgent(const PluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
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

void MediaLoader::UnregisterAll()
{
    while (!m_AgentMap.empty()) {
	AgentMapIter iter = m_AgentMap.begin();
	UnregisterPluginAgent(iter->first);
    }
}

void MediaLoader::AddMediaPack(const PluginAgent* pAgent)
{
    // Register agent.
    IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pPack));

    // Register MediaPack.
    vector<string> list;
    pPack->GetFileSuffix(list);
    for (size_t i = 0; i < list.size(); ++i) {
	string suffix = ToLower(list[i]);
	MediaPackMapIter iter = m_MediaPackMap.find(suffix);
	if (iter == m_MediaPackMap.end()) {
	    m_MediaPackMap.insert(MediaPackMapPair(suffix, pPack));
	}
    }
}

void MediaLoader::RemoveMediaPack(const PluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
	// Unregister MediaPack.
	vector<string> list;
	IMediaPack* pPack = (IMediaPack*)iter->second;
	pPack->GetFileSuffix(list);
	for (size_t i = 0; i < list.size(); ++i) {
	    string suffix = ToLower(list[i]);
	    MediaPackMapIter iter = m_MediaPackMap.find(suffix);
	    if (iter != m_MediaPackMap.end()) {
		m_MediaPackMap.erase(iter);
	    }
	}

	// Unregister PluginAgent.
	pAgent->ReleaseObject(pPack);
	m_AgentMap.erase(iter);
    }
}

void MediaLoader::AddTagParser(const PluginAgent* pAgent)
{
    // Register PluginAgent.
    ITagParser* pParser = (ITagParser*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pParser));

    // Register TagParser.
    vector<string> list;
    pParser->GetFileSuffix(list);
    for (size_t i = 0; i < list.size(); ++i) {
	string suffix = ToLower(list[i]);
	TagParserMapIter iter = m_TagParserMap.find(suffix);
	if (iter == m_TagParserMap.end()) {
	    m_TagParserMap.insert(TagParserMapPair(suffix, pParser));
	}
    }
}

void MediaLoader::RemoveTagParser(const PluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
	// Unregister TagParser.
	vector<string> list;
	ITagParser* pParser = (ITagParser*)iter->second;
	pParser->GetFileSuffix(list);
	for (size_t i = 0; i < list.size(); ++i) {
	    string suffix = ToLower(list[i]);
	    TagParserMapIter iter = m_TagParserMap.find(suffix);
	    if (iter != m_TagParserMap.end()) {
		m_TagParserMap.erase(iter);
	    }
	}

	// Unregister PluginAgent.
	pAgent->ReleaseObject(pParser);
	m_AgentMap.erase(iter);
    }
}

EmErrorCode MediaLoader::LoadMedia(const string& path, deque<MediaItem*>& list)
{
    TryUnpack(path, list);
    TryParseTag(list);

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryUnpack(const string& path, deque<MediaItem*>& list)
{
    // Find MediaPack.
    string suffix = ToLower(FileSuffix(path));
    MediaPackMapIter iter = m_MediaPackMap.find(suffix);
    if (iter == m_MediaPackMap.end()) {
	// General Media
	MediaItem* item = new MediaItem;
	item->url = path;
	item->hasRange = false;
	list.push_back(item);
    } else {
	// MediaPack
	IMediaPack* pack = iter->second;
	pack->DumpMedia(path, list, &m_MediaPackMap);
    }

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryParseTag(deque<MediaItem*>& list)
{
    for (size_t i = 0; i < list.size(); ++i) {
	// Find TagParser.
	MediaItem* item = list[i];
	string suffix = ToLower(FileSuffix(item->url));
	TagParserMapIter iter = m_TagParserMap.find(suffix);
	if (iter == m_TagParserMap.end()) {
	    iter = m_TagParserMap.find("*");
	    if (iter == m_TagParserMap.end())
		continue;
	}

	// Parse & Fill.
	ITagParser* parser = iter->second;
	parser->Open(item->url);
	if (!parser->IsEmpty()) {
	    item->title = parser->GetTitle();
	    item->artist = parser->GetArtist();
	    item->album = parser->GetAlbum();
	    item->comment = parser->GetComment();
	    item->genre = parser->GetGenre();
	    item->year = parser->GetYear();
	    item->track = parser->GetTrack();
	}
	parser->Close();
    }

    return ErrorCode::Ok;
}
