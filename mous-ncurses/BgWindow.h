#ifndef BGWINDOW_H
#define BGWINDOW_H

#include <ncurses.h>

class BgWindow
{
public:
    BgWindow():
        m_Wnd(NULL)
    {
    }

    ~BgWindow()
    {
        Cleanup();
    }

    void OnResize()
    {
        Cleanup();
        m_Wnd = newwin(LINES, COLS, 0, 0);
        box(m_Wnd, 0, 0);
        wrefresh(m_Wnd);
    }

    int GetWidth() const
    {
        return COLS;
    }

    int GetHeight() const
    {
        return LINES;
    }

    int GetInput() const
    {
        wmove(m_Wnd, 0, 0);
        curs_set(0);
        return wgetch(m_Wnd);
    }

private:
    void Cleanup()
    {
        if (m_Wnd != NULL)
            delwin(m_Wnd);
    }

private:
    WINDOW* m_Wnd;
};

#endif
