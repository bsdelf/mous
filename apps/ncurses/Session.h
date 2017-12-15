#pragma once

#include <scx/Socket.h>
#include <scx/BufObj.h>
using namespace scx;

#include <thread>
#include <vector>
#include <deque>
#include <string>
using namespace std;

#include "ServerContext.h"

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
    void HandleApp(char*, int);
    void HandlePlayer(char*, int);
    void HandlePlaylist(char*, int);

    void NotifySupportedSuffixes();

    void PlayerPause(BufObj&);
    void PlayerSeek(BufObj&);
    void PlayerVolume(BufObj&);
    void PlayerPlayMode(BufObj&);
    void PlayerPlayNext(BufObj&);
    void PlayerSync(BufObj&);

    void PlaylistSwitch(BufObj&);
    void PlaylistSelect(BufObj&);
    void PlaylistPlay(BufObj&);
    void PlaylistAppend(BufObj&);
    void PlaylistRemove(BufObj&);
    void PlaylistMove(BufObj&);
    void PlaylistClear(BufObj&);
    void PlaylistSync(BufObj&);

    void SlotPlayNextItem(const MediaItem&);

    char* GetPayloadBuffer(char, int);
    void SendOut();

    void TryConvertToUtf8(string& str) const;
    void SendMediaItemsByChunk(char, const deque<MediaItem>&);
    void SendMediaItemInfo(const MediaItem&);

private:
    ServerContext* m_Context;

    thread m_RecvThread;
    TcpSocket m_Socket;
    int m_NotifyFd;
    bool m_GotReqStopService;
    vector<char> m_SendOutBuf;

    string m_IfNotUtf8;
};

