#include "MainUi.h"

#include <ncurses.h>

#include <iostream>
#include <map>
#include <string>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/ConfigFile.hpp>
using namespace scx;

#include <util/MediaItem.h>
#include <util/Playlist.h>
using namespace mous;

#include "Config.h"
#include "Client.h"

struct PrivateMainUi
{
    Client client;

    typedef Playlist<MediaItem*> playlist_t;
    typedef map<string, playlist_t*>::iterator PlaylistMapIter;
    typedef map<string, playlist_t*>::const_iterator PlaylistMapConstIter;
    map<string, playlist_t*> playlistMap;

    WINDOW* wnd;
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

    d->wnd = newwin(3, 10, (LINES-3)/2, (COLS-10)/2);
    box(d->wnd, 0, 0);
    wborder(d->wnd, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(d->wnd);
    getch();

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
}

void MainUi::EndNcurses()
{
    endwin();
}
