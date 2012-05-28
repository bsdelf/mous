#ifndef SERVERCONTEXT_H
#define SERVERCONTEXT_H

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

struct ServerContext
{
    Mutex mutex;

    IPluginManager* mgr;
    IMediaLoader* loader;
    IPlayer* player;

    typedef Playlist<MediaItem*> playlist_t;
    vector<playlist_t> playlists;

    EmPlaylistMode playMode;

    int usedPlaylist;
    int selectedPlaylist;
    vector<int> selectedItem;

    Signal<void (const MediaItem*)> sigPlayNextItem;

public:
    ServerContext();
    ~ServerContext();

    bool Init();
    void Cleanup();
    void ClearPlaylists();

    void Dump();
    void Restore();

    void NextPlayMode();

    bool PlayAt(int iList, int iItem);
    bool PlayNext(char direct);
    void PausePlayer();
    const MediaItem* ItemInPlaying() const;

private:
    void SetPlayMode(EmPlaylistMode mode);

    void ClosePlayer();
    bool PlayItem(const MediaItem* item);

    void SlotFinished();
};

#endif
