#pragma once

#include <assert.h>
#include <unistd.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <thread>
using namespace std;

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
#include <scx/LPVBuffer.hpp>
#include <scx/SemVar.hpp>
using namespace scx;

#include <core/Plugin.h>
#include <plugin/IDecoder.h>
#include <plugin/IRenderer.h>

namespace mous {

struct UnitBuffer
{
    char* data;
    uint32_t used;
    uint32_t max;

    uint32_t unitCount;

    UnitBuffer()
      : data(nullptr)
      , used(0)
      , max(0)
      , unitCount(0)
    {
    }

    ~UnitBuffer()
    {
        if (data != nullptr) {
            delete[] data;
        }
        data = nullptr;
        used = 0;
        max = 0;
        unitCount = 0;
    }
};

struct DecoderPluginNode
{
    const Plugin* agent;
    IDecoder* decoder;
};

class Player::Impl
{
  public:
    Impl()
    {
        m_UnitBuffers.AllocBuffer(5);

        m_ThreadForDecoder = std::thread([this]() {
            while (true) {
                m_SemWakeDecoder.Wait();
                if (m_StopDecoder) {
                    break;
                }

                m_SemDecoderBegin.Clear();
                m_SemDecoderEnd.Clear();

                m_SemDecoderBegin.Post();

                for (UnitBuffer* buf = nullptr;;) {
                    if (m_PauseDecoder) {
                        break;
                    }

                    buf = m_UnitBuffers.TakeFree();
                    if (m_SuspendDecoder) {
                        break;
                    }

                    assert(buf != nullptr);
                    assert(buf->data != nullptr);

                    m_Decoder->DecodeUnit(buf->data, buf->used, buf->unitCount);
                    m_DecoderIndex += buf->unitCount;
                    m_UnitBuffers.RecycleFree();

                    if (m_DecoderIndex >= m_UnitEnd) {
                        m_SuspendDecoder = true;
                        break;
                    }
                }

                m_SemDecoderEnd.Post();
            }
        });

        m_ThreadForRenderer = std::thread([this]() {
            while (true) {
                m_SemWakeRenderer.Wait();
                if (m_StopRenderer) {
                    break;
                }

                m_SemRendererBegin.Clear();
                m_SemRendererEnd.Clear();

                m_SemRendererBegin.Post();

                for (UnitBuffer* buf = nullptr;;) {
                    buf = m_UnitBuffers.TakeData();
                    if (m_SuspendRenderer) {
                        break;
                    }

                    assert(buf != nullptr);
                    assert(buf->data != nullptr);

                    // avoid busy write
                    if (m_Renderer->Write(buf->data, buf->used) != ErrorCode::Ok) {
                        ::usleep(10 * 1000);
                    }
                    m_RendererIndex += buf->unitCount;
                    m_UnitBuffers.RecycleData();

                    if (m_RendererIndex >= m_UnitEnd) {
                        m_SuspendRenderer = true;
                        break;
                    }
                }

                m_SemRendererEnd.Post();

                if (m_RendererIndex >= m_UnitEnd) {
                    m_Status = PlayerStatus::Stopped;
                    std::thread([this]() { m_SigFinished(); }).detach();
                }
            }
        });
    }

    ~Impl()
    {
        Close();

        m_StopDecoder = true;
        m_StopRenderer = true;
        m_SemWakeDecoder.Post();
        m_SemWakeRenderer.Post();

        if (m_ThreadForDecoder.joinable()) {
            m_ThreadForDecoder.join();
        }
        if (m_ThreadForRenderer.joinable()) {
            m_ThreadForRenderer.join();
        }

        m_UnitBuffers.ClearBuffer();

        UnregisterAll();
    }

    EmPlayerStatus Status() const { return m_Status; }

    void RegisterDecoderPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::Decoder) {
            AddDecoderPlugin(pAgent);
        }
    }

    void RegisterDecoderPlugin(vector<const Plugin*>& agents)
    {
        for (const auto agent : agents) {
            RegisterDecoderPlugin(agent);
        }
    }

    void RegisterRendererPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::Renderer) {
            SetRendererPlugin(pAgent);
        }
    }

    void UnregisterPlugin(const Plugin* pAgent)
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

    void UnregisterPlugin(vector<const Plugin*>& agents)
    {
        for (const auto agent : agents) {
            UnregisterPlugin(agent);
        }
    }

    void UnregisterAll()
    {
        while (!m_DecoderPluginMap.empty()) {
            auto iter = m_DecoderPluginMap.begin();
            RemoveDecoderPlugin(iter->second.agent);
        }

        UnsetRendererPlugin(m_RendererPlugin);
    }

    vector<string> SupportedSuffixes() const
    {
        vector<string> list;
        list.reserve(m_DecoderPluginMap.size());
        for (const auto& entry : m_DecoderPluginMap)
            list.push_back(entry.first);
        return list;
    }

    int BufferCount() const { return m_UnitBuffers.BufferCount(); }

    void SetBufferCount(int count)
    {
        m_UnitBuffers.ClearBuffer();
        m_UnitBuffers.AllocBuffer(count);
    }

    int Volume() const { return m_Renderer != nullptr ? m_Renderer->VolumeLevel() : -1; }

    void SetVolume(int level)
    {
        if (m_Renderer != nullptr) {
            m_Renderer->SetVolumeLevel(level);
        }
    }

    EmErrorCode Open(const string& path)
    {
        string suffix = ToLower(FileHelper::FileSuffix(path));
        // cout << "Suffix:" << suffix << endl;
        auto iter = m_DecoderPluginMap.find(suffix);
        if (iter != m_DecoderPluginMap.end()) {
            m_Decoder = iter->second.decoder;
        } else {
            return ErrorCode::PlayerNoDecoder;
        }

        if (m_Renderer == nullptr) {
            return ErrorCode::PlayerNoRenderer;
        }

        EmErrorCode err = m_Decoder->Open(path);
        if (err != ErrorCode::Ok) {
            // cout << "FATAL: failed to open!" << endl;
            return err;
        } else {
            m_DecodeFile = path;
        }

        uint32_t maxBytesPerUnit = m_Decoder->MaxBytesPerUnit();
        for (size_t i = 0; i < m_UnitBuffers.BufferCount(); ++i) {
            UnitBuffer* buf = m_UnitBuffers.RawItemAt(i);
            buf->used = 0;
            if (buf->max < maxBytesPerUnit) {
                if (buf->data != nullptr) {
                    delete[] buf->data;
                    // cout << "free unit buf:" << buf->max << endl;
                }
                buf->data = new char[maxBytesPerUnit];
                buf->max = maxBytesPerUnit;
                // cout << "alloc unit buf:" << buf->max << endl;
            }
        }
        // cout << "unit buf size:" << maxBytesPerUnit << endl;

        m_UnitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();

        int32_t channels = m_Decoder->Channels();
        int32_t samleRate = m_Decoder->SampleRate();
        int32_t bitsPerSamle = m_Decoder->BitsPerSample();
        // cout << "channels:" << channels << endl;
        // cout << "samleRate:" << samleRate << endl;
        // cout << "bitsPerSamle:" << bitsPerSamle << endl;
        err = m_Renderer->Setup(channels, samleRate, bitsPerSamle);
        if (err != ErrorCode::Ok) {
            cout << "FATAL: failed to set renderer:" << err << endl;
            cout << "       channels:" << channels << endl;
            cout << "       samleRate:" << samleRate << endl;
            cout << "       bitsPerSamle:" << bitsPerSamle << endl;
            return err;
        }

        m_Status = PlayerStatus::Stopped;

        return err;
    }

    void Close()
    {
        if (m_Status == PlayerStatus::Closed) {
            return;
        }

        Pause();

        m_Decoder->Close();
        m_Decoder = nullptr;
        m_DecodeFile.clear();

        m_Status = PlayerStatus::Closed;
    }

    string FileName() const { return m_DecodeFile; }

    void Play()
    {
        uint64_t beg = 0;
        uint64_t end = m_Decoder->UnitCount();
        PlayRange(beg, end);
    }

    void Play(uint64_t msBegin, uint64_t msEnd)
    {
        const uint64_t total = m_Decoder->UnitCount();

        uint64_t beg = 0;
        uint64_t end = 0;

        beg = m_UnitPerMs * msBegin;
        if (beg > total) {
            beg = total;
        }

        if (msEnd != (uint64_t)-1) {
            end = m_UnitPerMs * msEnd;
            if (end > total) {
                end = total;
            }
        } else {
            end = total;
        }

        // cout << "begin:" << beg << endl;
        // cout << "end:" << end << endl;
        // cout << "total:" << total << endl;

        PlayRange(beg, end);
    }

    void PlayRange(uint64_t beg, uint64_t end)
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

    void Pause()
    {
        if (m_Status == PlayerStatus::Paused) {
            return;
        }

        // suspend renderer
        if (!m_SuspendRenderer) {
            m_SuspendRenderer = true;
            m_UnitBuffers.RecycleFree();
        }
        m_SemRendererEnd.Wait();

        // suspend decoder
        if (!m_SuspendDecoder) {
            m_SuspendDecoder = true;
            m_UnitBuffers.RecycleData();
        }
        m_SemDecoderEnd.Wait();

        m_UnitBuffers.ResetPV();

        m_Status = PlayerStatus::Paused;
    }

    void Resume()
    {
        m_DecoderIndex = m_RendererIndex;
        m_Decoder->SetUnitIndex(m_DecoderIndex);

        m_UnitBuffers.ResetPV();

        // resume renderer & decoder
        m_SuspendRenderer = false;
        m_SemWakeRenderer.Post();
        m_SuspendDecoder = false;
        m_SemWakeDecoder.Post();
        m_SemRendererBegin.Wait();
        m_SemDecoderBegin.Wait();

        m_Status = PlayerStatus::Playing;
    }

    void SeekTime(uint64_t msPos)
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

    void SeekPercent(double percent)
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

    void DoSeekTime(uint64_t msPos)
    {
        uint64_t unitPos = std::min((uint64_t)(m_UnitPerMs * msPos), m_Decoder->UnitCount());
        DoSeekUnit(unitPos);
    }

    void DoSeekUnit(uint64_t unit)
    {
        if (unit < m_UnitBeg) {
            unit = m_UnitBeg;
        } else if (unit > m_UnitEnd) {
            unit = m_UnitEnd;
        }

        m_Decoder->SetUnitIndex(unit);

        m_DecoderIndex = unit;
        m_RendererIndex = unit;
    }

    void PauseDecoder()
    {
        // cout << "data:" << m_UnitBuffers.DataCount() << endl;
        // cout << "free:" << m_UnitBuffers.FreeCount() << endl;

        if (!m_PauseDecoder) {
            m_PauseDecoder = true;
        }
        m_SemDecoderEnd.Wait();

        m_Decoder->Close();
    }

    void ResumeDecoder()
    {
        // cout << "data:" << m_UnitBuffers.DataCount() << endl;
        // cout << "free:" << m_UnitBuffers.FreeCount() << endl;

        m_Decoder->Open(m_DecodeFile);
        m_Decoder->SetUnitIndex(m_DecoderIndex);

        m_PauseDecoder = false;
        m_SemWakeDecoder.Post();
        m_SemDecoderBegin.Wait();
    }

    int32_t BitRate() const { return (m_Decoder != nullptr) ? m_Decoder->BitRate() : -1; }

    int32_t SamleRate() const { return (m_Decoder != nullptr) ? m_Decoder->SampleRate() : -1; }

    uint64_t Duration() const { return m_Decoder->Duration(); }

    uint64_t RangeBegin() const { return m_UnitBeg / m_UnitPerMs; }

    uint64_t RangeEnd() const { return m_UnitEnd / m_UnitPerMs; }

    uint64_t RangeDuration() const { return (m_UnitEnd - m_UnitBeg) / m_UnitPerMs; }

    uint64_t OffsetMs() const { return CurrentMs() - RangeBegin(); }

    uint64_t CurrentMs() const { return m_RendererIndex / m_UnitPerMs; }

    EmAudioMode AudioMode() const { return (m_Decoder != nullptr) ? m_Decoder->AudioMode() : AudioMode::None; }

    std::vector<PluginOption> DecoderPluginOption() const
    {
        std::vector<PluginOption> list;

        list.reserve(m_DecoderPluginMap.size());

        for (const auto entry : m_DecoderPluginMap) {
            auto node = entry.second;
            list.emplace_back(node.agent->Type(), node.agent->Info(), node.decoder->Options());
        }

        return list;
    }

    PluginOption RendererPluginOption() const
    {
        if (m_RendererPlugin != nullptr && m_Renderer != nullptr) {
            return { m_RendererPlugin->Type(), m_RendererPlugin->Info(), m_Renderer->Options() };
        } else {
            return {};
        }
    }

    Signal<void(void)>* SigFinished() { return &m_SigFinished; }

  private:
    void AddDecoderPlugin(const Plugin* pAgent)
    {
        // create Decoder & get suffix
        IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
        const vector<string>& list = pDecoder->FileSuffix();

        // try add
        bool usedAtLeastOnce = false;
        for (const string& item : list) {
            const string& suffix = ToLower(item);
            auto iter = m_DecoderPluginMap.find(suffix);
            if (iter == m_DecoderPluginMap.end()) {
                m_DecoderPluginMap.emplace(suffix, DecoderPluginNode{ pAgent, pDecoder });
                usedAtLeastOnce = true;
            }
        }

        // clear if not used
        if (!usedAtLeastOnce) {
            pAgent->FreeObject(pDecoder);
        }
    }

    void RemoveDecoderPlugin(const Plugin* pAgent)
    {
        // get suffix
        IDecoder* pDecoder = (IDecoder*)pAgent->CreateObject();
        const vector<string>& list = pDecoder->FileSuffix();
        pAgent->FreeObject(pDecoder);

        // find plugin
        bool freedOnce = false;
        for (const string& item : list) {
            const string& suffix = ToLower(item);
            auto iter = m_DecoderPluginMap.find(suffix);
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

    void SetRendererPlugin(const Plugin* pAgent)
    {
        if (pAgent == nullptr || m_RendererPlugin != nullptr) {
            return;
        }

        m_RendererPlugin = pAgent;
        m_Renderer = (IRenderer*)pAgent->CreateObject();
        m_Renderer->Open();
    }

    void UnsetRendererPlugin(const Plugin* pAgent)
    {
        if (pAgent != m_RendererPlugin || m_RendererPlugin == nullptr) {
            return;
        }

        m_Renderer->Close();
        m_RendererPlugin->FreeObject(m_Renderer);
        m_Renderer = nullptr;
        m_RendererPlugin = nullptr;
    }

  private:
    EmPlayerStatus m_Status = PlayerStatus::Closed;

    string m_DecodeFile;
    bool m_StopDecoder = false;
    bool m_SuspendDecoder = true;
    bool m_PauseDecoder = false;
    IDecoder* m_Decoder = nullptr;
    std::thread m_ThreadForDecoder;
    scx::SemVar m_SemWakeDecoder;
    scx::SemVar m_SemDecoderBegin;
    scx::SemVar m_SemDecoderEnd;

    bool m_StopRenderer = false;
    bool m_SuspendRenderer = true;
    IRenderer* m_Renderer = nullptr;
    std::thread m_ThreadForRenderer;
    scx::SemVar m_SemWakeRenderer;
    scx::SemVar m_SemRendererBegin;
    scx::SemVar m_SemRendererEnd;

    scx::LPVBuffer<UnitBuffer> m_UnitBuffers;

    uint64_t m_UnitBeg = 0;
    uint64_t m_UnitEnd = 0;

    uint64_t m_DecoderIndex = 0;
    uint64_t m_RendererIndex = 0;

    double m_UnitPerMs = 0;

    const Plugin* m_RendererPlugin = nullptr;

    std::map<std::string, DecoderPluginNode> m_DecoderPluginMap;

    scx::Signal<void(void)> m_SigFinished;
};
}
