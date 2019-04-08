#pragma once

#include <ncurses.h>

#include <string>
#include <functional>

#include "Ncurses.h"
#include "Text.h"

class IView
{
public:
    virtual ~IView() { }

    virtual void MoveTo(int x, int y) = 0;
    virtual void Resize(int w, int h) = 0;
    virtual void Refresh() = 0;
    virtual bool NeedRefresh() const { return false; }

    virtual bool InjectKey(int key) = 0;

    virtual void Show(bool show) = 0;
    virtual bool IsShown() const = 0;

    virtual void SetFocus(bool focus) { }
    virtual bool HasFocus() const { return false; }
};

struct Window
{
    Window()
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
        if (boxed) {
            box(win, 0, 0);
        }
    }

    void Cleanup()
    {
        if (win) {
            delwin(win);
            win = nullptr;
        }
    }

    void EnableKeypad(bool enable)
    {
        if (win) {
            keypad(win, enable ? TRUE : FALSE);
        }
    }

    void Refresh()
    {
        if (win) {
            wrefresh(win);
        }
    }

    void EnableEcho(bool enable)
    {
        if (enable) {
            echo();
        } else {
            noecho();
        }
    }

    int Input(int x, int y, bool showCursor = true)
    {
        curs_set(showCursor ? 1 : 0);
        return mvwgetch(win, y, x);
    }

    auto Draw(ncurses::HorizontalAlignment ha, int y, const Text& text)
    {
        const auto x = ha2x(ha, text);
        return Draw(x, y, text);
    }

    auto Draw(int x, ncurses::VerticalAlignment va, const Text& text)
    {
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(ncurses::HorizontalAlignment ha, ncurses::VerticalAlignment va, const Text& text)
    {
        const auto x = ha2x(ha, text);
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(ncurses::HorizontalAlignment ha, int y, const std::string& str)
    {
        Text text{str};
        const auto x = ha2x(ha, text);
        return Draw(x, y, text);
    }

    auto Draw(int x, ncurses::VerticalAlignment va, const std::string& str)
    {
        Text text{str};
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(ncurses::HorizontalAlignment ha, ncurses::VerticalAlignment va, const std::string& str)
    {
        Text text{str};
        const auto x = ha2x(ha, text);
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(int x, int y, const std::string& str)
    {
        // TODO: support more style
        Text text;
        std::string striped;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '^' && i + 1 < str.size()) {
                switch (str[i+1]) {
                    case 'b': {
                        text.EnableAttributes(ncurses::attribute::kBold);
                        ++i;
                        break;
                    }
                    default: {
                        striped += str[i+1];
                    }
                }
            } else {
                striped += str[i];
            }
        }
        text.SetString(striped);
        return Draw(x, y, text);
    }

    void Draw(int x, int y, const Text& text)
    {
        if (!win) {
            return;
        }
        // attr_t lastAttrs = 0;
        // short lastPair = 0;
        // wattr_get(win, &lastAttrs, &lastPair, nullptr);
        const auto attrs = text.Attributes();
        const auto fc = text.ForegroundColor();
        const auto bc = text.BackgroundColor();
        const short colorId = fc * 8 + bc + 1;
        init_pair(colorId, fc, bc);
        wattron(win, attrs | COLOR_PAIR(colorId));
        mvwaddstr(win, y, x, text.String().c_str());
        wattroff(win, attrs | COLOR_PAIR(colorId));
        // wattr_set(win, lastAttrs, lastPair, nullptr);
    }

    void Clear()
    {
        if (!win) {
            return;
        }
        werase(win);
        if (boxed) {
            box(win, 0, 0);
        }
    }

    void MoveTo(int _x, int _y)
    {
        x = _x;
        y = _y;
        if (win) {
            mvwin(win, y, x);
        }
    }

    void Resize(int _w, int _h)
    {
        w = _w;
        h = _h;
        if (win) {
            wresize(win, h, w);
            mvwin(win, y, x);
        }
    }

    void Show(bool show)
    {
        if (show) {
            if (!win) {
                Init(x, y, w, h, boxed);
            }
        } else {
            Cleanup();
        }
        shown = show;
    }

    WINDOW* win = nullptr;
    bool boxed = true;
    bool shown = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    void AttrOn(int attrs)
    {
        if (win) {
            wattron(win, attrs);
        }
    }

    void AttrSet(int attrs)
    {
        if (win) {
            wattrset(win, attrs);
        }
    }

    void AttrOff(int attrs)
    {
        if (win) {
            wattroff(win, attrs);
        }
    }

    void ResetAttrColor()
    {
        if (!win) {
            return;
        }
        init_pair(0, ncurses::color::kWhite, ncurses::color::kBlack);
        wattrset(win, ncurses::attribute::kNormal | COLOR_PAIR(0));
    }

    void ColorOn(int fg, int bg)
    {
        if (!win) {
            return;
        }
        const short colorId = fg * 8 + bg + 1;
        init_pair(colorId, fg, bg);
        wattron(win, COLOR_PAIR(colorId));
    }

    void ColorOff(short colorId)
    {
        if (!win) {
            return;
        }
        wattroff(win, COLOR_PAIR(colorId));
        wattroff(win, COLOR_PAIR(0));
    }

public:
    void SafelyDo(const std::function<void (void)>& fn)
    {
        use_window(win, &WindowCallback, const_cast<void*>(static_cast<const void*>(&fn)));
    }

private:
    static int WindowCallback(WINDOW* _w, void* p)
    {
        using namespace std;
        auto func = static_cast<std::function<void (void)>*>(p);
        (*func)();
        return 0;
    }

private:
    int ha2x(ncurses::HorizontalAlignment ha, const Text& text) const
    {
        int x = 0;
        switch (ha) {
            case ncurses::HorizontalAlignment::kLeft: {
                x = 0;
                break;
            }
            case ncurses::HorizontalAlignment::kCenter: {
                x = (w - text.Width()) / 2;
                break;
            }
            case ncurses::HorizontalAlignment::kRight: {
                x = w - text.Width();
                break;
            }
        }
        return x;
    }

    int va2y(ncurses::VerticalAlignment va) const
    {
        int y = 0;
        switch (va) {
            case ncurses::VerticalAlignment::kTop: {
                y = 0;
                break;
            }
            case ncurses::VerticalAlignment::kCenter: {
                y = h / 2;
                break;
            }
            case ncurses::VerticalAlignment::kBottom: {
                y = h;
                break;
            }
        }
        return y;
    }
};

