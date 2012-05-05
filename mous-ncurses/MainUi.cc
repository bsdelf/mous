#include "MainUi.h"

//#include <ncurses.h>

#include <iostream>
#include <set>
#include <stack>
#include <string>
using namespace std;

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
    set<IView*> shown;
    stack<IView*> focused;
};
typedef set<IView*> ShownSet;
typedef set<IView*>::iterator ShownSetIter;

struct PrivateMainUi
{
    Client client;

    BgWindow bgWindow;

    ExplorerView explorerView;
    PlaylistView playlistView[PLAYLIST_COUNT];
    HelpView helpView;
    StatusView statusView;

    int playlistIndex;
    stack<LayerInfo> layerStack;

    PrivateMainUi():
        playlistIndex(1)
    {
        LayerInfo layer;
        layer.mask = View::MaskPlaylist | View::MaskStatus;
        layer.focused.push(playlistView + playlistIndex);
        layer.shown.insert(layer.focused.top());
        layer.shown.insert(&statusView);
        layerStack.push(layer);
    }
};

MainUi::MainUi()
{
    d = new PrivateMainUi;
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
        int key = d->bgWindow.GetInput();
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
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    refresh();
}

void MainUi::EndNcurses()
{
    endwin();
}

bool MainUi::HandleTopKey(int key, bool& quit)
{
    switch (key) {
        case KEY_RESIZE:
            OnResize();
            break;

        case 'E':
            ShowOrHideExplorer();
            break;

        case 'H':
            ShowOrHideHelp();
            break;

        case '\t':
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

        case 'q':
            quit = true;
            break;

        case 'Q':
            quit = true;
            break;

        default:
            return false;
    }
    return true;
}

void MainUi::OnResize()
{
    d->bgWindow.OnResize();

    UpdateLayout();
}

void MainUi::UpdateLayout()
{
    const int w = d->bgWindow.GetWidth();
    const int h = d->bgWindow.GetHeight();

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
            int hStatus = d->statusView.GetMinHeight();
            int hPlaylist = h - hStatus;

            for (int i = 0; i < PLAYLIST_COUNT; ++i) {
                d->playlistView[i].MoveTo(x, y);
                d->playlistView[i].Resize(w, hPlaylist);
            }
            d->playlistView[d->playlistIndex].Show(true);
            d->playlistView[d->playlistIndex].Refresh();
            y += hPlaylist;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hStatus);
        }
            break;

        case View::MaskExplorer | View::MaskPlaylist | View::MaskStatus:
        {
            int wExplorer = w/2;
            int wPlaylist = w - wExplorer;
            int hStatus = d->statusView.GetMinHeight();
            int hExplorer = h - hStatus;
            int x = 0, y = 0;

            d->explorerView.MoveTo(x, y);
            d->explorerView.Resize(wExplorer, hExplorer);
            x += wExplorer;

            for (int i = 0; i < PLAYLIST_COUNT; ++i) {
                d->playlistView[i].MoveTo(x, y);
                d->playlistView[i].Resize(wPlaylist, hExplorer);
            }
            d->playlistView[d->playlistIndex].Show(true);
            d->playlistView[d->playlistIndex].Refresh();
            x = 0; y += hExplorer;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hStatus);
        }
            break;

        default:
            break;
    }

    ShownSet& shown = d->layerStack.top().shown;
    ShownSetIter iter = shown.begin();
    ShownSetIter end = shown.end();
    for (; iter != end; ++iter) {
        (*iter)->Show(true);
        (*iter)->Refresh();
    }
}

// on same layer
void MainUi::ShowOrHideExplorer()
{
    LayerInfo& layer = d->layerStack.top();
    if (d->explorerView.IsShown()) {
        d->explorerView.Show(false);
        // update layer info
        layer.mask ^= View::MaskExplorer;
        layer.shown.erase(&d->explorerView);
        layer.focused.pop();
    } else {
        layer.mask |= View::MaskExplorer;
        layer.shown.insert(&d->explorerView);
        layer.focused.push(&d->explorerView);
    }
    UpdateLayout();
}

// between different layers
void MainUi::ShowOrHideHelp()
{
    if (d->helpView.IsShown()) {
        d->helpView.Show(false);
        // only need to drop
        d->layerStack.pop();
    } else {
        LayerInfo layer;
        layer.mask = View::MaskHelp;
        layer.focused.push(&d->helpView);
        layer.shown.insert(&d->helpView);
        d->layerStack.push(layer);
    }
    UpdateLayout();
}

// on same layer
void MainUi::SwitchFocus()
{
}

void MainUi::SwitchPlaylist(int n)
{
    PlaylistView* v = d->playlistView;
    if (n == d->playlistIndex)
        return;
    d->playlistIndex = n;
    for (int i = 0; i < PLAYLIST_COUNT; ++i) {
        v[i].Show(i == n);
    }
}

