#include "ClientPlayerHandler.h"

#include "Protocol.h"
using namespace Protocol;

#define SEND_PACKET(stream)                              \
  {                                                      \
    int payloadSize = (BufObj(nullptr) stream).Offset(); \
    char* buf = fnGetPayloadBuffer(                      \
        Protocol::Group::Player, payloadSize);           \
    BufObj(buf) stream;                                  \
  }                                                      \
  fnSendOut()

ClientPlayerHandler::ClientPlayerHandler()
    : m_WaitSyncReply(false) {
}

ClientPlayerHandler::~ClientPlayerHandler() {
}

void ClientPlayerHandler::Handle(char* buf, int len) {
  using namespace Protocol;

  if (len < (int)sizeof(char))
    return;

  char op = Op::Player::None;

  BufObj bufObj(buf);
  bufObj >> op;
  switch (op) {
    case Op::Player::Pause: {
      m_SigPause();
    } break;

    case Op::Player::Seek: {
      m_SigSeek();
    } break;

    case Op::Player::Volume: {
      m_SigVolume(bufObj.Fetch<char>());
    } break;

    case Op::Player::PlayMode: {
      string mode;
      bufObj >> mode;
      m_SigPlayMode(mode);
    } break;

    case Op::Player::PlayNext: {
      bool hasNext = bufObj.Fetch<char>() == 1 ? true : false;
      m_SigPlayNext(hasNext);
    } break;

    case Op::Player::Sync: {
      unique_lock<mutex> locker(m_MutexWaitSyncReply);

      m_WaitSyncReply = false;

      locker.unlock();

      m_Status.playing = bufObj.Fetch<char>() == 1 ? true : false;
    } break;

    case Op::Player::ItemInfo: {
      m_Status.item << bufObj;
      bufObj >> m_Status.sampleRate >> m_Status.duration;
    } break;

    case Op::Player::ItemProgress: {
      bufObj >> m_Status.pos >> m_Status.bitRate;
    } break;

    default:
      break;
  }
}

void ClientPlayerHandler::StartSync() {
  m_Status.playing = false;
  m_Periodic.Add([this] {
    unique_lock<mutex> locker(m_MutexWaitSyncReply);

    if (m_WaitSyncReply) {
      return;
    }
    m_WaitSyncReply = true;

    locker.unlock();

    m_SigStatus(m_Status);

    SEND_PACKET(<< (char)Op::Player::Sync << (char)(m_Status.playing ? 1 : 0));
  },
                 200);
}

void ClientPlayerHandler::StopSync() {
  // m_Periodic.Clear();
}

void ClientPlayerHandler::QueryVolume() {
  SEND_PACKET(<< (char)Op::Player::Volume << (char)0);
}

void ClientPlayerHandler::VolumeUp() {
  SEND_PACKET(<< (char)Op::Player::Volume << (char)1);
}

void ClientPlayerHandler::VolumeDown() {
  SEND_PACKET(<< (char)Op::Player::Volume << (char)-1);
}

void ClientPlayerHandler::QueryPlayMode() {
  SEND_PACKET(<< (char)Op::Player::PlayMode << (char)0);
}

void ClientPlayerHandler::NextPlayMode() {
  SEND_PACKET(<< (char)Op::Player::PlayMode << (char)1);
}

void ClientPlayerHandler::Pause() {
  SEND_PACKET(<< (char)Op::Player::Pause);
}

void ClientPlayerHandler::SeekForward() {
  SEND_PACKET(<< (char)Op::Player::Seek << (char)1);
}

void ClientPlayerHandler::SeekBackward() {
  SEND_PACKET(<< (char)Op::Player::Seek << (char)-1);
}

void ClientPlayerHandler::PlayNext() {
  SEND_PACKET(<< (char)Op::Player::PlayNext << (char)1);
}

void ClientPlayerHandler::PlayPrevious() {
  SEND_PACKET(<< (char)Op::Player::PlayNext << (char)-1);
}
