#include "MainUi.h"

#include <locale.h>
//#include <ncurses.h>

#include <iostream>
#include <set>
#include <stack>

#include <scx/Conv.hpp>
#include <scx/ConfigFile.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

#include "Config.h"
#include "Client.h"
#include "BgWindow.h"
#include "IView.h"
#include "ExplorerView.h"
#include "PlaylistView.h"
#include "StatusView.h"
#include "HelpView.h"

namespace View {
enum Type
{
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

}
typedef View::Type EmViewType;

const int PLAYLIST_COUNT = 6;

struct LayerInfo
{
    View::mask_t mask;
    set<IView*> views;
    stack<IView*> focused;

    typedef set<IView*> ShownSet;
    typedef set<IView*>::iterator ShownSetIter;

    void RefreshViews()
    {
        ShownSetIter iter = views.begin();
        ShownSetIter end = views.end();
        for (; iter != end; ++iter) {
            (*iter)->Refresh();
        }
    }

    void ShowViews(bool show)
    {
        ShownSetIter iter = views.begin();
        ShownSetIter end = views.end();
        for (; iter != end; ++iter) {
            (*iter)->Show(show);
        }
    }
};

struct PrivateMainUi
{
    MainUi* parent;
    Client client;

    BgWindow bgWindow;

    ExplorerView explorerView;
    PlaylistView playlistView[PLAYLIST_COUNT];
    HelpView helpView;
    StatusView statusView;

    int iPlaylist;
    stack<LayerInfo> layerStack;

    PrivateMainUi(MainUi* p):
        parent(p),
        iPlaylist(1)
    {
        PlaylistView& playlist = playlistView[iPlaylist];

        LayerInfo layer;
        layer.mask = View::MaskPlaylist | View::MaskStatus;
        layer.views.insert(&playlist);
        layer.views.insert(&statusView);
        layer.focused.push(&playlist);
        layerStack.push(layer);

        explorerView.SigTmpOpen.Connect(&MainUi::SlotTmpOpen, parent);
        explorerView.SigUserOpen.Connect(&MainUi::SlotReqUserOpen, parent);

        for (int i = 0; i < PLAYLIST_COUNT; ++i) {
            playlistView[i].SetIndex(i);
            playlistView[i].SigSwitchPlaylist.Connect(&MainUi::SlotSwitchPlaylist, parent);
        }
        playlist.SetFocus(true);

        client.PlaylistHandler().SigAppend().Connect(&MainUi::SlotRetUserOpen, parent);
    }
};

MainUi::MainUi()
{
    d = new PrivateMainUi(this);
}

MainUi::~MainUi()
{
    delete d;
}

int MainUi::Exec()
{
    if (!StartClient())
        return 1;
    BeginNcurses();

    OnResize();

    for (bool quit = false; !quit; quit = false) {
        int key = d->bgWindow.Input();
        if (HandleTopKey(key, quit)) {
            if (quit)
                break;
            else
                continue;
        } else if (d->statusView.InjectKey(key)) {
            continue;
        } else  {
            d->layerStack.top().focused.top()->InjectKey(key);
        }
    }

    EndNcurses();
    StopClient();

    return 0;
}


void MainUi::SlotSwitchPlaylist(bool toNext)
{
    int n = d->iPlaylist + (toNext ? 1 : -1);
    n = std::min(std::max(n, 0), PLAYLIST_COUNT-1);
    SwitchPlaylist(n);
}

void MainUi::SlotTmpOpen(const string& path)
{
}

void MainUi::SlotReqUserOpen(const string& path)
{
    d->client.PlaylistHandler().Append(d->iPlaylist, path);
}

void MainUi::SlotRetUserOpen(int i, deque<MediaItem*>& list)
{
    d->playlistView[i].Append(list);
}

bool MainUi::StartClient()
{
    string serverIp;
    int serverPort = -1;

    ConfigFile conf;
    if (!conf.Load(Config::ConfigPath))
        return false;

    serverIp = conf[Config::ServerIp];
    serverPort = StrToNum<int>(conf[Config::ServerPort]);

    return d->client.Run(serverIp, serverPort);
}

void MainUi::StopClient()
{
    d->client.Stop();
}

void MainUi::BeginNcurses()
{  
    setlocale(LC_ALL,"");
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    refresh();
}

void MainUi::EndNcurses()
{
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
bool MainUi::HandleTopKey(int key, bool& quit)
{
    switch (key) {
        case KEY_RESIZE:
            OnResize();
            break;

        case 'E':
            if (d->layerStack.size() == 1)
                ShowOrHideExplorer();
            break;

        case 'H':
            ShowOrHideHelp();
            break;

        case '\t':
            if (d->layerStack.size() == 1)
                SwitchFocus();
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
                SwitchPlaylist(StrToNum<int>(string(1, (char)key)));
            break;

        case 'Q':
            quit = true;
            break;

        case 'X':
            quit = true;
            d->client.StopService();
            break;

        default:
            return false;
    }
    return true;
}

void MainUi::OnResize()
{
    d->bgWindow.OnResize();

    d->layerStack.top().ShowViews(false);
    UpdateTopLayout();
}

/* update the layout of top layer */
void MainUi::UpdateTopLayout()
{
    const int w = d->bgWindow.Width();
    const int h = d->bgWindow.Height();

    d->layerStack.top().ShowViews(true);

    View::mask_t mask = d->layerStack.top().mask;
    switch (mask) {
        case View::MaskHelp:
        {
            d->helpView.MoveTo(0, 0);
            d->helpView.Resize(w, h);
        }
            break;

        case View::MaskPlaylist | View::MaskStatus:
        {
            int x = 0, y = 0;
            int hStatus = d->statusView.MinHeight();
            int hPlaylist = h - hStatus;

            PlaylistView& playlist = d->playlistView[d->iPlaylist];
            playlist.MoveTo(x, y);
            playlist.Resize(w, hPlaylist);
            y += hPlaylist;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hStatus);
        }
            break;

        case View::MaskExplorer | View::MaskPlaylist | View::MaskStatus:
        {
            int wExplorer = w/2;
            int wPlaylist = w - wExplorer;
            int hStatus = d->statusView.MinHeight();
            int hExplorer = h - hStatus;
            int x = 0, y = 0;

            d->explorerView.MoveTo(x, y);
            d->explorerView.Resize(wExplorer, hExplorer);
            x += wExplorer;

            PlaylistView& playlist = d->playlistView[d->iPlaylist];
            playlist.MoveTo(x, y);
            playlist.Resize(wPlaylist, hExplorer);
            x = 0; y += hExplorer;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hStatus);
        }
            break;

        default:
            break;
    }

    d->layerStack.top().RefreshViews();
}

/* on same layer */
void MainUi::ShowOrHideExplorer()
{
    LayerInfo& layer = d->layerStack.top();
    layer.focused.top()->SetFocus(false);
    if (d->explorerView.IsShown()) {
        // remove view
        d->explorerView.Show(false);
        layer.mask &= ~View::MaskExplorer;
        layer.views.erase(&d->explorerView);
        layer.focused.pop();
        layer.focused.top() = d->playlistView + d->iPlaylist;
        layer.focused.top()->SetFocus(true);
    } else {
        // add view
        layer.mask |= View::MaskExplorer;
        layer.views.insert(&d->explorerView);
        layer.focused.push(&d->explorerView);
    }
    layer.focused.top()->SetFocus(true);
    UpdateTopLayout();
}

/* between different layers */
void MainUi::ShowOrHideHelp()
{
    bool helpViewShown = d->helpView.IsShown();
    d->layerStack.top().ShowViews(false);
    d->layerStack.top().focused.top()->SetFocus(false);
    if (helpViewShown) {
        // drop layer
        d->layerStack.pop();
    } else {
        // push layer
        LayerInfo layer;
        layer.mask = View::MaskHelp;
        layer.focused.push(&d->helpView);
        layer.views.insert(&d->helpView);
        d->layerStack.push(layer);
    }
    d->layerStack.top().focused.top()->SetFocus(true);
    UpdateTopLayout();
}

// on same layer
void MainUi::SwitchFocus()
{
    if (!d->explorerView.IsShown())
        return;

    LayerInfo& layer = d->layerStack.top();
    IView* focused = layer.focused.top();
    layer.focused.top()->SetFocus(false);
    layer.focused.top() = 
        (focused == (IView*)&d->explorerView) ? 
        (IView*)(d->playlistView+d->iPlaylist) : 
        (IView*)&d->explorerView;
    layer.focused.top()->SetFocus(true);
    layer.RefreshViews();
}

void MainUi::SwitchPlaylist(int n)
{
    if (n == d->iPlaylist)
        return;
    int oldn = d->iPlaylist;
    d->iPlaylist = n;

    d->playlistView[oldn].Show(false);

    LayerInfo& layer = d->layerStack.top();
    layer.views.erase(d->playlistView+oldn);
    layer.views.insert(d->playlistView+n);
    if (d->playlistView[oldn].HasFocus()) {
        d->playlistView[oldn].SetFocus(false);
        layer.focused.top() = d->playlistView + n;
        layer.focused.top()->SetFocus(true);
    }

    UpdateTopLayout();
}

