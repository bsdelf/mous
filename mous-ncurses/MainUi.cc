#include "MainUi.h"

//#include <ncurses.h>

#include <iostream>
#include <vector>
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

struct PrivateMainUi
{
    Client client;

    BgWindow bgWindow;

    IView* focusedView;
    int playlistIndex;

    ExplorerView explorerView;
    PlaylistView playlistView[PLAYLIST_COUNT];
    HelpView helpView;
    StatusView statusView;

    stack<View::mask_t> viewStack;
    vector<IView*> viewGroup;

    PrivateMainUi():
        focusedView(playlistView+1),
        playlistIndex(1)
    {
        viewStack.push(View::MaskPlaylist | View::MaskStatus);

        viewGroup.push_back(&explorerView);
        viewGroup.push_back(&helpView);
        viewGroup.push_back(&statusView);
        for (int i = 0; i < PLAYLIST_COUNT; ++i)
            viewGroup.push_back(playlistView+i);
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
            d->focusedView->InjectKey(key);
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

    const int w = d->bgWindow.GetWidth();
    const int h = d->bgWindow.GetHeight();

    for (size_t i = 0; i < d->viewGroup.size(); ++i)
        d->viewGroup[i]->OnResize(0, 0, w, h);

    RefreshViews();
}

void MainUi::RefreshViews()
{
    for (size_t i = 0; i < d->viewGroup.size(); ++i)
        d->viewGroup[i]->Show(false);

    const int w = d->bgWindow.GetWidth();
    const int h = d->bgWindow.GetHeight();

    View::mask_t mask = d->viewStack.top();
    switch (mask) {
        case View::MaskHelp:
        {
            d->helpView.Resize(w, h);
            d->helpView.Refresh();
            d->helpView.Show(true);
        }
            break;

        case View::MaskPlaylist | View::MaskStatus:
        {
            int x = 0, y = 0;
            int hsta = d->statusView.GetMinHeight();
            int hpla = h - hsta;

            for (int i = 0; i < PLAYLIST_COUNT; ++i) {
                d->playlistView[i].MoveTo(x, y);
                d->playlistView[i].Resize(w, hpla);
                d->playlistView[i].Refresh();
            }
            d->playlistView[d->playlistIndex].Show(true);
            y += hpla;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hsta);
            d->statusView.Refresh();
            d->statusView.Show(true);
        }
            break;

        case View::MaskExplorer | View::MaskPlaylist | View::MaskStatus:
        {
            int wexp = w/2;
            int wpla = w - wexp;
            int hsta = d->statusView.GetMinHeight();
            int hexpla = h - hsta;
            int x = 0, y = 0;

            d->explorerView.MoveTo(x, y);
            d->explorerView.Resize(wexp, hexpla);
            d->explorerView.Refresh();
            d->explorerView.Show(true);
            x += wexp;

            for (int i = 0; i < PLAYLIST_COUNT; ++i) {
                d->playlistView[i].MoveTo(x, y);
                d->playlistView[i].Resize(wpla, hexpla);
                d->playlistView[i].Refresh();
            }
            d->playlistView[d->playlistIndex].Show(true);
            x = 0; y += hexpla;

            d->statusView.MoveTo(x, y);
            d->statusView.Resize(w, hsta);
            d->statusView.Refresh();
            d->statusView.Show(true);
        }
            break;

        default:
            break;
    }
}

void MainUi::ShowOrHideExplorer()
{
    RefreshViews();
}

void MainUi::ShowOrHideHelp()
{
    if (d->helpView.IsShown()) {
        d->viewStack.pop();
    } else {
        d->viewStack.push(View::MaskHelp);
        d->focusedView = &d->helpView;
    }
    RefreshViews();
}

void MainUi::SwitchFocus()
{
    RefreshViews();
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

