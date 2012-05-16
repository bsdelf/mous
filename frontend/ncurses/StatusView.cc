#include "StatusView.h"

StatusView::StatusView()
{
}

StatusView::~StatusView()
{
}

void StatusView::Refresh()
{
    d.Clear();
    //d.CenterPrint(0, "Status");
    //d.Print(0, 0, "hello");
    d.Refresh();
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
    switch (key) {
        case ' ':
            break;

        case '>':
            break;

        case '<':
            break;

        case 'N':
            break;

        case 'P':
            break;

        case '+':
            break;

        case '-':
            break;

        default:
            return false;
    }
    return true;
}

void StatusView::Show(bool show)
{
    d.Show(show);
}

bool StatusView::IsShown() const
{
    return d.shown;
}

int StatusView::MinHeight() const
{
    return 3+2;
}

void StatusView::SetPlayerHandler(ClientPlayerHandler* handler)
{
    m_PlayerHandler = handler;
}
