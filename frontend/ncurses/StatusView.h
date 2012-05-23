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
    void SlotStatus(const ClientPlayerHandler::PlayerStatus& status);

private:
    Window d;
    int m_NeedRefresh;
    mutable Mutex m_NeedRefreshMutex;

    ClientPlayerHandler* m_PlayerHandler;
    mutable Mutex m_PlayerStatusMutex;
    ClientPlayerHandler::PlayerStatus m_PlayerStatus;
};

#endif
