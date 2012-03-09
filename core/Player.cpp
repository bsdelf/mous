#include "Player.h"
#include <scx/Function.hpp>
#include <scx/FileHelp.hpp>
#include <scx/Conv.hpp>
#include <mous/IDecoder.h>
#include <mous/IRenderer.h>
using namespace std;
using namespace scx;
using namespace mous;

#include <iostream>

Player::Player():
    m_Status(PlayerStatus::Closed),
    m_StopDecoder(false),
    m_SuspendDecoder(true),
    m_pDecoder(NULL),
    m_SemWakeDecoder(0, 0),
    m_SemDecoderSuspended(0, 0),
    m_StopRenderer(false),
    m_SuspendRenderer(true),
    m_pRenderer(NULL),
    m_SemWakeRenderer(0, 0),
    m_SemRendererSuspended(0, 0),
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
    m_StopDecoder = true;
    m_StopRenderer = true;
    m_SemWakeDecoder.Post();
    m_SemWakeRenderer.Post();
    //if (m_Status != MousStopped)
    //Stop();
    m_ThreadForDecoder.Join();
    m_ThreadForRenderer.Join();

    m_UnitBuffers.ClearBuffer();
}

EmPlayerStatus Player::GetStatus() const
{
    return m_Status;
}

void Player::RegisterPluginAgent(const PluginAgent* pAgent)
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

void Player::UnregisterPluginAgent(const PluginAgent* pAgent)
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

void Player::AddDecoder(const PluginAgent* pAgent)
{
    // Register agent.
    IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, pDecoder));

    // Register decoder.
    vector<string> list;
    pDecoder->GetFileSuffix(list);
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

void Player::RemoveDecoder(const PluginAgent* pAgent)
{
    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        // Unregister decoder.
        vector<string> list;
        IDecoder* pDecoder = (IDecoder*)iter->second;
        pDecoder->GetFileSuffix(list);
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
        pAgent->ReleaseObject(pDecoder);
        m_AgentMap.erase(iter);
    }
}

void Player::SetRenderer(const PluginAgent* pAgent)
{
    m_pRenderer = (IRenderer*)pAgent->CreateObject();
    m_AgentMap.insert(AgentMapPair(pAgent, m_pRenderer));

    m_pRenderer->OpenDevice(m_RendererDevice);
}

void Player::UnsetRenderer(const PluginAgent* pAgent)
{
    Stop();
    SigStopped();

    AgentMapIter iter = m_AgentMap.find(pAgent);
    if (iter != m_AgentMap.end()) {
        m_AgentMap.erase(iter);

        if (m_pRenderer != NULL) {
            m_pRenderer->CloseDevice();
            pAgent->ReleaseObject(m_pRenderer);
            m_pRenderer = NULL;
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

EmErrorCode Player::Open(const string& path)
{
    string suffix = ToLower(FileSuffix(path));
    cout << "Suffix:" << suffix << endl;
    DecoderMapIter iter = m_DecoderMap.find(suffix);
    if (iter != m_DecoderMap.end()) {
        m_pDecoder = (*(iter->second))[0];
    } else {
        return ErrorCode::PlayerNoDecoder;
    }

    if (m_pRenderer == NULL)
        return ErrorCode::PlayerNoRenderer;

    EmErrorCode err = m_pDecoder->Open(path);
    if (err != ErrorCode::Ok) {
        cout << "Failed to open!" << endl;
        return err;
    }

    uint32_t maxBytesPerUnit = m_pDecoder->GetMaxBytesPerUnit();
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

    m_UnitPerMs = (double)m_pDecoder->GetUnitCount() / m_pDecoder->GetDuration();

    int32_t channels = m_pDecoder->GetChannels();
    int32_t sampleRate = m_pDecoder->GetSampleRate();
    int32_t bitsPerSample = m_pDecoder->GetBitsPerSample();
    cout << "channels:" << channels << endl;
    cout << "sampleRate:" << sampleRate << endl;
    cout << "bitsPerSample:" << bitsPerSample << endl;
    err = m_pRenderer->SetupDevice(channels, sampleRate, bitsPerSample);
    if (err != ErrorCode::Ok) {
        cout << "failed to set renderer:" << err << endl;
        return err;
    }

    m_Status = PlayerStatus::Stopped;

    return ErrorCode::Ok;
}

void Player::Close()
{
    m_pDecoder->Close();

    m_Status = PlayerStatus::Closed;
}

void Player::Play()
{
    uint64_t beg = 0;
    uint64_t end = m_pDecoder->GetUnitCount();
    PlayRange(beg, end);
}

void Player::Play(uint64_t msBegin, uint64_t msEnd)
{
    const uint64_t total = m_pDecoder->GetUnitCount();

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

    m_pDecoder->SetUnitIndex(m_UnitBeg);

    m_UnitBuffers.ResetPV();

    m_SuspendRenderer = false;
    m_SemWakeRenderer.Post();

    m_SuspendDecoder = false;
    m_SemWakeDecoder.Post();

    m_Status = PlayerStatus::Playing;
}

void Player::Pause()
{
    if (!m_SuspendRenderer) {
        m_SuspendRenderer = true;
        m_UnitBuffers.RecycleFree(NULL);
        m_SemRendererSuspended.Wait();
    }

    if (!m_SuspendDecoder) {
        m_SuspendDecoder = true;
        m_UnitBuffers.RecycleData(NULL);
        m_SemDecoderSuspended.Wait();
    }

    m_UnitBuffers.ResetPV();

    m_Status = PlayerStatus::Paused;
}

void Player::Resume()
{
    m_DecoderIndex = m_RendererIndex;
    m_pDecoder->SetUnitIndex(m_DecoderIndex);

    m_UnitBuffers.ResetPV();

    m_SuspendRenderer = false;
    m_SuspendDecoder = false;
    m_SemWakeRenderer.Post();
    m_SemWakeDecoder.Post();

    m_Status = PlayerStatus::Playing;
}

void Player::Stop()
{
    Pause();

    m_Status = PlayerStatus::Stopped;
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
    if (unitPos > m_pDecoder->GetUnitCount())
        unitPos = m_pDecoder->GetUnitCount();
    m_pDecoder->SetUnitIndex(unitPos);
    m_DecoderIndex = unitPos;
    m_RendererIndex = unitPos;
}

int32_t Player::GetBitRate() const
{
    return (m_pDecoder != NULL) ? m_pDecoder->GetBitRate() : -1;
}

int32_t Player::GetSampleRate() const
{
    return (m_pDecoder != NULL) ? m_pDecoder->GetSampleRate() : -1;
}

EmAudioMode Player::GetAudioMode() const
{
    return (m_pDecoder != NULL) ? m_pDecoder->GetAudioMode() : AudioMode::None;
}

uint64_t Player::GetDuration() const
{
    return m_pDecoder->GetDuration();
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

void Player::WorkForDecoder()
{
    while (true) {
        m_SemWakeDecoder.Wait();
        if (m_StopDecoder)
            break;

        bool suspendedBySelf = false;

        for (UnitBuffer* buf = NULL; ; ) {
            buf = m_UnitBuffers.TakeFree();
            if (m_SuspendDecoder)
                break;

            if (buf == NULL) {
                cout << "FATAL: NULL buf!!" << endl;
                continue;
            }
            
            if (buf->data == NULL) {
                cout << "FATAL: NULL buf data!!" << endl;
                continue;
            }

            m_pDecoder->ReadUnit(buf->data, buf->used, buf->unitCount);
            m_UnitBuffers.RecycleFree(buf);

            m_DecoderIndex += buf->unitCount;
            if (m_DecoderIndex >= m_UnitEnd) {
                suspendedBySelf = true;
                m_SuspendDecoder = true;
                break;
            }
        }

        if (!suspendedBySelf)
            m_SemDecoderSuspended.Post();
    }
}

void Player::WorkForRenderer()
{
    while (true) {
        m_SemWakeRenderer.Wait();
        if (m_StopRenderer)
            break;

        bool suspendedBySelf = false;
        for (UnitBuffer* buf = NULL; ; ) {
            //cout << "(" << m_UnitBuffers.GetDataCount() << ")" << endl;
            buf = m_UnitBuffers.TakeData();
            if (m_SuspendRenderer)
                break;

            m_pRenderer->WriteDevice(buf->data, buf->used);
            m_UnitBuffers.RecycleData(buf);

            m_RendererIndex += buf->unitCount;
            if (m_RendererIndex >= m_UnitEnd) {
                suspendedBySelf = true;
                m_SuspendRenderer = true;
                break;
            }
        }

        if (!suspendedBySelf)
            m_SemRendererSuspended.Post();

        if (m_RendererIndex >= m_UnitEnd)
            SigFinished();
    }
}
