#ifndef CLIENT_H
#define CLIENT_H

#include <scx/Socket.hpp>
using namespace scx;

class Client
{
public:
    Client();
    ~Client();

    void Start();
    void Stop();

private:
    TcpSocket m_Socket;
};

#endif
