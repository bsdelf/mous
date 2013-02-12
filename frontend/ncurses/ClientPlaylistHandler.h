#ifndef CLIENTPLAYLISTHANDLER_H
#define CLIENTPLAYLISTHANDLER_H

#include <functional>
#include <deque>
using namespace std;

#include <scx/Signal.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

class ClientPlaylistHandler
{
    friend class Client;

private:
    ClientPlaylistHandler();
    ~ClientPlaylistHandler();

    void Handle(char* buf, int len);

public:
    // request
    void Switch(int playlist);
    void Select(int playlist, int pos);

    void Play(int playlist, int pos);

    void Append(int playlist, const string& path);
    void Remove(int playlist, int pos);
    void Move(int playlist, int pos, char direct);
    void Clear(int playlist);
    void Sync(int playlist);

    void MoveItem(int playlist, int oldPos, int newPos);

    // server reply
    Signal<void (int)>& SigSwitch()
    {
        return m_SigSwitch;
    }

    Signal<void (int, int)>& SigSelect()
    {
        return m_SigSelect;
    }

    Signal<void (int, bool)>& SigPlay()
    {
        return m_SigPlay;
    }

    Signal<void (int, deque<MediaItem*>&)>& SigAppend()
    {
        return m_SigAppend;
    }

    Signal<void (int, int)>& SigRemove()
    {
        return m_SigRemove;
    }

    Signal<void (int, int, char)>& SigMove()
    {
        return m_SigMove;
    }

    Signal<void (int)>& SigClear()
    {
        return m_SigClear;
    }

private:
    function<char* (char, int)> fnGetPayloadBuffer;
    function<void (void)> fnSendOut;

    Signal<void (int)> m_SigSwitch;
    Signal<void (int, int)> m_SigSelect;
    Signal<void (int, bool)> m_SigPlay;
    Signal<void (int, deque<MediaItem*>&)> m_SigAppend;
    Signal<void (int, int)> m_SigRemove;
    Signal<void (int, int, char)> m_SigMove;
    Signal<void (int)> m_SigClear;
};

#endif
