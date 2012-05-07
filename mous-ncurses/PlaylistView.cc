#include "PlaylistView.h"
#include <sstream>
#include <algorithm>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/CharsetHelper.hpp>
using namespace scx;

const string STR_TITLE = "Playlist";

PlaylistView::PlaylistView():
    m_Focused(false),
    m_Index(-1),
    m_ItemBegin(0),
    m_ItemSelected(0)
{
    for (int i = 0; i < 100; ++i) {
        MediaItem* item = new MediaItem;

        stringstream stream;
        stream << "title标题标题标题标题标题标题标题标题标题" << i;
        item->tag.title = stream.str();

        stream.str("artist");
        stream << "artist" << i;
        item->tag.artist = stream.str();

        stream.str("album");
        stream << "album" << i;
        item->tag.album = stream.str();

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
    using namespace CharsetHelper;
    using namespace ncurses;

    d.Clear();

    // title
    if (m_Focused)
        d.OpenStyle("b");
    stringstream stream;
    stream << "[ " << STR_TITLE << " " << m_Index
        << " (" << m_List.GetItemCount() << ") ]";
    d.CenterPrint(0, stream.str());
    if (m_Focused)
        d.CloseStyle();

    // content
    // { {title artist album~~00:00 }#}
    if (!m_List.Empty()) {
        const int w = d.w - 2;
        const int h = d.h - 2;
        const int x = 1;
        const int y = 1;
        int xoff = x;
        int yoff = y;

        const int wText = w - 2;
        const int hText = h;

        const int wTime = 5 + 1;
        const int wField = (wText - wTime) / 3;
        const int wLastField = (wText - wTime) - wField * 2;

        int lcount = std::min(hText, m_List.GetItemCount()-m_ItemBegin);
        for (int l = 0; l < lcount; ++l) {
            int index = m_ItemBegin + l;
            MediaItem* item = m_List.GetItem(index);

            int fieldAttr = Attr::Bold;
            int fieldColorF = Color::Blue;
            int fieldColorB = Color::Black;
            int timeAttr = Attr::Bold;
            int timeColorF = Color::Magenta;
            int timeColorB = Color::Black;

            if (index == m_ItemSelected) {
                fieldAttr = timeAttr = Attr::Normal;
                fieldColorF = timeColorF = Color::Black;
                fieldColorB = timeColorB = Color::White;

                d.AttrSet(Attr::Normal | Attr::Reverse);
                d.Print(x, yoff+l, string(w, ' '));
            }

            xoff = x + 1;
            d.AttrSet(fieldAttr);
            d.ColorOn(fieldColorF, fieldColorB);

            const string& field1 = MBStrWidth(item->tag.title) <= wField-1 ?
                item->tag.title : MBWidthStr(item->tag.title, wField-1-3) + "...";
            d.Print(xoff, yoff+l, field1);
            xoff += wField;

            const string& field2 = MBStrWidth(item->tag.artist) <= wField-1 ?
                item->tag.artist : MBWidthStr(item->tag.artist, wField-1-3) + "...";
            d.Print(xoff, yoff+l, field2);
            xoff += wField;

            const string& field3 = MBStrWidth(item->tag.album) <= wLastField-1 ?
                item->tag.album : MBWidthStr(item->tag.album, wLastField-1-3) + "...";
            d.Print(xoff, yoff+l, field3);
            xoff += wLastField;

            d.ColorOn(timeColorF, timeColorB);
            d.Print(xoff, yoff+l, string("00:00"));
            xoff += wTime;
        }

        xoff = x + 1 + wText;
        if (m_List.GetItemCount() > hText) {
            double percent = (double)(m_ItemBegin) / (m_List.GetItemCount()-hText+1);
            yoff = y + hText*percent;
            d.AttrSet(Attr::Bold | Attr::Reverse);
            d.ColorOn(Color::Green, Color::Black);
            d.Print(xoff, yoff, " ");
        }

        d.ResetAttrColor();
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
            return true;

        case 'l':
            SigSwitchPlaylist(true);
            return true;

        case 'j':
            if (!m_List.Empty()) {
                if (m_ItemSelected < m_List.GetItemCount()-1) {
                    ++m_ItemSelected;
                }
                if (m_ItemSelected > (d.h-2) / 2
                        && m_ItemBegin < m_List.GetItemCount()-(d.h-2)) {
                    ++m_ItemBegin;
                }
            }
            break;

        case 'k':
            if (!m_List.Empty()) {
                if (m_ItemSelected > 0) {
                    --m_ItemSelected;
                }
                if (m_ItemSelected < m_ItemBegin + (d.h-2) / 2
                        && m_ItemBegin > 0) {
                    --m_ItemBegin;
                }
            }
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

        case '\n':
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
