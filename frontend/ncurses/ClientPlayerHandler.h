#ifndef CLIENTPLAYERHANDLER_H
#define CLIENTPLAYERHANDLER_H

#include <scx/Signal.hpp>
#include <scx/Function.hpp>
using namespace scx;

class ClientPlayerHandler
{
    friend class Client;

public:
    ClientPlayerHandler();
    ~ClientPlayerHandler();

    void Handle(char* buf, int len);

public:
    void Play();
    void Pause();
    void Stop();

    void Next();
    void Previous();

    void VolumeUp();
    void VolumeDown();

private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void ()> m_SigStatusChanged;
};

#endif
