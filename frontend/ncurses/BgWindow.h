#ifndef BGWINDOW_H
#define BGWINDOW_H

#include <ncurses.h>

class BgWindow
{
public:
    BgWindow():
        m_Wnd(nullptr)
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
        keypad(m_Wnd, TRUE);
        wrefresh(m_Wnd);
    }

    int Width() const
    {
        return COLS;
    }

    int Height() const
    {
        return LINES;
    }

    int Input() const
    {
        wmove(m_Wnd, 0, 0);
        curs_set(0);
        return wgetch(m_Wnd);
    }

private:
    void Cleanup()
    {
        if (m_Wnd != nullptr) {
            delwin(m_Wnd);
            m_Wnd = nullptr;
        }
    }

private:
    WINDOW* m_Wnd;
};

#endif
