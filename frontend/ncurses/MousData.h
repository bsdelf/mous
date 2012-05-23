#ifndef MOUSDATA_H
#define MOUSDATA_H

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

struct MousData
{
    Mutex mutex;

    IPluginManager* mgr;
    IMediaLoader* loader;
    IPlayer* player;
    Mutex playerMutex;

    typedef Playlist<MediaItem*> playlist_t;
    vector<playlist_t> playlists;

    int usedPlaylist;
    int selectedPlaylist;
    vector<int> selectedItem;

    MousData();
    ~MousData();

    bool Init();
    void Cleanup();
    void ClearPlaylists();

    void PlayAt(int iList, int iItem);
    void ClosePlayer();
    void PausePlayer();

    const MediaItem* ItemInPlaying() const;

private:
    void SlotFinished();

    void PlayItem(const MediaItem* item);
};

#endif
