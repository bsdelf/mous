#ifndef IVIEW_H
#define IVIEW_H

#include <ncurses.h>

#include <string>

class IView
{
public:
    virtual ~IView() { }

    virtual void MoveTo(int x, int y) = 0;
    virtual void Resize(int w, int h) = 0;
    virtual void Refresh() = 0;

    virtual bool InjectKey(int key) = 0;

    virtual void Show(bool show) = 0;
    virtual bool IsShown() const = 0;

    virtual void SetFocus(bool focus) { }
    virtual bool HasFocus() const { return false; }
};

struct Window
{
    Window():
        win(NULL),
        boxed(true),
        shown(false),
        x(0),
        y(0),
        w(0),
        h(0)
    {
    }

    ~Window()
    {
        Cleanup();
    }

    void Init(int _x, int _y, int _w, int _h, bool _boxed)
    {
        x = _x;
        y = _y;
        w = _w;
        h = _h;
        boxed = _boxed;

        win = newwin(h, w, y, x);
        if (boxed)
            box(win, 0, 0);
    }

    void Cleanup()
    {
        if (win != NULL) {
            delwin(win);
            win = NULL;
        }
    }

    void Refresh()
    {
        if (win != NULL)
            wrefresh(win);
    }

    void Print(int x, int y, const std::string& _str)
    {
        if (win != NULL) {
            const std::string& str = ParseStyle(win, _str);
            mvwprintw(win, y, x, str.c_str());
            CloseStyle(win);
        }
    }

    void CenterPrint(int y, const std::string& _str)
    {
        if (win != NULL) {
            const std::string& str = ParseStyle(win, _str);
            WCenterPrint(win, y, w, str);
            CloseStyle(win);
        }
    }

    void Clear()
    {
        werase(win);
        if (boxed)
            box(win, 0, 0);
    }

    void MoveTo(int _x, int _y)
    {
        x = _x;
        y = _y;
        if (win != NULL)
            mvwin(win, y, x);
    }

    void Resize(int _w, int _h)
    {
        w = _w;
        h = _h;
        if (win != NULL) {
            wresize(win, h, w);
            mvwin(win, y, x);
        }
    }

    void Show(bool show)
    {
        if (show) {
            if (win == NULL)
                Init(x, y, w, h, boxed);
        } else {
            Cleanup();
        }
        shown = show;
    }

    WINDOW* win;
    bool boxed;
    bool shown;
    int x;
    int y;
    int w;
    int h;

    // NOTE: wide characters are not considered presently
    static int WCenterPrint(WINDOW* win, int y, int w, const std::string& str)
    {
        int x = (w - str.size()) / 2;
        return mvwprintw(win, y, x, str.c_str());
    }

    static std::string ParseStyle(WINDOW* win, const std::string& str)
    {
        if (str[0] != '^')
            return str;

        switch (str[1]) {
            case 'b':
                wattron(win, A_BOLD);
                break;
                
            default:
                break;
        }
        return str.substr(2, str.size());
    }

    static void CloseStyle(WINDOW* win)
    {
        wattrset(win, A_NORMAL);
    }
};

#endif
