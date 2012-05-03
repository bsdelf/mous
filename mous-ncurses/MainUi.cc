#include "MainUi.h"

//#include <ncurses.h>

#include <iostream>
#include <map>
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

namespace ViewType {
enum e
{
    Explorer = 0,
    Playlist,
    Help,
    Status,

    Count
};
}
typedef ViewType::e EmViewType;

const int PLAYLIST_COUNT = 6;

struct PrivateMainUi
{
    Client client;

    BgWindow bgWindow;

    IView* focusedView;
    PlaylistView* currentPlaylist;

    ExplorerView explorerView;
    PlaylistView playlistView[PLAYLIST_COUNT];
    HelpView helpView;
    StatusView statusView;

    PrivateMainUi():
        focusedView(playlistView),
        currentPlaylist(playlistView)
    {
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
}

void MainUi::ShowOrHideExplorer()
{
}

void MainUi::ShowOrHideHelp()
{
}

void MainUi::SwitchFocus()
{
}

void MainUi::SwitchPlaylist(int n)
{
    PlaylistView* v = d->playlistView;
    PlaylistView*& current = d->currentPlaylist;
    if (v+n != current) {
        current = v+n;
        for (int i = 0; i < PLAYLIST_COUNT; ++i) {
            v[i].Show(v+i == current);
        }
    }
}

