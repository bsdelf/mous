#include "ExplorerView.h"

#include <algorithm>
#include <utility>

#include <scx/CharsetHelper.h>
#include <scx/Env.h>
#include <scx/FileInfo.h>
#include <scx/Dir.h>
#include <scx/PinYinCompare.h>

#include "AppEnv.h"

using namespace std;
using namespace scx;

constexpr const char* const STR_TITLE = "[ Explorer ]";
constexpr const char* const SIZE_HINT = "BKMG";

ExplorerView::ExplorerView()
{
    //const AppEnv* config = GlobalAppEnv::Instance();

    m_BeginStack = { 0 };
    m_SelectionStack = { 0 };
    m_Path = Env::Get("HOME");

    BuildFileItems();
}

ExplorerView::~ExplorerView()
{
}

void ExplorerView::Refresh()
{
    using namespace CharsetHelper;
    using namespace ncurses;

    d.ColorOn(color::kWhite, color::kBlack);
    d.Clear();

    auto titleText = Text(STR_TITLE).SetAttributes(m_Focused ? attribute::kBold : attribute::kNormal);
    d.Draw(ncurses::HorizontalAlignment::kCenter, 0, titleText);

    // content
    // { {name~~~size }#}
    // { {foo~~~1023K }#}
    const int w = d.Width() - 2;
    const int h = d.Height() - 2;
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

            int pathNormalAttr = attribute::kNormal;
            int boldAttr = attribute::kBold;
            int pathDirColorF = color::kBlue;
            int pathRegColorF = color::kWhite;
            int pathColorB = color::kBlack;
            int sizeColorF = color::kMagenta;
            int sizeColorB = color::kBlack;

            if (index == selection) {
                boldAttr = attribute::kNormal;
                pathRegColorF = sizeColorF = color::kBlack;
                pathColorB = sizeColorB = color::kWhite;
                d.Draw(x, yoff + l, Text(std::string(w - 1, ' ')).SetAttributes(attribute::kNormal | attribute::kReverse).SetColor(color::kWhite, color::kBlack));
            }

            xoff = x + 1;
            if (!item.cacheOk) {
                item.nameCache = MBStrWidth(item.name) <= wPath-1 ?
                    item.name : MBWidthStr(item.name, wPath-1-3) + "...";
            }
            Text nameText(item.nameCache);
            if (item.isDir) {
                nameText.SetAttributes(boldAttr).SetColor(pathDirColorF, pathColorB);
            } else {
                nameText.SetAttributes(pathNormalAttr).SetColor(pathRegColorF, pathColorB);
            }
            d.Draw(xoff, yoff + l, nameText);
            xoff += wPath;

            const char* hint = SIZE_HINT;
            off_t size = item.size;
            for (int i = 0; i < 3; ++i, ++hint) {
                off_t s = size / 1024;
                if (s <= 0) {
                    break;
                }
                size = s;
            }
            if (!item.cacheOk) {
                string& str = item.sizeCache;
                str = std::to_string(size) + *hint;
                if (str.size() < 5) {
                    str = string(5 - str.size(), ' ') + str;
                }
            }
            auto sizeText = Text(item.sizeCache).SetAttributes(boldAttr).SetColor(sizeColorF, sizeColorB);
            d.Draw(xoff, yoff + l, sizeText);
            xoff += wSize;

            item.cacheOk = true;
        }

        xoff = x + 1 + wText;
        if (m_FileItems.size() > (size_t)hText) {
            double percent = (double)(selection+1) / m_FileItems.size() - 0.00001f;
            yoff = y + hText * percent;
            auto barText = Text(" ").SetAttributes(attribute::kBold | attribute::kReverse).SetColor(color::kGreen, color::kBlack);
            d.Draw(xoff, yoff, barText);
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
    auto pathText = Text(m_PathCache).SetAttributes(attribute::kBold).SetColor(color::kWhite, color::kBlack);
    d.Draw(xoff, yoff, pathText);

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
                int line= (d.Height() - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollDown();
                }
            }
            break;

        case KEY_PPAGE:
            if (!m_FileItems.empty()) {
                int line= (d.Height() - 3) / 2;
                for (int i = 0; i < line; ++i) {
                    ScrollUp();
                }
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
                m_BeginStack.back() = std::max((int)m_FileItems.size() - (d.Height() - 3), 0);
            }
            break;

        case 'a':
            if (!m_FileItems.empty()) {
                int sel = m_SelectionStack.back();
                if (!m_FileItems[sel].isDir) {
                    SigUserOpen(m_Path + '/' + m_FileItems[sel].name);
                }
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
            m_BeginStack = { 0 };
            m_SelectionStack = { 0 };
            m_HideDot = !m_HideDot;
            BuildFileItems();
            break;

        case 's':
            m_BeginStack = { 0 };
            m_SelectionStack = { 0 };
            m_HideUnknown = !m_HideUnknown;
            BuildFileItems();
            break;

        case 'r':
            m_BeginStack = { 0 };
            m_SelectionStack = { 0 };
            BuildFileItems();
            break;

        default:
            return false;
    }

    Refresh();
    return true;
}

void ExplorerView::Show(bool visible)
{
    d.SetVisible(visible);
}

bool ExplorerView::IsShown() const
{
    return d.IsVisible();
}

void ExplorerView::SetFocus(bool focus)
{
    m_Focused = focus;
}

bool ExplorerView::HasFocus() const
{
    return m_Focused;
}

void ExplorerView::AddSuffixes(const std::vector<std::string>& list)
{
    for (const string& ext: list) {
        m_Suffixes.insert(ext);
    }
}

void ExplorerView::BuildFileItems()
{
    m_FileItems.clear();

    const vector<string>& files = Dir::ListDir(m_Path);

    auto& dirItems = m_FileItems;
    dirItems.reserve(files.size());
    std::vector<FileItem> otherItems;
    otherItems.reserve(files.size());

    for (const string& file: files) {
        if (file == "." || file == "..") {
            continue;
        }
        if (m_HideDot && file[0] == '.') {
            continue;
        }

        FileInfo info(m_Path + "/" + file);

        if (m_HideUnknown && (info.Type() != FileType::Directory)) {
            if (m_Suffixes.find(info.Suffix()) == m_Suffixes.end()) {
                continue;
            }
        }

        FileItem item;
        item.name = file;
        item.isDir = info.Type() == FileType::Directory;
        item.size = info.Size();
        item.cacheOk = false;
        auto& dest = item.isDir ? dirItems : otherItems;
        dest.push_back(std::move(item));
    }

    PinYinCompare pyc;
    auto cmp = [&pyc](const FileItem& a, const FileItem& b) {
        return pyc.CmpUtf8(a.name, b.name);
    };
    std::sort(dirItems.begin(), dirItems.end(), cmp);
    std::sort(otherItems.begin(), otherItems.end(), cmp);

    m_FileItems.insert(m_FileItems.end(), otherItems.begin(), otherItems.end());
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
    if (sel > (d.Height()-2) / 2 && beg < (int)m_FileItems.size()-(d.Height()-2-1)) {
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
    if (sel < beg + (d.Height()-2) / 2 && beg > 0) {
        --beg;
    }
}
