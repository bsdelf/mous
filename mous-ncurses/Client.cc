#include "Client.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <vector>

#include "Protocol.h"
using namespace Protocol;

const int PAYLOADBUF_MAX_SIZE  = 1024;
const int SENDOUTBUF_MAX_SIZE = 256;

Client::Client():
    m_ConnectMaxRetry(25),
    m_ConnectRetryInterval(200)
{
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
    int payloadSize = (BufObj(NULL) << op).GetOffset();

    char* buf = GetPayloadBuffer(Op::Group::App, payloadSize);
    BufObj(buf) << op;

    SendOut();
}

void Client::PlayerPlay(const string& path)
{
    char op = Op::App::LoadPlay;
    int payloadSize = (BufObj(NULL) << op << path).GetOffset();

    char* buf = GetPayloadBuffer(Op::Group::App, payloadSize);
    BufObj(buf) << op << path;

    SendOut();
}

void Client::PlayerStop()
{
}

void Client::PlayerPause()
{
}

void Client::PlayerResume()
{
}

void Client::PlayerSeek(uint64_t ms)
{
}

void Client::ThRecvLoop(const string& ip, int port)
{
    for (int retryCount = 0; ; ++retryCount) {
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

    vector<char> headerBuf(Header::GetSize());
    vector<char> payloadBuf;
    Header header(Op::Group::None, -1);
    char* buf;
    int size;

    while (true) {
        if (!m_Socket.RecvN(&headerBuf[0], headerBuf.size()))
            break;
        if (!header.Read(&headerBuf[0]))
            continue;

        if (payloadBuf.size() <= PAYLOADBUF_MAX_SIZE || header.payloadSize > PAYLOADBUF_MAX_SIZE)
            payloadBuf.resize(header.payloadSize);
        else
            vector<char>(header.payloadSize).swap(payloadBuf);

        buf = &payloadBuf[0];
        size = payloadBuf.size();
        if (!m_Socket.RecvN(buf, size))
            break;

        switch (header.group) {
            case Op::Group::Player:
                HandlePlayer(buf, size);
                break;

            case Op::Group::Playlist:
                HandlePlaylist(buf, size);
                break;

            default:
                break;
        }
    }
}

void Client::HandlePlayer(char* buf, int size)
{
    using namespace Protocol;

    if (size < sizeof(char))
        return;

    char op = Op::Player::None;
    BufObj bufObj(buf);
    bufObj >> op;
    switch (op) {
        case Op::Player::TotalMs:
            SigPlayerTotalMs(bufObj.Fetch<uint64_t>());
            break;

        case Op::Player::CurrentMs:
            SigPlayerCurrentMs(bufObj.Fetch<uint64_t>());
            break;

        default:
            break;
    }
}

void Client::HandlePlaylist(char* buf, int size)
{
    if (size < sizeof(char))
        return;
}

char* Client::GetPayloadBuffer(char group, int payloadSize)
{
    Header header(group, payloadSize);
    int size = header.GetTotalSize();
    if (m_SendOutBuf.size() > SENDOUTBUF_MAX_SIZE && size < SENDOUTBUF_MAX_SIZE)
        vector<char>(SENDOUTBUF_MAX_SIZE).swap(m_SendOutBuf);
    m_SendOutBuf.resize(size);

    char* buf = &m_SendOutBuf[0];
    header.Write(buf);
    return buf + Header::GetSize();
}

void Client::SendOut()
{
    m_Socket.SendN(&m_SendOutBuf[0], m_SendOutBuf.size());
}
