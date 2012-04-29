#include "MainUi.h"
#include <ncurses.h>
#include <iostream>

int MainUi::Exec()
{
    int ch;
    std::cin >> ch;
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

    return 0;
}
