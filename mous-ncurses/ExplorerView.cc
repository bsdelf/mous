#include "ExplorerView.h"

#include <algorithm>
using namespace std;

#include "scx/Conv.hpp"
#include "scx/CharsetHelper.hpp"
#include "scx/Env.hpp"
#include "scx/FileInfo.hpp"
#include "scx/Dir.hpp"
using namespace scx;

const string STR_TITLE = "[ Explorer ]";

const char* const SIZE_HINT = "BKMG";

class FileItemCmp
{
public:
    explicit FileItemCmp(const UniPinYin& _py):
        py(_py)
    {
    }

    bool operator()(const ExplorerView::FileItem& a, const ExplorerView::FileItem& b) const
    {
        return py.Utf8StrCmp(a.name, b.name);
    }

private:
    const UniPinYin& py;
};

ExplorerView::ExplorerView():
    m_Focused(false),
    m_HideDot(true),
    m_ItemBegin(0),
    m_ItemSelected(0)
{
    m_UniPinYin.LoadMap("unipy.map");
    m_Path = Env::GetEnv(Env::Home);
    BuildFileItems();
}

ExplorerView::~ExplorerView()
{
}

void ExplorerView::Refresh()
{
    using namespace CharsetHelper;
    using namespace ncurses;

    d.Clear();

    if (m_Focused)
        d.AttrOn(ncurses::Attr::Bold);
    d.CenterPrint(0, STR_TITLE);
    d.ResetAttrColor();

    // content
    // { {name~~~size }#}
    // { {foo~~~1023K }#}
    if (!m_FileItems.empty()) {
        const int w = d.w - 2;
        const int h = d.h - 2;
        const int x = 1;
        const int y = 1;
        int xoff = x;
        int yoff = y;

        const int wText = w - 2;
        const int hText = h - 1;

        const int wSize = 5 + 1;
        const int wPath = wText - wSize;

        int lcount = std::min(hText, (int)(m_FileItems.size()-m_ItemBegin));
        for (int l = 0; l < lcount; ++l) {
            int index = m_ItemBegin + l;
            const FileItem& item = m_FileItems[index];

            int pathNormalAttr = Attr::Normal;
            int boldAttr = Attr::Bold;
            int pathDirColorF = Color::Blue;
            int pathRegColorF = Color::White;
            int pathColorB = Color::Black;
            int sizeColorF = Color::Magenta;
            int sizeColorB = Color::Black;

            if (index == m_ItemSelected) {
                boldAttr = Attr::Normal;
                pathRegColorF = pathDirColorF = sizeColorF = Color::Black;
                pathColorB = sizeColorB = Color::White;

                d.AttrSet(Attr::Normal | Attr::Reverse);
                d.Print(x, yoff+l, string(w-1, ' '));
            }

            xoff = x + 1;
            if (item.isDir) {
                d.AttrSet(boldAttr);
                d.ColorOn(pathDirColorF, pathColorB);
            } else {
                d.AttrSet(pathNormalAttr);
                d.ColorOn(pathRegColorF, pathColorB);
            }

            const string& path = MBStrWidth(item.name) <= wPath-1 ?
                item.name : MBWidthStr(item.name, wPath-1-3) + "...";
            d.Print(xoff, yoff+l, path);
            xoff += wPath;

            const char* hint = SIZE_HINT;
            int size = item.size;
            for (int i = 0; i < 3; ++i, ++hint) {
                int s = size / 1024;
                if (s <= 0)
                    break;
                size = s;
            }
            string strSize = NumToStr(size) + *hint;
            if (strSize.size() < 5)
            strSize = string(5 - strSize.size(), ' ') + strSize;

            d.AttrSet(boldAttr);
            d.ColorOn(sizeColorF, sizeColorB);
            d.Print(xoff, yoff+l, strSize);
            xoff += wSize;
        }

        xoff = x + 1 + wText;
        if (m_FileItems.size() > hText) {
            double percent = (double)(m_ItemBegin) / (m_FileItems.size()-hText+1);
            yoff = y + hText*percent;
            d.AttrSet(Attr::Bold | Attr::Reverse);
            d.ColorOn(Color::Green, Color::Black);
            d.Print(xoff, yoff, " ");
        }

        xoff = x;
        yoff = y + hText;
        d.AttrSet(Attr::Bold);
        d.ColorOn(Color::White, Color::Black);
        d.Print(xoff, yoff, m_Path);
        d.ResetAttrColor();
    }

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
            if (!m_FileItems.empty()) {
                m_Path = FileInfo(m_Path).AbsPath();
                BuildFileItems();
            }
            break;

        case 'l':
            if (!m_FileItems.empty()) {
                if (m_FileItems[m_ItemSelected].isDir) {
                    m_Path += (m_Path != "/" ? "/" : "") + m_FileItems[m_ItemSelected].name;
                    BuildFileItems();
                }
            }
            break;

        case 'j':
            if (!m_FileItems.empty()) {
                if (m_ItemSelected < m_FileItems.size()-1) {
                    ++m_ItemSelected;
                }
                if (m_ItemSelected > (d.h-2) / 2
                        && m_ItemBegin < m_FileItems.size()-(d.h-2-1)) {
                    ++m_ItemBegin;
                }
            }
            break;

        case 'k':
            if (!m_FileItems.empty()) {
                if (m_ItemSelected > 0) {
                    --m_ItemSelected;
                }
                if (m_ItemSelected < m_ItemBegin + (d.h-2) / 2
                        && m_ItemBegin > 0) {
                    --m_ItemBegin;
                }
            }
            break;

        case 'a':
            break;

        case '\n':
            break;

        case '/':
            break;

        case '.':
            m_HideDot = !m_HideDot;
            BuildFileItems();
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

void ExplorerView::BuildFileItems()
{
    vector<string> files = Dir::ListDir(m_Path);
    m_FileItems.clear();
    m_FileItems.reserve(files.size());
    for (size_t i = 0; i < files.size(); ++i) {
        if (files[i] == "." || files[i] == "..")
            continue;
        if (m_HideDot && files[i][0] == '.')
            continue;

        FileInfo info(m_Path + "/" + files[i]);
        FileItem item;
        item.name = files[i];
        item.isDir = info.Type() == FileType::Directory;
        item.isExe = false;
        item.size = info.Size();
        m_FileItems.push_back(item);
    }

    std::sort(m_FileItems.begin(), m_FileItems.end(), FileItemCmp(m_UniPinYin));

    m_ItemBegin = 0;
    m_ItemSelected = 0;
}


