#include "Player.h"

#include <unistd.h>
#include <assert.h>

#include <iostream>
#include <algorithm>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
using namespace scx;

#include <plugin/IDecoder.h>
#include <plugin/IRenderer.h>
#include <core/IPluginAgent.h>
using namespace mous;

IPlayer* IPlayer::Create()
{
    return new Player;
}

void IPlayer::Free(IPlayer* player)
{
    if (player != NULL)
        delete player;
}

Player::Player():
    m_Status(PlayerStatus::Closed),
    m_StopDecoder(false),
    m_SuspendDecoder(true),
    m_Decoder(NULL),
    m_StopRenderer(false),
    m_SuspendRenderer(true),
    m_Renderer(NULL),
    m_UnitBeg(0),
    m_UnitEnd(0),
    m_DecoderIndex(0),
    m_RendererIndex(0),
    m_UnitPerMs(0),
    m_RendererPlugin(NULL)
{
    m_UnitBuffers.AllocBuffer(5);

    m_ThreadForDecoder.Run(Function<void (void)>(&Player::ThDecoder, this));

    m_ThreadForRenderer.Run(Function<void (void)>(&Player::ThRenderer, this));
}

Player::~Player()
{
    Close();

    m_StopDecoder = true;
    m_StopRenderer = true;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();

    m_ThreadForDecoder.Join();
    m_ThreadForRenderer.Join();

    m_UnitBuffers.ClearBuffer();

    UnregisterAll();
}

EmPlayerStatus Player::Status() const
{
    return m_Status;
}

void Player::RegisterDecoderPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::Decoder)
        AddDecoderPlugin(pAgent);
}

void Player::RegisterDecoderPlugin(vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterDecoderPlugin(agents[i]);
    }
}

void Player::RegisterRendererPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::Renderer)
        SetRendererPlugin(pAgent);
}

void Player::UnregisterPlugin(const IPluginAgent* pAgent)
{
    switch (pAgent->Type()) {
        case PluginType::Decoder:
            RemoveDecoderPlugin(pAgent);
            break;

        case PluginType::Renderer:
            UnsetRendererPlugin(pAgent);
            break;

        default:
            break;
    }
}

void Player::UnregisterPlugin(vector<const IPluginAgent*>& agents)
{
    for (size_t i = 0; i < agents.size(); ++i) {
        UnregisterPlugin(agents[i]);
    }
}

void Player::AddDecoderPlugin(const IPluginAgent* pAgent)
{
    // create Decoder & get suffix
    IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
    const vector<string>& list = pDecoder->FileSuffix();

    // try add
    bool usedAtLeastOnce = false;
    for (size_t i = 0; i < list.size(); ++i) {
        const string& suffix = ToLower(list[i]);
        DecoderPluginMapIter iter = m_DecoderPluginMap.find(suffix);
        if (iter == m_DecoderPluginMap.end()) {
            DecoderPluginNode node = { pAgent, pDecoder };
            m_DecoderPluginMap.insert(DecoderPluginMapPair(suffix, node));
            usedAtLeastOnce = true;
        }
    }

    // clear if not used
    if (!usedAtLeastOnce) {
        pAgent->FreeObject(pDecoder);
    }
}

void Player::RemoveDecoderPlugin(const IPluginAgent* pAgent)
{
    // get suffix
    IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
    const vector<string>& list = pDecoder->FileSuffix();
    pAgent->FreeObject(pDecoder);

    // find plugin
    bool freedOnce = false;
    for (size_t i = 0; i < list.size(); ++i) {
        const string& suffix = ToLower(list[i]);
        DecoderPluginMapIter iter = m_DecoderPluginMap.find(suffix);
        if (iter != m_DecoderPluginMap.end()) {
            const DecoderPluginNode& node = iter->second;
            if (node.agent == pAgent) {
                if (!freedOnce) {
                    if (node.decoder == m_Decoder) {
                        Close();
                    }
                    pAgent->FreeObject(node.decoder);
                    freedOnce = true;
                }
                m_DecoderPluginMap.erase(iter);
            }
        }
    }
}

void Player::SetRendererPlugin(const IPluginAgent* pAgent)
{
    if (pAgent == NULL || m_RendererPlugin != NULL)
        return;

    m_RendererPlugin = pAgent;
    m_Renderer = (IRenderer*)pAgent->CreateObject();
    m_Renderer->Open();
}

void Player::UnsetRendererPlugin(const IPluginAgent* pAgent)
{
    if (pAgent != m_RendererPlugin || m_RendererPlugin == NULL)
        return;

    m_Renderer->Close();
    m_RendererPlugin->FreeObject(m_Renderer);
    m_Renderer = NULL;
    m_RendererPlugin = NULL;
}

void Player::UnregisterAll()
{
    while (!m_DecoderPluginMap.empty()) {
        DecoderPluginMapIter iter = m_DecoderPluginMap.begin();
        RemoveDecoderPlugin(iter->second.agent);
    }

    UnsetRendererPlugin(m_RendererPlugin);
}

int Player::Volume() const
{
    return m_Renderer != NULL ? m_Renderer->VolumeLevel() : -1;
}

void Player::SetVolume(int level)
{
    if (m_Renderer != NULL)
        m_Renderer->SetVolumeLevel(level);
}

EmErrorCode Player::Open(const string& path)
{
    string suffix = ToLower(FileHelper::FileSuffix(path));
    cout << "Suffix:" << suffix << endl;
    DecoderPluginMapIter iter = m_DecoderPluginMap.find(suffix);
    if (iter != m_DecoderPluginMap.end()) {
        m_Decoder = iter->second.decoder;
    } else {
        return ErrorCode::PlayerNoDecoder;
    }

    if (m_Renderer == NULL)
        return ErrorCode::PlayerNoRenderer;

    EmErrorCode err = m_Decoder->Open(path);
    if (err != ErrorCode::Ok) {
        cout << "Failed to open!" << endl;
        return err;
    }

    uint32_t maxBytesPerUnit = m_Decoder->MaxBytesPerUnit();
    for (size_t i = 0; i < m_UnitBuffers.BufferCount(); ++i) {
        UnitBuffer* buf = m_UnitBuffers.RawItemAt(i);
        buf->used = 0;
        if (buf->max < maxBytesPerUnit) {
            if (buf->data != NULL) {
                delete[] buf->data;
                cout << "free unit buf:" << buf->max << endl;
            }
            buf->data = new char[maxBytesPerUnit];
            buf->max = maxBytesPerUnit;
            cout << "alloc unit buf:" << buf->max << endl;
        }
    }

    m_UnitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();

    int32_t channels = m_Decoder->Channels();
    int32_t samleRate = m_Decoder->SampleRate();
    int32_t bitsPerSamle = m_Decoder->BitsPerSample();
    cout << "channels:" << channels << endl;
    cout << "samleRate:" << samleRate << endl;
    cout << "bitsPerSamle:" << bitsPerSamle << endl;
    err = m_Renderer->Setup(channels, samleRate, bitsPerSamle);
    if (err != ErrorCode::Ok) {
        cout << "failed to set renderer:" << err << endl;
        cout << "   channels:" << channels << endl;
        cout << "   samleRate:" << samleRate << endl;
        cout << "   bitsPerSamle:" << bitsPerSamle << endl;
        return err;
    }

    m_Status = PlayerStatus::Stopped;

    return err;
}

void Player::Close()
{
    if (m_Status == PlayerStatus::Closed)
        return;

    Pause();

    m_Decoder->Close();
    m_Decoder = NULL;

    m_Status = PlayerStatus::Closed;
}

void Player::Play()
{
    uint64_t beg = 0;
    uint64_t end = m_Decoder->UnitCount();
    PlayRange(beg, end);
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    const uint64_t total = m_Decoder->UnitCount();

    uint64_t beg = 0;
    uint64_t end = 0;

    beg = m_UnitPerMs * msBegin;
    if (beg > total)
        beg = total;

    if (msEnd != (uint64_t)-1) {
        end = m_UnitPerMs * msEnd;
        if (end > total)
            end = total;
    } else {
        end = total;
    }

    cout << "begin:" << beg << endl;
    cout << "end:" << end << endl;
    cout << "total:" << total << endl;

    PlayRange(beg, end);
}

void Player::PlayRange(uint64_t beg, uint64_t end)
{
    m_UnitBeg = beg;
    m_UnitEnd = end;

    m_DecoderIndex = m_UnitBeg;
    m_RendererIndex = m_UnitBeg;

    m_Decoder->SetUnitIndex(m_UnitBeg);

    m_UnitBuffers.ResetPV();

    m_SuspendRenderer = false;
    m_SemWakeRenderer.Post();
    m_SuspendDecoder = false;
    m_SemWakeDecoder.Post();
    m_SemRendererBegin.Wait();
    m_SemDecoderBegin.Wait();

    m_Status = PlayerStatus::Playing;
}

void Player::Pause()
{
    if (m_Status == PlayerStatus::Paused)
        return;

    if (!m_SuspendRenderer) {
        m_SuspendRenderer = true;
        m_UnitBuffers.RecycleFree(NULL);
    }
    m_SemRendererEnd.Wait();

    if (!m_SuspendDecoder) {
        m_SuspendDecoder = true;
        m_UnitBuffers.RecycleData(NULL);
    }
    m_SemDecoderEnd.Wait();

    m_UnitBuffers.ResetPV();

    m_Status = PlayerStatus::Paused;
}

void Player::Resume()
{
    m_DecoderIndex = m_RendererIndex;
    m_Decoder->SetUnitIndex(m_DecoderIndex);

    m_UnitBuffers.ResetPV();

    m_SuspendRenderer = false;
    m_SemWakeRenderer.Post();
    m_SuspendDecoder = false;
    m_SemWakeDecoder.Post();
    m_SemRendererBegin.Wait();
    m_SemDecoderBegin.Wait();

    m_Status = PlayerStatus::Playing;
}

void Player::SeekTime(uint64_t msPos)
{
    switch (m_Status) {
        case PlayerStatus::Playing:
            Pause();
            DoSeekTime(msPos);
            Resume();
            break;

        case PlayerStatus::Paused:
        case PlayerStatus::Stopped:
            DoSeekTime(msPos);
            break;

        default:
            break;
    }
}

void Player::SeekPercent(double percent)
{
    uint64_t unit = m_UnitBeg + (m_UnitEnd - m_UnitBeg) * percent;

    switch (m_Status) {
        case PlayerStatus::Playing:
            Pause();
            DoSeekUnit(unit);
            Resume();
            break;

        case PlayerStatus::Paused:
        case PlayerStatus::Stopped:
            DoSeekUnit(unit);
            break;

        default:
            break;
    }
}

void Player::DoSeekTime(uint64_t msPos)
{
    uint64_t unitPos = std::min((uint64_t)(m_UnitPerMs*msPos), m_Decoder->UnitCount());
    DoSeekUnit(unitPos);
}

void Player::DoSeekUnit(uint64_t unit)
{
    if (unit < m_UnitBeg) 
        unit = m_UnitBeg;
    else if (unit > m_UnitEnd)
        unit = m_UnitEnd;

    m_Decoder->SetUnitIndex(unit);

    m_DecoderIndex = unit;
    m_RendererIndex = unit;
}

int32_t Player::BitRate() const
{
    return (m_Decoder != NULL) ? m_Decoder->BitRate() : -1;
}

int32_t Player::SamleRate() const
{
    return (m_Decoder != NULL) ? m_Decoder->SampleRate() : -1;
}

uint64_t Player::Duration() const
{
    return m_Decoder->Duration();
}

uint64_t Player::RangeBegin() const
{
    return m_UnitBeg / m_UnitPerMs;
}

uint64_t Player::RangeEnd() const
{
    return m_UnitEnd / m_UnitPerMs;
}

uint64_t Player::RangeDuration() const
{
    return (m_UnitEnd - m_UnitBeg) / m_UnitPerMs;
}

uint64_t Player::OffsetMs() const
{
    return CurrentMs() - RangeBegin();
}

uint64_t Player::CurrentMs() const
{
    return m_RendererIndex / m_UnitPerMs;
}

EmAudioMode Player::AudioMode() const
{
    return (m_Decoder != NULL) ? m_Decoder->AudioMode() : AudioMode::None;
}

bool Player::DecoderPluginOption(std::vector<PluginOption>& list) const
{
    list.clear();
    PluginOption optionItem;

    DecoderPluginMapConstIter iter = m_DecoderPluginMap.begin();
    DecoderPluginMapConstIter end = m_DecoderPluginMap.end();
    for (; iter != end; ++iter) {
        const DecoderPluginNode& node = iter->second;
        optionItem.pluginType = node.agent->Type();
        optionItem.pluginInfo = node.agent->Info();
        if (node.decoder->Options(optionItem.options))
            list.push_back(optionItem);
    }

    return !list.empty();
}

bool Player::RendererPluginOption(PluginOption& option) const
{
    if (m_RendererPlugin != NULL) {
        option.pluginType = m_RendererPlugin->Type();
        option.pluginInfo = m_RendererPlugin->Info();
        if (m_Renderer->Options(option.options))
            return true;
    }

    return false;
}

const Signal<void (void)>* Player::SigFinished() const
{
    return &m_SigFinished;
}

void Player::ThDecoder()
{
    while (true) {
        m_SemWakeDecoder.Wait();
        if (m_StopDecoder)
            break;

        m_SemDecoderBegin.Clear();
        m_SemDecoderEnd.Clear();

        m_SemDecoderBegin.Post();

        for (UnitBuffer* buf = NULL; ; ) {
            buf = m_UnitBuffers.TakeFree();
            if (m_SuspendDecoder)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            m_Decoder->DecodeUnit(buf->data, buf->used, buf->unitCount);
            m_DecoderIndex += buf->unitCount;
            m_UnitBuffers.RecycleFree(buf);

            if (m_DecoderIndex >= m_UnitEnd) {
                m_SuspendDecoder = true;
                break;
            }
        }

        m_SemDecoderEnd.Post();
    }
}

void Player::ThRenderer()
{
    while (true) {
        m_SemWakeRenderer.Wait();
        if (m_StopRenderer)
            break;

        m_SemRendererBegin.Clear();
        m_SemRendererEnd.Clear();

        m_SemRendererBegin.Post();

        for (UnitBuffer* buf = NULL; ; ) {
            //cout << m_UnitBuffers.DataCount() << endl;
            buf = m_UnitBuffers.TakeData();
            if (m_SuspendRenderer)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            if (m_Renderer->Write(buf->data, buf->used) != ErrorCode::Ok)
                usleep(10*1000);
            m_RendererIndex += buf->unitCount;
            m_UnitBuffers.RecycleData(buf);

            if (m_RendererIndex >= m_UnitEnd) {
                m_SuspendRenderer = true;
                break;
            }
        }

        m_SemRendererEnd.Post();

        if (m_RendererIndex >= m_UnitEnd) {
            m_Status = PlayerStatus::Stopped;
            Function<void (void)> fn(&Player::ThPostSigFinished, this);
            m_ThPostSigFinished.Run(fn);
            m_ThPostSigFinished.Detach();
        }
    }
}

void Player::ThPostSigFinished()
{
    m_SigFinished();
}
