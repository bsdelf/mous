#ifndef CLIENTPLAYERHANDLER_H
#define CLIENTPLAYERHANDLER_H

#include <scx/Signal.hpp>
#include <scx/Function.hpp>
#include <scx/Mutex.hpp>
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
    void VolumeUp();
    void VolumeDown();

    void QueryPlayMode();
    void NextPlayMode();

    void Pause();
    void PlayNext();
    void PlayPrev();

    const Signal<void (const std::string&)>& SigPlayMode() const
    {
        return m_SigPlayMode;
    }

    const Signal<void (const PlayerStatus&)>& SigStatus() const
    {
        return m_SigStatus;
    }

private:
    void OnSyncTask();

private:
    Function<char* (char, int)> fnGetPayloadBuffer;
    Function<void (void)> fnSendOut;

    Signal<void (const std::string&)> m_SigPlayMode;
    Signal<void (const PlayerStatus&)> m_SigStatus;

    PlayerStatus m_Status;

    bool m_WaitSyncReply;
    Mutex m_MutexWaitSyncReply;

    TaskSchedule m_SyncSchedule;
};

#endif
