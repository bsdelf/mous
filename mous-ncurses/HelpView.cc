#include "HelpView.h"
using namespace ViewHelper;

#include <algorithm>

const char* const STR_ARRAY[] = 
{
    "",

    "^b==== Shortcut ====",
    "",

    "^bGlobal Window:",
    "H      show/hide help",
    "E      show/hide file explorer",
    "q      quit",
    "Q      quit and stop server",
    "0      switch to temporary playlist",
    "1-5    switch to user playlist",
    "",
    "SPACE  pause/resume",
    "P      play previous one",
    "N      play next one",
    "",
    "TAB    switch focus between file explorer/playlist",
    "",

    "^bFile Explorer View:",
    "k      scroll up",
    "j      scroll down",
    "h      to parent dir",
    "l      enter dir",
    "ENTER  open item in temporary playlist",
    "a      add item to current playlist",
    "",

    "^bPlaylist View:",
    "k      scroll up",
    "j      scroll down",
    "h      show previous playlist",
    "l      show next playlist",
    "d      remove item",
    "c      cut item and put it into queue",
    "y      copy item and put it into queue",
    "p      paste the first item of queue to next line",
    "C      clear queue",
    "",

    "^bHelp View:",
    "k      scroll up",
    "j      scroll down",
    "",

    "^b==== About ====",
    "Since:     2012.02.11",
    "License:   New BSD License",
    "Author:    Yanhui Shen <shen.elf@gmail.com>",
    ""
};

HelpView::HelpView():
    m_Wnd(NULL),
    m_Panel(NULL),
    m_Width(0),
    m_Height(0),
    m_LineBegin(0),
    m_LineCount(sizeof(STR_ARRAY)/sizeof(STR_ARRAY[0]))
{
}

HelpView::~HelpView()
{
    Cleanup();
}

void HelpView::OnResize(int x, int y, int w, int h)
{
    Cleanup();

    m_Wnd = newwin(h, w, y, x);
    m_Panel = new_panel(m_Wnd);
    box(m_Wnd, 0, 0);

    m_Width = w;
    m_Height = h;
}

void HelpView::Refresh()
{
    werase(m_Wnd);
    box(m_Wnd, 0, 0);

    CenterPrint(m_Wnd, 0, m_Width, "^b[Help]");

    for (int l = 0; l < m_Height-2 && l < m_LineCount - m_LineBegin; ++l) {
        int index = m_LineBegin+l;
        mvwprintw(m_Wnd, l+1, 8, STR_ARRAY[index]);
    }

    wrefresh(m_Wnd);
}

void HelpView::MoveTo(int x, int y)
{
    mvwin(m_Wnd, y, x);
}

void HelpView::Resize(int w, int h)
{
    wresize(m_Wnd, h, w);
}

bool HelpView::InjectKey(int key)
{
    switch (key) {
        case 'j':
            if (m_LineBegin < m_LineCount-(m_Height-2))
                ++m_LineBegin;
            break;

        case 'k':
            if (m_LineBegin > 0)
                --m_LineBegin;
            break;

        default:
            return false;
    }
    Refresh();
    return true;
}

void HelpView::Show(bool show)
{
    if (show)
        show_panel(m_Panel);
    else
        hide_panel(m_Panel);
    update_panels();
    doupdate();

    m_Shown = show;
}

bool HelpView::IsShown() const
{
    return m_Shown;
}

void HelpView::Cleanup()
{
    if (m_Panel != NULL) {
        del_panel(m_Panel);
        m_Panel = NULL;
    }

    if (m_Wnd != NULL) {
        delwin(m_Wnd);
        m_Wnd = NULL;
    }
}
