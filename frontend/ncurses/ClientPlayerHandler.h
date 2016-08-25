#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>

#include <scx/Signal.hpp>
#include <scx/TaskSchedule.hpp>
using namespace scx;

#include <util/MediaItem.h>
using namespace mous;

class ClientPlayerHandler
{
    friend class Client;

public:
    struct PlayerStatus
    {
        bool playing;

        MediaItem item;
        int32_t sampleRate;
        uint64_t duration;
        uint64_t pos;
        int32_t bitRate;
    };

private:
    ClientPlayerHandler();
    ~ClientPlayerHandler();

    void Handle(char* buf, int len);

public:
    void StartSync();
    void StopSync();

    // request
    void QueryVolume();
    void VolumeUp();
    void VolumeDown();

    void QueryPlayMode();
    void NextPlayMode();

    void Pause();
    void SeekForward();
    void SeekBackward();
    void PlayNext();
    void PlayPrevious();

    Signal<void ()>& SigPause()
    {
        return m_SigPause;
    }

    Signal<void ()>& SigSeek()
    {
        return m_SigSeek;
    }

    Signal<void (int)>& SigVolume()
    {
        return m_SigVolume;
    }

    Signal<void (bool)>& SigPlayNext()
    {
        return m_SigPlayNext;
    }

    Signal<void (const std::string&)>& SigPlayMode()
    {
        return m_SigPlayMode;
    }

    Signal<void (const PlayerStatus&)>& SigStatus()
    {
        return m_SigStatus;
    }

private:
    void OnSyncTask();

private:
    std::function<char* (char, int)> fnGetPayloadBuffer;
    std::function<void (void)> fnSendOut;

    Signal<void ()> m_SigPause;
    Signal<void ()> m_SigSeek;
    Signal<void (int)> m_SigVolume;
    Signal<void (bool)> m_SigPlayNext;
    Signal<void (const std::string&)> m_SigPlayMode;
    Signal<void (const PlayerStatus&)> m_SigStatus;

    PlayerStatus m_Status;

    bool m_WaitSyncReply;
    std::mutex m_MutexWaitSyncReply;

    TaskSchedule m_SyncSchedule;
};

