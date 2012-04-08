#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <vector>
#include <map>
#include <core/IPlayer.h>
#include <scx/LPVBuffer.hpp>
#include <scx/Signal.hpp>
#include <scx/Mutex.hpp>
#include <scx/SemVar.hpp>
#include <scx/Thread.hpp>
using namespace std;

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

    void RegisterDecoderPlugin(const IPluginAgent* pAgent);
    void RegisterDecoderPlugin(vector<const IPluginAgent*>& agents);

    void RegisterRendererPlugin(const IPluginAgent* pAgent);

    void UnregisterPlugin(const IPluginAgent* pAgent);
    void UnregisterPlugin(vector<const IPluginAgent*>& agents);
    void UnregisterAll();

    int GetVolume() const;
    void SetVolume(int level);

    EmErrorCode Open(const std::string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void Seek(uint64_t msPos);
    void SeekPercent(double percent);

    int32_t GetBitRate() const;
    int32_t GetSamleRate() const;
    uint64_t GetDuration() const;
    uint64_t GetRangeBegin() const;
    uint64_t GetRangeEnd() const;
    uint64_t GetRangeDuration() const;
    uint64_t GetOffsetMs() const;
    uint64_t GetCurrentMs() const;
    EmAudioMode GetAudioMode() const;

    bool GetDecoderPluginOption(std::vector<PluginOption>& list) const;
    bool GetRendererPluginOption(PluginOption& option) const;

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

    const IPluginAgent* m_RendererPlugin;

    std::map<std::string, DecoderPluginNode*> m_DecoderPluginMap;
    typedef std::pair<std::string, DecoderPluginNode*> DecoderPluginMapPair;
    typedef std::map<std::string, DecoderPluginNode*>::iterator DecoderPluginMapIter;
    typedef std::map<std::string, DecoderPluginNode*>::const_iterator DecoderPluginMapConstIter;

    scx::Signal<void (void)> m_SigFinished;
};

}

#endif
