#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <vector>
#include <map>
#include <common/ErrorCode.h>
#include <common/AudioMode.h>
#include <core/IPlayer.h>
#include <scx/LPVBuffer.hpp>
#include <scx/AsyncSignal.hpp>
#include <scx/Mutex.hpp>
#include <scx/SemVar.hpp>
#include <scx/Thread.hpp>

namespace mous {

class IPluginAgent;
class IDecoder;
class IRenderer;

class Player: public IPlayer
{
public:
    Player();
    ~Player();

public:
    EmPlayerStatus GetStatus() const;

    void RegisterPluginAgent(const IPluginAgent* pAgent);
    void UnregisterPluginAgent(const IPluginAgent* pAgent);
    void UnregisterAll();

    void SetRendererDevice(const std::string& path);
    int GetRendererVolume() const;
    void SetRendererVolume(int level);

    EmErrorCode Open(const std::string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void Seek(uint64_t msPos);

    int32_t GetBitRate() const;
    int32_t GetSamleRate() const;
    uint64_t GetDuration() const;
    uint64_t GetRangeBegin() const;
    uint64_t GetRangeEnd() const;
    uint64_t GetRangeDuration() const;
    uint64_t GetOffsetMs() const;
    uint64_t GetCurrentMs() const;
    EmAudioMode GetAudioMode() const;

public:
    const scx::AsyncSignal<void (void)>* SigFinished() const;
    const scx::AsyncSignal<void (void)>* SigStopped() const;
    //scx::Signal<void (void)> SigPaused;
    //scx::Signal<void (void)> SigResumed;

public:
    virtual const scx::AsyncSignal<void (void)>* SigStartPlay() const;
    virtual const scx::AsyncSignal<void (void)>* SigStopPlaying() const;

private:
    void AddDecoder(const IPluginAgent* pAgent);
    void RemoveDecoder(const IPluginAgent* pAgent);
    void SetRenderer(const IPluginAgent* pAgent);
    void UnsetRenderer(const IPluginAgent* pAgent);
    void AddEventListener(const IPluginAgent* pAgent);
    void RemoveEventListener(const IPluginAgent* pAgent);

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
    EmPlayerStatus m_Status;

    std::string m_RendererDevice;

    bool m_StopDecoder;
    bool m_SuspendDecoder;
    IDecoder* m_Decoder;
    scx::Thread m_ThreadForDecoder;
    scx::SemVar m_SemWakeDecoder;
    scx::Mutex m_MutexDecoderSuspended;

    bool m_StopRenderer;
    bool m_SuspendRenderer;
    IRenderer* m_Renderer;
    scx::Thread m_ThreadForRenderer;
    scx::SemVar m_SemWakeRenderer;
    scx::Mutex m_MutexRendererSuspended;

    scx::LPVBuffer<UnitBuffer> m_UnitBuffers;

    uint64_t m_UnitBeg;
    uint64_t m_UnitEnd;

    uint64_t m_DecoderIndex;
    uint64_t m_RendererIndex;

    double m_UnitPerMs;

    std::map<const IPluginAgent*, void*> m_AgentMap;
    typedef std::pair<const IPluginAgent*, void*> AgentMapPair;
    typedef std::map<const IPluginAgent*, void*>::iterator AgentMapIter;

    std::map<std::string, std::vector<IDecoder*>*> m_DecoderMap;
    typedef std::pair<std::string, std::vector<IDecoder*>*> DecoderMapPair;
    typedef std::map<std::string, std::vector<IDecoder*>*>::iterator DecoderMapIter;

    scx::AsyncSignal<void (void)> m_SigFinished;
    scx::AsyncSignal<void (void)> m_SigStopped;

    scx::AsyncSignal<void (void)> m_SigStartPlay;
    scx::AsyncSignal<void (void)> m_SigStopPlaying;
};

}

#endif
