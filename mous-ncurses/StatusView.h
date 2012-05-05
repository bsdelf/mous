#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include <ncurses.h>
#include <panel.h>

#include "IView.h"

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
    int GetMinHeight() const;

private:
    Window d;
};

#endif
