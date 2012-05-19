#ifndef MOUSDATA_H
#define MOUSDATA_H

#include <string>
#include <vector>
using namespace std;

#include <scx/Mutex.hpp>
#include <scx/Signal.hpp>
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

    int usedPlaylist;
    int selectedPlaylist;
    vector<int> selectedItem;

    MousData():
        playlists(6),
        usedPlaylist(-1),
        selectedPlaylist(1),
        selectedItem(6, 0)
    {
        mgr = IPluginManager::Create();
        loader = IMediaLoader::Create();
        player = IPlayer::Create();
        player->SigFinished()->Connect(&MousData::SlotFinished, this);
    }

    ~MousData()
    {
        player->SigFinished()->DisconnectReceiver(this);

        ClosePlayer();

        IPluginManager::Free(mgr);
        IMediaLoader::Free(loader);
        IPlayer::Free(player);

        ClearPlaylists();
    }

    bool Init()
    {
        const Config* config = GlobalConfig::Instance();
        if (config == NULL)
            return false;

        if (!mgr->LoadPluginDir(config->pluginDir))
            return false;

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

        return true;
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
            playlist_t& list = playlists[i];
            for (int n = 0; n < list.Count(); ++n)
                delete list[n];
        }
    }

    void PlayAt(int iList, int iItem)
    {
        usedPlaylist = iList;
        MousData::playlist_t& list = playlists[iList];

        MediaItem* item = NULL;
        list.SeqJumpTo(iItem);
        list.SeqCurrent(item);

        PlayItem(item);
    }

    void ClosePlayer()
    {
        if (player->Status() != PlayerStatus::Closed)
            player->Close();
    }

    void PausePlayer()
    {
        switch (player->Status()) {
            case PlayerStatus::Playing:
                player->Pause();
                break;
                
            case PlayerStatus::Paused:
                player->Resume();
                break;

            default:
                break;
        }
    }

private:
    void SlotFinished()
    {
        MutexLocker locker(&mutex);

        playlist_t& list = playlists[usedPlaylist];
        MediaItem* item = NULL;
        if (list.SeqCurrent(item, 1)) {
            list.SeqMoveNext();
            list.SeqCurrent(item);
        }
        if (item != NULL) {
            PlayItem(item);
        }
    }

    void PlayItem(const MediaItem* item)
    {
        ClosePlayer();

        player->Open(item->url);
        if (item->hasRange)
            player->Play(item->msBeg, item->msEnd);
        else
            player->Play();
    }
};

#endif
