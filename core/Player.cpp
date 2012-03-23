#include "Player.h"
#include <cassert>
#include <iostream>
#include <scx/Conv.hpp>
#include <scx/FileHelp.hpp>
#include <plugin/IDecoder.h>
#include <plugin/IRenderer.h>
#include <core/IPluginAgent.h>
using namespace std;
using namespace scx;
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
    m_SemWakeDecoder(SemVar(0, 0)),
    m_StopRenderer(false),
    m_SuspendRenderer(true),
    m_Renderer(NULL),
    m_SemWakeRenderer(SemVar(0, 0)),
    m_UnitBeg(0),
    m_UnitEnd(0),
    m_DecoderIndex(0),
    m_RendererIndex(0),
    m_UnitPerMs(0)
{
    m_UnitBuffers.AllocBuffer(5);

    m_ThreadForDecoder.Run(Function<void (void)>(&Player::WorkForDecoder, this));

    m_ThreadForRenderer.Run(Function<void (void)>(&Player::WorkForRenderer, this));
}

Player::~Player()
{
    Pause();

    m_StopDecoder = true;
    m_StopRenderer = true;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();

    m_ThreadForDecoder.Join();
    m_ThreadForRenderer.Join();

    m_UnitBuffers.ClearBuffer();
}

EmPlayerStatus Player::GetStatus() const
{
    return m_Status;
}

void Player::RegisterPluginAgent(const IPluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
        case PluginType::Decoder:
            AddDecoder(pAgent);
            break;

        case PluginType::Renderer:
            SetRenderer(pAgent);
            break;

        default:
            break;
    }
}

void Player::UnregisterPluginAgent(const IPluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
        case PluginType::Decoder:
            RemoveDecoder(pAgent);
            break;

        case PluginType::Renderer:
            UnsetRenderer(pAgent);
            break;

        default:
            break;
    }
}

void Player::AddDecoder(const IPluginAgent* pAgent)
{
    // Register agent.
    IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pDecoder));

    // Register decoder.
    const vector<string>& list = pDecoder->GetFileSuffix();
    for (size_t i = 0; i < list.size(); ++i) {
        string suffix = ToLower(list[i]);
        DecoderMapIter iter = m_DecoderMap.find(suffix);
        if (iter == m_DecoderMap.end()) {
            vector<IDecoder*>* dlist = new vector<IDecoder*>();
            dlist->push_back(pDecoder);
            m_DecoderMap.insert(DecoderMapPair(suffix, dlist));
        } else {
            vector<IDecoder*>* dlist = iter->second;
            dlist->push_back(pDecoder);
        }
    }
}

void Player::RemoveDecoder(const IPluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister decoder.
        IDecoder* pDecoder = (IDecoder*)iter->second;
        const vector<string>& list = pDecoder->GetFileSuffix();
        for (size_t i = 0; i < list.size(); ++i) {
            string suffix = ToLower(list[i]);
            DecoderMapIter iter = m_DecoderMap.find(suffix);
            if (iter != m_DecoderMap.end()) {
                vector<IDecoder*>* dlist = iter->second;
                for (size_t i = 0; i < dlist->size(); ++i) {
                    if ((*dlist)[i] == pDecoder) {
                        dlist->erase(dlist->begin()+i);
                        break;
                    }
                }
                if (dlist->empty()) {
                    delete dlist;
                    m_DecoderMap.erase(iter);
                }
            }
        }

        // Unregister agent.
        pAgent->FreeObject(pDecoder);
        m_AgentMap.erase(iter);
    }
}

void Player::SetRenderer(const IPluginAgent* pAgent)
{
    m_Renderer = (IRenderer*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, m_Renderer));

    m_Renderer->OpenDevice(m_RendererDevice);
}

void Player::UnsetRenderer(const IPluginAgent* pAgent)
{
    m_SigStopped.Post();

    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        m_AgentMap.erase(iter);

        if (m_Renderer != NULL) {
            m_Renderer->CloseDevice();
            pAgent->FreeObject(m_Renderer);
            m_Renderer = NULL;
        }
    }
}

void Player::UnregisterAll()
{
    while (!m_AgentMap.empty()) {
        AgentMapIter iter = m_AgentMap.begin();
        UnregisterPluginAgent(iter->first);
    }
}

void Player::SetRendererDevice(const string& path)
{
    m_RendererDevice = path;
}

int Player::GetRendererVolume() const
{
    return m_Renderer != NULL ? m_Renderer->GetVolumeLevel() : -1;
}

void Player::SetRendererVolume(int level)
{
    if (m_Renderer != NULL)
        m_Renderer->SetVolumeLevel(level);
}

EmErrorCode Player::Open(const string& path)
{
    string suffix = ToLower(FileSuffix(path));
    cout << "Suffix:" << suffix << endl;
    DecoderMapIter iter = m_DecoderMap.find(suffix);
    if (iter != m_DecoderMap.end()) {
        m_Decoder = (*(iter->second))[0];
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

    uint32_t maxBytesPerUnit = m_Decoder->GetMaxBytesPerUnit();
    for (size_t i = 0; i < m_UnitBuffers.GetBufferCount(); ++i) {
        UnitBuffer* buf = m_UnitBuffers.GetRawItem(i);
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

    m_UnitPerMs = (double)m_Decoder->GetUnitCount() / m_Decoder->GetDuration();

    int32_t channels = m_Decoder->GetChannels();
    int32_t samleRate = m_Decoder->GetSampleRate();
    int32_t bitsPerSamle = m_Decoder->GetBitsPerSample();
    cout << "channels:" << channels << endl;
    cout << "samleRate:" << samleRate << endl;
    cout << "bitsPerSamle:" << bitsPerSamle << endl;
    err = m_Renderer->SetupDevice(channels, samleRate, bitsPerSamle);
    if (err != ErrorCode::Ok) {
        cout << "failed to set renderer:" << err << endl;
        cout << "   channels:" << channels << endl;
        cout << "   samleRate:" << samleRate << endl;
        cout << "   bitsPerSamle:" << bitsPerSamle << endl;
        return err;
    }

    m_Status = PlayerStatus::Stopped;

    return ErrorCode::Ok;
}

void Player::Close()
{
    Pause();

    m_Decoder->Close();
    m_Status = PlayerStatus::Closed;
}

void Player::Play()
{
    uint64_t beg = 0;
    uint64_t end = m_Decoder->GetUnitCount();
    PlayRange(beg, end);
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    const uint64_t total = m_Decoder->GetUnitCount();

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

    m_Status = PlayerStatus::Playing;

    m_SigStartPlay.Post();
}

void Player::Pause()
{
    if (m_Status != PlayerStatus::Playing)
        return;

    if (!m_SuspendRenderer) {
        m_SuspendRenderer = true;
        m_UnitBuffers.RecycleFree(NULL);
    }
    m_MutexRendererSuspended.Lock();
    m_MutexRendererSuspended.Unlock();

    if (!m_SuspendDecoder) {
        m_SuspendDecoder = true;
        m_UnitBuffers.RecycleData(NULL);
    }
    m_MutexDecoderSuspended.Lock();
    m_MutexDecoderSuspended.Unlock();

    m_UnitBuffers.ResetPV();

    m_Status = PlayerStatus::Paused;

    m_SigStopPlaying.Post();
}

void Player::Resume()
{
    m_DecoderIndex = m_RendererIndex;
    m_Decoder->SetUnitIndex(m_DecoderIndex);

    m_UnitBuffers.ResetPV();

    m_SuspendRenderer = false;
    m_SuspendDecoder = false;
    m_SemWakeRenderer.Post();
    m_SemWakeDecoder.Post();

    m_Status = PlayerStatus::Playing;
}

void Player::Seek(uint64_t msPos)
{
    switch (m_Status) {
        case PlayerStatus::Playing:
            Pause();
            DoSeek(msPos);
            Resume();
            break;

        case PlayerStatus::Paused:
        case PlayerStatus::Stopped:
            DoSeek(msPos);
            break;

        default:
            break;
    }
}

void Player::DoSeek(uint64_t msPos)
{
    uint64_t unitPos = m_UnitPerMs * msPos;
    if (unitPos > m_Decoder->GetUnitCount())
        unitPos = m_Decoder->GetUnitCount();
    m_Decoder->SetUnitIndex(unitPos);
    m_DecoderIndex = unitPos;
    m_RendererIndex = unitPos;
}

int32_t Player::GetBitRate() const
{
    return (m_Decoder != NULL) ? m_Decoder->GetBitRate() : -1;
}

int32_t Player::GetSamleRate() const
{
    return (m_Decoder != NULL) ? m_Decoder->GetSampleRate() : -1;
}

EmAudioMode Player::GetAudioMode() const
{
    return (m_Decoder != NULL) ? m_Decoder->GetAudioMode() : AudioMode::None;
}

uint64_t Player::GetDuration() const
{
    return m_Decoder->GetDuration();
}

uint64_t Player::GetRangeBegin() const
{
    return m_UnitBeg / m_UnitPerMs;
}

uint64_t Player::GetRangeEnd() const
{
    return m_UnitEnd / m_UnitPerMs;
}

uint64_t Player::GetRangeDuration() const
{
    return (m_UnitEnd - m_UnitBeg) / m_UnitPerMs;
}

uint64_t Player::GetOffsetMs() const
{
    return GetCurrentMs() - GetRangeBegin();
}

uint64_t Player::GetCurrentMs() const
{
    return m_RendererIndex / m_UnitPerMs;
}

const AsyncSignal<void (void)>* Player::SigFinished() const
{
    return &m_SigFinished;
}

const AsyncSignal<void (void)>* Player::SigStopped() const
{
    return &m_SigStopped;
}

const AsyncSignal<void (void)>* Player::SigStartPlay() const
{
    return &m_SigStartPlay;
}

const AsyncSignal<void (void)>* Player::SigStopPlaying() const
{
    return &m_SigStopPlaying;
}

void Player::WorkForDecoder()
{
    while (true) {
        m_SemWakeDecoder.Wait();
        if (m_StopDecoder)
            break;

        m_MutexDecoderSuspended.Lock();

        for (UnitBuffer* buf = NULL; ; ) {
            buf = m_UnitBuffers.TakeFree();
            if (m_SuspendDecoder)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            m_Decoder->ReadUnit(buf->data, buf->used, buf->unitCount);
            m_UnitBuffers.RecycleFree(buf);

            m_DecoderIndex += buf->unitCount;
            if (m_DecoderIndex >= m_UnitEnd) {
                m_SuspendDecoder = true;
                break;
            }
        }

        m_MutexDecoderSuspended.Unlock();
    };
}

void Player::WorkForRenderer()
{
    while (true) {
        m_SemWakeRenderer.Wait();
        if (m_StopRenderer)
            break;

        m_MutexRendererSuspended.Lock();

        for (UnitBuffer* buf = NULL; ; ) {
            //cout << m_UnitBuffers->GetDataCount() << flush;
            buf = m_UnitBuffers.TakeData();
            if (m_SuspendRenderer)
                break;

            assert(buf != NULL);
            assert(buf->data != NULL);

            m_Renderer->WriteDevice(buf->data, buf->used);
            m_UnitBuffers.RecycleData(buf);

            m_RendererIndex += buf->unitCount;
            if (m_RendererIndex >= m_UnitEnd) {
                m_SuspendRenderer = true;
                break;
            }
        }

        m_MutexRendererSuspended.Unlock();

        if (m_RendererIndex >= m_UnitEnd) {
            m_Status = PlayerStatus::Stopped;
            m_SigFinished.Post();
        }
    }
}
