#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include <ncurses.h>
#include <panel.h>

#include "IView.h"
#include "ClientPlayerHandler.h"

class StatusView: public IView
{
public:
    StatusView();
    ~StatusView();

    void Refresh();
    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

public:
    int MinHeight() const;

    void SetPlayerHandler(ClientPlayerHandler* handler);

private:
    Window d;
    int m_CurrentMs;
    int m_Duration;
    int m_BitRate;
    int m_SamepleRate;
    
    ClientPlayerHandler* m_PlayerHandler;
};

#endif
