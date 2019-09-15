#include "Session.h"

#include <algorithm>
#include <mutex>
#include <vector>
using namespace std;

#include <scx/CharsetHelper.h>
#include <scx/IconvHelper.h>
using namespace scx;

#include "AppEnv.h"
#include "Protocol.h"
using namespace Protocol;

const size_t PAYLOADBUF_MAX_KEEP = 1024;
const size_t SENDOUTBUF_MAX_KEEP = 1024 * 4;
const size_t MEDIAITEMS_IN_CHUNK = 20;

#define SEND_PACKET(group, stream)                       \
  {                                                      \
    int payloadSize = (BufObj(nullptr) stream).Offset(); \
    char* buf = GetPayloadBuffer(group, payloadSize);    \
    BufObj(buf) stream;                                  \
  }                                                      \
  SendOut()

#define SEND_PLAYER_PACKET(stream) \
  SEND_PACKET(Protocol::Group::Player, stream)

#define SEND_PLAYLIST_PACKET(stream) \
  SEND_PACKET(Protocol::Group::Playlist, stream)

#define BINARY_MASK(a, b) \
  (((a) << 1) | (b))

Session::Session(ServerContext* data)
    : m_Context(data),
      m_GotReqStopService(false) {
  lock_guard<mutex> locker(m_Context->mtx);
  m_Context->sigPlayNextItem.Connect(&Session::SlotPlayNextItem, this);
}

Session::~Session() {
  m_Socket.Close();

  lock_guard<mutex> locker(m_Context->mtx);
  m_Context->sigPlayNextItem.Disconnect(this);
  m_Context = nullptr;
}

bool Session::Run(const TcpSocket& socket, int notifyFd) {
  m_GotReqStopService = false;
  m_Socket = socket;
  m_NotifyFd = notifyFd;

  m_RecvThread = thread([this] {
    vector<char> headerBuf(Header::Size());
    vector<char> payloadBuf;
    Header header(Group::None, -1);
    char* buf;
    int len;

    NotifySupportedSuffixes();

    while (!m_GotReqStopService) {
      if (!m_Socket.RecvN(headerBuf.data(), headerBuf.size()))
        break;
      if (!header.Read(headerBuf.data()))
        break;
      if (header.payloadSize <= 0)
        continue;

      payloadBuf.resize(header.payloadSize);
      if (payloadBuf.size() > PAYLOADBUF_MAX_KEEP && (size_t)header.payloadSize <= PAYLOADBUF_MAX_KEEP)
        payloadBuf.shrink_to_fit();

      buf = payloadBuf.data();
      len = header.payloadSize;
      if (!m_Socket.RecvN(buf, len))
        break;

      switch (header.group) {
        case Group::App:
          HandleApp(buf, len);
          break;

        case Group::Player:
          HandlePlayer(buf, len);
          break;

        case Group::Playlist:
          HandlePlaylist(buf, len);
          break;
      }
    }

    if (!m_GotReqStopService) {
      ptr_t ptr = reinterpret_cast<ptr_t>(this);
      write(m_NotifyFd, "q", 1);
      write(m_NotifyFd, &ptr, sizeof(ptr));
    }
  });

  return true;
}

void Session::Stop() {
  m_Socket.Shutdown();
  if (m_RecvThread.joinable())
    m_RecvThread.join();
}

void Session::NotifySupportedSuffixes() {
  lock_guard<mutex> locker(m_Context->mtx);
  const auto& s1 = m_Context->player.SupportedSuffixes();
  auto s2 = m_Context->loader.SupportedSuffixes();
  s2.insert(s2.begin(), s1.begin(), s1.end());
  SEND_PACKET((char)Protocol::Group::App, << (char)Op::App::Suffixes << s2);
}

void Session::HandleApp(char* buf, int len) {
  char op;
  BufObj(buf) >> op;
  switch (op) {
    case Op::App::StopService: {
      m_GotReqStopService = true;
      write(m_NotifyFd, "Q", 1);
    } break;

    default:
      break;
  }
}

void Session::HandlePlayer(char* _buf, int len) {
  if (len < 1)
    return;

  BufObj buf(_buf);

  char op;
  buf >> op;

  switch (op) {
    case Op::Player::Pause:
      PlayerPause(buf);
      break;

    case Op::Player::Seek:
      PlayerSeek(buf);
      break;

    case Op::Player::Volume:
      PlayerVolume(buf);
      break;

    case Op::Player::PlayMode:
      PlayerPlayMode(buf);
      break;

    case Op::Player::PlayNext:
      PlayerPlayNext(buf);
      break;

    case Op::Player::Sync:
      PlayerSync(buf);
      break;

    default:
      break;
  }
}

void Session::PlayerPause(BufObj&) {
  lock_guard<mutex> locker(m_Context->mtx);

  m_Context->PausePlayer();

  SEND_PLAYER_PACKET(<< (char)Op::Player::Pause);
}

void Session::PlayerSeek(BufObj& buf) {
  char direct = buf.Fetch<char>();

  lock_guard<mutex> locker(m_Context->mtx);

  switch (direct) {
    case 1:
    case -1: {
      int64_t delta = direct * 1000 * 3;
      uint64_t pos = m_Context->player.CurrentMs() + delta;
      m_Context->player.SeekTime(pos);

      SEND_PLAYER_PACKET(<< (char)Op::Player::Seek);
    } break;

    default:
      break;
  }
}

void Session::PlayerVolume(BufObj& buf) {
  char change = buf.Fetch<char>();

  lock_guard<mutex> locker(m_Context->mtx);

  switch (change) {
    case 0:
      break;

    case 1:
    case -1: {
      int vol = m_Context->player.Volume() + change * 5;
      if (vol > 100)
        vol = 100;
      else if (vol < 0)
        vol = 0;
      m_Context->player.SetVolume(vol);
    } break;

    default:
      return;
  }

  SEND_PLAYER_PACKET(<< (char)Op::Player::Volume << (char)m_Context->player.Volume());
}

void Session::PlayerPlayMode(BufObj& buf) {
  char next = buf.Fetch<char>();

  lock_guard<mutex> locker(m_Context->mtx);

  switch (next) {
    case 1: {
      m_Context->NextPlayMode();
    }
    case 0: {
      string mode = mous::ToString(m_Context->playMode);
      SEND_PLAYER_PACKET(<< (char)Op::Player::PlayMode << mode);
    } break;

    default:
      break;
  }
}

void Session::PlayerPlayNext(BufObj& buf) {
  char direct = buf.Fetch<char>();

  lock_guard<mutex> locker(m_Context->mtx);

  bool hasNext = m_Context->PlayNext(direct);

  SEND_PLAYER_PACKET(<< (char)Op::Player::PlayNext << (char)(hasNext ? 1 : 0));
}

void Session::PlayerSync(BufObj& buf) {
  int running = buf.Fetch<char>();

  lock_guard<mutex> locker(m_Context->mtx);

  PlayerStatus status = m_Context->player.Status();
  int nowRunning = status == PlayerStatus::Playing ? 1 : 0;

  int mask = BINARY_MASK(running, nowRunning);
  switch (mask) {
    case BINARY_MASK(0, 1): {
      const MediaItem* item = m_Context->ItemInPlaying();
      if (item) {
        SendMediaItemInfo(*item);
      }
    }
    case BINARY_MASK(1, 1): {
      uint64_t ms = m_Context->player.OffsetMs();
      int32_t bitRate = m_Context->player.BitRate();

      SEND_PLAYER_PACKET(<< (char)Op::Player::ItemProgress << ms << bitRate);
    } break;

    case BINARY_MASK(1, 0):
    case BINARY_MASK(0, 0):
      break;

    default:
      break;
  }

  SEND_PLAYER_PACKET(<< (char)Op::Player::Sync << (char)nowRunning);
}

void Session::HandlePlaylist(char* _buf, int len) {
  if (len < 1)
    return;

  BufObj buf(_buf);

  char op;
  buf >> op;

  switch (op) {
    case Op::Playlist::Switch:
      PlaylistSwitch(buf);
      break;

    case Op::Playlist::Select:
      PlaylistSelect(buf);
      break;

    case Op::Playlist::Play:
      PlaylistPlay(buf);
      break;

    case Op::Playlist::Append:
      PlaylistAppend(buf);
      break;

    case Op::Playlist::Remove:
      PlaylistRemove(buf);
      break;

    case Op::Playlist::Move:
      PlaylistMove(buf);
      break;

    case Op::Playlist::Clear:
      PlaylistClear(buf);
      break;

    case Op::Playlist::Sync:
      PlaylistSync(buf);
      break;

    default:
      break;
  }
}

void Session::PlaylistSwitch(BufObj& buf) {
  char iList;
  buf >> iList;

  lock_guard<mutex> locker(m_Context->mtx);

  if (iList < 0 || (size_t)iList >= m_Context->playlists.size())
    return;

  m_Context->selectedPlaylist = iList;
}

void Session::PlaylistSelect(BufObj& buf) {
  char iList;
  int32_t iItem;

  buf >> iList >> iItem;

  if (iList < 0 || (size_t)iList >= m_Context->selectedItem.size())
    return;

  if ((iItem < 0 || iItem >= m_Context->playlists[iList].Count()) && !m_Context->playlists[iList].Empty())
    return;

  m_Context->selectedItem[iList] = iItem;
}

void Session::PlaylistPlay(BufObj& buf) {
  char iList;
  int32_t iItem;
  buf >> iList >> iItem;

  lock_guard<mutex> locker(m_Context->mtx);

  if (iList < 0 || (size_t)iList >= m_Context->playlists.size())
    return;

  if (iItem < 0 || iItem >= m_Context->playlists[iList].Count())
    return;

  bool ok = m_Context->PlayAt(iList, iItem);
  if (ok) {
    const MediaItem* item = m_Context->ItemInPlaying();
    if (item) {
      SendMediaItemInfo(*item);
    }
  }

  SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Play << (char)iList << (char)(ok ? 1 : 0));
}

void Session::PlaylistAppend(BufObj& buf) {
  char iList;
  string path;
  buf >> iList >> path;

  lock_guard<mutex> locker(m_Context->mtx);

  if (iList < 0 || (size_t)iList >= m_Context->playlists.size())
    return;

  deque<MediaItem> list;
  if (m_Context->loader.LoadMedia(path, list) != ErrorCode::Ok)
    return;
  if (list.empty())
    return;

  for (auto& item : list) {
    MediaTag& tag = item.tag;
    TryConvertToUtf8(tag.title);
    TryConvertToUtf8(tag.artist);
    TryConvertToUtf8(tag.album);
  }

  m_Context->playlists[iList].Append(list);

  SendMediaItemsByChunk(iList, list);
}

void Session::PlaylistRemove(BufObj& buf) {
  char iList;
  int32_t iItem;
  buf >> iList >> iItem;

  unique_lock<mutex> locker(m_Context->mtx);

  if (iList >= 0 && (size_t)iList < m_Context->playlists.size()) {
    if (iItem >= 0 && iItem < m_Context->playlists[iList].Count()) {
      m_Context->playlists[iList].Remove(iItem);
    }
  }

  locker.unlock();

  SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Remove << iList << iItem);
}

void Session::PlaylistMove(BufObj& buf) {
  char iList;
  int32_t iItem;
  char direct;
  buf >> iList >> iItem >> direct;

  unique_lock<mutex> locker(m_Context->mtx);

  const int total = m_Context->playlists[iList].Count();
  if (total > 0 && iList >= 0 && (size_t)iList < m_Context->playlists.size()) {
    int newPos = direct > 0 ? iItem + 2 : iItem - 1;
    if (newPos >= 0 && newPos <= total) {
      if (newPos >= 0 && iItem <= total) {
        m_Context->playlists[iList].Move(iItem, newPos);
      }
    }
  }

  locker.unlock();

  SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Move << iList << iItem << direct);
}

void Session::PlaylistClear(BufObj& buf) {
  char iList;
  buf >> iList;

  unique_lock<mutex> locker(m_Context->mtx);

  if (iList >= 0 && (size_t)iList < m_Context->playlists.size()) {
    m_Context->playlists[iList].Clear();
  }

  locker.unlock();

  SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Clear << iList);
}

void Session::PlaylistSync(BufObj& buf) {
  char iList;
  buf >> iList;

  lock_guard<mutex> locker(m_Context->mtx);

  // send playlist
  if (iList >= 0 && (size_t)iList < m_Context->playlists.size()) {
    const deque<MediaItem>& list = m_Context->playlists[iList].Items();
    SendMediaItemsByChunk(iList, list);
  }

  // recover previous status
  if ((int)iList == m_Context->selectedPlaylist) {
    SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Switch
                         << (char)m_Context->selectedPlaylist);
  }
  for (size_t i = 0; i < m_Context->selectedItem.size(); ++i) {
    SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Select
                         << (char)i << (int32_t)m_Context->selectedItem[i]);
  }
}

void Session::SlotPlayNextItem(const mous::MediaItem& item) {
  SendMediaItemInfo(item);
}

char* Session::GetPayloadBuffer(char group, int payloadSize) {
  Header header(group, payloadSize);
  size_t totalSize = header.TotalSize();

  m_SendOutBuf.resize(totalSize);
  if (m_SendOutBuf.size() > SENDOUTBUF_MAX_KEEP && totalSize <= SENDOUTBUF_MAX_KEEP)
    m_SendOutBuf.shrink_to_fit();

  char* buf = m_SendOutBuf.data();
  header.Write(buf);
  return buf + Header::Size();
}

void Session::SendOut() {
  m_Socket.SendN(m_SendOutBuf.data(), m_SendOutBuf.size());
}

void Session::TryConvertToUtf8(string& str) const {
  using namespace CharsetHelper;
  using namespace IconvHelper;

  if (str.empty()) {
    str = "(empty)";
    return;
  }

  if (IsUtf8(str.c_str()))
    return;

  const char* bad = "?????";
  const auto env = GlobalAppEnv::Instance();
  string tmp;
  if (ConvFromTo(env->ifNotUtf8, "UTF-8", str.data(), str.size(), tmp)) {
    str = tmp;
  } else {
    str = bad;
  }
}

void Session::SendMediaItemsByChunk(char index, const deque<MediaItem>& list) {
  // assume less than 65535
  for (size_t off = 0, count = 0; off < list.size(); off += count) {
    count = std::min(list.size() - off, MEDIAITEMS_IN_CHUNK);

    BufObj buf(nullptr);
    buf << (char)Op::Playlist::Append << index << (int32_t)count;
    for (size_t i = 0; i < count; ++i) {
      list[off + i] >> buf;
    }

    buf.SetBuffer(GetPayloadBuffer(Group::Playlist, buf.Offset()));
    buf << (char)Op::Playlist::Append << index << (int32_t)count;
    for (size_t i = 0; i < count; ++i) {
      list[off + i] >> buf;
    }

    SendOut();
    cout << count << endl;
  }
}

void Session::SendMediaItemInfo(const MediaItem& item) {
  char op = Op::Player::ItemInfo;

  int32_t sampleRate = m_Context->player.SamleRate();
  uint64_t duration = m_Context->player.RangeDuration();

  BufObj buf(nullptr);
  buf << op;
  item >> buf;
  buf << sampleRate << duration;

  buf.SetBuffer(GetPayloadBuffer(Group::Player, buf.Offset()));
  buf << op;
  item >> buf;
  buf << sampleRate << duration;

  SendOut();
}
