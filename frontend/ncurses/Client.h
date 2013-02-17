#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
#include <mutex>
#include <thread>
using namespace std;

#include <scx/Signal.hpp>
#include <scx/Socket.hpp>
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

    Signal<void ()>& SigTryConnect();
    Signal<void ()>& SigConnected();

    Signal<void (const std::vector<std::string>&)>& SigSuffixes()
    {
        return m_SigSuffixes;
    }

private:
    void ThRecvLoop(const string&, int);

    char* GetPayloadBuffer(char, int);
    inline void SendOut();

private:
    thread m_RecvThread;

    int m_ConnectMaxRetry;
    int m_ConnectRetryInterval;
    bool m_ConnectStopRetry;

    TcpSocket m_Socket;
    mutex m_SendOutBufMutex;
    vector<char> m_SendOutBuf;

    ClientPlayerHandler m_PlayerHandler;
    ClientPlaylistHandler m_PlaylistHandler;

    Signal<void ()> m_SigTryConnect;
    Signal<void ()> m_SigConnected;
    Signal<void (const std::vector<std::string>&)> m_SigSuffixes;
};

#endif
