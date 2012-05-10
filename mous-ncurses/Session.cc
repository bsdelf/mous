#include "Session.h"

#include <vector>
using namespace std;

#include <scx/BufObj.hpp>
using namespace scx;

#include "MousData.h"
#include "Protocol.h"
using namespace Protocol;

const int PAYLOAD_MAX_SIZE = 1024;

Session::Session():
    m_Data(NULL),
    m_GotReqStopService(false)
{
}

Session::~Session()
{
    m_Socket.Close();
}

bool Session::Run(const TcpSocket& socket, MousData* data, int notifyFd)
{
    m_GotReqStopService = false;
    m_Socket = socket;
    m_Data = data;
    m_NotifyFd = notifyFd;
    Function<void ()> fn(&Session::ThRecvLoop, this);
    return m_RecvThread.Run(fn) == 0;
}

void Session::Stop()
{
    m_Socket.Shutdown();
    m_RecvThread.Join();
}

void Session::ThRecvLoop()
{
    vector<char> headerBuf(Header::Size());
    vector<char> payloadBuf;
    Header header(Op::Group::None, -1);
    char* buf;
    int size;

    while (!m_GotReqStopService) {
        if (!m_Socket.RecvN(&headerBuf[0], headerBuf.size()))
            break;
        if (!header.Read(&headerBuf[0]))
            break;

        if ((int)payloadBuf.size() <= PAYLOAD_MAX_SIZE || header.payloadSize > PAYLOAD_MAX_SIZE)
            payloadBuf.resize(header.payloadSize);
        else
            vector<char>(header.payloadSize).swap(payloadBuf);

        buf = &payloadBuf[0];
        size = header.payloadSize;
        if (!m_Socket.RecvN(buf, size))
            break;

        switch (header.group) {
            case Op::Group::App:
                HandleApp(buf, size);
                break;

            case Op::Group::Player:
                HandlePlayer(buf, size);
                break;

            case Op::Group::Playlist:
                HandlePlaylist(buf, size);
                break;
        }
    }

    if (!m_GotReqStopService) {
        ptr_t ptr = reinterpret_cast<ptr_t>(this); 
        write(m_NotifyFd, "q", 1);
        write(m_NotifyFd, &ptr, sizeof(ptr));
    }
}

void Session::HandleApp(char* buf, int size)
{
    char op;
    BufObj(buf) >> op;
    switch (op) {
        case Op::App::StopService:
        {
            m_GotReqStopService = true;
            write(m_NotifyFd, "Q", 1);
        }
            break;

        default:
            break;
    }
}

void Session::HandlePlayer(char* buf, int size)
{
}

void Session::HandlePlaylist(char* buf, int size)
{
}
