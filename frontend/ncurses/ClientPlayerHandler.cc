#include "ClientPlayerHandler.h"

#include "Protocol.h"
using namespace Protocol;

#define SEND_PACKET(stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();   \
    char* buf = fnGetPayloadBuffer(                     \
            Protocol::Group::Player, payloadSize);      \
    BufObj(buf) stream;                                 \
}\
    fnSendOut()

ClientPlayerHandler::ClientPlayerHandler():
    m_WaitSyncReply(false)
{
}

ClientPlayerHandler::~ClientPlayerHandler()
{
}

void ClientPlayerHandler::Handle(char* buf, int len)
{
    using namespace Protocol;

    if (len < (int)sizeof(char))
        return;

    char op = Op::Player::None;

    BufObj bufObj(buf);
    bufObj >> op;
    switch (op) {
        case Op::Player::PlayMode:
        {
            string mode;
            bufObj >> mode;
            m_SigPlayMode(mode);
        }
            break;

        case Op::Player::Sync:
        {
            MutexLocker locker(&m_MutexWaitSyncReply);

            m_WaitSyncReply = false;
            locker.Unlock();

            m_Status.playing = bufObj.Fetch<char>() == 1 ? true : false;
        }
            break;

        case Op::Player::ItemInfo:
        {
            m_Status.item << bufObj;
            bufObj >> m_Status.sampleRate >> m_Status.duration;
        }
            break;

        case Op::Player::ItemProgress:
        {
            bufObj >> m_Status.pos >> m_Status.bitRate;
        }
            break;

        default:
            break;
    }
}

void ClientPlayerHandler::StartSync()
{
    m_Status.playing = false;
    m_SyncSchedule.Start();
    m_SyncSchedule.Schedule(&ClientPlayerHandler::OnSyncTask, this, 200);
}

void ClientPlayerHandler::StopSync()
{
    m_SyncSchedule.Stop(true);
}

void ClientPlayerHandler::VolumeUp()
{
}

void ClientPlayerHandler::VolumeDown()
{
}

void ClientPlayerHandler::QueryPlayMode()
{
    SEND_PACKET(<< (char)Op::Player::PlayMode << (char)0);
}

void ClientPlayerHandler::NextPlayMode()
{
    SEND_PACKET(<< (char)Op::Player::PlayMode << (char)1);
}

void ClientPlayerHandler::Pause()
{
    SEND_PACKET(<< (char)Op::Player::Pause);
}

void ClientPlayerHandler::PlayNext()
{
    SEND_PACKET(<< (char)Op::Player::PlayNext << (char)1);
}

void ClientPlayerHandler::PlayPrev()
{
    SEND_PACKET(<< (char)Op::Player::PlayNext << (char)-1);
}

void ClientPlayerHandler::OnSyncTask()
{
    MutexLocker locker(&m_MutexWaitSyncReply);

    if (m_WaitSyncReply)
        return;
    m_WaitSyncReply = true;

    locker.Unlock();

    m_SigStatus(m_Status);

    SEND_PACKET(<< (char)Op::Player::Sync << (char)(m_Status.playing ? 1 : 0));
}
