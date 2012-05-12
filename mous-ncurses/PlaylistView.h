#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <deque>
using namespace std;

#include <scx/Signal.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

#include "IView.h"

class PlaylistView: public IView
{
public:
    PlaylistView();
    ~PlaylistView();

    void Refresh();
    void MoveTo(int x, int y);
    void Resize(int w, int h);

    bool InjectKey(int key);

    void Show(bool show);
    bool IsShown() const;

    void SetFocus(bool focus);
    bool HasFocus() const;

public:
    int Index() const;
    void SetIndex(int i);

    void Append(deque<MediaItem*>&);

public:
    Signal<void (bool)> SigSwitchPlaylist;

private:
    Window d;
    bool m_Focused;
    int m_Index;
    int m_ItemBegin;
    int m_ItemSelected;
    std::string m_Title;
    deque<MediaItem*> m_List;
};

#endif
