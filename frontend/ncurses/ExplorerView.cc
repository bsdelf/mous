#include "ExplorerView.h"

#include <algorithm>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/CharsetHelper.hpp>
#include <scx/Env.hpp>
#include <scx/FileInfo.hpp>
#include <scx/Dir.hpp>
using namespace scx;

#include "AppEnv.h"

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
    m_HideDot(true)
{
    const AppEnv* config = GlobalAppEnv::Instance();
    if (config != nullptr)
        m_UniPinYin.LoadMap(config->pyMapFile);

    m_BeginStack.push_back(0);
    m_SelectionStack.push_back(0);

    m_Path = Env::Env(Env::Home);
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

    const int begin = m_BeginStack.back();
    const int selection = m_SelectionStack.back();
    if (!m_FileItems.empty()) {
        int lcount = std::min(hText, (int)(m_FileItems.size()-begin));
        for (int l = 0; l < lcount; ++l) {
            int index = begin + l;
            const FileItem& item = m_FileItems[index];

            int pathNormalAttr = Attr::Normal;
            int boldAttr = Attr::Bold;
            int pathDirColorF = Color::Blue;
            int pathRegColorF = Color::White;
            int pathColorB = Color::Black;
            int sizeColorF = Color::Magenta;
            int sizeColorB = Color::Black;

            if (index == selection) {
                boldAttr = Attr::Normal;
                pathRegColorF = sizeColorF = Color::Black;
                pathColorB = sizeColorB = Color::White;

                d.AttrSet(Attr::Normal | Attr::Reverse);
                d.ColorOn(Color::White, Color::Black);
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

            if (!item.cacheOk) {
                item.nameCache = MBStrWidth(item.name) <= wPath-1 ?
                    item.name : MBWidthStr(item.name, wPath-1-3) + "...";
            }
            d.Print(xoff, yoff+l, item.nameCache);
            xoff += wPath;

            const char* hint = SIZE_HINT;
            off_t size = item.size;
            for (int i = 0; i < 3; ++i, ++hint) {
                off_t s = size / 1024;
                if (s <= 0)
                    break;
                size = s;
            }
            if (!item.cacheOk) {
                string& str = item.sizeCache;
                str = NumToStr(size) + *hint;
                if (str.size() < 5)
                    str = string(5 - str.size(), ' ') + str;
            }

            d.AttrSet(boldAttr);
            d.ColorOn(sizeColorF, sizeColorB);
            d.Print(xoff, yoff+l, item.sizeCache);
            xoff += wSize;

            item.cacheOk = true;
        }

        xoff = x + 1 + wText;
        if (m_FileItems.size() > (size_t)hText) {
            double percent = (double)(selection+1) / m_FileItems.size() - 0.00001f;
            yoff = y + hText*percent;
            d.AttrSet(Attr::Bold | Attr::Reverse);
            d.ColorOn(Color::Green, Color::Black);
            d.Print(xoff, yoff, " ");
        }

    }

    // status bar
    if (m_PathCache.empty()) {
        m_PathCache = m_Path;
        if (MBStrWidth(m_PathCache) > wText) {
            do {
                m_PathCache = MBSubStr(m_PathCache, MBStrLen(m_PathCache)-1, 1);
            } while (MBStrWidth(m_PathCache) > (wText - 3));
            m_PathCache.insert(0, "...");
        }
    }
    xoff = x + 1;
    yoff = y + hText;
    d.AttrSet(Attr::Bold);
    d.ColorOn(Color::White, Color::Black);
    d.Print(xoff, yoff, m_PathCache);

    d.ResetAttrColor();

    d.Refresh();
}

void ExplorerView::MoveTo(int x, int y)
{
    d.MoveTo(x, y);
}

void ExplorerView::Resize(int w, int h)
{
    // invalidate cache
    m_PathCache.clear();
    for (auto& item: m_FileItems) {
        item.cacheOk = false;
    }

    d.Resize(w, h);
    d.EnableKeypad(true);
}

bool ExplorerView::InjectKey(int key)
{
    switch (key) {
        case KEY_LEFT:
        case 'h':
            CdUp();
            break;

        case KEY_RIGHT:
        case 'l':
            if (!m_FileItems.empty()) {
                CdIn();
            }
            break;

        case KEY_DOWN:
        case 'j':
            if (!m_FileItems.empty()) {
                ScrollDown();
            }
            break;

        case KEY_UP:
        case 'k':
            if (!m_FileItems.empty()) {
                ScrollUp();
            }
            break;

        case KEY_NPAGE:
            if (!m_FileItems.empty()) {
                int line= (d.h - 3) / 2;
                for (int i = 0; i < line; ++i)
                    ScrollDown();
            }
            break;

        case KEY_PPAGE:
            if (!m_FileItems.empty()) {
                int line= (d.h - 3) / 2;
                for (int i = 0; i < line; ++i)
                    ScrollUp();
            }
            break;

        case KEY_HOME:
            if (!m_FileItems.empty()) {
                m_BeginStack.back() = 0;
                m_SelectionStack.back() = 0;
            }
            break;

        case KEY_END:
            if (!m_FileItems.empty()) {
                m_SelectionStack.back() = m_FileItems.size() - 1;
                m_BeginStack.back() = std::max((int)m_FileItems.size() - (d.h - 3), 0);
            }
            break;

        case 'a':
            if (!m_FileItems.empty()) {
                int sel = m_SelectionStack.back();
                if (!m_FileItems[sel].isDir)
                    SigUserOpen(m_Path + '/' + m_FileItems[sel].name);
            }
            return true;

        case '\n':
            if (!m_FileItems.empty()) {
                int sel = m_SelectionStack.back();
                if (!m_FileItems[sel].isDir) {
                    SigTmpOpen(m_Path + '/' + m_FileItems[sel].name);
                    return true;
                } else {
                    CdIn();
                }
            }
            break;

        case '/':
            if (!m_FileItems.empty()) {
            }
            break;

        case '.':
            m_BeginStack.resize(1);
            m_BeginStack.back() = 0;
            m_SelectionStack.resize(1);
            m_SelectionStack.back() = 0;
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
    for (const string& file: files) {
        if (file == "." || file == "..")
            continue;
        if (m_HideDot && file[0] == '.')
            continue;

        FileInfo info(m_Path + "/" + file);
        FileItem item;
        item.name = file;
        item.isDir = info.Type() == FileType::Directory;
        item.size = info.Size();
        item.cacheOk = false;
        m_FileItems.push_back(item);
    }

    std::sort(m_FileItems.begin(), m_FileItems.end(), FileItemCmp(m_UniPinYin));
}

void ExplorerView::CdUp()
{
    if (m_BeginStack.size() > 1) {
        m_BeginStack.pop_back();
        m_SelectionStack.pop_back();
    } else {
        m_BeginStack.back() = 0;
        m_SelectionStack.back() = 0;
    }

    m_Path = FileInfo(m_Path).AbsPath();
    m_PathCache.clear();
    BuildFileItems();
}

void ExplorerView::CdIn()
{
    int sel = m_SelectionStack.back();
    if (m_FileItems[sel].isDir) {
        m_Path += (m_Path != "/" ? "/" : "") + m_FileItems[sel].name;
        m_PathCache.clear();
        BuildFileItems();

        m_BeginStack.push_back(0);
        m_SelectionStack.push_back(0);
    }
}

void ExplorerView::ScrollDown()
{
    int& beg = m_BeginStack.back();
    int& sel = m_SelectionStack.back();
    if (sel < (int)m_FileItems.size()-1) {
        ++sel;
    }
    if (sel > (d.h-2) / 2
            && beg < (int)m_FileItems.size()-(d.h-2-1)) {
        ++beg;
    }
}

void ExplorerView::ScrollUp()
{
    int& beg = m_BeginStack.back();
    int& sel = m_SelectionStack.back();
    if (sel > 0) {
        --sel;
    }
    if (sel < beg + (d.h-2) / 2
            && beg > 0) {
        --beg;
    }
}
