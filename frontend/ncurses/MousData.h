#ifndef MOUSDATA_H
#define MOUSDATA_H

#include <string>
#include <vector>
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

    typedef Playlist<MediaItem*> playlist_t;
    vector<playlist_t> playlists;

    MousData()
    {
        mgr = IPluginManager::Create();
        loader = IMediaLoader::Create();
        player = IPlayer::Create();
        playlists.resize(6);
    }

    ~MousData()
    {
        IPluginManager::Free(mgr);
        IMediaLoader::Free(loader);
        IPlayer::Free(player);

        ClearPlaylists();
    }

    void Init()
    {
        mgr->LoadPluginDir(Config::PluginDir);

        typedef vector<const IPluginAgent*> PluginAgentArray;

        PluginAgentArray decoders;
        mgr->DumpPluginAgent(decoders, PluginType::Decoder);
        //PluginAgentArray encoders;
        //mgr->DumpPluginAgent(encoders, PluginType::Encoder);
        PluginAgentArray renderers;
        mgr->DumpPluginAgent(renderers, PluginType::Renderer);
        PluginAgentArray mediaPacks;
        mgr->DumpPluginAgent(mediaPacks, PluginType::MediaPack);
        PluginAgentArray tagParsers;
        mgr->DumpPluginAgent(tagParsers, PluginType::TagParser);

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

    void ClearPlaylists()
    {
        for (size_t i = 0; i < playlists.size(); ++i) {
            for (int n = 0; n < playlists[i].Count(); ++n)
                delete playlists[i][n];
        }
    }
};

#endif
