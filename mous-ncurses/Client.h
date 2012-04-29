#ifndef CLIENT_H
#define CLIENT_H

#include <vector>
#include <string>
using namespace std;

#include <scx/Function.hpp>
#include <scx/Signal.hpp>
#include <scx/Socket.hpp>
#include <scx/Thread.hpp>
using namespace scx;

class Client
{
public:
    Client();
    ~Client();

    bool Run(const string& ip, int port);
    void Stop();

public:
    void PlayerPlay(const string& path);
    void PlayerStop();
    void PlayerPause();
    void PlayerResume();
    void PlayerSeek(uint64_t ms);

    Signal<void (uint64_t)> SigPlayerTotalMs;
    Signal<void (uint64_t)> SigPlayerCurrentMs;

private:
    void ThRecvLoop();
    void HandlePlayer(char* buf, int size);
    void HandlePlaylist(char* buf, int size);

    char* GetSendOutBuffer(int size);

private:
    Thread m_ListenThread;
    TcpSocket m_Socket;

    vector<char> m_SendOutBuf;
};

#endif
