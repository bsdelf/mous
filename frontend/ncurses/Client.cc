#include "Client.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <vector>
#include <functional>

#include "Protocol.h"
#include "AppEnv.h"
using namespace Protocol;

const size_t PAYLOADBUF_MAX_KEEP = 1024;
const size_t SENDOUTBUF_MAX_KEEP = 256;

namespace phs = std::placeholders;

Client::Client()
{
    m_PlaylistHandler.fnGetPayloadBuffer =
    m_PlayerHandler.fnGetPayloadBuffer =
        std::bind(&Client::GetPayloadBuffer, this, phs::_1, phs::_2);

    m_PlayerHandler.fnSendOut =
    m_PlaylistHandler.fnSendOut =
        std::bind(&Client::SendOut, this);
}

Client::~Client()
{
}

bool Client::Run()
{
    m_ConnectStopRetry = false;

    const auto env = GlobalAppEnv::Instance();

    m_RecvThread = thread([this](const string& ip, int port) {
        for (int retryCount = 0; ; ++retryCount) {
            m_SigTryConnect();

            if (m_Socket.Connect(InetAddr(ip, port))) {
                break;
            }

            if (retryCount < m_ConnectMaxRetry && !m_ConnectStopRetry) {
                m_Socket = TcpSocket();
                ::usleep(m_ConnectRetryInterval*1000);
            } else {
                ::perror("Failed to connect");
                return;
            }
        }

        m_SigConnected();

        vector<char> headerBuf(Header::Size());
        vector<char> payloadBuf;
        Header header(Group::None, -1);
        char* buf;
        int size;

        while (true) {
            if (!m_Socket.RecvN(headerBuf.data(), headerBuf.size()))
                break;
            if (!header.Read(headerBuf.data()))
                continue;
            if (header.payloadSize <= 0)
                continue;

            payloadBuf.resize(header.payloadSize);
            if (payloadBuf.size() > PAYLOADBUF_MAX_KEEP && (size_t)header.payloadSize <= PAYLOADBUF_MAX_KEEP)
                payloadBuf.shrink_to_fit();

            buf = payloadBuf.data();
            size = payloadBuf.size();
            if (!m_Socket.RecvN(buf, size))
                break;

            switch (header.group) {
                case Group::App:
                {
                    if (size < (int)sizeof(char))
                        return;

                    char op = Op::App::None;
                    BufObj bufObj(buf);
                    bufObj >> op;
                    switch (op) {
                        case Op::App::Suffixes:
                            {
                                vector<string> list;
                                bufObj >> list;
                                m_SigSuffixes(list);
                            }
                            break;

                        default:
                            break;
                    }
                }
                    break;

                case Group::Player:
                    m_PlayerHandler.Handle(buf, size);
                    break;

                case Group::Playlist:
                    m_PlaylistHandler.Handle(buf, size);
                    break;

                default:
                    break;
            }
        }
    }, env->serverIp, env->serverPort);

    return true;
}

void Client::Stop()
{
    m_ConnectStopRetry = true;
    m_Socket.Shutdown();
    if (m_RecvThread.joinable())
        m_RecvThread.join();
    m_Socket.Close();
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
    const char op = Op::App::StopService;
    const int payloadSize = (BufObj(nullptr) << op).Offset();

    char* buf = GetPayloadBuffer(Group::App, payloadSize);
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

Signal<void ()>& Client::SigTryConnect()
{
    return m_SigTryConnect;
}

Signal<void ()>& Client::SigConnected()
{
    return m_SigConnected;
}

char* Client::GetPayloadBuffer(char group, int payloadSize)
{
    Header header(group, payloadSize);
    const auto totalSize = header.TotalSize();

    m_SendOutBufMutex.lock();

    m_SendOutBuf.resize(totalSize);
    if (m_SendOutBuf.size() > SENDOUTBUF_MAX_KEEP && totalSize <= SENDOUTBUF_MAX_KEEP)
        m_SendOutBuf.shrink_to_fit();

    char* buf = m_SendOutBuf.data();
    header.Write(buf);
    return buf + Header::Size();
}

void Client::SendOut()
{
    m_Socket.SendN(m_SendOutBuf.data(), m_SendOutBuf.size());

    m_SendOutBufMutex.unlock();
}
