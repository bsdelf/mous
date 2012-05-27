#ifndef SESSION_H
#define SESSION_H

#include <scx/Thread.hpp>
#include <scx/Socket.hpp>
#include <scx/Mutex.hpp>
#include <scx/BufObj.hpp>
using namespace scx;

#include <vector>
#include <deque>
#include <string>
using namespace std;

struct ServerContext;

namespace mous {
    struct MediaItem;
}

class Session
{
public:
    typedef unsigned long ptr_t;

public:
    explicit Session(ServerContext* data);
    ~Session();

    bool Run(const TcpSocket& socket, int notifyFd);
    void Stop();

private:
    void ThRecvLoop();
    void HandleApp(char*, int);
    void HandlePlayer(char*, int);
    void HandlePlaylist(char*, int);

    void PlayerPause(BufObj&);
    void PlayerSeek(BufObj&);
    void PlayerVolume(BufObj&);
    void PlayerPlayMode(BufObj&);
    void PlayerSync(BufObj&);

    void PlaylistSwitch(BufObj&);
    void PlaylistSelect(BufObj&);
    void PlaylistPlay(BufObj&);
    void PlaylistAppend(BufObj&);
    void PlaylistRemove(BufObj&);
    void PlaylistClear(BufObj&);
    void PlaylistSync(BufObj&);

    void SlotPlayNextItem(const mous::MediaItem*);

    char* GetPayloadBuffer(char, int);
    void SendOut();

    void TryConvertToUtf8(string& str) const;
    void SendMediaItemsByChunk(char, const deque<mous::MediaItem*>&);
    void SendMediaItemInfo(const mous::MediaItem*);

private:
    ServerContext* m_Context;

    Thread m_RecvThread;
    TcpSocket m_Socket;
    int m_NotifyFd;
    bool m_GotReqStopService;
    vector<char> m_SendOutBuf;

    string m_IfNotUtf8;
};

#endif
