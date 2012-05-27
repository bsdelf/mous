#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include <util/MediaItem.h>
using namespace mous;

#include <scx/Mutex.hpp>
using namespace scx;

#include "IView.h"
#include "ClientPlayerHandler.h"

class StatusView: public IView
{
public:
    StatusView();
    ~StatusView();

    void Refresh();
    bool NeedRefresh() const;

    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

public:
    int MinHeight() const;

    void SetPlayerHandler(ClientPlayerHandler* handler);

private:
    void SlotVolume(int);
    void SlotPlayMode(const std::string&);
    void SlotStatus(const ClientPlayerHandler::PlayerStatus&);

private:
    Window d;

    ClientPlayerHandler* m_PlayerHandler;

    bool m_WaitReply;

    mutable Mutex m_RefreshMutex;
    int m_NeedRefresh;
    int m_Volume;
    std::string m_PlayMode;
    ClientPlayerHandler::PlayerStatus m_PlayerStatus;
};

#endif
