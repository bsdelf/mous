#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <map>
#include <mous/ErrorCode.h>
#include <mous/AudioMode.h>
#include <scx/Thread.hpp>
#include <scx/SemVar.hpp>
#include <scx/PVBuffer.hpp>
#include <scx/Signal.hpp>
#include "PluginAgent.h"

namespace mous {

class IPluginAgent;
class IDecoder;
class IRenderer;

namespace PlayerStatus {
enum e
{
    Closed,
    Playing,
    Paused,
    Stopped
};
}
typedef PlayerStatus::e EmPlayerStatus;

class Player
{
public:
    Player();
    ~Player();

public:
    EmPlayerStatus GetStatus() const;

    void RegisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterPluginAgent(const PluginAgent* pAgent);
    void UnregisterAll();

    void SetRendererDevice(const string& path);

    EmErrorCode Open(const string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void Stop();
    void Seek(uint64_t msPos);

    int32_t GetBitRate() const;
    int32_t GetSampleRate() const;
    uint64_t GetDuration() const;
    uint64_t GetRangeBegin() const;
    uint64_t GetRangeEnd() const;
    uint64_t GetRangeDuration() const;
    uint64_t GetOffsetMs() const;
    uint64_t GetCurrentMs() const;
    EmAudioMode GetAudioMode() const;

public:
    scx::Signal<void (void)> SigFinished;
    scx::Signal<void (void)> SigStopped;
    scx::Signal<void (void)> SigPaused;
    scx::Signal<void (void)> SigResumed;

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
    EmPlayerStatus m_Status;

    std::string m_RendererDevice;

    bool m_StopDecoder;
    bool m_SuspendDecoder;
    IDecoder* m_pDecoder;
    scx::Thread m_ThreadForDecoder;
    scx::SemVar m_SemWakeDecoder;
    scx::SemVar m_SemDecoderSuspended;

    bool m_StopRenderer;
    bool m_SuspendRenderer;
    IRenderer* m_pRenderer;
    scx::Thread m_ThreadForRenderer;
    scx::SemVar m_SemWakeRenderer;
    scx::SemVar m_SemRendererSuspended;

    scx::PVBuffer<UnitBuffer> m_UnitBuffers;

    uint64_t m_UnitBeg;
    uint64_t m_UnitEnd;

    uint64_t m_DecoderIndex;
    uint64_t m_RendererIndex;

    double m_UnitPerMs;

    std::map<const PluginAgent*, void*> m_AgentMap;
    typedef std::pair<const PluginAgent*, void*> AgentMapPair;
    typedef std::map<const PluginAgent*, void*>::iterator AgentMapIter;

    std::map<std::string, std::vector<IDecoder*>*> m_DecoderMap;
    typedef std::pair<std::string, std::vector<IDecoder*>*> DecoderMapPair;
    typedef std::map<std::string, std::vector<IDecoder*>*>::iterator DecoderMapIter;
};

}

#endif
