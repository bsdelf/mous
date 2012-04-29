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
    Session();
    ~Session();

    bool Run(const TcpSocket& socket, MousData* data, int notifyFd);
    void Stop();

private:
    void ThHandleLoop();

private:
    Thread m_Thread;
    TcpSocket m_Socket;
    MousData* m_Data;
    int m_NotifyFd;
};

#endif
