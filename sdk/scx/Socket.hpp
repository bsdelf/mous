#ifndef SCX_SOCKET_HPP
#define SCX_SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>

#include <string>
#include <iostream>
using namespace std;

#ifndef SO_REUSEPORT
#define SCX_SOCKET_SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

namespace scx {

struct SocketOpt
{
    bool reuseAddr;
    bool reusePort;
    bool keepAlive;

    SocketOpt():
        reuseAddr(false),
        reusePort(false),
        keepAlive(true)
    { }
};

class UdpSocket;
class TcpSocket;

class InetAddr
{
friend class UdpSocket;
friend class TcpSocket;

public:
    InetAddr(): 
        m_AddrLen(sizeof(struct sockaddr_in))
    {
        bzero(&m_Addr, sizeof(m_Addr));
        m_Addr.sin_family = AF_INET;
    }

    InetAddr(const std::string& ip, int port): 
        m_AddrLen(sizeof(struct sockaddr_in))
    {
        bzero(&m_Addr, sizeof(m_Addr));
        m_Addr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &m_Addr.sin_addr);
        m_Addr.sin_port = htons(port);
    }

    explicit InetAddr(int port): 
        m_AddrLen(sizeof(struct sockaddr_in))
    {
        bzero(&m_Addr, sizeof(m_Addr));
        m_Addr.sin_family = AF_INET;
        m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_Addr.sin_port = htons(port);
    }

    void Assign(const std::string& ip, int port)
    {
        inet_pton(AF_INET, ip.c_str(), &m_Addr.sin_addr);
        m_Addr.sin_port = htons(port);
    }

    void Assign(int port)
    {
        m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);
        m_Addr.sin_port = htons(port);
    }

    std::string Ip() const
    {
        char buf[INET6_ADDRSTRLEN];
        const char* p = inet_ntop(AF_INET, &m_Addr.sin_addr, buf, sizeof(buf));
        return p != NULL ? p : "";
    }

    int Port() const
    {
        return ntohs(m_Addr.sin_port);
    }

private:
    struct sockaddr_in m_Addr;
    socklen_t m_AddrLen;
};

namespace HowShutdown {
    const int Read = SHUT_RD;
    const int Write = SHUT_WR;
    const int ReadWrite = SHUT_RDWR;
}

#define SCX_COPY_SOCKETCOMMON(Socket)\
public:\
    Socket(const Socket& socket)\
    {                               \
        m_Fd = dup(socket.m_Fd);    \
    }                               \
\
    ~Socket()\
    {               \
        Close();    \
    }               \
\
    Socket& operator=(const Socket& socket)\
    {                               \
        Close();                    \
        m_Fd = dup(socket.m_Fd);    \
        return *this;               \
    }                               \
\
    void Shutdown(int how = HowShutdown::ReadWrite)\
    {                               \
        if (m_Fd != -1)             \
            shutdown(m_Fd, how);    \
    }                               \
\
    void Close()\
    {                       \
        if (m_Fd != -1) {   \
            close(m_Fd);    \
            m_Fd = -1;      \
        }                   \
    }                       \
\
    int Fd() const\
    {                   \
        return m_Fd;    \
    }                   \
\
    InetAddr& Addr()\
    {                       \
        return m_InetAddr;  \
    }                       \
\
    const InetAddr& Addr() const\
    {                       \
        return m_InetAddr;  \
    }                       \
\
    void SetOption(const SocketOpt& opt)\
    {                                                                       \
        int ret;                                                            \
        int on;                                                             \
        on = opt.reuseAddr;                                                 \
        ret = setsockopt(m_Fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));  \
        on = opt.reusePort;                                                 \
        ret = setsockopt(m_Fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));  \
        on = opt.keepAlive;                                                 \
        ret = setsockopt(m_Fd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));  \
    }                                                                       \
\
    bool Bind(const std::string& ip, int port)\
    {                                                                                       \
        m_InetAddr.Assign(ip, port);                                                        \
        return bind(m_Fd, (struct sockaddr*)&m_InetAddr.m_Addr, m_InetAddr.m_AddrLen) == 0; \
    }                                                                                       \
\
    bool Bind(int port)\
    {                                                                                       \
        m_InetAddr.Assign(port);                                                            \
        return bind(m_Fd, (struct sockaddr*)&m_InetAddr.m_Addr, m_InetAddr.m_AddrLen) == 0; \
    }                                                                                       \
\
    bool Bind(const InetAddr& addr)\
    {                                                                                       \
        m_InetAddr = addr;                                                                  \
        return bind(m_Fd, (struct sockaddr*)&m_InetAddr.m_Addr, m_InetAddr.m_AddrLen) == 0; \
    }                                                                                       \
\
private:\
    int m_Fd;\
    InetAddr m_InetAddr

class UdpSocket
{
    SCX_COPY_SOCKETCOMMON(UdpSocket);

public:
    UdpSocket()
    {
        m_Fd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    ssize_t RecvFrom(void* buf, size_t len, InetAddr& iadr, int flags = 0) const
    {
        return recvfrom(m_Fd, buf, len, flags, (struct sockaddr*)&iadr.m_Addr, &iadr.m_AddrLen);
    }
    
    ssize_t SendTo(const void* buf, size_t len, InetAddr& iadr, int flags = 0) const
    {
        return sendto(m_Fd, buf, len, flags, (struct sockaddr*)&iadr.m_Addr, iadr.m_AddrLen);
    }
    
};

class TcpSocket
{
    SCX_COPY_SOCKETCOMMON(TcpSocket);

public:
    TcpSocket()
    {
        m_Fd = socket(AF_INET, SOCK_STREAM, 0);
    }

    bool Listen(int backlog = -1) const
    {
        return listen(m_Fd, backlog) == 0;
    }

    bool Accept(TcpSocket& clientSocket) const
    {
        clientSocket.Close();
        InetAddr& clientAddr = clientSocket.Addr();
        int connFd = accept(m_Fd, (struct sockaddr*)&clientAddr.m_Addr, &clientAddr.m_AddrLen);
        clientSocket.m_Fd = connFd;
        return connFd != -1;
    }

    bool Connect(const InetAddr& iadr) const
    {
        return connect(m_Fd, (struct sockaddr*)&iadr.m_Addr, iadr.m_AddrLen) == 0;
    }

    ssize_t Recv(void* buf, size_t len, int flags = 0) const
    {
        return recv(m_Fd, buf, len, flags);
    }

    ssize_t Send(const void* buf, size_t len, int flags = 0) const
    {
        return send(m_Fd, buf, len, flags);
    }

    bool RecvN(void* buf, size_t rest, int flags = 0) const
    {
        while (rest > 0) {
            ssize_t n = recv(m_Fd, buf, rest, flags);
            if (n > 0) {
                rest -= n;
                buf = (char*)buf + n;
            } else {
                break;
            }
        }
        return rest == 0;
    }

    bool SendN(const void* buf, size_t rest, int flags = 0) const
    {
        while (rest > 0) {
            ssize_t n = send(m_Fd, buf, rest, flags);
            if (n > 0) {
                rest -= n;
                buf = (char*)buf + n;
            } else {
                break;
            }
        }
        return rest == 0;
    }
};

class UnixSocket
{

};

}

#undef SCX_COPY_SOCKETCOMMON

#ifdef SCX_SOCKET_SO_REUSEPORT
#undef SO_REUSEPORT
#undef SCX_SOCKET_SO_REUSEPORT
#endif

#endif
