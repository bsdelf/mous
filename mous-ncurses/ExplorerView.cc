#include "ExplorerView.h"
#include <string>
using namespace std;

const string STR_TITLE = "[ Explorer ]";

ExplorerView::ExplorerView():
    m_Focused(false)
{
}

ExplorerView::~ExplorerView()
{
}

void ExplorerView::Refresh()
{
    d.Clear();

    string title = m_Focused ?
        ("^b" + STR_TITLE): STR_TITLE;
    d.CenterPrint(0, title);

    d.Refresh();
}

void ExplorerView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void ExplorerView::Resize(int w, int h)
{
    d.Resize(w, h);
}

bool ExplorerView::InjectKey(int key)
{
    switch (key) {
        case 'h':
            break;

        case 'l':
            break;

        case 'j':
            break;

        case 'k':
            break;

        case 'a':
            break;

        case KEY_ENTER:
            break;

        default:
            return false;
    }
    Refresh();
    return true;
}

void ExplorerView::Show(bool show)
{
    d.Show(show);
}

bool ExplorerView::IsShown() const
{
    return d.shown;
}

void ExplorerView::SetFocus(bool focus)
{
    m_Focused = focus;
}

bool ExplorerView::HasFocus() const
{
    return m_Focused;
}
