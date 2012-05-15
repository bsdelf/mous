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

const int PAYLOADBUF_MAX_KEEP = 1024;
const int SENDOUTBUF_MAX_KEEP = 1024*4;
const int MEDIAITEMS_IN_CHUNK = 20;

#define SEND_PACKET(group, stream)  \
{\
    int payloadSize = (BufObj(NULL) stream).Offset();   \
    char* buf = GetPayloadBuffer(group, payloadSize);   \
    BufObj(buf) stream;                                 \
}\
    SendOut()

#define SEND_PACKET_PLAYER(stream) \
    SEND_PACKET(Protocol::Group::Player, stream)

#define SEND_PACKET_PLAYLIST(stream) \
    SEND_PACKET(Protocol::Group::Playlist, stream)

Session::Session(const ConfigFile& config):
    m_Config(config),
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

        if ((int)payloadBuf.size() <= PAYLOADBUF_MAX_KEEP || header.payloadSize > PAYLOADBUF_MAX_KEEP)
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
            break;

        case Op::Player::ItemProgress:
            break;

        default:
            break;
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
        case Op::Playlist::Play:
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

void Session::PlaylistAppend(BufObj& buf)
{
    char index;
    string path;
    buf >> index >> path;

    MutexLocker locker(&m_Data->mutex);

    if (index < 0 || index >= m_Data->playlists.size())
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

    m_Data->playlists[index].Append(list);

    SendMediaItemsByChunk(index, list);
}

void Session::PlaylistRemove(BufObj& buf)
{
    char index;
    int32_t pos;
    buf >> index >> pos;

    MutexLocker locker(&m_Data->mutex);

    if (index >= 0 && index < m_Data->playlists.size()) {
        if (pos >= 0 && pos < m_Data->playlists[index].Count()) {
            delete m_Data->playlists[index][pos];
            m_Data->playlists[index].Remove(pos);
        }
    }

    SEND_PACKET_PLAYLIST(<< (char)Op::Playlist::Remove << index << pos);
}

void Session::PlaylistClear(BufObj& buf)
{
    char index;
    buf >> index;

    MutexLocker locker(&m_Data->mutex);

    if (index >= 0 && index < m_Data->playlists.size()) {
        for (int i = 0; i < m_Data->playlists[index].Count(); ++i) {
            delete m_Data->playlists[index][i];
        }
        m_Data->playlists[index].Clear();
    }

    SEND_PACKET_PLAYLIST(<< (char)Op::Playlist::Clear << index);
}

void Session::PlaylistSync(BufObj& buf)
{
    char index;
    buf >> index;

    MutexLocker locker(&m_Data->mutex);

    if (index >= 0 && index < m_Data->playlists.size()) {
        deque<MediaItem*>& list = m_Data->playlists[index].Items();
        SendMediaItemsByChunk(index, list);
    }
}

char* Session::GetPayloadBuffer(char group, int payloadSize)
{
    Header header(group, payloadSize);
    int totalSize = header.TotalSize();
    if ((int)m_SendOutBuf.size() <= SENDOUTBUF_MAX_KEEP || totalSize > SENDOUTBUF_MAX_KEEP)
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
    if (!IsUtf8(c) && !ConvFromTo(m_Config[Config::IfNotUtf8], "UTF-8", c, n, str)) {
        str = bad;
    }
}

void Session::SendMediaItemsByChunk(char index, const deque<MediaItem*>& list)
{
    // assume less than 65535
    for (int off = 0, count = 0; off < list.size(); off += count) {
        count = std::min((int)list.size() - off, MEDIAITEMS_IN_CHUNK);

        BufObj buf(NULL);
        buf << (char)Op::Playlist::Append << index << (int32_t)count;
        for (int i = 0; i < count; ++i) {
            *list[off+i] >> buf;
        }

        buf.SetBuffer(GetPayloadBuffer(Group::Playlist, buf.Offset()));
        buf << (char)Op::Playlist::Append << index << (int32_t)count;
        for (int i = 0; i < count; ++i) {
            *list[off+i] >> buf;
        }

        SendOut();
        cout << count << endl;
    }
}
