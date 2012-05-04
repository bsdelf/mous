#include "StatusView.h"

StatusView::StatusView()
{
}

StatusView::~StatusView()
{
}

void StatusView::OnResize(int x, int y, int w, int h)
{
}

void StatusView::Refresh()
{
}

void StatusView::MoveTo(int x, int y)
{
}

void StatusView::Resize(int w, int h)
{
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

        case 'n':
            break;

        case 'p':
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

void StatusView::Show(bool shown)
{
}

bool StatusView::IsShown() const
{
}

void StatusView::SetFocus(bool focused)
{
}

bool StatusView::HasFocus() const
{
}

int StatusView::GetMinHeight() const
{
    return 3;
}
