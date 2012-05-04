#ifndef HELPVIEW_H
#define HELPVIEW_H

#include <ncurses.h>
#include <panel.h>

#include "IView.h"

class HelpView: public IView
{
public:
    HelpView();
    ~HelpView();

    void OnResize(int x, int y, int w, int h);
    void Refresh(int x, int y, int w, int h);

    bool InjectKey(int key);

    void Show(bool shown);
    bool IsShown() const;

    void SetFocus(bool focused);
    bool HasFocus() const;
};

#endif
