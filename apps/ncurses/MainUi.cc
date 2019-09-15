#include "MainUi.h"

#include <deque>
#include <mutex>
#include <set>
#include <stack>
#include <string>
#include <vector>
using namespace std;

#include <util/MediaItem.h>

#include "BaseView.h"
#include "Client.h"
#include "ExplorerView.h"
#include "HelpView.h"
#include "PlaylistView.h"
#include "StatusView.h"

using namespace scx;
using namespace mous;

namespace View {
enum Type {
  Explorer = 0,
  Playlist,
  Help,
  Status,

  Count
};

typedef unsigned int mask_t;

const mask_t MaskExplorer = 1 << Explorer;
const mask_t MaskPlaylist = 1 << Playlist;
const mask_t MaskHelp = 1 << Help;
const mask_t MaskStatus = 1 << Status;
}  // namespace View
typedef View::Type EmViewType;

const int PLAYLIST_COUNT = 6;

struct Layer {
  View::mask_t mask;
  set<BaseView*> views;
  stack<BaseView*> focused;

  void RefreshViews(bool forced) {
    for (auto view : views) {
      if (forced || view->NeedRefresh()) {
        view->Refresh();
      }
    }
  }

  void ShowViews(bool show) {
    for (auto view : views) {
      view->Show(show);
    }
  }
};

class MainUi::Impl {
 public:
  Impl() {
    client.SigTryConnect().Connect(&Impl::SlotTryConnect, this);
    client.SigConnected().Connect(&Impl::SlotConnected, this);
    client.SigSuffixes().Connect(&Impl::SlotGotSuffixes, this);
    client.PlaylistHandler().SigSwitch().Connect(&Impl::SwitchPlaylist, this);

    statusView.SetPlayerHandler(&client.PlayerHandler());

    PlaylistView& playlist = playlistView[iPlaylist];

    Layer layer;
    layer.mask = View::MaskPlaylist | View::MaskStatus;
    layer.views.insert(&playlist);
    layer.views.insert(&statusView);
    layer.focused.push(&playlist);
    layerStack.push(std::move(layer));

    explorerView.SigTmpOpen.Connect(&Impl::SlotTmpOpen, this);
    explorerView.SigUserOpen.Connect(&Impl::SlotReqUserOpen, this);

    for (int i = 0; i < PLAYLIST_COUNT; ++i) {
      playlistView[i].SetIndex(i);
      playlistView[i].SetPlaylistHandle(&client.PlaylistHandler());
      playlistView[i].SigSwitchPlaylist.Connect(&Impl::SlotSwitchPlaylist, this);
    }
    playlist.SetFocus(true);
  }

  int Exec() {
    BeginNcurses();
    OnResize();

    if (StartClient()) {
      for (bool quit = false; !quit; quit = false) {
        SyncRefresh();

        int key = wgetch(curscr);
        if (key != ERR && HandleTopKey(key, quit)) {
          if (quit) {
            break;
          } else {
            continue;
          }
        } else if (statusView.InjectKey(key)) {
          continue;
        } else {
          layerStack.top().focused.top()->InjectKey(key);
        }
      }
    }

    EndNcurses();
    StopClient();

    return 0;
  }

 private:
  void SlotTryConnect() {
  }

  void SlotConnected() {
    client.PlayerHandler().StartSync();
    client.PlayerHandler().QueryVolume();
    client.PlayerHandler().QueryPlayMode();

    for (int i = 0; i < PLAYLIST_COUNT; ++i) {
      client.PlaylistHandler().Sync(i);
    }
  }

  void SlotGotSuffixes(const std::vector<std::string>& list) {
    explorerView.AddSuffixes(list);
  }

  void SlotSwitchPlaylist(bool toNext) {
    int n = iPlaylist + (toNext ? 1 : -1);
    n = std::min(std::max(n, 0), PLAYLIST_COUNT - 1);

    lock_guard<mutex> locker(needSwitchPlaylistMutex);
    switchPlaylistTo = n;
    ++needSwitchPlaylist;
  }

  void SlotTmpOpen(const string& path) {
  }

  void SlotReqUserOpen(const string& path) {
    client.PlaylistHandler().Append(iPlaylist, path);
  }

  bool StartClient() {
    return client.Run();
  }

  void StopClient() {
    client.Stop();
  }

  void BeginNcurses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    refresh();
    halfdelay(1);
    curs_set(0);
  }

  void EndNcurses() {
    endwin();
  }

  /*
     * NOTE: the best solution is that
     * each layer has its own HandleTopKey()
     * which is stored in layerStack as a function,
     * and handle KEY_RESIZE in HandleResize().
     * by this way,
     * we do not need to check layerStack.size()!
     */
  bool HandleTopKey(int key, bool& quit) {
    switch (key) {
      case KEY_RESIZE:
        OnResize();
        break;

      case 'e':
        if (layerStack.size() == 1)
          ShowOrHideExplorer();
        break;

      case 'H':
        ShowOrHideHelp();
        break;

      case '\t':
        if (layerStack.size() == 1)
          SwitchFocus();
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
        SwitchPlaylist(std::stoi(string(1, (char)key)));
        break;

      case 'q':
        quit = true;
        break;

      case 'x':
        quit = true;
        client.StopService();
        break;

      case ERR:
        break;

      default:
        return false;
    }
    return true;
  }

  void SyncRefresh() {
    // switch playlist as needed
    {
      lock_guard<mutex> locker(needSwitchPlaylistMutex);
      if (needSwitchPlaylist > 0) {
        SwitchPlaylist(switchPlaylistTo);
        --needSwitchPlaylist;
      }
    }

    // refresh top layer as needed
    {
      layerStack.top().RefreshViews(false);
    }
  }

  void OnResize() {
    layerStack.top().ShowViews(false);
    UpdateTopLayout();
  }

  /* update the layout of top layer */
  void UpdateTopLayout() {
    const int w = COLS;
    const int h = LINES;

    layerStack.top().ShowViews(true);

    View::mask_t mask = layerStack.top().mask;
    switch (mask) {
      case View::MaskHelp: {
        helpView.MoveTo(0, 0);
        helpView.Resize(w, h);
        break;
      }

      case View::MaskPlaylist | View::MaskStatus: {
        int x = 0, y = 0;
        int hStatus = statusView.MinHeight();
        int hPlaylist = h - hStatus;
        PlaylistView& playlist = playlistView[iPlaylist];
        playlist.MoveTo(x, y);
        playlist.Resize(w, hPlaylist);
        y += hPlaylist;
        statusView.MoveTo(x, y);
        statusView.Resize(w, hStatus);
        break;
      }

      case View::MaskExplorer | View::MaskPlaylist | View::MaskStatus: {
        int wExplorer = w / 2;
        int wPlaylist = w - wExplorer;
        int hStatus = statusView.MinHeight();
        int hExplorer = h - hStatus;
        int x = 0, y = 0;
        explorerView.MoveTo(x, y);
        explorerView.Resize(wExplorer, hExplorer);
        x += wExplorer;
        PlaylistView& playlist = playlistView[iPlaylist];
        playlist.MoveTo(x, y);
        playlist.Resize(wPlaylist, hExplorer);
        x = 0;
        y += hExplorer;
        statusView.MoveTo(x, y);
        statusView.Resize(w, hStatus);
        break;
      }

      default: {
        break;
      }
    }

    layerStack.top().RefreshViews(true);
  }

  /* on same layer */
  void ShowOrHideExplorer() {
    Layer& layer = layerStack.top();
    layer.focused.top()->SetFocus(false);
    if (explorerView.IsShown()) {
      // remove view
      explorerView.Show(false);
      layer.mask &= ~View::MaskExplorer;
      layer.views.erase(&explorerView);
      layer.focused.pop();
      layer.focused.top() = playlistView + iPlaylist;
      layer.focused.top()->SetFocus(true);
    } else {
      // add view
      layer.mask |= View::MaskExplorer;
      layer.views.insert(&explorerView);
      layer.focused.push(&explorerView);
    }
    layer.focused.top()->SetFocus(true);
    UpdateTopLayout();
  }

  /* between different layers */
  void ShowOrHideHelp() {
    bool helpViewShown = helpView.IsShown();
    layerStack.top().ShowViews(false);
    layerStack.top().focused.top()->SetFocus(false);
    if (helpViewShown) {
      // drop layer
      layerStack.pop();
    } else {
      // push layer
      Layer layer;
      layer.mask = View::MaskHelp;
      layer.focused.push(&helpView);
      layer.views.insert(&helpView);
      layerStack.push(std::move(layer));
    }
    layerStack.top().focused.top()->SetFocus(true);
    UpdateTopLayout();
  }

  // on same layer
  void SwitchFocus() {
    if (!explorerView.IsShown()) {
      return;
    }

    auto& focused = layerStack.top().focused;
    focused.top()->SetFocus(false);
    if (focused.top() == &explorerView) {
      focused.top() = playlistView + iPlaylist;
    } else {
      focused.top() = &explorerView;
    }
    focused.top()->SetFocus(true);

    // NOTE: avoid refresh status view
    // layer.RefreshViews();
    explorerView.Refresh();
    playlistView[iPlaylist].Refresh();
  }

  void SwitchPlaylist(int n) {
    if (n == iPlaylist) {
      return;
    }
    int oldn = iPlaylist;
    iPlaylist = n;

    playlistView[oldn].Show(false);

    Layer& layer = layerStack.top();
    layer.views.erase(playlistView + oldn);
    layer.views.insert(playlistView + n);
    if (playlistView[oldn].HasFocus()) {
      playlistView[oldn].SetFocus(false);
      layer.focused.top() = playlistView + n;
      layer.focused.top()->SetFocus(true);
    }

    UpdateTopLayout();

    // tell server
    ClientPlaylistHandler& handle = client.PlaylistHandler();
    handle.Switch(n);
  }

 private:
  Client client;

  ExplorerView explorerView;
  PlaylistView playlistView[PLAYLIST_COUNT];
  HelpView helpView;
  StatusView statusView;

  int iPlaylist = 1;
  stack<Layer> layerStack;

  mutex needSwitchPlaylistMutex;
  int needSwitchPlaylist = 0;
  int switchPlaylistTo = -1;
};

MainUi::MainUi()
    : impl(make_unique<Impl>()) {
}

MainUi::~MainUi() {
}

int MainUi::Exec() {
  return impl->Exec();
}
