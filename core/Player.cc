#include <core/Player.h>
#include <core/Plugin.h>

#include "PlayerImpl.h"

namespace mous {

Player::Player()
    : impl(std::make_unique<Impl>())
{
}

Player::~Player()
{
}

EmPlayerStatus Player::Status() const
{
    return impl->Status();
}

void Player::RegisterDecoderPlugin(const Plugin* pAgent)
{
    return impl->RegisterDecoderPlugin(pAgent);
}

void Player::RegisterDecoderPlugin(vector<const Plugin*>& agents)
{
    return impl->RegisterDecoderPlugin(agents);
}

void Player::RegisterRendererPlugin(const Plugin* pAgent)
{
    return impl->RegisterRendererPlugin(pAgent);
}

void Player::UnregisterPlugin(const Plugin* pAgent)
{
    return impl->UnregisterPlugin(pAgent);
}

void Player::UnregisterPlugin(vector<const Plugin*>& agents)
{
    return impl->UnregisterPlugin(agents);
}

void Player::UnregisterAll()
{
    return impl->UnregisterAll();
}

vector<string> Player::SupportedSuffixes() const
{
    return impl->SupportedSuffixes();
}

int Player::BufferCount() const
{
    return impl->BufferCount();
}

void Player::SetBufferCount(int count)
{
    return impl->SetBufferCount(count);
}

int Player::Volume() const
{
    return impl->Volume();
}

void Player::SetVolume(int level)
{
    return impl->SetVolume(level);
}

EmErrorCode Player::Open(const string& path)
{
    return impl->Open(path);
}

void Player::Close()
{
    return impl->Close();
}

string Player::FileName() const
{
    return impl->FileName();
}

void Player::Play()
{
    return impl->Play();
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    return impl->Play(msBegin, msEnd);
}

void Player::Pause()
{
    return impl->Pause();
}

void Player::Resume()
{
    return impl->Resume();
}

void Player::SeekTime(uint64_t msPos)
{
    return impl->SeekTime(msPos);
}

void Player::SeekPercent(double percent)
{
    return impl->SeekPercent(percent);
}

void Player::PauseDecoder()
{
    return impl->PauseDecoder();
}

void Player::ResumeDecoder()
{
    return impl->ResumeDecoder();
}

int32_t Player::BitRate() const
{
    return impl->BitRate();
}

int32_t Player::SamleRate() const
{
    return impl->SamleRate();
}

uint64_t Player::Duration() const
{
    return impl->Duration();
}

uint64_t Player::RangeBegin() const
{
    return impl->RangeBegin();
}

uint64_t Player::RangeEnd() const
{
    return impl->RangeEnd();
}

uint64_t Player::RangeDuration() const
{
    return impl->RangeDuration();
}

uint64_t Player::OffsetMs() const
{
    return impl->OffsetMs();
}

uint64_t Player::CurrentMs() const
{
    return impl->CurrentMs();
}

EmAudioMode Player::AudioMode() const
{
    return impl->AudioMode();
}

std::vector<PluginOption> Player::DecoderPluginOption() const
{
    return impl->DecoderPluginOption();
}

PluginOption Player::RendererPluginOption() const
{
    return impl->RendererPluginOption();
}

Signal<void (void)>* Player::SigFinished()
{
    return impl->SigFinished();
}

}
