#ifndef HELPVIEW_H
#define HELPVIEW_H

#include "IView.h"

class HelpView: public IView
{
public:
    HelpView();
    ~HelpView();

    void OnResize(int x, int y, int w, int h);

    void MoveTo(int x, int y);
    void Resize(int w, int h);
    void Refresh();

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

private:
    void Cleanup();

private:
    ViewData d;
    int m_LineBegin;
    int m_LineCount;
};

#endif
