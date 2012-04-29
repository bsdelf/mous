#include "Server.h"
#include "Config.h"
#include "Session.h"

#include <unistd.h>
#include <sys/select.h>

#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/ConfigFile.hpp>

Server::Server()
{
    pipe(m_PipeFd);
}

Server::~Server()
{
    close(m_PipeFd[0]);
    close(m_PipeFd[1]);
}

bool Server::Exec()
{
    string serverIp;
    int serverPort = -1;
    {
        ConfigFile conf;
        if (!conf.Load(Config::ConfigPath))
            return false;

        serverIp = conf[Config::ServerIp];
        serverPort = StrToNum<int>(conf[Config::ServerPort]);
    }

    SocketOpt opt;
    opt.reuseAddr = true;
    opt.reusePort = true;
    opt.keepAlive = true;

    m_Socket.SetOption(opt);
    if (!m_Socket.Bind(serverIp, serverPort))
        return false;
    if (!m_Socket.Listen())
        return false;

    int maxfd = std::max(m_Socket.GetFd(), m_PipeFd[0]) + 1;
    struct fd_set rset;
    FD_ZERO(&rset);

    TcpSocket clientSocket;
    bool stopService = false;
    while (!stopService) {
        FD_SET(m_Socket.GetFd(), &rset);
        FD_SET(m_PipeFd[0], &rset);
        if (select(maxfd, &rset, NULL, NULL, NULL) <= 0)
            break;

        if (!m_Socket.Accept(clientSocket))
            break;

        // open session
        if (FD_ISSET(m_Socket.GetFd(), &rset)) {
            OpenSession(clientSocket);
        }

        // close session / stop service
        if (FD_ISSET(m_PipeFd[0], &rset)) {
            char cmd;
            read(m_PipeFd[0], &cmd, 1);
            switch (cmd) {
                case 'q':
                {
                    unsigned long ptr = 0;
                    read(m_PipeFd[0], &ptr, sizeof(ptr));
                    CloseSession(reinterpret_cast<Session*>(ptr));
                }
                    break;

                case 'Q':
                    StopService();
                    stopService = true;
                    break;

                default:
                    break;
            }
        }
    }

    return true;
}

void Server::StopService()
{
    m_Socket.Shutdown();

    SessionSetIter iter = m_SessionSet.begin();
    SessionSetIter end = m_SessionSet.end();
    for (; iter != end; ++iter) {
        (*iter)->Stop();
        delete *iter;
    }
    m_SessionSet.clear();

    cout << "StopService()" << endl;
}

void Server::OpenSession(TcpSocket& clientSocket)
{
    Session* session = new Session();
    m_SessionSet.insert(session);
    session->Run(clientSocket, &m_Data, m_PipeFd[1]);

    cout << "OpenSession()" << endl;
}

void Server::CloseSession(Session* session)
{
    SessionSetIter iter = m_SessionSet.find(session);
    if (iter != m_SessionSet.end()) {
        m_SessionSet.erase(iter);
        session->Stop();
        delete session;
    }

    cout << "CloseSession()" << endl;
}
