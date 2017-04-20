#pragma once

#include <inttypes.h>
#include <vector>
#include <memory>
#include <string>

#include <core/Plugin.h>

#include <util/ErrorCode.h>
#include <util/AudioMode.h>
#include <util/PluginOption.h>

#include <scx/Signal.hpp>

namespace mous {

namespace PlayerStatus {
enum e
{
    Closed,
    Playing,
    Paused,
    Stopped,
};
}
using EmPlayerStatus = PlayerStatus::e;

class Player
{
    class Impl;

public:
    Player();
    ~Player();

public:
    EmPlayerStatus Status() const;

    void RegisterDecoderPlugin(const Plugin* pAgent);
    void RegisterDecoderPlugin(std::vector<const Plugin*>& agents);

    void RegisterRendererPlugin(const Plugin* pAgent);

    void UnregisterPlugin(const Plugin* pAgent);
    void UnregisterPlugin(std::vector<const Plugin*>& agents);
    void UnregisterAll();

    std::vector<std::string> SupportedSuffixes() const;

    int BufferCount() const;
    void SetBufferCount(int count);

    int Volume() const;
    void SetVolume(int level);

    EmErrorCode Open(const std::string& path);
    void Close();
    std::string FileName() const;

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void SeekTime(uint64_t msPos);
    void SeekPercent(double percent);

    void PauseDecoder();
    void ResumeDecoder();

    int32_t BitRate() const;
    int32_t SamleRate() const;
    uint64_t Duration() const;
    uint64_t RangeBegin() const;
    uint64_t RangeEnd() const;
    uint64_t RangeDuration() const;
    uint64_t OffsetMs() const;
    uint64_t CurrentMs() const;
    EmAudioMode AudioMode() const;

    std::vector<PluginOption> DecoderPluginOption() const;
    PluginOption RendererPluginOption() const;

public:
    scx::Signal<void (void)>* SigFinished();

private:
    std::unique_ptr<Impl> impl;
};

}
