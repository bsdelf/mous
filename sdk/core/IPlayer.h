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

    virtual EmPlayerStatus Status() const = 0;

    virtual void RegisterDecoderPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterDecoderPlugin(std::vector<const IPluginAgent*>& agents) = 0;

    virtual void RegisterRendererPlugin(const IPluginAgent* pAgent) = 0;

    virtual void UnregisterPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual int BufferCount() const = 0;
    virtual void SetBufferCount(int count) = 0;

    virtual int Volume() const = 0;
    virtual void SetVolume(int level) = 0;

    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;
    virtual std::string FileName() const = 0;

    virtual void Play() = 0;
    virtual void Play(uint64_t msBegin, uint64_t msEnd) = 0;
    virtual void Pause() = 0;
    virtual void Resume() = 0;
    virtual void SeekTime(uint64_t msPos) = 0;
    virtual void SeekPercent(double percent) = 0;

    // with these pair of routines, 
    // you'll be able to close the media file temporarily,
    // but keep the renderer still working.
    // after resume, the media file will be opened for decoding again.
    virtual void PauseDecoder() = 0;
    virtual void ResumeDecoder() = 0;

    virtual int32_t BitRate() const = 0;
    virtual int32_t SamleRate() const = 0;
    virtual uint64_t Duration() const = 0;
    virtual uint64_t RangeBegin() const = 0;
    virtual uint64_t RangeEnd() const = 0;
    virtual uint64_t RangeDuration() const = 0;
    virtual uint64_t OffsetMs() const = 0;
    virtual uint64_t CurrentMs() const = 0;
    virtual EmAudioMode AudioMode() const = 0;

    // reimplement this to provide options
    virtual std::vector<PluginOption> DecoderPluginOption() const = 0;
    virtual PluginOption RendererPluginOption() const = 0;

public:
    virtual scx::Signal<void (void)>* SigFinished() = 0;
};

}

#endif
