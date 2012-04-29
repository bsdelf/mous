#include "Client.h"

#include <vector>
#include <iostream>

#include "Protocol.h"
using namespace Protocol;

const int PAYLOADBUF_MAX_SIZE  = 1024;
const int SENDOUTBUF_MAX_SIZE = 256;

Client::Client()
{
    /*
    char buf[1024];
    cout << WriteString(buf, "hello") << endl;
    string txt;
    ReadString(buf, txt);
    cout << txt << endl;
    BufCast<int>(buf) = 10;
    cout << BufCast<int>(buf) << endl;
    */
}

Client::~Client()
{
    Stop();
    m_Socket.Close();
}

bool Client::Run(const string& ip, int port)
{
    if (!m_Socket.Connect(InetAddr(ip, port)))
        return false;

    Function<void (void)> fn(&Client::ThRecvLoop, this);
    return m_ListenThread.Run(fn) == 0;
}

void Client::Stop()
{
    m_Socket.Shutdown();
    m_ListenThread.Join();
}

void Client::PlayerPlay(const string& path)
{
    char op = Op::App::LoadPlay;
    int payloadSize = (BufObj(NULL) << op << path).GetOffset();

    Header header;
    header.group = Op::Group::Player;
    header.payloadSize = payloadSize;

    int size = header.GetTotalSize();
    char* buf = GetSendOutBuffer(size);

    header.Write(buf);
    buf += Header::GetSize();
    BufObj(buf) << op << path;

    m_Socket.SendN(buf, size);
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

void Client::ThRecvLoop()
{
    vector<char> headerBuf(Header::GetSize());
    vector<char> payloadBuf;
    Header header;
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

char* Client::GetSendOutBuffer(int size)
{
    if (m_SendOutBuf.size() > SENDOUTBUF_MAX_SIZE && size < SENDOUTBUF_MAX_SIZE)
        vector<char>(SENDOUTBUF_MAX_SIZE).swap(m_SendOutBuf);
    m_SendOutBuf.resize(size);
    return &m_SendOutBuf[0];
}
