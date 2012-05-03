#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include <ncurses.h>
#include <panel.h>

class StatusView
{
public:
    StatusView();
    ~StatusView();

public:
    bool InjectKey(int key);
};

#endif
