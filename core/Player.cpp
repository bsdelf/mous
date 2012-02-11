#include "Player.h"
#include <scx/Function.hpp>
#include <mous/IDecoder.h>
#include <mous/IRenderer.h>
using namespace scx;
using namespace mous;

#include <iostream>

Player::Player():
    m_Status(MousStopped),
    m_StopDecoder(false),
    m_SuspendDecoder(true),
    m_IsDecoding(false),
    m_pDecoder(NULL),
    m_SemWakeDecoder(0, 0),
    m_SemDecoderSuspended(0, 0),
    m_StopRenderer(false),
    m_SuspendRenderer(true),
    m_IsRendering(false),
    m_pRenderer(NULL),
    m_SemWakeRenderer(0, 0),
    m_SemRendererSuspended(0, 0)
{
    m_FrameBuffer.AllocBuffer(5);

    m_ThreadForDecoder.Run(Function<void (void)>(&Player::WorkForDecoder, this));

    m_ThreadForRenderer.Run(Function<void (void)>(&Player::WorkForRenderer, this));
}

Player::~Player()
{
    m_StopDecoder = true;
    m_StopRenderer = true;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();
    //if (m_Status != MousStopped)
    //Stop();
    m_ThreadForDecoder.Join();
    m_ThreadForRenderer.Join();
}

PlayerStatus Player::GetStatus() const
{
    return m_Status;
}

void Player::AddDecoder(IDecoder* pDecoder)
{
}

void Player::RemoveDecoder(IDecoder* pDecoder)
{
}

void Player::RemoveAllDecoders()
{

}

void Player::AddRenderer(IRenderer* pRenderer)
{
}

void Player::RemoveRenderer(IRenderer* pRenderer)
{

}

void Player::RemoveAllRenderers()
{

}

ErrorCode Player::Open(const string& path)
{
    if (m_pDecoder == NULL)
	return MousFormatIsNotSupported;

    if (m_pRenderer == NULL)
	return MousRendererIsNotSupported;

    return MousOk;
}

void Player::Close()
{
}

void Player::Play()
{
    m_SuspendRenderer = false;
    m_SuspendDecoder = false;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
}

void Player::Pause()
{
    cout << "Pause()" << endl;
    if (m_IsRendering) {
	m_SuspendRenderer = true;
	m_SemRendererSuspended.Wait();
    }

    cout << "Pause()1" << endl;
    if (m_IsDecoding) {
	m_SuspendDecoder = true;
	m_SemDecoderSuspended.Wait();
    }

    cout << "Pause() done" << endl;
}

void Player::Resume()
{
    m_SuspendRenderer = false;
    m_SuspendDecoder = false;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();
}

void Player::Stop()
{
    cout << "Stop()" << endl;
    if (m_IsRendering) {
	m_SuspendRenderer = true;
	m_FrameBuffer.RecycleFree(NULL);
	m_SemRendererSuspended.Wait();
    }

    cout << "Stop() 1" << endl;
    if (m_IsDecoding) {
	m_SuspendDecoder = true;
	m_FrameBuffer.RecycleData(NULL);
	m_SemDecoderSuspended.Wait();
    }

    m_FrameBuffer.ResetPV();

    cout << "Stop() done" << endl;
}

ErrorCode Player::Seek(uint64_t msPos)
{
    return MousOk;
}

uint64_t Player::GetDuration() const
{
    return 0;
}

void Player::WorkForDecoder()
{
    while (true) {
	m_SemWakeDecoder.Wait();
	if (m_StopDecoder)
	    break;

	m_IsDecoding = true;
	while (true) {
	    FrameBuffer* item = m_FrameBuffer.TakeFree();

	    m_FrameBuffer.RecycleFree(item);

	    if (m_SuspendDecoder)
		break;
	}
	m_IsDecoding = false;

	m_SemDecoderSuspended.Post();
    }
}

void Player::WorkForRenderer()
{
    while (true) {
	m_SemWakeRenderer.Wait();
	if (m_StopRenderer)
	    break;

	int a = 0;
	m_IsRendering = true;
	while (true) {
	    FrameBuffer* item = m_FrameBuffer.TakeData();

	    usleep(1000);
	    cout << a++ << '\r' << flush;

	    m_FrameBuffer.RecycleData(item);

	    if (m_SuspendRenderer)
		break;
	}
	m_IsRendering = false;

	m_SemRendererSuspended.Post();
    }
}
