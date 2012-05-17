#ifndef SERVER_H
#define SERVER_H

#include <set>
#include <fstream>
using namespace std;

#include <scx/Socket.hpp>
#include <scx/Thread.hpp>
using namespace scx;

struct MousData;
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
    MousData* m_Data;
    TcpSocket m_Socket;
    int m_PipeFd[2];
    set<Session*> m_SessionSet;
    typedef set<Session*>::iterator SessionSetIter;

    fstream log;
};

#endif
