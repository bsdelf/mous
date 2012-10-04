#include <iostream>
using namespace std;

#include <X11/Xlib.h>
#include <X11/keysymdef.h>

/*
 * http://tronche.com/gui/x/xlib/input/XGrabKey.html
 * http://wiki.debian.org/Keyboard/MultimediaKeys
 * http://subforge.org/projects/subtle/wiki/Grabs
 * https://bugreports.qt-project.org/secure/attachment/23178/example1.cpp
 */

void GrabKeys(Display* display) 
{ 
    int i; 
    int min, max; 
    int screen; 
    unsigned int modifier = 0; 
    screen = DefaultScreen(display);

    KeyCode key; 
    key = XKeysymToKeycode(display, XStringToKeysym("XF86AudioPlay")); 
    XGrabKey(display, key, modifier, RootWindow(display, screen), 
              True, GrabModeAsync, GrabModeAsync); 

    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress && event.xkey.keycode == key) {
            cout << "!!" << endl;
        }
    }

    XAllowEvents(display, AsyncKeyboard, CurrentTime);
    XUngrabKey(display, key, AnyModifier, RootWindow(display, screen));
    XSync(display, False);
}

int main()
{
    Display* display = XOpenDisplay(NULL);
    if (display == NULL)
        return 1;

    GrabKeys(display);

    return XCloseDisplay(display);
}
