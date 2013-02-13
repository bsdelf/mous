#include "ServerContext.h"

#include <fstream>
#include <sstream>
using namespace std;

#include <scx/BufObj.hpp>
#include <scx/Signal.hpp>
using namespace scx;

#include <util/PlaylistSerializer.h>
using namespace mous;

#include "AppEnv.h"

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
    player->SigFinished()->DisconnectObject(this);

    ClosePlayer();

    IPluginManager::Free(mgr);
    IMediaLoader::Free(loader);
    IPlayer::Free(player);
}

bool ServerContext::Init()
{
    const AppEnv* env = GlobalAppEnv::Instance();
    if (env == nullptr)
        return false;

    if (!mgr->LoadPluginDir(env->pluginDir))
        return false;

    typedef vector<const IPluginAgent*> PluginAgentArray;

    PluginAgentArray decoders = mgr->PluginAgents(PluginType::Decoder);
    //PluginAgentArray encoders;
    //mgr->DumpPluginAgent(encoders, PluginType::Encoder);
    PluginAgentArray renderers = mgr->PluginAgents(PluginType::Renderer);
    PluginAgentArray mediaPacks = mgr->PluginAgents(PluginType::MediaPack);
    PluginAgentArray tagParsers = mgr->PluginAgents(PluginType::TagParser);

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

void ServerContext::Dump()
{
    typedef PlaylistSerializer<MediaItem> Serializer;

    const AppEnv* env = GlobalAppEnv::Instance();
    if (env == nullptr)
        return;

    // save context
    BufObj buf(nullptr);
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf.PutArray(selectedItem);

    vector<char> outbuf(buf.Offset());

    buf.SetBuffer(&outbuf[0]);
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf.PutArray(selectedItem);

    fstream outfile;
    outfile.open(env->contextFile.c_str(), ios::binary | ios::out);
    outfile.write(&outbuf[0], outbuf.size());
    outfile.close();
    
    // save playlists
    vector<char> nameBuf(env->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(&nameBuf[0], nameBuf.size(), env->playlistFile.c_str(), i);
        Serializer::Store(playlists[i], &nameBuf[0]);
    }
}

void ServerContext::Restore()
{
    typedef PlaylistSerializer<MediaItem> Serializer;

    const AppEnv* env = GlobalAppEnv::Instance();
    if (env == nullptr)
        return;

    // load context
    stringstream stream;
    fstream infile;
    infile.open(env->contextFile.c_str(), ios::binary | ios::in);
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
    vector<char> nameBuf(env->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(&nameBuf[0], nameBuf.size(), env->playlistFile.c_str(), i);
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

    list.JumpTo(iItem);
    const MediaItem& item = list.NextItem(0, false);
    return PlayItem(item);
}

bool ServerContext::PlayNext(char direct)
{
    playlist_t& list = playlists[usedPlaylist];
    if (list.HasNext(direct)) {
        const MediaItem& item = list.NextItem(direct, true);
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
    return list.HasNext(0) ? &list.NextItem(0, false) : nullptr;
}

bool ServerContext::PlayItem(const MediaItem& item)
{
    ClosePlayer();

    if (player->Open(item.url) != mous::ErrorCode::Ok)
        return false;

    if (item.hasRange)
        player->Play(item.msBeg, item.msEnd);
    else
        player->Play();

    return true;
}

void ServerContext::SetPlayMode(EmPlaylistMode mode)
{
    playMode = mode;
    for (auto& list: playlists) {
        list.SetMode(playMode);
    }
}

void ServerContext::ClosePlayer()
{
    if (player->Status() != PlayerStatus::Closed)
        player->Close();
}

void ServerContext::SlotFinished()
{
    lock_guard<mutex> locker(mtx);

    playlist_t& list = playlists[usedPlaylist];
    if (list.HasNext(1)) {
        const MediaItem& item = list.NextItem(1, true);
        PlayItem(item);
        sigPlayNextItem(item);
    }
}
