#ifndef PLAYLISTVIEW_H
#define PLAYLISTVIEW_H

#include <deque>
using namespace std;

#include <scx/Signal.hpp>
#include <scx/Mutex.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

#include "IView.h"
#include "ClientPlaylistHandler.h"

class PlaylistView: public IView
{
public:
    PlaylistView();
    ~PlaylistView();

    void Refresh();
    bool NeedRefresh() const;

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

    void SetPlaylistHandle(ClientPlaylistHandler* handler);

private:
    void ScrollUp();
    void ScrollDown();

private:
    void ReqSelect();
    void ReqPlay(int);
    void ReqRemove(int);
    void ReqClear();

    void SlotSelect(int, int);
    void SlotPlay(int, bool);
    void SlotAppend(int, deque<MediaItem*>&);
    void SlotRemove(int, int);
    void SlotClear(int);

public:
    Signal<void (bool)> SigSwitchPlaylist;

private:
    Window d;

    int m_NeedRefresh;
    mutable Mutex m_NeedRefreshMutex;

    bool m_Focused;
    int m_Index;
    int m_ItemBegin;
    int m_ItemSelected;
    std::string m_Title;
    deque<MediaItem*> m_List;
    bool m_WaitReply;

    ClientPlaylistHandler* m_PlaylistHandler;
};

#endif
