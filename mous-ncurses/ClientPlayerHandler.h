#ifndef CLIENTPLAYERHANDLER_H
#define CLIENTPLAYERHANDLER_H

#include <scx/Signal.hpp>
#include <scx/Function.hpp>
using namespace scx;

#include "Protocol.h"

class ClientPlayerHandler
{
    friend class Client;

public:
    ClientPlayerHandler()
    {
    }

    ~ClientPlayerHandler()
    {
    }

    void Handle(char* buf, int len)
    {
        using namespace Protocol;

        if (len < (int)sizeof(char))
            return;

        char op = Op::Player::None;

        BufObj bufObj(buf);
        bufObj >> op;
        switch (op) {
            case Op::Player::Status:
                break;

            default:
                break;
        }
    }

public:
    void Play()
    {
    }

    void Pause()
    {
    }

    void Resume()
    {
    }

    void Stop()
    {
    }

    void Next()
    {
    }

    void Previous()
    {
    }

private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void ()> m_SigStatusChanged;
};

#endif
