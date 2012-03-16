#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <mous/ErrorCode.h>
#include <mous/AudioMode.h>

namespace scx {
    class Mutex;
    class Thread;
    class SemVar;

    template<typename T>
    class PVBuffer;

    template<typename signature>
    class AsyncSignal;
}

namespace mous {

class PluginAgent;
class IDecoder;
class IRenderer;

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

class Player
{
public:
    Player();
    ~Player();

public:
    EmPlayerStatus Status() const;

    void RegisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterAll();

    void SetRendererDevice(const std::string& path);

    EmErrorCode Open(const std::string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void Seek(uint64_t msPos);

    int32_t BitRate() const;
    int32_t SamleRate() const;
    uint64_t Duration() const;
    uint64_t RangeBegin() const;
    uint64_t RangeEnd() const;
    uint64_t RangeDuration() const;
    uint64_t OffsetMs() const;
    uint64_t CurrentMs() const;
    EmAudioMode AudioMode() const;

public:
    const scx::AsyncSignal<void (void)>& SigFinished() const;
    const scx::AsyncSignal<void (void)>& SigStopped() const;
    //scx::Signal<void (void)> SigPaused;
    //scx::Signal<void (void)> SigResumed;

private:
    void AddDecoder(const PluginAgent* pAgent);
    void RemoveDecoder(const PluginAgent* pAgent);
    void SetRenderer(const PluginAgent* pAgent);
    void UnsetRenderer(const PluginAgent* pAgent);

    void PlayRange(uint64_t beg, uint64_t end);
    void DoSeek(uint64_t msPos);
    void WorkForDecoder();
    void WorkForRenderer();

private:
    struct UnitBuffer
    {
        char* data;
        uint32_t used;
        uint32_t max;

        uint32_t unitCount;

        UnitBuffer(): 
            data(NULL),
            used(0),
            max(0),
            unitCount(0)
        {
        }

        ~UnitBuffer()
        {
            if (data != NULL)
                delete[] data;
            data = NULL;
            used = 0;
            max = 0;
            unitCount = 0;
        }
    };

private:
    EmPlayerStatus mStatus;

    std::string mRendererDevice;

    bool mStopDecoder;
    bool mSuspendDecoder;
    IDecoder* mDecoder;
    scx::Thread* mThreadForDecoder;
    scx::SemVar* mSemWakeDecoder;
    scx::Mutex* mMutexDecoderSuspended;

    bool mStopRenderer;
    bool mSuspendRenderer;
    IRenderer* mRenderer;
    scx::Thread* mThreadForRenderer;
    scx::SemVar* mSemWakeRenderer;
    scx::Mutex* mMutexRendererSuspended;

    scx::PVBuffer<UnitBuffer>* mUnitBuffers;

    uint64_t mUnitBeg;
    uint64_t mUnitEnd;

    uint64_t mDecoderIndex;
    uint64_t mRendererIndex;

    double mUnitPerMs;

    std::map<const PluginAgent*, void*> mAgentMap;
    typedef std::pair<const PluginAgent*, void*> AgentMapPair;
    typedef std::map<const PluginAgent*, void*>::iterator AgentMapIter;

    std::map<std::string, std::vector<IDecoder*>*> mDecoderMap;
    typedef std::pair<std::string, std::vector<IDecoder*>*> DecoderMapPair;
    typedef std::map<std::string, std::vector<IDecoder*>*>::iterator DecoderMapIter;

    scx::AsyncSignal<void (void)>* mSigFinished;
    scx::AsyncSignal<void (void)>* mSigStopped;
};

}

#endif
