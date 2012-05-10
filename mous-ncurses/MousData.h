#ifndef MOUSDATA_H
#define MOUSDATA_H

#include <string>
#include <vector>
#include <map>
using namespace std;

#include <scx/Mutex.hpp>
using namespace scx;

#include <util/MediaItem.h>
#include <util/Playlist.h>
#include <core/IPlayer.h>
#include <core/IMediaLoader.h>
#include <core/IPluginManager.h>
#include <core/ITagParserFactory.h>
using namespace mous;

#include "Config.h"

struct MousData
{
    Mutex mutex;
    IPluginManager* mgr;
    IMediaLoader* loader;
    IPlayer* player;

    typedef Playlist<MediaItem> playlist_t;
    typedef map<string, playlist_t*>::iterator PlaylistMapIter;
    typedef map<string, playlist_t*>::const_iterator PlaylistMapConstIter;
    typedef pair<string, playlist_t*> PlaylistMapPair;
    map<string, playlist_t*> playlistMap;

    MousData()
    {
        mgr = IPluginManager::Create();
        loader = IMediaLoader::Create();
        player = IPlayer::Create();
    }

    ~MousData()
    {
        IPluginManager::Free(mgr);
        IMediaLoader::Free(loader);
        IPlayer::Free(player);

        ClearPlaylist();
    }

    void Init()
    {
        mgr->LoadPluginDir(Config::PluginDir);

        typedef vector<const IPluginAgent*> PluginAgentArray;

        PluginAgentArray decoders;
        mgr->Plugins(decoders, PluginType::Decoder);
        //PluginAgentArray encoders;
        //mgr->Plugins(encoders, PluginType::Encoder);
        PluginAgentArray renderers;
        mgr->Plugins(renderers, PluginType::Renderer);
        PluginAgentArray mediaPacks;
        mgr->Plugins(mediaPacks, PluginType::MediaPack);
        PluginAgentArray tagParsers;
        mgr->Plugins(tagParsers, PluginType::TagParser);

        loader->RegisterMediaPackPlugin(mediaPacks);
        loader->RegisterTagParserPlugin(tagParsers);

        player->RegisterRendererPlugin(renderers[0]);
        player->RegisterDecoderPlugin(decoders);
    }

    void Cleanup()
    {
        loader->UnregisterAll();
        player->UnregisterAll();
        mgr->UnloadAll();
    }

    void ClearPlaylist()
    {
        PlaylistMapIter iter = playlistMap.begin();
        PlaylistMapIter end = playlistMap.end();
        for (; iter != end; ++iter) {
            delete iter->second;
        }
        playlistMap.clear();
    }
};

#endif
