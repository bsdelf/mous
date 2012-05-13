#include "PlaylistView.h"

#include <stdio.h>
#include <sstream>
#include <algorithm>

#include <scx/CharsetHelper.hpp>
using namespace scx;

const string STR_TITLE = "Playlist";

PlaylistView::PlaylistView():
    m_Focused(false),
    m_Index(-1),
    m_ItemBegin(0),
    m_ItemSelected(0),
    m_Title(STR_TITLE),
    m_WaitReply(false),
    m_PlaylistHandler(NULL)
{
}

PlaylistView::~PlaylistView()
{
}

void PlaylistView::Refresh()
{
    using namespace CharsetHelper;
    using namespace ncurses;

    d.Clear();

    // title
    if (m_Focused)
        d.OpenStyle("b");
    d.CenterPrint(0, m_Title);
    if (m_Focused)
        d.CloseStyle();

    // content
    // { {title artist album~~00:00 }#}
    const int w = d.w - 2;
    const int h = d.h - 2;
    const int x = 1;
    const int y = 1;
    int xoff = x;
    int yoff = y;

    const int wText = w - 2;
    const int hText = h - 1;

    const int wTime = 5 + 1;
    const int wField = (wText - wTime) / 3;
    const int wLastField = (wText - wTime) - wField * 2;

    if (!m_List.empty()) {
            int lcount = std::min(hText, (int)m_List.size()-m_ItemBegin);
        for (int l = 0; l < lcount; ++l) {
            int index = m_ItemBegin + l;
            MediaItem& item = *m_List[index];

            int fieldAttr = Attr::Bold;
            int fieldColorF = Color::Green;
            int fieldColorB = Color::Black;
            int timeAttr = Attr::Bold;
            int timeColorF = Color::Magenta;
            int timeColorB = Color::Black;

            if (index == m_ItemSelected) {
                fieldAttr = timeAttr = Attr::Normal;
                fieldColorF = timeColorF = Color::Black;
                fieldColorB = timeColorB = Color::White;

                d.AttrSet(Attr::Normal | Attr::Reverse);
                d.ColorOn(Color::White, Color::Black);
                d.Print(x, yoff+l, string(w-1, ' '));
            }

            xoff = x + 1;
            d.AttrSet(fieldAttr);
            d.ColorOn(fieldColorF, fieldColorB);

            const string& field1 = MBStrWidth(item.tag.title) <= wField-1 ?
                item.tag.title : MBWidthStr(item.tag.title, wField-1-3) + "...";
            d.Print(xoff, yoff+l, field1);
            xoff += wField;

            const string& field2 = MBStrWidth(item.tag.artist) <= wField-1 ?
                item.tag.artist : MBWidthStr(item.tag.artist, wField-1-3) + "...";
            d.Print(xoff, yoff+l, field2);
            xoff += wField;

            const string& field3 = MBStrWidth(item.tag.album) <= wLastField-1 ?
                item.tag.album : MBWidthStr(item.tag.album, wLastField-1-3) + "...";
            d.Print(xoff, yoff+l, field3);
            xoff += wLastField;

            // I suppose duration < 59min
            string duration;
            {
                if (item.hasRange && item.msEnd == -1)
                    item.msEnd = item.duration;

                int total = (item.hasRange ? item.msEnd - item.msBeg : item.duration) / 1000;
                int min = total / 60;
                int sec = total % 60;
                char buf[6];
                snprintf(buf, sizeof(buf), "%.2d:%.2d", min, sec);
                duration.assign(buf, 5);
            }
            d.ColorOn(timeColorF, timeColorB);
            d.Print(xoff, yoff+l, duration);
            xoff += wTime;
        }

        xoff = x + 1 + wText;
        if ((int)m_List.size() > hText) {
            double percent = (double)(m_ItemSelected+1) / m_List.size() - 0.00001f;
            yoff = y + hText*percent;
            d.AttrSet(Attr::Bold | Attr::Reverse);
            d.ColorOn(Color::Green, Color::Black);
            d.Print(xoff, yoff, " ");
        }
    }

    // status bar
    xoff = x + 1;
    yoff = y + hText;
    stringstream status;
    if (!m_List.empty())
        status << (m_ItemSelected+1) << "/" << m_List.size();
    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::White, Color::Black);
    d.Print(xoff, yoff, status.str());

    d.ResetAttrColor();

    d.Refresh();
}

void PlaylistView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void PlaylistView::Resize(int w, int h)
{
    d.Resize(w, h);
    d.EnableKeypad(true);
}

bool PlaylistView::InjectKey(int key)
{
    const bool empty = m_List.empty();

    switch (key) {
        case KEY_LEFT:
        case 'h':
            SigSwitchPlaylist(false);
            return true;

        case KEY_RIGHT:
        case 'l':
            SigSwitchPlaylist(true);
            return true;

        case KEY_DOWN:
        case 'j':
            if (!empty) {
                ScrollDown();
            }
            break;

        case KEY_UP:
        case 'k':
            if (!empty) {
                ScrollUp();
            }
            break;

        case KEY_NPAGE:
            if (!empty) {
                int line = (d.h - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollDown();
                }
            }
            break;

        case KEY_PPAGE:
            if (!empty) {
                int line = (d.h - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollUp();
                }
            }
            break;

        case KEY_HOME:
            if (!empty) {
                m_ItemBegin = 0;
                m_ItemSelected = 0;
            }
            break;

        case KEY_END:
            if (!empty) {
                m_ItemSelected = m_List.size() - 1;
                m_ItemBegin = std::max((int)m_List.size() - (d.h - 3), 0);
            }
            break;

        case 'd':
            if (!empty) {
                Remove(m_ItemSelected);
            }
            return true;

        case 'c':
            break;

        case 'y':
            break;

        case 'p':
            break;

        case 'e':
            break;

        case 'C':
            break;

        case '\n':
            break;

        case '/':
            break;

        default:
            return false;
    }
    Refresh();
    return true;
}

void PlaylistView::Show(bool show)
{
    d.Show(show);
}

bool PlaylistView::IsShown() const
{
    return d.shown;
}

void PlaylistView::SetFocus(bool focus)
{
    m_Focused = focus;
}

bool PlaylistView::HasFocus() const
{
    return m_Focused;
}

int PlaylistView::Index() const
{
    return m_Index;
}

void PlaylistView::SetIndex(int index)
{
    m_Index = index;

    stringstream str;
    str << "[ " << STR_TITLE << " " << index << " ]";
    m_Title = str.str();
}

void PlaylistView::SetPlaylistHandle(ClientPlaylistHandler* handler)
{
    if (m_PlaylistHandler != NULL) {
        m_PlaylistHandler->SigAppend().DisconnectReceiver(this);
        m_PlaylistHandler->SigRemove().DisconnectReceiver(this);
    }

    if (handler != NULL) {
        handler->SigAppend().Connect(&PlaylistView::SlotAppend, this);
        handler->SigRemove().Connect(&PlaylistView::SlotRemove, this);
    }

    m_PlaylistHandler = handler;
}

void PlaylistView::ScrollUp()
{
    if (m_ItemSelected > 0) {
        --m_ItemSelected;
    }
    if (m_ItemSelected < m_ItemBegin + (d.h-2) / 2
            && m_ItemBegin > 0) {
        --m_ItemBegin;
    }
}

void PlaylistView::ScrollDown()
{
    if (m_ItemSelected < (int)m_List.size()-1) {
        ++m_ItemSelected;
    }
    if (m_ItemSelected > (d.h-2) / 2
            && m_ItemBegin < (int)m_List.size()-(d.h-2-1)) {
        ++m_ItemBegin;
    }
}

void PlaylistView::Remove(int pos)
{
    if (m_WaitReply)
        return;

    if (m_PlaylistHandler != NULL) {
        m_WaitReply = true;
        m_PlaylistHandler->Remove(m_Index, pos);
    }
}

void PlaylistView::SlotAppend(int i, deque<MediaItem*>& list)
{
    if (i != m_Index)
        return;

    m_List.insert(m_List.end(), list.begin(), list.end());

    Refresh();
}

void PlaylistView::SlotRemove(int i, int pos)
{
    if (i != m_Index)
        return;

    if (pos >= 0 && pos < m_List.size()) {
        delete m_List[pos];
        m_List.erase(m_List.begin()+pos);

        if (!m_List.empty() && m_ItemSelected >= m_List.size()-1)
            m_ItemSelected = m_List.size() - 1;

        Refresh();
    }

    m_WaitReply = false;
}
