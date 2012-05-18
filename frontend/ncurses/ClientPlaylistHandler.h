#ifndef CLIENTPLAYLISTHANDLER_H
#define CLIENTPLAYLISTHANDLER_H

#include <scx/Signal.hpp>
#include <scx/Function.hpp>
using namespace scx;

#include <deque>
using namespace std;

#include <util/MediaItem.h>
using namespace mous;

class ClientPlaylistHandler
{
    friend class Client;

public:
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
    void Clear(int playlist);
    void Sync(int playlist);

    void MoveItem(int playlist, int oldPos, int newPos);

    // server reply
    const Signal<void (int)>& SigSwitch() const
    {
        return m_SigSwitch;
    }

    const Signal<void (int, int)>& SigSelect() const
    {
        return m_SigSelect;
    }

    const Signal<void (int, bool)>& SigPlay() const
    {
        return m_SigPlay;
    }

    const Signal<void (int, deque<MediaItem*>&)>& SigAppend() const
    {
        return m_SigAppend;
    }

    const Signal<void (int, int)>& SigRemove() const
    {
        return m_SigRemove;
    }

    const Signal<void (int)>& SigClear() const
    {
        return m_SigClear;
    }

private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void (int)> m_SigSwitch;
    Signal<void (int, int)> m_SigSelect;
    Signal<void (int, bool)> m_SigPlay;
    Signal<void (int, deque<MediaItem*>&)> m_SigAppend;
    Signal<void (int, int)> m_SigRemove;
    Signal<void (int)> m_SigClear;
};

#endif
