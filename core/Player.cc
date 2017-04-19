#include <core/Plugin.h>
#include <core/Player.h>

#include "PlayerPrivate.h"

namespace mous {

Player::Player()
    : d(std::make_unique<PlayerPrivate>())
{
}

Player::~Player()
{
}

EmPlayerStatus Player::Status() const
{
    return d->Status();
}

void Player::RegisterDecoderPlugin(const Plugin* pAgent)
{
    d->RegisterDecoderPlugin(pAgent);
}

void Player::RegisterDecoderPlugin(vector<const Plugin*>& agents)
{
    d->RegisterDecoderPlugin(agents);
}

void Player::RegisterRendererPlugin(const Plugin* pAgent)
{
    d->RegisterRendererPlugin(pAgent);
}

void Player::UnregisterPlugin(const Plugin* pAgent)
{
    d->UnregisterPlugin(pAgent);
}

void Player::UnregisterPlugin(vector<const Plugin*>& agents)
{
    d->UnregisterPlugin(agents);
}

void Player::UnregisterAll()
{
    d->UnregisterAll();
}

vector<string> Player::SupportedSuffixes() const
{
    return d->SupportedSuffixes();
}

int Player::BufferCount() const
{
    return d->BufferCount();
}

void Player::SetBufferCount(int count)
{
    d->SetBufferCount(count);
}

int Player::Volume() const
{
    return d->Volume();
}

void Player::SetVolume(int level)
{
    d->SetVolume(level);
}

EmErrorCode Player::Open(const string& path)
{
    return d->Open(path);
}

void Player::Close()
{
    d->Close();
}

string Player::FileName() const
{
    return d->FileName();
}

void Player::Play()
{
    d->Play();
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    d->Play(msBegin, msEnd);
}

void Player::Pause()
{
    d->Pause();
}

void Player::Resume()
{
    d->Resume();
}

void Player::SeekTime(uint64_t msPos)
{
    d->SeekTime(msPos);
}

void Player::SeekPercent(double percent)
{
    d->SeekPercent(percent);
}

void Player::PauseDecoder()
{
    d->PauseDecoder();
}

void Player::ResumeDecoder()
{
    d->ResumeDecoder();
}

int32_t Player::BitRate() const
{
    return d->BitRate();
}

int32_t Player::SamleRate() const
{
    return d->SamleRate();
}

uint64_t Player::Duration() const
{
    return d->Duration();
}

uint64_t Player::RangeBegin() const
{
    return d->RangeBegin();
}

uint64_t Player::RangeEnd() const
{
    return d->RangeEnd();
}

uint64_t Player::RangeDuration() const
{
    return d->RangeDuration();
}

uint64_t Player::OffsetMs() const
{
    return d->OffsetMs();
}

uint64_t Player::CurrentMs() const
{
    return d->CurrentMs();
}

EmAudioMode Player::AudioMode() const
{
    return d->AudioMode();
}

std::vector<PluginOption> Player::DecoderPluginOption() const
{
    return d->DecoderPluginOption();
}

PluginOption Player::RendererPluginOption() const
{
    return d->RendererPluginOption();
}

Signal<void (void)>* Player::SigFinished()
{
    return d->SigFinished();
}

}
