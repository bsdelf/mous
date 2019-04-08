#pragma once

#include <string>
// #include <functional>

#include <ncurses.h>

namespace ncurses {

namespace color {
    const int kBlack = COLOR_BLACK;
    const int kRed = COLOR_RED;
    const int kGreen = COLOR_GREEN;
    const int kYellow = COLOR_YELLOW;
    const int kBlue = COLOR_BLUE;
    const int kMagenta = COLOR_MAGENTA;
    const int kCyan = COLOR_CYAN;
    const int kWhite = COLOR_WHITE;
}

namespace attribute {
    const int kNormal = A_NORMAL;
    const int kStandout = A_STANDOUT;
    const int kUnderline = A_UNDERLINE;
    const int kReverse = A_REVERSE;
    const int kBlink = A_BLINK;
    const int kDim = A_DIM;
    const int kBold = A_BOLD;
}

enum class HorizontalAlignment {
    kLeft,
    kCenter,
    kRight
};

enum class VerticalAlignment {
    kTop,
    kCenter,
    kBottom
};

class Text {
public:
    Text() = default;
    Text(const Text&) = default;
    Text(Text&&) = default;
    Text(const std::string& str): str_(str) {}
    Text(std::string&& str): str_(std::move(str)) {}

    // TODO: non-ascii characters
    const size_t Width() const {
        return str_.size();
    }

    const std::string& String() const {
        return str_;
    }

    int Attributes() const {
        return attrs_;
    }

    int ForegroundColor() const {
        return fc_;
    }

    int BackgroundColor() const {
        return bc_;
    }

    Text& SetString(const std::string& str) {
        str_ = str;
        return *this;
    }

    Text& SetString(std::string&& str) {
        str_ = std::move(str);
        return *this;
    }

    Text& SetAttributes(int attrs) {
        attrs_ = attrs;
        return *this;
    }

    Text& EnableAttributes(int attr) {
        attrs_ |= attr;
        return *this;
    }

    Text& SetForegroundColor(int fc) {
        fc_ = fc;
        return *this;
    }

    Text& SetBackgroundColor(int bc) {
        bc_ = bc;
        return *this;
    }

    Text& SetColor(int fc, int bc) {
        fc_ = fc;
        bc_ = bc;
        return *this;
    }

private:
    std::string str_;
    int attrs_ = A_NORMAL;
    int fc_ = COLOR_WHITE;
    int bc_ = COLOR_BLACK;
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

    auto Draw(HorizontalAlignment ha, int y, const Text& text)
    {
        const auto x = ha2x(ha, text);
        return Draw(x, y, text);
    }

    auto Draw(int x, VerticalAlignment va, const Text& text)
    {
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(HorizontalAlignment ha, VerticalAlignment va, const Text& text)
    {
        const auto x = ha2x(ha, text);
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(HorizontalAlignment ha, int y, const std::string& str)
    {
        Text text{str};
        const auto x = ha2x(ha, text);
        return Draw(x, y, text);
    }

    auto Draw(int x, VerticalAlignment va, const std::string& str)
    {
        Text text{str};
        const auto y = va2y(va);
        return Draw(x, y, text);
    }

    auto Draw(HorizontalAlignment ha, VerticalAlignment va, const std::string& str)
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
                        text.EnableAttributes(attribute::kBold);
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
        init_pair(0, color::kWhite, color::kBlack);
        wattrset(win, attribute::kNormal | COLOR_PAIR(0));
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

// public:
//     void SafelyDo(const std::function<void (void)>& fn)
//     {
//         use_window(win, &WindowCallback, const_cast<void*>(static_cast<const void*>(&fn)));
//     }

// private:
//     static int WindowCallback(WINDOW* _w, void* p)
//     {
//         using namespace std;
//         auto func = static_cast<std::function<void (void)>*>(p);
//         (*func)();
//         return 0;
//     }

private:
    int ha2x(HorizontalAlignment ha, const Text& text) const
    {
        int x = 0;
        switch (ha) {
            case HorizontalAlignment::kLeft: {
                x = 0;
                break;
            }
            case HorizontalAlignment::kCenter: {
                x = (w - text.Width()) / 2;
                break;
            }
            case HorizontalAlignment::kRight: {
                x = w - text.Width();
                break;
            }
        }
        return x;
    }

    int va2y(VerticalAlignment va) const
    {
        int y = 0;
        switch (va) {
            case VerticalAlignment::kTop: {
                y = 0;
                break;
            }
            case VerticalAlignment::kCenter: {
                y = h / 2;
                break;
            }
            case VerticalAlignment::kBottom: {
                y = h;
                break;
            }
        }
        return y;
    }
};

}