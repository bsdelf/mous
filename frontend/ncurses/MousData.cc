#include "MousData.h"

#include <scx/Signal.hpp>
using namespace scx;

#include "Config.h"

MousData::MousData():
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

MousData::~MousData()
{
    player->SigFinished()->DisconnectReceiver(this);

    ClosePlayer();

    IPluginManager::Free(mgr);
    IMediaLoader::Free(loader);
    IPlayer::Free(player);

    ClearPlaylists();
}

bool MousData::Init()
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

void MousData::Cleanup()
{
    loader->UnregisterAll();
    player->UnregisterAll();
    mgr->UnloadAll();
}

void MousData::ClearPlaylists()
{
    for (size_t i = 0; i < playlists.size(); ++i) {
        playlist_t& list = playlists[i];
        for (int n = 0; n < list.Count(); ++n)
            delete list[n];
    }
}

bool MousData::PlayAt(int iList, int iItem)
{
    usedPlaylist = iList;
    MousData::playlist_t& list = playlists[iList];

    MediaItem* item = NULL;
    list.SeqJumpTo(iItem);
    list.SeqCurrent(item);

    return PlayItem(item);
}

void MousData::PausePlayer()
{
    MutexLocker locker(&playerMutex);

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

const MediaItem* MousData::ItemInPlaying() const
{
    MediaItem* item = NULL;
    const playlist_t& list = playlists[usedPlaylist];
    list.SeqCurrent(item, 0);
    return item;
}

bool MousData::PlayItem(const MediaItem* item)
{
    MutexLocker locker(&playerMutex);

    ClosePlayer();

    if (player->Open(item->url) != mous::ErrorCode::Ok)
        return false;

    if (item->hasRange)
        player->Play(item->msBeg, item->msEnd);
    else
        player->Play();

    return true;
}

void MousData::ClosePlayer()
{
    if (player->Status() != PlayerStatus::Closed)
        player->Close();
}

void MousData::SlotFinished()
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
        m_SigPlayNextItem(item);
    }
}
