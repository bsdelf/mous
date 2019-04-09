#include "StatusView.h"

#include <stdio.h>

#include <string>
using namespace std;

#include <scx/CharsetHelper.h>
#include <scx/FileInfo.h>

inline static string FormatTime(int ms)
{
    if (ms <= 0) {
        return "00:00";
    }
    char buf[5+1];
    const int sec = ms / 1000;
    snprintf(buf, sizeof(buf), "%.2d:%.2d", (int)(sec/60), (int)(sec%60));
    return string(buf, 5);
}

inline static string FormatBitRate(int rate)
{
    if (rate <= 0) {
        return "   0";
    }
    char buf[4+1];
    snprintf(buf, sizeof(buf), "%4.d", rate);
    return string(buf, 4);
}

StatusView::StatusView()
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

    d.ColorOn(ncurses::color::kWhite, ncurses::color::kBlack);
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

    // { title - artist (file)}
    string currentItem = tag.title + " - " + tag.artist + " (" + FileInfo(item.url).BaseName() + ")";
    currentItem = MBStrWidth(currentItem) <= wCurrentItem ?
        currentItem : MBWidthStr(currentItem, wCurrentItem-3) + "...";
    d.Draw(xoff, yoff, Text(currentItem).SetAttributes(attribute::kBold).SetColor(color::kGreen, color::kBlack));

    // { 00:00/00:00 bitRate bps sampleRate Hz ~~~~~~ -|=======|+ }
    xoff = x;
    yoff += 1;
    {
        const int tab = 4;
        string sp = "/";
        string ms = FormatTime(m_PlayerStatus.pos);
        string dur = FormatTime(m_PlayerStatus.duration);
        string bps = FormatBitRate(m_PlayerStatus.bitRate);
        string strBps = " Kbps";
        string rate = std::to_string(m_PlayerStatus.sampleRate);
        string strRate = " Hz";

        d.Draw(xoff, yoff, Text(ms).SetAttributes(attribute::kBold).SetColor(color::kMagenta, color::kBlack));
        xoff += ms.size();

        d.Draw(xoff, yoff, Text(sp).SetAttributes(attribute::kNormal).SetColor(color::kWhite, color::kBlack));
        xoff += sp.size();

        d.Draw(xoff, yoff, Text(dur).SetAttributes(attribute::kBold).SetColor(color::kMagenta, color::kBlack));
        xoff += dur.size() + tab;

        d.Draw(xoff, yoff, Text(bps).SetAttributes(attribute::kBold).SetColor(color::kMagenta, color::kBlack));
        xoff += bps.size();

        d.Draw(xoff, yoff, Text(strBps).SetAttributes(attribute::kNormal).SetColor(color::kWhite, color::kBlack));
        xoff += strBps.size() + tab;

        d.Draw(xoff, yoff, Text(rate).SetAttributes(attribute::kBold).SetColor(color::kMagenta, color::kBlack));
        xoff += rate.size();

        d.Draw(xoff, yoff, Text(strRate).SetAttributes(attribute::kNormal).SetColor(color::kWhite, color::kBlack));
        xoff += strRate.size() + tab;

        d.Draw(xoff, yoff, Text(m_PlayMode).SetAttributes(attribute::kBold).SetColor(color::kWhite, color::kBlack));
    }
    xoff = x + w - (wVolSlider + wVolLabel) - 1;

    d.Draw(xoff, yoff, Text("-|").SetAttributes(attribute::kBold).SetColor(color::kYellow, color::kBlack));
    xoff += 2;

    std::string volStr(wVolSlider * (m_Volume/100.f), '#');
    d.Draw(xoff, yoff, Text(volStr).SetAttributes(attribute::kNormal).SetColor(color::kWhite, color::kBlack));
    xoff += wVolSlider;

    d.Draw(xoff, yoff, Text("|+").SetAttributes(attribute::kBold).SetColor(color::kYellow, color::kBlack));
    xoff += 2;

    // { ---------->~~~~~~~~~~~ }
    xoff = x;
    yoff += 1;
    if (m_PlayerStatus.duration != 0) {
        float percent = std::min((float)m_PlayerStatus.pos / m_PlayerStatus.duration, 1.0000f);
        int wSlider = (w - 1 - 1) * percent;
        string slider(wSlider + 1, '-');
        slider.back() = '>';
        d.Draw(xoff, yoff, Text(slider).SetAttributes(attribute::kBold).SetColor(color::kYellow, color::kBlack));
    }

    d.ResetAttrColor();

    d.Refresh();

    if (m_NeedRefresh > 0) {
        --m_NeedRefresh;
    }
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
    d.SetVisible(show);
}

bool StatusView::IsShown() const
{
    return d.IsVisible();
}

int StatusView::MinHeight() const
{
    return 3+2;
}

void StatusView::SetPlayerHandler(ClientPlayerHandler* handler)
{
    if (m_PlayerHandler) {
        m_PlayerHandler->SigPause().Disconnect(this);
        m_PlayerHandler->SigSeek().Disconnect(this);
        m_PlayerHandler->SigVolume().Disconnect(this);
        m_PlayerHandler->SigPlayNext().Disconnect(this);
        m_PlayerHandler->SigPlayMode().Disconnect(this);
        m_PlayerHandler->SigStatus().Disconnect(this);
    }

    if (handler) {
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
