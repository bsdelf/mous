#pragma once

#include "IView.h"

#include <string>
#include <vector>
#include <deque>
#include <set>

#include <scx/Signal.hpp>

class ExplorerView: public IView
{
public:
    ExplorerView();
    ~ExplorerView();

    void Refresh();
    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

    void SetFocus(bool focus);
    bool HasFocus() const;

public:
    scx::Signal<void (const std::string&)> SigTmpOpen;
    scx::Signal<void (const std::string&)> SigUserOpen;

    void AddSuffixes(const std::vector<std::string>&);

private:
    void BuildFileItems();
    void CdUp();
    void CdIn();
    void ScrollUp();
    void ScrollDown();

private:
    struct FileItem
    {
        std::string name;
        bool isDir;
        off_t size;
        mutable bool cacheOk;
        mutable std::string nameCache;
        mutable std::string sizeCache;
    };

private:
    Window d;
    bool m_Focused = false;
    std::string m_Path;
    std::string m_PathCache;
    bool m_HideDot = true;
    bool m_HideUnknown = false;
    std::deque<int> m_BeginStack;
    std::deque<int> m_SelectionStack;
    std::vector<FileItem> m_FileItems;
    std::set<std::string> m_Suffixes;
};

