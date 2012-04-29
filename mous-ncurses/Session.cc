#include "Session.h"
#include "Protocol.h"
#include "MousData.h"

#include <vector>
using namespace std;

const int PAYLOAD_MAX_SIZE = 1024;

Session::Session():
    m_Data(NULL)
{
}

Session::~Session()
{
}

bool Session::Run(const TcpSocket& socket, MousData* data, int notifyFd)
{
    m_Socket = socket;
    m_Data = data;
    m_NotifyFd = notifyFd;
    Function<void ()> fn(&Session::ThHandleLoop, this);
    return m_Thread.Run(fn) == 0;
}

void Session::Stop()
{
    m_Socket.Shutdown();
    m_Thread.Join();
}

void Session::ThHandleLoop()
{
    using namespace Protocol;

    vector<char> headerBuf(Header::GetSize());
    vector<char> payloadBuf;
    Header header;
    char* buf;
    int size;

    while (true) {
        if (!m_Socket.RecvN(&headerBuf[0], headerBuf.size()))
            break;
        if (!header.Read(&headerBuf[0]))
            break;

        if (payloadBuf.size() <= PAYLOAD_MAX_SIZE || header.payloadSize > PAYLOAD_MAX_SIZE)
            payloadBuf.resize(header.payloadSize);
        else
            vector<char>(header.payloadSize).swap(payloadBuf);

        buf = &payloadBuf[0];
        size = header.payloadSize;
        if (!m_Socket.RecvN(buf, size))
            break;
    }
}

