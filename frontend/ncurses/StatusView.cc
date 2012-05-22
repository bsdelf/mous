#include "StatusView.h"

#include <string>
using namespace std;

#include <scx/CharsetHelper.hpp>
#include <scx/Conv.hpp>

StatusView::StatusView():
    m_NeedRefresh(0),
    m_PlayerHandler(NULL)
{
}

StatusView::~StatusView()
{
}

void StatusView::Refresh()
{
    using namespace ncurses;
    using namespace CharsetHelper;

    d.Clear();

    const int w = d.w - 3;
    const int h = d.h - 2;
    const int x = 2, y = 1;

    int xoff = x, yoff = y;

    const int wVolLabel = 2*2;
    const int wVolSlider = 20;
    const int wCurrentItem = w - (wVolLabel + wVolSlider + 1) - 1;

    MutexLocker playerStatuslocker(&m_PlayerStatusMutex);
    const MediaTag& tag = m_PlayerStatus.item.tag;

    // { title - artist~~~~~~-|=======|+}
    string currentItem = tag.title + " - " + tag.artist;
    currentItem = MBStrWidth(currentItem) <= wCurrentItem ?
        currentItem : MBWidthStr(currentItem, wCurrentItem-3) + "...";
    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::Green, Color::Black);
    d.Print(xoff, yoff, currentItem);

    xoff = x + w - (wVolSlider + wVolLabel);
    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::Yellow, Color::Black);
    d.Print(xoff, yoff, "-|");
    xoff += 2;

    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::White, Color::Black);
    d.Print(xoff, yoff, string(wVolSlider/2, '='));
    xoff += wVolSlider;

    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::Yellow, Color::Black);
    d.Print(xoff, yoff,  "|+");
    xoff += 2;

    d.ResetAttrColor();

    d.Refresh();

    MutexLocker needRefreshLocker(&m_NeedRefreshMutex);
    if (m_NeedRefresh > 0)
        --m_NeedRefresh;
}

bool StatusView::NeedRefresh() const
{
    MutexLocker locker(&m_NeedRefreshMutex);
    return m_NeedRefresh != 0;
}

void StatusView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void StatusView::Resize(int w, int h)
{
    d.Resize(w, h);
}

bool StatusView::InjectKey(int key)
{
    switch (key) {
        case ' ':
            m_PlayerHandler->Pause();
            break;

        case '>':
            break;

        case '<':
            break;

        case 'N':
            break;

        case 'P':
            break;

        case '+':
            break;

        case '-':
            break;

        default:
            return false;
    }
    return true;
}

void StatusView::Show(bool show)
{
    d.Show(show);
}

bool StatusView::IsShown() const
{
    return d.shown;
}

int StatusView::MinHeight() const
{
    return 3+2;
}

void StatusView::SetPlayerHandler(ClientPlayerHandler* handler)
{
    if (m_PlayerHandler != NULL) {
        m_PlayerHandler->SigStatus().DisconnectReceiver(this);
    }

    if (handler != NULL) {
        handler->SigStatus().Connect(&StatusView::SlotStatus, this);
    }

    m_PlayerHandler = handler;
}

void StatusView::SlotStatus(const ClientPlayerHandler::PlayerStatus& status)
{
    {
        MutexLocker locker(&m_PlayerStatusMutex);
        m_PlayerStatus = status;
    }

    {
        MutexLocker locker(&m_NeedRefreshMutex);
        ++m_NeedRefresh;
    }
}

void StatusView::DoRefresh()
{
    using namespace ncurses;

    if (!d.shown || d.win == NULL)
        return;

    d.Clear();

    d.ResetAttrColor();
    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::Green, Color::Black);
    d.Print(1, 2, NumToStr(m_PlayerStatus.pos/1000));

    d.ResetAttrColor();

    d.Refresh();
}
