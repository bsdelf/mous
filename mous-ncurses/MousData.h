#ifndef MOUSDATA_H
#define MOUSDATA_H

#include <string>
#include <map>
using namespace std;

#include <util/MediaItem.h>
#include <util/Playlist.h>
#include <core/IPlayer.h>
#include <core/IMediaLoader.h>
#include <core/IPluginManager.h>
#include <core/ITagParserFactory.h>
using namespace mous;

struct MousData
{
    IPluginManager* mgr;
    IMediaLoader* loader;
    IPlayer* player;

    typedef Playlist<MediaItem> playlist_t;
    typedef map<string, playlist_t*>::iterator PlaylistMapIter;
    typedef map<string, playlist_t*>::const_iterator PlaylistMapConstIter;
    typedef pair<string, playlist_t*> PlaylistMapPair;
    map<string, playlist_t*> playlistMap;

    MousData()
    {
        mgr = IPluginManager::Create();
        loader = IMediaLoader::Create();
        player = IPlayer::Create();
    }

    ~MousData()
    {
        IPluginManager::Free(mgr);
        IMediaLoader::Free(loader);
        IPlayer::Free(player);

        ClearPlaylist();
    }

    void ClearPlaylist()
    {
        PlaylistMapIter iter = playlistMap.begin();
        PlaylistMapIter end = playlistMap.end();
        for (; iter != end; ++iter) {
            delete iter->second;
        }
        playlistMap.clear();
    }
};

#endif
