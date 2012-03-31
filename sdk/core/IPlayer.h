#ifndef MOUS_IPLAYER_H
#define MOUS_IPLAYER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <util/ErrorCode.h>
#include <util/AudioMode.h>
#include <util/PluginOption.h>

namespace scx {

template<typename signature> class Signal;

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

    virtual void RegisterDecoderPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterDecoderPlugin(std::vector<const IPluginAgent*>& agents) = 0;

    virtual void RegisterRendererPlugin(const IPluginAgent* pAgent) = 0;

    virtual void UnregisterPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual int GetVolume() const = 0;
    virtual void SetVolume(int level) = 0;

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

    // reimplement this to provide options
    virtual bool GetDecoderPluginOption(std::vector<PluginOption>& list) const = 0;
    virtual bool GetRendererPluginOption(PluginOption& option) const = 0;

public:
    virtual const scx::Signal<void (void)>* SigFinished() const = 0;
};

}

#endif
