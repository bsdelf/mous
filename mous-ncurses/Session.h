#ifndef SESSION_H
#define SESSION_H

#include <scx/Thread.hpp>
#include <scx/Socket.hpp>
#include <scx/Mutex.hpp>
#include <scx/BufObj.hpp>
#include <scx/ConfigFile.hpp>
using namespace scx;

#include <vector>
#include <fstream>
#include <string>
using namespace std;

struct MousData;

class Session
{
public:
    typedef unsigned long ptr_t;

public:
    Session(const ConfigFile& config);
    ~Session();

    bool Run(const TcpSocket& socket, MousData* data, int notifyFd);
    void Stop();

private:
    void ThRecvLoop();
    void HandleApp(char*, int);
    void HandlePlayer(char*, int);
    void HandlePlaylist(char*, int);

    void DoPlaylistAppend(BufObj&);
    void DoPlaylistRemove(BufObj&);

    char* GetPayloadBuffer(char, int);
    void SendOut();

    void TryConvertToUtf8(string& str) const;

private:
    const ConfigFile m_Config;
    Thread m_RecvThread;
    TcpSocket m_Socket;
    MousData* m_Data;
    int m_NotifyFd;
    bool m_GotReqStopService;
    vector<char> m_SendOutBuf;

    string m_IfNotUtf8;

    fstream log;
};

#endif
