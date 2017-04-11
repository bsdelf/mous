#pragma once

#include <set>
#include <thread>
#include <memory>
using namespace std;

#include <scx/Socket.hpp>
using namespace scx;

struct ServerContext;
class Session;

class Server
{
public:
    Server();
    ~Server();

    int Exec();

private:
    void StopService();
    void OpenSession(TcpSocket&);
    void CloseSession(Session*);

private:
    unique_ptr<ServerContext> m_Context;
    TcpSocket m_Socket;
    int m_PipeFd[2];
    set<Session*> m_SessionSet;
};

