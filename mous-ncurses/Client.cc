#include "Client.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <vector>

#include "Protocol.h"
using namespace Protocol;

const int PAYLOADBUF_MAX_KEEP = 1024;
const int SENDOUTBUF_MAX_KEEP = 256;

Client::Client():
    m_ConnectMaxRetry(25),
    m_ConnectRetryInterval(200)
{
    m_PlayerHandler.fnGetPayloadBuffer.Bind(&Client::GetPayloadBuffer, this);
    m_PlayerHandler.fnSendOut.Bind(&Client::SendOut, this);

    m_PlaylistHandler.fnGetPayloadBuffer.Bind(&Client::GetPayloadBuffer, this);
    m_PlaylistHandler.fnSendOut.Bind(&Client::SendOut, this);
}

Client::~Client()
{
    Stop();
    m_Socket.Close();
}

bool Client::Run(const string& ip, int port)
{
    m_ConnectStopRetry = false;
    Function<void (const string&, int)> fn(&Client::ThRecvLoop, this);
    return m_RecvThread.Run(fn, ip, port) == 0;
}

void Client::Stop()
{
    m_ConnectStopRetry = true;
    m_Socket.Shutdown();
    m_RecvThread.Join();
}

void Client::SetConnectMaxRetry(int max)
{
    m_ConnectMaxRetry = max;
}

void Client::SetConnectRetryInterval(int ms)
{
    m_ConnectRetryInterval = ms;
}

void Client::StopService()
{
    char op = Op::App::StopService;
    int payloadSize = (BufObj(NULL) << op).Offset();

    char* buf = GetPayloadBuffer(Op::Group::App, payloadSize);
    BufObj(buf) << op;

    SendOut();
}

ClientPlayerHandler& Client::PlayerHandler()
{
    return m_PlayerHandler;
}

ClientPlaylistHandler& Client::PlaylistHandler()
{
    return m_PlaylistHandler;
}

const Signal<void ()>& Client::SigTryConnect() const
{
    return m_SigTryConnect;
}

const Signal<void ()>& Client::SigConnected() const
{
    return m_SigConnected;
}

void Client::ThRecvLoop(const string& ip, int port)
{
    for (int retryCount = 0; ; ++retryCount) {
        m_SigTryConnect();

        if (m_Socket.Connect(InetAddr(ip, port))) {
            break;
        }

        if (retryCount < m_ConnectMaxRetry && !m_ConnectStopRetry) {
            m_Socket = TcpSocket();
            usleep(m_ConnectRetryInterval*1000);
        } else {
            perror("Failed to connect");
            return;
        }
    }

    m_SigConnected();

    vector<char> headerBuf(Header::Size());
    vector<char> payloadBuf;
    Header header(Op::Group::None, -1);
    char* buf;
    int size;

    while (true) {
        if (!m_Socket.RecvN(&headerBuf[0], headerBuf.size()))
            break;
        if (!header.Read(&headerBuf[0]))
            continue;

        if ((int)payloadBuf.size() <= PAYLOADBUF_MAX_KEEP || header.payloadSize > PAYLOADBUF_MAX_KEEP)
            payloadBuf.resize(header.payloadSize);
        else
            vector<char>(header.payloadSize).swap(payloadBuf);

        buf = &payloadBuf[0];
        size = payloadBuf.size();
        if (!m_Socket.RecvN(buf, size))
            break;

        switch (header.group) {
            case Op::Group::Player:
                m_PlayerHandler.Handle(buf, size);
                break;

            case Op::Group::Playlist:
                m_PlaylistHandler.Handle(buf, size);
                break;

            default:
                break;
        }
    }
}

char* Client::GetPayloadBuffer(char group, int payloadSize)
{
    Header header(group, payloadSize);
    int totalSize = header.TotalSize();

    m_SendOutBufMutex.Lock();

    if ((int)m_SendOutBuf.size() <= SENDOUTBUF_MAX_KEEP || totalSize > SENDOUTBUF_MAX_KEEP)
        m_SendOutBuf.resize(totalSize);
    else
        vector<char>(totalSize).swap(m_SendOutBuf);

    char* buf = &m_SendOutBuf[0];
    header.Write(buf);
    return buf + Header::Size();
}

void Client::SendOut()
{
    m_Socket.SendN(&m_SendOutBuf[0], m_SendOutBuf.size());

    m_SendOutBufMutex.Unlock();
}
