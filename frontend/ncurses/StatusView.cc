#include "StatusView.h"

#include <stdio.h>

#include <string>
using namespace std;

#include <scx/CharsetHelper.hpp>
#include <scx/Conv.hpp>
#include <scx/FileInfo.hpp>

inline static string FormatTime(int ms)
{
    if (ms <= 0)
        return "00:00";

    char buf[5+1];
    int sec = ms / 1000;
    snprintf(buf, sizeof(buf), "%.2d:%.2d", (int)(sec/60), (int)(sec%60));
    return string(buf, 5);
}

inline static string FormatBitRate(int rate)
{
    if (rate <= 0)
        return "   0";

    char buf[4+1];
    snprintf(buf, sizeof(buf), "%4.d", rate);
    return string(buf, 4);
}

StatusView::StatusView():
    m_PlayerHandler(nullptr),
    m_WaitReply(false),
    m_NeedRefresh(0),
    m_Volume(0)
{
}

StatusView::~StatusView()
{
}

void StatusView::Refresh()
{
    using namespace ncurses;
    using namespace CharsetHelper;

    std::lock_guard<std::mutex> locker(m_RefreshMutex);

    d.ColorOn(ncurses::Color::White, ncurses::Color::Black);
    d.Clear();

    const int w = d.w - 3;
    //const int h = d.h - 2;
    const int x = 2, y = 1;

    int xoff = x, yoff = y;

    const int wVolLabel = 2*2;
    const int wVolSlider = 20;
    const int wCurrentItem = w - (wVolLabel + wVolSlider + 1) - 1;

    const MediaItem& item = m_PlayerStatus.item;
    const MediaTag& tag = m_PlayerStatus.item.tag;

    // { title - artist (file)~~~~~~-|=======|+}
    string currentItem = tag.title + " - " + tag.artist + " (" + FileInfo(item.url).BaseName() + ")";
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

    d.AttrSet(Attr::Normal);
    d.ColorOn(Color::White, Color::Black);
    d.Print(xoff, yoff, string(wVolSlider * (m_Volume/100.f), '='));
    xoff += wVolSlider;

    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::Yellow, Color::Black);
    d.Print(xoff, yoff,  "|+");
    xoff += 2;

    // { 00:00/00:00 bitRate bps sampleRate Hz }
    xoff = x;
    yoff += 1;
    {
        const int tab = 4;
        string sp = "/";
        string ms = FormatTime(m_PlayerStatus.pos);
        string dur = FormatTime(m_PlayerStatus.duration);
        string bps = FormatBitRate(m_PlayerStatus.bitRate);
        string strBps = " Kbps";
        string rate = NumToStr(m_PlayerStatus.sampleRate);
        string strRate = " Hz";

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::Magenta, Color::Black);
        d.Print(xoff, yoff, ms);
        xoff += ms.size();

        d.AttrSet(Attr::Normal);
        d.ColorOn(Color::White, Color::Black);
        d.Print(xoff, yoff, sp);
        xoff += sp.size();

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::Magenta, Color::Black);
        d.Print(xoff, yoff, dur);
        xoff += dur.size();
        xoff += tab;

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::Magenta, Color::Black);
        d.Print(xoff, yoff, bps);
        xoff += bps.size();

        d.AttrSet(Attr::Normal);
        d.ColorOn(Color::White, Color::Black);
        d.Print(xoff, yoff, strBps);
        xoff += strBps.size() + tab;

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::Magenta, Color::Black);
        d.Print(xoff, yoff, rate);
        xoff += rate.size();

        d.AttrSet(Attr::Normal);
        d.ColorOn(Color::White, Color::Black);
        d.Print(xoff, yoff, strRate);
        xoff += strRate.size() + tab;

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::White, Color::Black);
        d.Print(xoff, yoff, m_PlayMode);
    }

    // { ---------->~~~~~~~~~~~ }
    xoff = x;
    yoff += 1;
    if (m_PlayerStatus.duration != 0) {
        float percent = std::min((float)m_PlayerStatus.pos / m_PlayerStatus.duration, 1.0000f);
        int wSlider = (w - 1 - 1) * percent;
        string slider = string(wSlider, '-') + ">";

        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::Yellow, Color::Black);
        d.Print(xoff, yoff, slider);
    }

    d.ResetAttrColor();

    d.Refresh();

    if (m_NeedRefresh > 0)
        --m_NeedRefresh;
}

bool StatusView::NeedRefresh() const
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
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
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    switch (key) {
        case ' ':
            if (!m_WaitReply) {
                m_PlayerHandler->Pause();
                m_WaitReply = true;
            }
            break;

        case 'm':
            if (!m_WaitReply) {
                m_PlayerHandler->NextPlayMode();
                m_WaitReply = true;
            }
            break;

        case 'n':
            if (!m_WaitReply) {
                m_PlayerHandler->PlayNext();
                m_WaitReply = true;
            }
            break;

        case 'p':
            if (!m_WaitReply) {
                m_PlayerHandler->PlayPrevious();
                m_WaitReply = true;
            }
            break;

        case '>':
            if (!m_WaitReply) {
                m_PlayerHandler->SeekForward();
                m_WaitReply = true;
            }
            break;

        case '<':
            if (!m_WaitReply) {
                m_PlayerHandler->SeekBackward();
                m_WaitReply = true;
            }
            break;

        case '+':
        case '=':
            if (!m_WaitReply) {
                m_PlayerHandler->VolumeUp();
                m_WaitReply = true;
            }
            break;

        case '-':
        case '_':
            if (!m_WaitReply) {
                m_PlayerHandler->VolumeDown();
                m_WaitReply = true;
            }
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
    if (m_PlayerHandler != nullptr) {
        m_PlayerHandler->SigPause().DisconnectObject(this);
        m_PlayerHandler->SigSeek().DisconnectObject(this);
        m_PlayerHandler->SigVolume().DisconnectObject(this);
        m_PlayerHandler->SigPlayNext().DisconnectObject(this);
        m_PlayerHandler->SigPlayMode().DisconnectObject(this);
        m_PlayerHandler->SigStatus().DisconnectObject(this);
    }

    if (handler != nullptr) {
        handler->SigPause().Connect(&StatusView::SlotPause, this);
        handler->SigSeek().Connect(&StatusView::SlotSeek, this);
        handler->SigVolume().Connect(&StatusView::SlotVolume, this);
        handler->SigPlayNext().Connect(&StatusView::SlotPlayNext, this);
        handler->SigPlayMode().Connect(&StatusView::SlotPlayMode, this);
        handler->SigStatus().Connect(&StatusView::SlotStatus, this);
    }

    m_PlayerHandler = handler;
}

void StatusView::SlotPause()
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_WaitReply = false;
}

void StatusView::SlotSeek()
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_WaitReply = false;
}

void StatusView::SlotVolume(int vol)
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_Volume = vol;
    ++m_NeedRefresh;
    m_WaitReply = false;
}

void StatusView::SlotPlayNext(bool hasNext)
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_WaitReply = false;
}

void StatusView::SlotPlayMode(const std::string& mode)
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_PlayMode = mode;
    ++m_NeedRefresh;
    m_WaitReply = false;
}

void StatusView::SlotStatus(const ClientPlayerHandler::PlayerStatus& status)
{
    std::lock_guard<std::mutex> locker(m_RefreshMutex);
    m_PlayerStatus = status;
    ++m_NeedRefresh;
}
