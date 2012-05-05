#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <ncurses.h>
#include <panel.h>

#include <scx/Signal.hpp>
using namespace scx;

#include <util/MediaItem.h>
#include <util/Playlist.h>
using namespace mous;

#include "IView.h"

class PlaylistView: public IView
{
public:
    PlaylistView();
    ~PlaylistView();

    void OnResize(int x, int y, int w, int h);

    void Refresh();
    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

    void SetFocus(bool focus);
    bool HasFocus() const;

public:
    void SetIndex(int i);
    int GetIndex() const;

    Signal<void (bool)> SigSwitchPlaylist;

private:
    bool m_Focused;
    int m_Index;
    Window d;
    Playlist<MediaItem*> m_List;
};

#endif
