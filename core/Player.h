#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <string>
#include <vector>
#include <map>
#include <mous/ErrorCode.h>
#include <scx/Thread.hpp>
#include <scx/SemVar.hpp>
#include <scx/PVBuffer.hpp>

namespace mous {

enum PlayerStatus
{
    MousPlaying,
    MousStopped,
    MousPaused
};

class IDecoder;
class IRenderer;

class Player
{
public:
    Player();
    ~Player();

public:
    PlayerStatus GetStatus() const;

    void AddDecoder(IDecoder* pDecoder);
    void RemoveDecoder(IDecoder* pDecoder);
    void RemoveAllDecoders();

    void SetRenderer(IRenderer* pRenderer);
    void UnsetRenderer();

    ErrorCode Open(const string& path);
    void Close();

    void Play();
    void Play(uint64_t msBegin, uint64_t msEnd);
    void Pause();
    void Resume();
    void Stop();
    ErrorCode Seek(uint64_t msPos);
    uint64_t GetDuration() const;

private:
    void WorkForDecoder();
    void WorkForRenderer();

private:
    struct FrameBuffer
    {
	char* data;
	uint32_t used;
	uint32_t max;

	FrameBuffer(): 
	    data(NULL),
	    used(0),
	    max(0)
	{
	}

	FrameBuffer(uint32_t size):
	    data(new char[size]),
	    used(0),
	    max(size)
	{
	}

	~FrameBuffer()
	{
	    if (data != NULL)
		delete[] data;
	    data = NULL;
	    used = 0;
	    max = 0;
	}
    };

private:
    PlayerStatus m_Status;

    bool m_StopDecoder;
    bool m_SuspendDecoder;
    bool m_IsDecoding;
    IDecoder* m_pDecoder;
    scx::Thread m_ThreadForDecoder;
    scx::SemVar m_SemWakeDecoder;
    scx::SemVar m_SemDecoderSuspended;

    bool m_StopRenderer;
    bool m_SuspendRenderer;
    bool m_IsRendering;
    IRenderer* m_pRenderer;
    scx::Thread m_ThreadForRenderer;
    scx::SemVar m_SemWakeRenderer;
    scx::SemVar m_SemRendererSuspended;

    scx::PVBuffer<FrameBuffer> m_FrameBuffer;

    uint64_t m_RangeBeg;
    uint64_t m_RangeEnd;

    uint64_t m_DecoderIndex;
    uint64_t m_RendererIndex;

    std::map<std::string, std::vector<IDecoder*>*> m_DecoderMap;
    typedef std::map<std::string, std::vector<IDecoder*>*>::iterator DecoderMapIter;
    typedef std::pair<std::string, std::vector<IDecoder*>*> DecoderMapPair;
};

}

#endif
