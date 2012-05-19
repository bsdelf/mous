#include "Session.h"

#include <vector>
using namespace std;

#include <scx/CharsetHelper.hpp>
#include <scx/IconvHelper.hpp>
using namespace scx;

#include "Config.h"
#include "MousData.h"
#include "Protocol.h"
using namespace Protocol;

const size_t PAYLOADBUF_MAX_KEEP = 1024;
const size_t SENDOUTBUF_MAX_KEEP = 1024*4;
const size_t MEDIAITEMS_IN_CHUNK = 20;

#define SEND_PACKET(group, stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();   \
    char* buf = GetPayloadBuffer(group, payloadSize);   \
    BufObj(buf) stream;                                 \
}\
    SendOut()

#define SEND_PLAYER_PACKET(stream) \
    SEND_PACKET(Protocol::Group::Player, stream)

#define SEND_PLAYLIST_PACKET(stream) \
    SEND_PACKET(Protocol::Group::Playlist, stream)

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
    Header header(Group::None, -1);
    char* buf;
    int len;

    while (!m_GotReqStopService) {
        if (!m_Socket.RecvN(&headerBuf[0], headerBuf.size()))
            break;
        if (!header.Read(&headerBuf[0]))
            break;
        if (header.payloadSize <= 0)
            continue;

        if (payloadBuf.size() <= PAYLOADBUF_MAX_KEEP || (size_t)header.payloadSize > PAYLOADBUF_MAX_KEEP)
            payloadBuf.resize(header.payloadSize);
        else
            vector<char>(header.payloadSize).swap(payloadBuf);

        buf = &payloadBuf[0];
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
}

void Session::HandleApp(char* buf, int len)
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

void Session::HandlePlayer(char* _buf, int len)
{
    if (len < 1)
        return;

    BufObj buf(_buf);

    char op;
    buf >> op;

    switch (op) {
        case Op::Player::Pause:
            PlayerPause(buf);
            break;

        case Op::Player::ItemProgress:
            PlayerItemProgress(buf);
            break;

        default:
            break;
    }
}

void Session::PlayerPause(BufObj&)
{
    MutexLocker locker(&m_Data->mutex);
    
    m_Data->PausePlayer();
}

void Session::PlayerItemProgress(BufObj&)
{
    char running = 0;
    uint64_t ms = 0;
    int32_t bitRate = 0;

    MutexLocker locker(&m_Data->mutex);
    if (m_Data->player->Status() == PlayerStatus::Playing) {
        running = 1;
        ms = m_Data->player->OffsetMs();
        bitRate = m_Data->player->BitRate();
    }
    locker.Unlock();

    if (running) {
        SEND_PLAYER_PACKET(<< Op::Player::ItemProgress << running << ms << bitRate);
    } else {
        SEND_PLAYER_PACKET(<< Op::Player::ItemProgress << running);
    }
}

void Session::HandlePlaylist(char* _buf, int len)
{
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

void Session::PlaylistSwitch(BufObj& buf)
{
    char iList;
    buf >> iList;

    MutexLocker locker(&m_Data->mutex);

    if (iList < 0 || (size_t)iList >= m_Data->playlists.size())
        return;

    m_Data->selectedPlaylist = iList;
}

void Session::PlaylistSelect(BufObj& buf)
{
    char iList;
    int32_t iItem;

    buf >> iList >> iItem;

    if (iList < 0 || (size_t)iList >= m_Data->selectedItem.size())
        return;

    if ((iItem < 0 || iItem >= m_Data->playlists[iList].Count())
            && !m_Data->playlists[iList].Empty())
        return;

    m_Data->selectedItem[iList] = iItem;
}

void Session::PlaylistPlay(BufObj& buf)
{
    char iList;
    int32_t iItem;
    buf >> iList >> iItem;

    MutexLocker locker(&m_Data->mutex);

    if (iList < 0 || (size_t)iList >= m_Data->playlists.size())
        return;

    if (iItem < 0 || iItem >= m_Data->playlists[iList].Count())
        return;

    m_Data->PlayAt(iList, iItem);
}

void Session::PlaylistAppend(BufObj& buf)
{
    char iList;
    string path;
    buf >> iList >> path;

    MutexLocker locker(&m_Data->mutex);

    if (iList < 0 || (size_t)iList >= m_Data->playlists.size())
        return;

    deque<MediaItem*> list;
    if (m_Data->loader->LoadMedia(path, list) != ErrorCode::Ok)
        return;
    if (list.empty())
        return;

    for (size_t i = 0; i < list.size(); ++i) {
        MediaTag& tag = list[i]->tag;
        TryConvertToUtf8(tag.title);
        TryConvertToUtf8(tag.artist);
        TryConvertToUtf8(tag.album);
    }

    m_Data->playlists[iList].Append(list);

    SendMediaItemsByChunk(iList, list);
}

void Session::PlaylistRemove(BufObj& buf)
{
    char iList;
    int32_t iItem;
    buf >> iList >> iItem;

    MutexLocker locker(&m_Data->mutex);

    if (iList >= 0 && (size_t)iList < m_Data->playlists.size()) {
        if (iItem >= 0 && iItem < m_Data->playlists[iList].Count()) {
            delete m_Data->playlists[iList][iItem];
            m_Data->playlists[iList].Remove(iItem);
        }
    }

    SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Remove << iList << iItem);
}

void Session::PlaylistClear(BufObj& buf)
{
    char iList;
    buf >> iList;

    MutexLocker locker(&m_Data->mutex);

    if (iList >= 0 && (size_t)iList < m_Data->playlists.size()) {
        for (int i = 0; i < m_Data->playlists[iList].Count(); ++i) {
            delete m_Data->playlists[iList][i];
        }
        m_Data->playlists[iList].Clear();
    }

    SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Clear << iList);
}

void Session::PlaylistSync(BufObj& buf)
{
    char iList;
    buf >> iList;

    MutexLocker locker(&m_Data->mutex);

    // send playlist
    if (iList >= 0 && (size_t)iList < m_Data->playlists.size()) {
        deque<MediaItem*>& list = m_Data->playlists[iList].Items();
        SendMediaItemsByChunk(iList, list);
    }

    // recover previous status
    if ((int)iList == m_Data->selectedPlaylist) {
        SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Switch
                << (char)m_Data->selectedPlaylist);
    }
    for (size_t i = 0; i < m_Data->selectedItem.size(); ++i) {
        SEND_PLAYLIST_PACKET(<< (char)Op::Playlist::Select
                << (char)i << (int32_t)m_Data->selectedItem[i]);
    }
}

char* Session::GetPayloadBuffer(char group, int payloadSize)
{
    Header header(group, payloadSize);
    size_t totalSize = header.TotalSize();

    if (m_SendOutBuf.size() <= SENDOUTBUF_MAX_KEEP || totalSize > SENDOUTBUF_MAX_KEEP)
        m_SendOutBuf.resize(totalSize);
    else
        vector<char>(totalSize).swap(m_SendOutBuf);

    char* buf = &m_SendOutBuf[0];
    header.Write(buf);
    return buf + Header::Size();
}

void Session::SendOut()
{
    m_Socket.SendN(&m_SendOutBuf[0], m_SendOutBuf.size());
}

void Session::TryConvertToUtf8(string& str) const
{
    using namespace CharsetHelper;
    using namespace IconvHelper;

    const char* c = str.c_str();
    const size_t n = str.size();
    const char* bad = "?????";
    const Config* config = GlobalConfig::Instance();
    if (!IsUtf8(c) && (config == NULL || !ConvFromTo(config->ifNotUtf8, "UTF-8", c, n, str))) {
        str = bad;
    }
}

void Session::SendMediaItemsByChunk(char index, const deque<MediaItem*>& list)
{
    // assume less than 65535
    for (size_t off = 0, count = 0; off < list.size(); off += count) {
        count = std::min(list.size() - off, MEDIAITEMS_IN_CHUNK);

        BufObj buf(NULL);
        buf << (char)Op::Playlist::Append << index << (int32_t)count;
        for (size_t i = 0; i < count; ++i) {
            *list[off+i] >> buf;
        }

        buf.SetBuffer(GetPayloadBuffer(Group::Playlist, buf.Offset()));
        buf << (char)Op::Playlist::Append << index << (int32_t)count;
        for (size_t i = 0; i < count; ++i) {
            *list[off+i] >> buf;
        }

        SendOut();
        cout << count << endl;
    }
}
