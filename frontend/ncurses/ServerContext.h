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
    Mutex playerMutex;

    typedef Playlist<MediaItem*> playlist_t;
    vector<playlist_t> playlists;

    int usedPlaylist;
    int selectedPlaylist;
    vector<int> selectedItem;

    ServerContext();
    ~ServerContext();

    bool Init();
    void Cleanup();
    void ClearPlaylists();

    bool PlayAt(int iList, int iItem);
    void PausePlayer();

    const MediaItem* ItemInPlaying() const;

    const Signal<void (const MediaItem*)>& SigPlayNextItem() const
    {
        return m_SigPlayNextItem;
    }

private:
    void ClosePlayer();
    bool PlayItem(const MediaItem* item);

    void SlotFinished();

    Signal<void (const MediaItem*)> m_SigPlayNextItem;
};

#endif
