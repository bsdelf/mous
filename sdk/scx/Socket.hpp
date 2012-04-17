#ifndef SCX_SOCKET_HPP
#define SCX_SOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

#include <string>

namespace scx {

struct SocketOpt
{
    bool reuseAddr;
    bool reusePort;
    bool keepAlive;

    SocketOpt():
    reuseAddr(true),
    reusePort(true),
    keepAlive(true) {}
};

class UdpSocket;
class TcpSocket;

class InetAddr
{
friend class UdpSocket;
friend class TcpSocket;

public:
    InetAddr(): 
        mAddrLen(sizeof(sockaddr_in))
    {
        bzero(&mAddr, sizeof(mAddr));
        mAddr.sin_family = AF_INET;
    }

    InetAddr(const std::string& ip, int port): 
        mAddrLen(sizeof(sockaddr_in))
    {
        bzero(&mAddr, sizeof(mAddr));
        mAddr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &mAddr.sin_addr);
        mAddr.sin_port = htons(port);
    }

    explicit InetAddr(int port): 
        mAddrLen(sizeof(sockaddr_in))
    {
        bzero(&mAddr, sizeof(mAddr));
        mAddr.sin_family = AF_INET;
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mAddr.sin_port = htons(port);
    }

    void Assign(const std::string& ip, int port)
    {
        inet_pton(AF_INET, ip.c_str(), &mAddr.sin_addr);
        mAddr.sin_port = htons(port);
    }

    void Assign(int port)
    {
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        mAddr.sin_port = htons(port);
    }

    std::string GetIp() const
    {
        char buf[INET_ADDRSTRLEN];
        const char* p = inet_ntop(AF_INET, &mAddr.sin_addr, buf, sizeof(buf));
        return p != NULL ? std::string(p) : "";
    }

    int GetPort() const
    {
        return ntohs(mAddr.sin_port);
    }

private:
    sockaddr_in mAddr;
    socklen_t mAddrLen;
};

#define SCX_SOCKET_COPY_SOCKET_COMMON(ClassName)           \
public:\
    ~ClassName()            \
    {                       \
        if (mFd != -1)      \
            close(mFd);     \
    }                       \
    \
    int GetFd() const       \
    {                       \
        return mFd;         \
    }                       \
    \
    InetAddr& GetAddr()     \
    {                       \
        return mInetAddr;   \
    }                       \
    \
    void SetOption(const SocketOpt& opt) {                                      \
        int ret;                                                                \
        int on;                                                                 \
        on = opt.reuseAddr;                                                     \
        ret = setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));       \
        on = opt.reusePort;                                                     \
        ret = setsockopt(mFd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));       \
        on = opt.keepAlive;                                                     \
        ret = setsockopt(mFd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));       \
    }                                                                           \
    \
    int Bind(const std::string& ip, int port) {                             \
        mInetAddr.Assign(ip, port);                                         \
        return bind(mFd, (sockaddr*)&mInetAddr.mAddr, mInetAddr.mAddrLen);  \
    }                                                                       \
    \
    int Bind(int port) {                                                    \
        mInetAddr.Assign(port);                                             \
        return bind(mFd, (sockaddr*)&mInetAddr.mAddr, mInetAddr.mAddrLen);  \
    }                                                                       \
    \
    int Bind(const InetAddr& addr) {                                        \
        mInetAddr = addr;                                                   \
        return bind(mFd, (sockaddr*)&mInetAddr.mAddr, mInetAddr.mAddrLen);  \
    }                                                                       \
    \
private:\
    int mFd;                \
    InetAddr mInetAddr;     \

class UdpSocket
{
    SCX_SOCKET_COPY_SOCKET_COMMON(UdpSocket);

public:
    UdpSocket()
    {
        mFd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    ssize_t RecvFrom(void* buf, size_t len, InetAddr& iadr, int flags = 0)
    {
        return recvfrom(mFd, buf, len, flags, (sockaddr*)&iadr.mAddr, &iadr.mAddrLen);
    }
    
    ssize_t SendTo(const void* buf, size_t len, InetAddr& iadr, int flags = 0) const
    {
        return sendto(mFd, buf, len, flags, (sockaddr*)&iadr.mAddr, iadr.mAddrLen);
    }
    
};

class TcpSocket
{
    SCX_SOCKET_COPY_SOCKET_COMMON(TcpSocket);

public:
    TcpSocket()
    {
        mFd = socket(AF_INET, SOCK_STREAM, 0);
    }

    int Listen(int backlog) const
    {
        return listen(mFd, backlog);
    }

    void Accept(TcpSocket& clientSocket) const
    {
        InetAddr& clientAddr = clientSocket.GetAddr();
        int connFd = accept(mFd, (sockaddr*)&clientAddr.mAddr, &clientAddr.mAddrLen);
        clientSocket.mFd = connFd;
    }

    int Connect(const InetAddr& iadr) const
    {
        return connect(mFd, (sockaddr*)&iadr.mAddr, iadr.mAddrLen);
    }

    ssize_t Recv(void* buf, size_t len, int flags = 0) const
    {
        return recv(mFd, buf, len, flags);
    }

    ssize_t Send(const void* buf, size_t len, int flags = 0) const
    {
        return send(mFd, buf, len, flags);
    }
};

#undef SCX_SOCKET_COPY_SOCKET_COMMON

class UnixSocket
{

};


}
#endif
