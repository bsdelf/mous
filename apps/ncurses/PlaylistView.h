#pragma once

#include <deque>
#include <mutex>
using namespace std;

#include <scx/Signal.h>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

#include "BaseView.h"
#include "ClientPlaylistHandler.h"
#include "Ncurses.h"

class PlaylistView : public BaseView {
 public:
  PlaylistView();
  ~PlaylistView();

  void Refresh();
  bool NeedRefresh() const;

  void MoveTo(int x, int y);
  void Resize(int w, int h);

  bool InjectKey(int key);

  void Show(bool show);
  bool IsShown() const;

  void SetFocus(bool focus);
  bool HasFocus() const;

 public:
  int Index() const;
  void SetIndex(int i);

  void SetPlaylistHandle(ClientPlaylistHandler* handler);

 private:
  void ScrollUp();
  void ScrollDown();

 private:
  void ReqSelect();
  void ReqPlay(int);
  void ReqRemove(int);
  void ReqMove(int, char);
  void ReqClear();

  void SlotSelect(int, int);
  void SlotPlay(int, bool);
  void SlotAppend(int, deque<MediaItem*>&);
  void SlotRemove(int, int);
  void SlotMove(int, int, char);
  void SlotClear(int);

 public:
  Signal<void(bool)> SigSwitchPlaylist;

 private:
  ncurses::Window d;

  int m_NeedRefresh = 0;
  mutable mutex m_NeedRefreshMutex;

  bool m_Focused = false;
  int m_Index = -1;
  int m_ItemBegin = 0;
  int m_ItemSelected = 0;
  std::string m_Title = "Playlist";
  deque<MediaItem*> m_List;
  bool m_WaitReply = false;

  ClientPlaylistHandler* m_PlaylistHandler = nullptr;
};
