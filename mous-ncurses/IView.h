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

namespace ncurses {
    namespace Color {
        const int Black = COLOR_BLACK;
        const int Red = COLOR_RED;
        const int Green = COLOR_GREEN;
        const int Yellow = COLOR_YELLOW;
        const int Blue = COLOR_BLUE;
        const int Magenta = COLOR_MAGENTA;
        const int Cyan = COLOR_CYAN;
        const int White = COLOR_WHITE;
    };

    namespace Attr {
        const int Normal = A_NORMAL;
        const int Underline = A_UNDERLINE;
        const int Reverse = A_REVERSE;
        const int Blink = A_BLINK;
        const int Bold = A_BOLD;
    }
}

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

    void Print(int x, int y, const std::string& _str, bool styled = false)
    {
        if (win != NULL) {
            if (!styled) {
                mvwprintw(win, y, x, "%s", _str.c_str());
            } else {
                const std::string& str = ParseStyle(_str);
                mvwprintw(win, y, x, "%s", str.c_str());
                CloseStyle();
            }
        }
    }

    void CenterPrint(int y, const std::string& _str, bool styled = false)
    {
        if (win != NULL) {
            if (!styled) {
                DoCenterPrint(y, w, _str);
            } else {
                const std::string& str = ParseStyle(_str);
                DoCenterPrint(y, w, str);
                CloseStyle();
            }
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

    void AttrOn(int attrs)
    {
        wattron(win, attrs);
    }

    void AttrSet(int attrs)
    {
        wattrset(win, attrs);
    }

    void AttrOff(int attrs)
    {
        wattroff(win, attrs);
    }

    void ResetAttrColor()
    {
        init_pair(0, ncurses::Color::White, ncurses::Color::Black);
        wattrset(win, ncurses::Attr::Normal | COLOR_PAIR(0));
    }

    short ColorOn(int f, int b)
    {
        short colorId = f*8 + b + 1;
        init_pair(colorId, f, b);
        wattron(win, COLOR_PAIR(colorId));
        return colorId;
    }

    void ColorOff(short colorId)
    {
        wattroff(win, COLOR_PAIR(colorId));
        wattroff(win, COLOR_PAIR(0));
    }

    int OpenStyle(const std::string& style)
    {
        if (style.empty())
            return 0;

        int n = 1;
        switch (style[0]) {
            case '^':
                n = 0;
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

                // F|B, 0-7
            case 'c':
                if (style.size() >= 3) {
                    char f = style[1]-48;
                    char b = style[2]-48;
                    ColorOn(f, b);
                }
                n += 2;
                break;

            default:
                break;
        }
        return n;
    }

    void CloseStyle()
    {
        ResetAttrColor();
    }

private:
    // NOTE: wide characters are not considered presently
    int DoCenterPrint(int y, int w, const std::string& str)
    {
        int x = (w - str.size()) / 2;
        return mvwprintw(win, y, x, "%s", str.c_str());
    }

    // NOTE: very limited usage
    std::string ParseStyle(const std::string& str)
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

                default:
                    ++off;
                    off += OpenStyle(str.substr(off, 3));
                    break;
            }
        }

LABEL_END:
        return str.substr(off, str.size());
    }

private:
    short colorId;

};

#endif
