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

    void SetConnectMaxRetry(int max);
    void SetConnectRetryInterval(int ms);

public:
    void StopService();

    void PlayerPlay(const string& path);
    void PlayerStop();
    void PlayerPause();
    void PlayerResume();
    void PlayerSeek(uint64_t ms);

    Signal<void (uint64_t)> SigPlayerTotalMs;
    Signal<void (uint64_t)> SigPlayerCurrentMs;

private:
    void ThRecvLoop(const string&, int);
    void HandlePlayer(char*, int);
    void HandlePlaylist(char*, int);

    char* GetPayloadBuffer(char, int);
    void SendOut();

private:
    Thread m_RecvThread;

    int m_ConnectMaxRetry;
    int m_ConnectRetryInterval;
    bool m_ConnectStopRetry;

    TcpSocket m_Socket;
    vector<char> m_SendOutBuf;
};

#endif
