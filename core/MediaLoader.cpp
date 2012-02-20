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

void MediaLoader::AddMediaPack(const PluginAgent* pAgent)
{
    // Register agent.
    IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pPack));

    /*
    // Register MediaPack.
    vector<string> list;
    pDecoder->GetFileSuffix(list);
    for (size_t i = 0; i < list.size(); ++i) {
	string suffix = ToLower(list[i]);
	DecoderMapIter iter = m_DecoderMap.find(suffix);
	if (iter == m_DecoderMap.end()) {
	    vector<IDecoder*>* dlist = new vector<IDecoder*>();
	    dlist->push_back(pDecoder);
	    m_DecoderMap.insert(DecoderMapPair(suffix, dlist));
	} else {
	    vector<IDecoder*>* dlist = iter->second;
	    dlist->push_back(pDecoder);
	}
    }
    */
}

void MediaLoader::RemoveMediaPack(const PluginAgent* pAgent)
{

}
#include <iostream>
void MediaLoader::AddTagParser(const PluginAgent* pAgent)
{
    // Register agent.
    ITagParser* pParser = (ITagParser*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pParser));

    // Register TagParser
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

    /*
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
	// Unregister decoder.
	vector<string> list;
	IDecoder* pDecoder = (IDecoder*)iter->second;
	pDecoder->GetFileSuffix(list);
	for (size_t i = 0; i < list.size(); ++i) {
	    string suffix = ToLower(list[i]);
	    DecoderMapIter iter = m_DecoderMap.find(suffix);
	    if (iter != m_DecoderMap.end()) {
		vector<IDecoder*>* dlist = iter->second;
		for (size_t i = 0; i < dlist->size(); ++i) {
		    if ((*dlist)[i] == pDecoder) {
			dlist->erase(dlist->begin()+i);
			break;
		    }
		}
		if (dlist->empty()) {
		    delete dlist;
		    m_DecoderMap.erase(iter);
		}
	    }
	}

	// Unregister agent.
	pAgent->ReleaseObject(pDecoder);
	m_AgentMap.erase(iter);
    }
    */
}

EmErrorCode MediaLoader::LoadMedia(const string& path, deque<MediaItem*>& list)
{
    MediaItem* item = new MediaItem;
    item->url = path;
    item->hasRange = false;
    list.push_back(item);

    TryParseTag(list);

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryUnpack(const string& path, deque<MediaItem*>& list)
{
    string suffix = ToLower(FileSuffix(path));
    //DecoderMapIter iter = m_DecoderMap.find(suffix);
    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryParseTag(deque<MediaItem*>& list)
{
    for (size_t i = 0; i < list.size(); ++i) {
	MediaItem* item = list[i];
	string suffix = ToLower(FileSuffix(item->url));
	TagParserMapIter iter = m_TagParserMap.find(suffix);
	if (iter == m_TagParserMap.end()) {
	    iter = m_TagParserMap.find("*");
	    if (iter == m_TagParserMap.end())
		continue;
	}

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
