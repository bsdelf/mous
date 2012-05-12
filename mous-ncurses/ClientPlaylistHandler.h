#ifndef CLIENTPLAYLISTHANDLER_H
#define CLIENTPLAYLISTHANDLER_H

#include <scx/Signal.hpp>
#include <scx/Function.hpp>
using namespace scx;

#include <deque>
#include <iostream>
using namespace std;

#include <util/MediaItem.h>
using namespace mous;

#include "Protocol.h"

#define SEND_PACKET(stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();       \
    char* buf = fnGetPayloadBuffer(                         \
            Protocol::Op::Group::Playlist, payloadSize);    \
    BufObj(buf) stream;                                     \
}\
    fnSendOut()

class ClientPlaylistHandler
{
    friend class Client;

public:
    ClientPlaylistHandler()
    {
    }

    ~ClientPlaylistHandler()
    {
    }

    void Handle(char* buf, int len)
    {
        using namespace Protocol;

        if (len < (int)sizeof(char))
            return;

        char op = Op::Playlist::None;

        BufObj bufObj(buf);
        bufObj >> op;
        switch (op) {
            case Op::Playlist::Append:
            {
                char playlist;
                int32_t count;
                bufObj >> playlist >> count;

                deque<MediaItem*> list;
                for (int i = 0; i < count; ++i) {
                    MediaItem* item = new MediaItem;
                    *item << bufObj;
                    list.push_back(item);
                }

                m_SigAppend(playlist, list);
            }
                break;

            case Op::Playlist::Remove:
            {
                char playlist;
                int32_t pos;
                bufObj >> playlist >> pos;
                m_SigRemove(playlist, pos);
            }
                break;

            default:
                break;
        }
    }

public:
    const Signal<void (int, deque<MediaItem*>&)>& SigAppend() const
    {
        return m_SigAppend;
    }

    const Signal<void (int, int)>& SigRemove() const
    {
        return m_SigRemove;
    }

    void Append(int playlist, const string& path)
    {
        using namespace Protocol::Op;
        SEND_PACKET(<< (char)Playlist::Append << (char)playlist << path);
    }

    void Remove(int playlist, int pos)
    {
        using namespace Protocol::Op;
        SEND_PACKET(<< (char)Playlist::Remove << (char)playlist << (int32_t)pos);
    }

    void Clear(int playlist)
    {
        using namespace Protocol;
    }

    void Sync()
    {
        using namespace Protocol;
    }

    void MoveItem(int playlist, int oldPos, int newPos)
    {
    }

#undef SEND_PACKET
    
private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void (int, deque<MediaItem*>&)> m_SigAppend;
    Signal<void (int, int)> m_SigRemove;
};

#endif
