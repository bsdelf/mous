#include "PlaylistView.h"

PlaylistView::PlaylistView():
    m_Focused(false),
    m_Index(-1)
{
}

PlaylistView::~PlaylistView()
{
    for (int i = 0; i < m_List.GetItemCount(); ++i)
        delete m_List.GetItem(i);
    m_List.Clear();
}

void PlaylistView::OnResize(int x, int y, int w, int h)
{
    if (d.shown) {
        d.Cleanup();
        d.Init(x, y, w, h, true);
    }
}

void PlaylistView::Refresh()
{
    d.Clear();

    d.CenterPrint(0, "^b[Playlist]");

    d.Refresh();
}

void PlaylistView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void PlaylistView::Resize(int w, int h)
{
    d.Resize(w, h);
}

bool PlaylistView::InjectKey(int key)
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

        case 'd':
            break;

        case 'c':
            break;

        case 'y':
            break;

        case 'p':
            break;

        case 'e':
            break;

        case 'C':
            break;

        case KEY_ENTER:
            break;

        default:
            return false;
    }
    Refresh();
    return true;
}

void PlaylistView::Show(bool show)
{
    d.Show(show);
}

bool PlaylistView::IsShown() const
{
    return d.shown;
}

void PlaylistView::SetFocus(bool focus)
{
    m_Focused = focus;
}

bool PlaylistView::HasFocus() const
{
    return m_Focused;
}

void PlaylistView::SetIndex(int index)
{
    m_Index = index;
}

int PlaylistView::GetIndex() const
{
    return m_Index;
}
