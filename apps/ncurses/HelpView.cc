#include "HelpView.h"

#include <algorithm>

static const char* const STR_ARRAY[] = 
{
    "",

    "^b==== Shortcut ====",
    "",

    "^bGlobal Keys:",
    "H          show/hide help",
    "e          show/hide file explorer",
    "q          quit",
    "x          quit and stop server",
//"0      switch to temporary playlist",
    "0-5        switch playlist",
    "",
    "Space      pause/resume",
    "m          next play mode",
    "p          play previous one",
    "n          play next one",
    "<          -3 seconds",
    ">          +3 seconds",
    "-/_        volume up",
    "+/=        volume down",
    "",
    "TAB        switch focus between file explorer/playlist",
    "",

    "^bIn File Explorer:",
    "k          scroll up",
    "j          scroll down",
    "h          to parent dir",
    "l/Enter    enter dir",
    "PgUp       page up",
    "PgDn       page down",
    "Home       first line",
    "End        last line",
    "",
//"ENTER  open item in temporary playlist",
    "a          add file to playlist",
    "r          refresh",
    "s          show/hide unsupported files",
    ".          show/hide files whose names begin with a dot",
    "",

    "^bIn Playlist:",
    "k          scroll up",
    "j          scroll down",
    "h          show previous playlist",
    "l          show next playlist",
    "PgUp       page up",
    "PgDn       page down",
    "Home       first line",
    "End        last line",
    "",
//    "c      cut item and put it into queue",
//    "y      copy item and put it into queue",
//    "p      pop and paste the first item in queue to next line",
//    "e      clear queue",
    "ENTER      play item",
    "d          remove item",
    "C          clear playlist",
    "K          move item up",
    "J          move item down",
    "",

    "^bIn Help:",
    "k          scroll up",
    "j          scroll down",
    "",

    "^b==== About ====",
    "Since:     2012.02.11",
    "License:   New BSD License",
    "Author:    Yanhui Shen <shen.elf@gmail.com>",
    "",

    "^b==== Thanks ====",
    "fataltrap",
    "gehaowu",
    "lichray",
    "XiaoQing Yu",
    ""
};

HelpView::HelpView():
    m_LineBegin(0),
    m_LineCount(sizeof(STR_ARRAY)/sizeof(STR_ARRAY[0]))
{
}

HelpView::~HelpView()
{
}

void HelpView::Refresh()
{
    using namespace ncurses;

    d.ColorOn(ncurses::color::kWhite, ncurses::color::kBlack);
    d.Clear();

    auto titleText = Text("[ Help ]").SetAttributes(attribute::kBold);
    d.Draw(ncurses::HorizontalAlignment::kCenter, 0, titleText);

    const int lcount = std::min(d.h - 2, m_LineCount - m_LineBegin);
    for (int l = 0; l < lcount; ++l) {
        const int index = m_LineBegin + l;
        d.ColorOn(ncurses::color::kWhite, ncurses::color::kBlack);
        d.Draw(8, l + 1, STR_ARRAY[index]);
    }

    d.Refresh();
}

void HelpView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void HelpView::Resize(int w, int h)
{
    d.Resize(w, h);
    d.EnableKeypad(true);
}

bool HelpView::InjectKey(int key)
{
    switch (key) {
        case 'j': {
            if (m_LineBegin < m_LineCount - (d.h - 2)) {
                ++m_LineBegin;
            }
            break;
        }
        case 'k': {
            if (m_LineBegin > 0) {
                --m_LineBegin;
            }
            break;
        }
        default: {
            return false;
        }
    }
    Refresh();
    return true;
}

void HelpView::Show(bool visible)
{
    d.SetVisible(visible);
}

bool HelpView::IsShown() const
{
    return d.IsVisible();
}
