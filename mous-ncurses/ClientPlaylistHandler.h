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

            default:
                break;
        }
    }

public:
    const Signal<void (int, deque<MediaItem*>&)>& SigAppend() const
    {
        return m_SigAppend;
    }

#define SEND_PACKET(group, stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();   \
    char* buf = fnGetPayloadBuffer(group, payloadSize); \
    BufObj(buf) stream;                                 \
}\
    fnSendOut()

    void Append(int playlist, const string& path)
    {
        using namespace Protocol::Op;
        SEND_PACKET(Group::Playlist, 
                << (char)Playlist::Append << (char)playlist << path);
    }

    void MoveItem(int playlist, int oldPos, int newPos)
    {
    }

    void RemoveItem(int playlist, int pos)
    {
    }

    void Clear(int playlist)
    {
        using namespace Protocol;
    }

    void SyncAll()
    {
        using namespace Protocol;
    }

#undef SEND_PACKET
    
private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void (int, deque<MediaItem*>&)> m_SigAppend;
};

#endif
