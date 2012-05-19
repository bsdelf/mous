#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <vector>
#include <map>
#include <core/IPlayer.h>
#include <scx/LPVBuffer.hpp>
#include <scx/Signal.hpp>
#include <scx/Thread.hpp>

#ifndef __MACH__
#include <scx/SemVar.hpp>
#else
#include <scx/SoftSemVar.hpp>
#endif

using namespace std;

namespace mous {

class IPluginAgent;
class IDecoder;
class IRenderer;

class Player: public IPlayer
{

#ifndef __MACH__
    typedef scx::SemVar Semaphore;
#else
    typedef scx::SoftSemVar Semaphore;
#endif

public:
    Player();
    ~Player();

public:
    EmPlayerStatus Status() const;

    void RegisterDecoderPlugin(const IPluginAgent* pAgent);
    void RegisterDecoderPlugin(vector<const IPluginAgent*>& agents);

    void RegisterRendererPlugin(const IPluginAgent* pAgent);

    void UnregisterPlugin(const IPluginAgent* pAgent);
    void UnregisterPlugin(vector<const IPluginAgent*>& agents);
    void UnregisterAll();

    int Volume() const;
    void SetVolume(int level);

    EmErrorCode Open(const std::string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void SeekTime(uint64_t msPos);
    void SeekPercent(double percent);

    int32_t BitRate() const;
    int32_t SamleRate() const;
    uint64_t Duration() const;
    uint64_t RangeBegin() const;
    uint64_t RangeEnd() const;
    uint64_t RangeDuration() const;
    uint64_t OffsetMs() const;
    uint64_t CurrentMs() const;
    EmAudioMode AudioMode() const;

    bool DecoderPluginOption(std::vector<PluginOption>& list) const;
    bool RendererPluginOption(PluginOption& option) const;

public:
    const scx::Signal<void (void)>* SigFinished() const;

private:
    void AddDecoderPlugin(const IPluginAgent* pAgent);
    void RemoveDecoderPlugin(const IPluginAgent* pAgent);

    void SetRendererPlugin(const IPluginAgent* pAgent);
    void UnsetRendererPlugin(const IPluginAgent* pAgent);

    void AddEventListener(const IPluginAgent* pAgent);
    void RemoveEventListener(const IPluginAgent* pAgent);

    void PlayRange(uint64_t beg, uint64_t end);
    inline void DoSeekTime(uint64_t msPos);
    inline void DoSeekUnit(uint64_t unit);

    void ThDecoder();
    void ThRenderer();

    void ThPostSigFinished();

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

    struct DecoderPluginNode
    {
        const IPluginAgent* agent;
        IDecoder* decoder;
    };

private:
    EmPlayerStatus m_Status;

    bool m_StopDecoder;
    bool m_SuspendDecoder;
    IDecoder* m_Decoder;
    scx::Thread m_ThreadForDecoder;
    Semaphore m_SemWakeDecoder;
    Semaphore m_SemDecoderBegin;
    Semaphore m_SemDecoderEnd;

    bool m_StopRenderer;
    bool m_SuspendRenderer;
    IRenderer* m_Renderer;
    scx::Thread m_ThreadForRenderer;
    Semaphore m_SemWakeRenderer;
    Semaphore m_SemRendererBegin;
    Semaphore m_SemRendererEnd;

    scx::LPVBuffer<UnitBuffer> m_UnitBuffers;

    uint64_t m_UnitBeg;
    uint64_t m_UnitEnd;

    uint64_t m_DecoderIndex;
    uint64_t m_RendererIndex;

    double m_UnitPerMs;

    const IPluginAgent* m_RendererPlugin;

    std::map<std::string, DecoderPluginNode> m_DecoderPluginMap;
    typedef std::pair<std::string, DecoderPluginNode> DecoderPluginMapPair;
    typedef std::map<std::string, DecoderPluginNode>::iterator DecoderPluginMapIter;
    typedef std::map<std::string, DecoderPluginNode>::const_iterator DecoderPluginMapConstIter;

    scx::Thread m_ThPostSigFinished;
    scx::Signal<void (void)> m_SigFinished;
};

}

#endif
