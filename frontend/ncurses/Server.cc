#include "AppEnv.h"
#include "Server.h"
#include "Session.h"
#include "ServerContext.h"

#include <unistd.h>
#include <sys/select.h>

#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

Server::Server()
{
    m_Context = new ServerContext;
    pipe(m_PipeFd);
}

Server::~Server()
{
    delete m_Context;
    close(m_PipeFd[0]);
    close(m_PipeFd[1]);
    m_Socket.Close();
}

int Server::Exec()
{
    if (!m_Context->Init())
        return 1;

    const AppEnv* env = GlobalAppEnv::Instance();
    if (env == nullptr)
        return 2;

    m_Context->Restore();

    SocketOpt opt;
    opt.reuseAddr = true;
    opt.reusePort = true;
    opt.keepAlive = true;
    m_Socket.SetOption(opt);

    if (!m_Socket.Bind(env->serverIp, env->serverPort))
        return 1;
    if (!m_Socket.Listen())
        return 1;

    const int maxfd = std::max(m_Socket.Fd(), m_PipeFd[0]) + 1;
    fd_set rset;
    FD_ZERO(&rset);

    TcpSocket clientSocket;
    for (bool stopService = false; !stopService; ) {
        FD_SET(m_Socket.Fd(), &rset);
        FD_SET(m_PipeFd[0], &rset);
        if (select(maxfd, &rset, nullptr, nullptr, nullptr) <= 0)
            break;

        // open session
        if (FD_ISSET(m_Socket.Fd(), &rset)) {
            if (!m_Socket.Accept(clientSocket))
                break;
            OpenSession(clientSocket);
        }

        // close session / stop service
        if (FD_ISSET(m_PipeFd[0], &rset)) {
            char cmd;
            read(m_PipeFd[0], &cmd, 1);
            switch (cmd) {
                case 'q':
                {
                    Session::ptr_t ptr = 0;
                    read(m_PipeFd[0], &ptr, sizeof(ptr));
                    CloseSession(reinterpret_cast<Session*>(ptr));
                }
                    break;

                case 'Q':
                {
                    StopService();
                    stopService = true;
                }
                    break;

                default:
                    break;
            }
        }
    }

    m_Context->Dump();
    m_Context->Cleanup();

    return 0;
}

void Server::StopService()
{
    m_Socket.Shutdown();

    for (auto session: m_SessionSet) {
        session->Stop();
        delete session;
    }
    m_SessionSet.clear();

    cout << "StopService()" << endl;
}

void Server::OpenSession(TcpSocket& clientSocket)
{
    Session* session = new Session(m_Context);
    m_SessionSet.insert(session);
    session->Run(clientSocket, m_PipeFd[1]);

    cout << "OpenSession()" << endl;
}

void Server::CloseSession(Session* session)
{
    auto iter = m_SessionSet.find(session);
    if (iter != m_SessionSet.end()) {
        m_SessionSet.erase(iter);
        session->Stop();
        delete session;
    }

    cout << "CloseSession()" << endl;
}
