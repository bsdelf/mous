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
            const std::string& str = ParseStyle(win, _str, colorId);
            mvwprintw(win, y, x, "%s", str.c_str());
            CloseStyle(win, colorId);
        }
    }

    void CenterPrint(int y, const std::string& _str)
    {
        if (win != NULL) {
            const std::string& str = ParseStyle(win, _str, colorId);
            WCenterPrint(win, y, w, str);
            CloseStyle(win, colorId);
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
        return mvwprintw(win, y, x, "%s", str.c_str());
    }

    // NOTE: very limited usage
    static std::string ParseStyle(WINDOW* win, const std::string& str, short& colorId)
    {
        int off = 0;
        while (off+1 < str.size()) {
            if (str[off] != '^')
                break;

            switch (str[off+1]) {
                case '^':
                    off = 1;
                    goto LABEL_END;
                    break;

                case 'b':
                    wattron(win, A_BOLD);
                    break;

                case 'h':
                    wattron(win, A_STANDOUT);
                    break;

                case 'r':
                    wattron(win, A_REVERSE);
                    break;

                // F|G, 0-7
                case 'c':
                    if (off+1+2 < str.size()) {
                        char f = str[off+1+1]-48;
                        char b = str[off+1+2]-48;
                        colorId = f * 8 + b;
                        init_pair(colorId, f, b);
                        wattron(win, COLOR_PAIR(colorId));
                    }
                    off += 2;
                    break;

                default:
                    break;
            }

            off += 2;
        }

LABEL_END:
        return str.substr(off, str.size());
    }

    static void CloseStyle(WINDOW* win, short colorId)
    {
        wattrset(win, A_NORMAL);
        wattroff(win, COLOR_PAIR(colorId));
    }

private:
    short colorId;

};

#endif
