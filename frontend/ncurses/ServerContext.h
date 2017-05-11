#pragma once

#include <vector>
#include <mutex>
using namespace std;

#include <scx/Signal.hpp>
using namespace scx;

#include <util/MediaItem.h>
#include <util/Playlist.h>
#include <core/Player.h>
#include <core/MediaLoader.h>
#include <core/PluginManager.h>
using namespace mous;

struct ServerContext
{
    mutex mtx;

    PluginManager mgr;
    MediaLoader loader;
    Player player;

    typedef Playlist<MediaItem> playlist_t;
    vector<playlist_t> playlists;

    PlaylistMode playMode;

    int usedPlaylist;
    int selectedPlaylist;
    vector<int> selectedItem;

    Signal<void (const MediaItem&)> sigPlayNextItem;

public:
    ServerContext();
    ~ServerContext();

    bool Init();
    void Cleanup();

    void Dump();
    void Restore();

    void NextPlayMode();

    bool PlayAt(int iList, int iItem);
    bool PlayNext(char direct);
    void PausePlayer();
    const MediaItem* ItemInPlaying() const;

private:
    void SetPlayMode(PlaylistMode mode);

    void ClosePlayer();
    bool PlayItem(const MediaItem& item);

    void SlotFinished();
};

