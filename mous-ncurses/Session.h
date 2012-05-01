#ifndef SESSION_H
#define SESSION_H

#include <scx/Thread.hpp>
#include <scx/Socket.hpp>
#include <scx/Mutex.hpp>
using namespace scx;

struct MousData;

class Session
{
public:
    typedef unsigned long ptr_t;

public:
    Session();
    ~Session();

    bool Run(const TcpSocket& socket, MousData* data, int notifyFd);
    void Stop();

private:
    void ThRecvLoop();
    void HandleApp(char*, int);
    void HandlePlayer(char*, int);
    void HandlePlaylist(char*, int);

private:
    Thread m_RecvThread;
    TcpSocket m_Socket;
    MousData* m_Data;
    int m_NotifyFd;
    bool m_GotReqStopService;
};

#endif
