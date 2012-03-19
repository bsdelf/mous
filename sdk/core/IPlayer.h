#ifndef MOUS_IPLAYER_H
#define MOUS_IPLAYER_H

#include <inttypes.h>
#include <string>
#include <common/ErrorCode.h>
#include <common/AudioMode.h>

namespace scx {

template<typename signature> class AsyncSignal;

}

namespace mous {

class IPluginAgent;

namespace PlayerStatus {
enum e
{
    Closed,
    Playing,
    Paused,
    Stopped,
};
}
typedef PlayerStatus::e EmPlayerStatus;

class IPlayer
{
public:
    static IPlayer* Create();
    static void Free(IPlayer*);

public:
    virtual ~IPlayer() { }

    virtual EmPlayerStatus GetStatus() const = 0;

    virtual void RegisterPluginAgent(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPluginAgent(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterAll() = 0;

    virtual void SetRendererDevice(const std::string& path) = 0;
    virtual int GetRendererVolume() const = 0;
    virtual void SetRendererVolume(int level) = 0;

    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    virtual void Play() = 0;
    virtual void Play(uint64_t msBegin, uint64_t msEnd) = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void Seek(uint64_t msPos) = 0;

    virtual int32_t GetBitRate() const = 0;
    virtual int32_t GetSamleRate() const = 0;
    virtual uint64_t GetDuration() const = 0;
    virtual uint64_t GetRangeBegin() const = 0;
    virtual uint64_t GetRangeEnd() const = 0;
    virtual uint64_t GetRangeDuration() const = 0;
    virtual uint64_t GetOffsetMs() const = 0;
    virtual uint64_t GetCurrentMs() const = 0;
    virtual EmAudioMode GetAudioMode() const = 0;

public:
    virtual const scx::AsyncSignal<void (void)>* SigFinished() const = 0;
    virtual const scx::AsyncSignal<void (void)>* SigStopped() const = 0;
};

}

#endif
