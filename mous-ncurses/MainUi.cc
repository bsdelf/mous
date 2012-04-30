#include "MainUi.h"

#include <ncurses.h>

#include <iostream>

#include <scx/Conv.hpp>
#include <scx/ConfigFile.hpp>

#include "Config.h"
#include "Client.h"

MainUi::MainUi()
{
    m_Client = new Client();
}

MainUi::~MainUi()
{
    delete m_Client;
}

int MainUi::Exec()
{
    string serverIp;
    int serverPort = -1;
    {
        ConfigFile conf;
        if (!conf.Load(Config::ConfigPath))
            return 1;

        serverIp = conf[Config::ServerIp];
        serverPort = StrToNum<int>(conf[Config::ServerPort]);
    }

    m_Client->Run(serverIp, serverPort);

    char ch;
    std::cin >> ch;
    if (ch == 'q')
        m_Client->StopService();

    //pause();

    /*
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    WINDOW* wnd = newwin(3, 10, (LINES-3)/2, (COLS-10)/2);
    box(wnd, 0, 0);
    wrefresh(wnd);

    getch();
    endwin();
    */

    m_Client->Stop();

    return 0;
}
