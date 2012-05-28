#include "ServerContext.h"

#include <fstream>
#include <sstream>
using namespace std;

#include <scx/BufObj.hpp>
#include <scx/Signal.hpp>
using namespace scx;

#include <util/PlaylistSerializer.h>
using namespace mous;

#include "Config.h"

const int VERSION = 1;

ServerContext::ServerContext():
    playlists(6),
    usedPlaylist(-1),
    selectedPlaylist(1),
    selectedItem(6, 0)
{
    mgr = IPluginManager::Create();
    loader = IMediaLoader::Create();
    player = IPlayer::Create();
    player->SigFinished()->Connect(&ServerContext::SlotFinished, this);

    SetPlayMode(PlaylistMode::Normal);
}

ServerContext::~ServerContext()
{
    player->SigFinished()->DisconnectReceiver(this);

    ClosePlayer();

    IPluginManager::Free(mgr);
    IMediaLoader::Free(loader);
    IPlayer::Free(player);

    ClearPlaylists();
}

bool ServerContext::Init()
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

void ServerContext::Cleanup()
{
    loader->UnregisterAll();
    player->UnregisterAll();
    mgr->UnloadAll();
}

void ServerContext::ClearPlaylists()
{
    for (size_t i = 0; i < playlists.size(); ++i) {
        playlist_t& list = playlists[i];
        for (int n = 0; n < list.Count(); ++n)
            delete list[n];
    }
}

void ServerContext::Dump()
{
    typedef PlaylistSerializer<MediaItem*> Serializer;

    const Config* config = GlobalConfig::Instance();
    if (config == NULL)
        return;

    // save context
    BufObj buf(NULL);
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf.PutArray(selectedItem);

    vector<char> outbuf(buf.Offset());

    buf.SetBuffer(&outbuf[0]);
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf.PutArray(selectedItem);

    fstream outfile;
    outfile.open(config->contextFile.c_str(), ios::binary | ios::out);
    outfile.write(&outbuf[0], outbuf.size());
    outfile.close();
    
    // save playlists
    vector<char> nameBuf(config->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(&nameBuf[0], nameBuf.size(), config->playlistFile.c_str(), i);
        Serializer::Store(playlists[i], &nameBuf[0]);
    }
}

void ServerContext::Restore()
{
    typedef PlaylistSerializer<MediaItem*> Serializer;

    const Config* config = GlobalConfig::Instance();
    if (config == NULL)
        return;

    // load context
    stringstream stream;
    fstream infile;
    infile.open(config->contextFile.c_str(), ios::binary | ios::in);
    stream << infile.rdbuf();
    infile.close();

    BufObj buf(const_cast<char*>(stream.str().data()));

    int version;
    buf >> version;
    if (version != VERSION)
        return;

    char mode;
    int used;
    int selected;
    buf >> mode >> used >> selected;
    buf.TakeArray(selectedItem);

    playMode = (EmPlaylistMode)mode;
    usedPlaylist = used;
    selectedPlaylist = selected;

    // load playlists
    vector<char> nameBuf(config->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(&nameBuf[0], nameBuf.size(), config->playlistFile.c_str(), i);
        Serializer::Load(playlists[i], &nameBuf[0]);
    }
}

void ServerContext::NextPlayMode()
{
    int mode = static_cast<int>(playMode) + 1;
    if (mode >= static_cast<int>(PlaylistMode::Top))
        mode = PlaylistMode::Normal;
    SetPlayMode(static_cast<EmPlaylistMode>(mode));
}

bool ServerContext::PlayAt(int iList, int iItem)
{
    usedPlaylist = iList;
    ServerContext::playlist_t& list = playlists[iList];

    list.SeqJumpTo(iItem);
    const MediaItem* item = list.SeqItemAtOffset(0, false);
    return PlayItem(item);
}

bool ServerContext::PlayNext(char direct)
{
    playlist_t& list = playlists[usedPlaylist];
    if (list.SeqHasOffset(direct)) {
        const MediaItem* item = list.SeqItemAtOffset(direct, true);
        PlayItem(item);
        sigPlayNextItem(item);
        return true;
    } else {
        return false;
    }
}

void ServerContext::PausePlayer()
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

const MediaItem* ServerContext::ItemInPlaying() const
{
    const playlist_t& list = playlists[usedPlaylist];
    return list.SeqHasOffset(0) ? list.SeqItemAtOffset(0, false) : NULL;
}

bool ServerContext::PlayItem(const MediaItem* item)
{
    ClosePlayer();

    if (player->Open(item->url) != mous::ErrorCode::Ok)
        return false;

    if (item->hasRange)
        player->Play(item->msBeg, item->msEnd);
    else
        player->Play();

    return true;
}

void ServerContext::SetPlayMode(EmPlaylistMode mode)
{
    playMode = mode;
    for (size_t i = 0; i < playlists.size(); ++i) {
        playlists[i].SetMode(playMode);
    }
}

void ServerContext::ClosePlayer()
{
    if (player->Status() != PlayerStatus::Closed)
        player->Close();
}

void ServerContext::SlotFinished()
{
    MutexLocker locker(&mutex);

    playlist_t& list = playlists[usedPlaylist];
    if (list.SeqHasOffset(1)) {
        const MediaItem* item = list.SeqItemAtOffset(1, true);
        PlayItem(item);
        sigPlayNextItem(item);
    }
}
