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

    void OnResize(int x, int y, int w, int h);

    void Refresh();
    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool shown);
    bool IsShown() const;

    void SetFocus(bool focused);
    bool HasFocus() const;

public:
    int GetMinHeight() const;
};

#endif
