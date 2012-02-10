#ifndef MOUS_PLAYER_H
#define MOUS_PLAYER_H

#include <string>
#include <map>
#include <mous/ErrorCode.h>
#include <mous/IDecoder.h>
#include <mous/IFilter.h>
#include <mous/IRenderer.h>
#include <PluginAgent.h>
#include <scx/Thread.hpp>
#include <scx/SemVar.hpp>
#include <scx/PVBuffer.hpp>
using namespace std;

namespace mous {

enum PlayerStatus
{
    MousPlaying,
    MousStopped,
    MousPaused
};

class Player
{
public:
    Player();
    ~Player();

public:
    PlayerStatus GetStatus() const;

    ErrorCode LoadDecoder(PluginAgent<IDecoder>* agent);
    void UnloadDecoder(PluginAgent<IDecoder>* agent);
    void UnloadAllDecoders();

    ErrorCode LoadRenderer(PluginAgent<IRenderer>* agent);
    void UnloadRenderer(PluginAgent<IRenderer>* agent);
    void UnloadAllRenderers();

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
    struct FrameBuffer {
	char* data;
	int32_t length;

	FrameBuffer(): data(NULL), length(0) {

	}

	~FrameBuffer() {
	    if (data != NULL)
		delete[] data;
	    data = NULL;
	    length = 0;
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
};

}

#endif
