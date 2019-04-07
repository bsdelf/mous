#include "PlaylistView.h"

#include <stdio.h>
#include <sstream>
#include <algorithm>

#include <scx/CharsetHelper.h>
using namespace scx;

PlaylistView::PlaylistView()
{
}

PlaylistView::~PlaylistView()
{
}

void PlaylistView::Refresh()
{
    using namespace CharsetHelper;
    using namespace ncurses;

    lock_guard<mutex> locker(m_NeedRefreshMutex);

    d.ColorOn(color::kWhite, color::kBlack);
    d.Clear();

    // title
    auto titleText = Text(m_Title).EnableAttribute(m_Focused ? attribute::kBold : attribute::kNormal);
    d.Draw(ncurses::HorizontalAlignment::kCenter, 0, titleText);

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
    const int wField1 = (wText - wTime) * (2/5.f);
    const int wField2 = (wText - wTime) * (1/5.f);
    const int wField3 = (wText - wTime) - (wField1 + wField2);

    if (!m_List.empty()) {
        int lcount = std::min(hText, (int)m_List.size()-m_ItemBegin);
        if (m_ItemSelected >= m_ItemBegin + lcount) {
            // use the same algorithm in ScrollDown()
            m_ItemBegin = 0;
            for (int i = 0; i < m_ItemSelected; ++i) {
                if (m_ItemSelected > (d.h-2) / 2
                    && m_ItemBegin < (int)m_List.size()-(d.h-2-1)) {
                    ++m_ItemBegin;
                }
            }
            lcount = std::min(hText, (int)m_List.size()-m_ItemBegin);
        }
        for (int l = 0; l < lcount; ++l) {
            int index = m_ItemBegin + l;
            auto& item = *m_List[index];

            int fieldAttr = attribute::kBold;
            int fieldColorF = color::kGreen;
            int fieldColorB = color::kBlack;
            int timeAttr = attribute::kBold;
            int timeColorF = color::kMagenta;
            int timeColorB = color::kBlack;

            if (index == m_ItemSelected) {
                fieldAttr = timeAttr = attribute::kNormal;
                fieldColorF = timeColorF = color::kBlack;
                fieldColorB = timeColorB = color::kWhite;
                d.Draw(x, yoff + l, Text(std::string(w - 1, ' ')).SetAttributes(attribute::kReverse).SetColor(color::kWhite, color::kBlack));
            }

            xoff = x + 1;

            const string& field1 = MBStrWidth(item.tag.title) <= wField1-1 ?
                item.tag.title : MBWidthStr(item.tag.title, wField1-1-3) + "...";
            d.Draw(xoff, yoff + l, Text(field1).SetAttributes(fieldAttr).SetColor(fieldColorF, fieldColorB));
            xoff += wField1;

            const string& field2 = MBStrWidth(item.tag.artist) <= wField2-1 ?
                item.tag.artist : MBWidthStr(item.tag.artist, wField2-1-3) + "...";
            d.Draw(xoff, yoff + l, Text(field2).SetAttributes(fieldAttr).SetColor(fieldColorF, fieldColorB));
            xoff += wField2;

            const string& field3 = MBStrWidth(item.tag.album) <= wField3-1 ?
                item.tag.album : MBWidthStr(item.tag.album, wField3-1-3) + "...";
            d.Draw(xoff, yoff + l, Text(field3).SetAttributes(fieldAttr).SetColor(fieldColorF, fieldColorB));
            xoff += wField3;

            // TODO: duration >= 60min
            string duration;
            {
                if (item.hasRange && item.msEnd == (size_t)-1) {
                    item.msEnd = item.duration;
                }

                int total = (item.hasRange ? item.msEnd - item.msBeg : item.duration) / 1000;
                int min = total / 60;
                int sec = total % 60;
                char buf[6];
                snprintf(buf, sizeof(buf), "%.2d:%.2d", min, sec);
                duration.assign(buf, 5);
            }
            d.Draw(xoff, yoff + l, Text(duration).SetAttributes(timeAttr).SetColor(timeColorF, timeColorB));
            xoff += wTime;
        }

        xoff = x + 1 + wText;
        if ((int)m_List.size() > hText) {
            double percent = (double)(m_ItemSelected + 1) / m_List.size() - 0.00001f;
            yoff = y + hText * percent;
            auto barText = Text(" ").SetAttributes(attribute::kBold | attribute::kReverse).SetColor(color::kGreen, color::kBlack);
            d.Draw(xoff, yoff, barText);
        }
    }

    // status bar
    xoff = x + 1;
    yoff = y + hText;
    stringstream status;
    if (!m_List.empty()) {
        status << (m_ItemSelected+1) << "/" << m_List.size();
    }
    auto statusText = Text(status.str()).SetAttributes(attribute::kBold).SetColor(color::kWhite, color::kBlack);
    d.Draw(xoff, yoff, statusText);

    d.Refresh();

    if (m_NeedRefresh > 0) {
        --m_NeedRefresh;
    }
}

bool PlaylistView::NeedRefresh() const
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    return m_NeedRefresh != 0;
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
                const int line = (d.h - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollDown();
                }
            }
            break;

        case KEY_PPAGE:
            if (!empty) {
                const int line = (d.h - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollUp();
                }
            }
            break;

        case KEY_HOME:
            if (!empty) {
                m_ItemBegin = 0;
                m_ItemSelected = 0;
                ReqSelect();
            }
            break;

        case KEY_END:
            if (!empty) {
                m_ItemSelected = m_List.size() - 1;
                m_ItemBegin = std::max((int)m_List.size() - (d.h - 3), 0);
                ReqSelect();
            }
            break;

        case 'd':
            if (!empty) {
                ReqRemove(m_ItemSelected);
            }
            return true;

        case 'K':
        case 'J':
            if (!empty) {
                ReqMove(m_ItemSelected, key == 'K' ? -1 : 1);
            }
            return true;

        case 'C':
            if (!empty) {
                ReqClear();
            }
            return true;

        case '\n':
            if (!empty) {
                ReqPlay(m_ItemSelected);
            }
            return true;

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
    str << "[ " << m_Title << " " << index << " ]";
    m_Title = str.str();
}

void PlaylistView::SetPlaylistHandle(ClientPlaylistHandler* handler)
{
    if (m_PlaylistHandler) {
        m_PlaylistHandler->SigSelect().Disconnect(this);
        m_PlaylistHandler->SigPlay().Disconnect(this);
        m_PlaylistHandler->SigAppend().Disconnect(this);
        m_PlaylistHandler->SigRemove().Disconnect(this);
        m_PlaylistHandler->SigMove().Disconnect(this);
        m_PlaylistHandler->SigClear().Disconnect(this);
    }
    if (handler) {
        handler->SigSelect().Connect(&PlaylistView::SlotSelect, this);
        handler->SigPlay().Connect(&PlaylistView::SlotPlay, this);
        handler->SigAppend().Connect(&PlaylistView::SlotAppend, this);
        handler->SigRemove().Connect(&PlaylistView::SlotRemove, this);
        handler->SigMove().Connect(&PlaylistView::SlotMove, this);
        handler->SigClear().Connect(&PlaylistView::SlotClear, this);
    }
    m_PlaylistHandler = handler;
}

void PlaylistView::ScrollUp()
{
    if (m_ItemSelected > 0) {
        --m_ItemSelected;
    }
    if (m_ItemSelected < m_ItemBegin + (d.h-2) / 2 && m_ItemBegin > 0) {
        --m_ItemBegin;
    }
    ReqSelect();
}

void PlaylistView::ScrollDown()
{
    if (m_ItemSelected < (int)m_List.size()-1) {
        ++m_ItemSelected;
    }
    if (m_ItemSelected > (d.h-2) / 2 && m_ItemBegin < (int)m_List.size()-(d.h-2-1)) {
        ++m_ItemBegin;
    }
    ReqSelect();
}

void PlaylistView::ReqSelect()
{
    if (m_PlaylistHandler) {
        m_PlaylistHandler->Select(m_Index, m_ItemSelected);
    }
}

void PlaylistView::ReqPlay(int pos)
{
    if (m_WaitReply) {
        return;
    }
    if (m_PlaylistHandler) {
        m_WaitReply = true;
        m_PlaylistHandler->Play(m_Index, pos);
    }
}

void PlaylistView::ReqRemove(int pos)
{
    if (m_WaitReply) {
        return;
    }
    if (m_PlaylistHandler) {
        m_WaitReply = true;
        m_PlaylistHandler->Remove(m_Index, pos);
    }
}

void PlaylistView::ReqMove(int pos, char direct)
{
    if (m_WaitReply) {
        return;
    }
    if (m_PlaylistHandler) {
        m_WaitReply = true;
        m_PlaylistHandler->Move(m_Index, pos, direct);
    }
}

void PlaylistView::ReqClear()
{
    if (m_WaitReply) {
        return;
    }
    if (m_PlaylistHandler) {
        m_WaitReply = true;
        m_PlaylistHandler->Clear(m_Index);
    }
}

void PlaylistView::SlotSelect(int i, int pos)
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    if (i != m_Index) {
        return;
    }
    m_ItemSelected = pos;
    if (d.shown) {
        ++m_NeedRefresh;
    }
}

void PlaylistView::SlotPlay(int i, bool ok)
{
    if (i != m_Index) {
        return;
    }
    m_WaitReply = false;
}

void PlaylistView::SlotAppend(int i, deque<MediaItem*>& list)
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    if (i != m_Index) {
        return;
    }
    m_List.insert(m_List.end(), list.begin(), list.end());
    if (d.shown) {
        ++m_NeedRefresh;
    }
}

void PlaylistView::SlotRemove(int i, int pos)
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    if (i != m_Index) {
        return;
    }
    if (pos >= 0 && (size_t)pos < m_List.size()) {
        delete m_List[pos];
        m_List.erase(m_List.begin()+pos);
        if (!m_List.empty() && m_ItemSelected >= (int)m_List.size()-1) {
            m_ItemSelected = m_List.size() - 1;
        }
        if (d.shown) {
            ++m_NeedRefresh;
        }
    }
    m_WaitReply = false;
    ReqSelect();
}

void PlaylistView::SlotMove(int i, int pos, char direct)
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    if (i != m_Index) {
        return;
    }
    int newPos = direct > 0 ? pos+1 : pos-1;
    if (!m_List.empty() && (newPos >= 0 && (size_t)newPos < m_List.size())) {
        std::swap(m_List[pos], m_List[newPos]);
        m_ItemSelected = newPos;
        if (d.shown) {
            ++m_NeedRefresh;
        }
    }
    m_WaitReply = false;
    ReqSelect();
}

void PlaylistView::SlotClear(int i)
{
    lock_guard<mutex> locker(m_NeedRefreshMutex);
    if (i != m_Index) {
        return;
    }
    for (auto entry: m_List) {
        delete entry;
    }
    m_List.clear();
    m_ItemSelected = m_ItemBegin = 0;
    if (d.shown) {
        ++m_NeedRefresh;
    }
    m_WaitReply = false;
    ReqSelect();
}

