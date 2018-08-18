#include "ServerContext.h"

#include <fstream>
#include <sstream>
using namespace std;

#include <scx/BufObj.h>
#include <scx/Signal.h>
using namespace scx;

#include <util/PlaylistSerializer.h>
#include <util/PluginScanner.h>
using namespace mous;

#include "AppEnv.h"

const int VERSION = 1;

ServerContext::ServerContext():
    playlists(6),
    usedPlaylist(-1),
    selectedPlaylist(1),
    selectedItem(6, 0)
{
    player.SigFinished()->Connect(&ServerContext::SlotFinished, this);

    SetPlayMode(PlaylistMode::Normal);
}

ServerContext::~ServerContext()
{
    player.SigFinished()->Disconnect(this);

    ClosePlayer();
}

bool ServerContext::Init()
{
    bool hasDecoder = false;
    bool hasOutput = false;
    const auto env = GlobalAppEnv::Instance();
    PluginScanner()
        .OnPlugin(PluginType::Decoder, [&, this](const std::shared_ptr<Plugin>& plugin) {
            player.LoadDecoderPlugin(plugin);
            hasDecoder = true;
        })
        .OnPlugin(PluginType::Output, [&, this](const std::shared_ptr<Plugin>& plugin) {
            player.LoadOutputPlugin(plugin);
            hasOutput = true;
        })
        .OnPlugin(PluginType::SheetParser, [this](const std::shared_ptr<Plugin>& plugin) {
            loader.LoadSheetParserPlugin(plugin);
        })
        .OnPlugin(PluginType::TagParser, [this](const std::shared_ptr<Plugin>& plugin) {
            loader.LoadTagParserPlugin(plugin);
        })
        .Scan(env->pluginDir);

    return hasDecoder && hasOutput;
}

void ServerContext::Cleanup()
{
    loader.UnloadPlugin();
    player.UnloadPlugin();
}

void ServerContext::Dump()
{
    typedef PlaylistSerializer<MediaItem> Serializer;

    const auto env = GlobalAppEnv::Instance();

    // save context
    BufObj buf(nullptr);
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf << selectedItem;

    vector<char> outbuf(buf.Offset());

    buf.SetBuffer(outbuf.data());
    buf << (int)VERSION;
    buf << (char)playMode << (int)usedPlaylist << (int)selectedPlaylist;
    buf << selectedItem;

    fstream outfile;
    outfile.open(env->contextFile.c_str(), ios::binary | ios::out);
    outfile.write(outbuf.data(), outbuf.size());
    outfile.close();
    
    // save playlists
    vector<char> nameBuf(env->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(nameBuf.data(), nameBuf.size(), env->playlistFile.c_str(), i);
        Serializer::Store(playlists[i], nameBuf.data());
    }
}

void ServerContext::Restore()
{
    typedef PlaylistSerializer<MediaItem> Serializer;

    const auto env = GlobalAppEnv::Instance();

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

    playMode = (PlaylistMode)mode;
    usedPlaylist = used;
    selectedPlaylist = selected;

    // load playlists
    vector<char> nameBuf(env->playlistFile.size() + 2);
    for (size_t i = 0; i < playlists.size(); ++i) {
        snprintf(nameBuf.data(), nameBuf.size(), env->playlistFile.c_str(), i);
        Serializer::Load(playlists[i], nameBuf.data());
    }
}

void ServerContext::NextPlayMode()
{
    int mode = static_cast<int>(playMode) + 1;
    if (mode >= static_cast<int>(PlaylistMode::Top))
        mode = static_cast<int>(PlaylistMode::Normal);
    SetPlayMode(static_cast<PlaylistMode>(mode));
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
    switch (player.Status()) {
        case PlayerStatus::Playing:
            player.Pause();
            break;
            
        case PlayerStatus::Paused:
            player.Resume();
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

    if (player.Open(item.url) != mous::ErrorCode::Ok)
        return false;

    if (item.hasRange)
        player.Play(item.msBeg, item.msEnd);
    else
        player.Play();

    return true;
}

void ServerContext::SetPlayMode(PlaylistMode mode)
{
    playMode = mode;
    for (auto& list: playlists) {
        list.SetMode(playMode);
    }
}

void ServerContext::ClosePlayer()
{
    if (player.Status() != PlayerStatus::Closed)
        player.Close();
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
