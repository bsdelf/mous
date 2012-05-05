#include "PlaylistView.h"
#include <sstream>
#include <algorithm>
using namespace std;

const string STR_TITLE = "Playlist";

PlaylistView::PlaylistView():
    m_Focused(false),
    m_Index(-1),
    m_ItemBegin(0)
{
    for (int i = 0; i < 200; ++i) {
        MediaItem* item = new MediaItem;
        stringstream stream;
        stream << i;
        item->tag.title = stream.str();
        m_List.AppendItem(item);
    }
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

    // title
    stringstream stream;
    stream << (m_Focused ? "^b" :  "") 
        << "[ " << STR_TITLE << " " << m_Index << " ]";
    d.CenterPrint(0, stream.str());

    // content
    if (!m_List.Empty()) {
        int w = d.w - 2;
        int h = d.h - 2;
        int xoff = 1;
        int yoff = 1;

        int lcount = std::min(h, m_List.GetItemCount()-m_ItemBegin);
        for (int l = 0; l < lcount; ++l) {
            int index = m_ItemBegin + l;
            MediaItem* item = m_List.GetItem(index);
            d.Print(xoff, yoff+l, item->tag.title);
        }

        if (m_List.GetItemCount() > h) {
            double percent = (double)(m_ItemBegin) / (m_List.GetItemCount()-h+1);
            d.Print(w, yoff+h*percent, "^b |");
        }
    }

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
            SigSwitchPlaylist(false);
            break;

        case 'l':
            SigSwitchPlaylist(true);
            break;

        case 'j':
            if (!m_List.Empty() && m_ItemBegin < m_List.GetItemCount()-(d.h-2))
                ++m_ItemBegin;
            break;

        case 'k':
            if (!m_List.Empty() && m_ItemBegin > 0)
                --m_ItemBegin;
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
