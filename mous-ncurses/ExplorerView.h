#ifndef EXPLORERVIEW_H
#define EXPLORERVIEW_H

#include "IView.h"

#include <string>
#include <vector>

#include "scx/UniPinYin.hpp"
class FileItemCmp;

class ExplorerView: public IView
{
    friend class FileItemCmp;

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

private:
    void BuildFileItems();

private:
    struct FileItem
    {
        std::string name;
        bool isDir;
        bool isExe;
        long size;
    };

private:
    bool m_Focused;
    Window d;
    std::string m_Path;
    bool m_HideDot;
    int m_ItemBegin;
    int m_ItemSelected;
    std::vector<FileItem> m_FileItems;
    scx::UniPinYin m_UniPinYin;
};

#endif
