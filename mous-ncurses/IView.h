#ifndef IVIEW_H
#define IVIEW_H

#include <ncurses.h>
#include <panel.h>

#include <string>

class IView
{
public:
    virtual ~IView() { }

    virtual void OnResize(int x, int y, int w, int h) = 0;

    virtual void MoveTo(int x, int y) = 0;
    virtual void Resize(int w, int h) = 0;
    virtual void Refresh() = 0;

    virtual bool InjectKey(int key) = 0;

    virtual void Show(bool show) = 0;
    virtual bool IsShown() const = 0;

    virtual void SetFocus(bool focus) { }
    virtual bool HasFocus() const { return false; }
};

struct ViewData
{
    ViewData():
        win(NULL),
        panel(NULL),
        width(0),
        height(0)
    {
    }

    ~ViewData()
    {
        Cleanup();
    }

    void Init(int x, int y, int w, int h, bool boxed = true)
    {
        win = newwin(h, w, y, x);
        panel = new_panel(win);
        if (boxed)
            box(win, 0, 0);

        width = w;
        height = h;
    }

    void Cleanup()
    {
        if (panel != NULL) {
            del_panel(panel);
            panel = NULL;
        }

        if (win != NULL) {
            delwin(win);
            win = NULL;
        }
    }

    void Refresh()
    {
        wrefresh(win);
    }

    void Print(int x, int y, const std::string& data)
    {
        mvwprintw(win, y, x, data.c_str());
    }

        // NOTE: wide character not considered presently
    void CenterPrint(int y, const std::string& str)
    {
        int x = (width - str.size()) / 2;
        mvwprintw(win, y, x, str.c_str());
    }

    void Clear(bool boxed = true)
    {
        werase(win);
        if (boxed)
            box(win, 0, 0);
    }

    void MoveTo(int x, int y)
    {
        mvwin(win, y, x);
    }

    void Resize(int w, int h)
    {
        wresize(win, h, w);
    }

    void Show(bool show)
    {
        if (show)
            show_panel(panel);
        else
            hide_panel(panel);
        update_panels();
        doupdate();

        shown = show;
    }

    WINDOW* win;
    PANEL* panel;
    bool shown;
    int width;
    int height;
};

#endif
