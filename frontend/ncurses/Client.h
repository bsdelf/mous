#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
using namespace std;

#include <scx/Mutex.hpp>
#include <scx/Function.hpp>
#include <scx/Signal.hpp>
#include <scx/Socket.hpp>
#include <scx/Thread.hpp>
using namespace scx;

#include "ClientPlayerHandler.h"
#include "ClientPlaylistHandler.h"

class Client
{
    friend class PayloadSender;

public:
    Client();
    ~Client();

    bool Run();
    void Stop();

    void SetConnectMaxRetry(int max);
    void SetConnectRetryInterval(int ms);

    void StopService();

    ClientPlayerHandler& PlayerHandler();
    ClientPlaylistHandler& PlaylistHandler();

    const Signal<void ()>& SigTryConnect() const;
    const Signal<void ()>& SigConnected() const;

private:
    void ThRecvLoop(const string&, int);

    char* GetPayloadBuffer(char, int);
    inline void SendOut();

private:
    Thread m_RecvThread;

    int m_ConnectMaxRetry;
    int m_ConnectRetryInterval;
    bool m_ConnectStopRetry;

    TcpSocket m_Socket;
    Mutex m_SendOutBufMutex;
    vector<char> m_SendOutBuf;

    ClientPlayerHandler m_PlayerHandler;
    ClientPlaylistHandler m_PlaylistHandler;

    Signal<void ()> m_SigTryConnect;
    Signal<void ()> m_SigConnected;
};

#endif
