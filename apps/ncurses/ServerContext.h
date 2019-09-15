#pragma once

#include <mutex>
#include <vector>
using namespace std;

#include <scx/Signal.h>
using namespace scx;

#include <core/MediaLoader.h>
#include <core/Player.h>
#include <util/MediaItem.h>
#include <util/Playlist.h>
using namespace mous;

struct ServerContext {
  mutex mtx;

  MediaLoader loader;
  Player player;

  typedef Playlist<MediaItem> playlist_t;
  vector<playlist_t> playlists;

  PlaylistMode playMode;

  int usedPlaylist;
  int selectedPlaylist;
  vector<int> selectedItem;

  Signal<void(const MediaItem&)> sigPlayNextItem;

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
