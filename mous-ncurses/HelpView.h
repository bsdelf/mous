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

    void Show(bool show);
    bool IsShown() const;

private:
    void Cleanup();

private:
    WINDOW* m_Wnd;
    PANEL* m_Panel;
    bool m_Shown;
    bool m_Focused;
    int m_Width;
    int m_Height;
    int m_LineBegin;
    int m_LineCount;
};

#endif
