#pragma once

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
using namespace std;

#include <scx/Conv.h>
#include <scx/FileHelper.h>
#include <scx/Mailbox.h>
using namespace scx;

#include <core/Plugin.h>
#include <plugin/IDecoder.h>
#include <plugin/IRenderer.h>

namespace mous {
struct UnitBuffer
{
    uint32_t used;
    uint32_t unitCount;
    char data[];
};

struct DecoderPluginNode
{
    const Plugin* agent;
    IDecoder* decoder;
};

enum : std::size_t
{
    // mail contents
    TYPE = 0,
    DATA = 1,
    FROM = 2,

    // worker status
    IDLE = 1u << 0,
    RUNNING = 1u << 1,

    // mail type
    PROCEED = 1u << 2,
    SUSPEND = 1u << 3,
    DECODE = 1u << 4,
    RENDER = 1u << 5,
    QUIT = 1u << 6
};

class Player::Impl
{
  using Mailbox = scx::Mailbox<int, UnitBuffer*>;
  using Mail = Mailbox::Mail;

  public:
    Impl()
    {
        m_DecoderThread = std::thread([this]() {
            int status = IDLE;
            int quit = false;

            while (!quit) {
                auto mail = m_DecoderMailbox.Take();

                const int opcode = status | std::get<TYPE>(mail);

                switch (opcode) {
                    case IDLE | QUIT:
                    case RUNNING | QUIT: {
                        quit = true;
                    } break;

                    case IDLE | PROCEED: {
                        status = RUNNING;
                    } break;

                    case IDLE | DECODE: {
                        std::get<TYPE>(mail) = 0;
                        m_BufferMailbox.PushBack(std::move(mail));
                    } break;

                    case IDLE | SUSPEND:
                    case RUNNING | SUSPEND: {
                        status = IDLE;
                    } break;

                    case RUNNING | DECODE: {
                        auto& buf = std::get<DATA>(mail);

                        if (m_DecoderIndex < m_UnitEnd) {
                            m_Decoder->DecodeUnit(buf->data, buf->used, buf->unitCount);
                            m_DecoderIndex += buf->unitCount;

                            std::get<TYPE>(mail) = RENDER;
                            m_RendererMailbox.PushBack(std::move(mail));

                            if (m_DecoderIndex >= m_UnitEnd) {
                                status = IDLE;
                            }
                        } else {
                            std::get<TYPE>(mail) = 0;
                            m_BufferMailbox.PushBack(std::move(mail));
                        }
                    } break;

                    default: {
                        // unexpected
                    } break;
                }
            }
        });

        m_RendererThread = std::thread([this]() {
            int status = IDLE;
            int quit = false;

            while (!quit) {
                auto mail = m_RendererMailbox.Take();

                const int opcode = status | std::get<TYPE>(mail);

                switch (opcode) {
                    case IDLE | QUIT:
                    case RUNNING | QUIT: {
                        quit = true;
                    } break;

                    case IDLE | PROCEED: {
                        status = RUNNING;
                    } break;

                    case IDLE | RENDER: {
                        std::get<TYPE>(mail) = 0;
                        m_BufferMailbox.PushBack(std::move(mail));
                    } break;

                    case IDLE | SUSPEND:
                    case RUNNING | SUSPEND: {
                        status = IDLE;
                    } break;

                    case RUNNING | RENDER: {
                        auto& buf = std::get<DATA>(mail);

                        if (m_RendererIndex < m_UnitEnd) {
                            if (m_Renderer->Write(buf->data, buf->used) != ErrorCode::Ok) {
                                // avoid busy write
                                const int64_t delay = buf->unitCount / m_UnitPerMs * 1e6;
                                std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
                            }
                            m_RendererIndex += buf->unitCount;

                            std::get<TYPE>(mail) = DECODE;
                            m_DecoderMailbox.PushBack(std::move(mail));

                            if (m_RendererIndex >= m_UnitEnd) {
                                status = IDLE;
                                std::thread([this]() { m_SigFinished(); }).detach();
                            }
                        } else {
                            std::get<TYPE>(mail) = 0;
                            m_BufferMailbox.PushBack(std::move(mail));
                        }
                    } break;

                    default: {
                        // unexpected
                    } break;
                }
            }
        });
    }

    ~Impl()
    {
        Close();

        m_DecoderMailbox.EmplaceFront(QUIT);
        m_RendererMailbox.EmplaceFront(QUIT);

        if (m_DecoderThread.joinable()) {
            m_DecoderThread.join();
        }
        if (m_RendererThread.joinable()) {
            m_RendererThread.join();
        }

        m_BufferMailbox.Clear();

        UnregisterAll();
    }

    PlayerStatus Status() const { return m_Status; }

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
        for (const auto& entry : m_DecoderPluginMap) {
            list.push_back(entry.first);
        }
        return list;
    }

    int BufferCount() const
    {
        return m_BufferCount;
    }

    void SetBufferCount(int count)
    {
        m_BufferCount = count;
    }

    int Volume() const { return m_Renderer != nullptr ? m_Renderer->VolumeLevel() : -1; }

    void SetVolume(int level)
    {
        if (m_Renderer != nullptr) {
            m_Renderer->SetVolumeLevel(level);
        }
    }

    ErrorCode Open(const string& path)
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

        ErrorCode err = m_Decoder->Open(path);
        if (err != ErrorCode::Ok) {
            // cout << "FATAL: failed to open!" << endl;
            return err;
        } else {
            m_DecodeFile = path;
        }

        const uint32_t maxBytesPerUnit = m_Decoder->MaxBytesPerUnit();
        // cout << "unit buf size:" << maxBytesPerUnit << endl;

        m_BufferMailbox.Clear();
        m_Buffer = std::make_unique<char[]>(m_BufferCount * (sizeof(UnitBuffer) + maxBytesPerUnit));
        for (size_t i = 0; i < m_BufferCount; ++i) {
            auto ptr = m_Buffer.get() + (sizeof(UnitBuffer) + maxBytesPerUnit) * i;
            UnitBuffer* unitBuffer = reinterpret_cast<UnitBuffer*>(ptr);
            m_BufferMailbox.EmplaceBack(0, unitBuffer);
        }

        m_UnitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();

        int32_t channels = m_Decoder->Channels();
        int32_t samleRate = m_Decoder->SampleRate();
        int32_t bitsPerSamle = m_Decoder->BitsPerSample();
        // cout << "channels:" << channels << endl;
        // cout << "samleRate:" << samleRate << endl;
        // cout << "bitsPerSamle:" << bitsPerSamle << endl;
        err = m_Renderer->Setup(channels, samleRate, bitsPerSamle);
        if (err != ErrorCode::Ok) {
            cout << "FATAL: failed to set renderer:" << static_cast<uint8_t>(err) << endl;
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

        m_RendererIndex = m_UnitBeg;
        m_RendererMailbox.EmplaceBack(PROCEED);

        m_DecoderIndex = m_UnitBeg;
        m_Decoder->SetUnitIndex(m_UnitBeg);
        m_DecoderMailbox.EmplaceBack(PROCEED);
        while (!m_BufferMailbox.Empty()) {
            auto mail = m_BufferMailbox.Take();
            std::get<TYPE>(mail) = DECODE;
            m_DecoderMailbox.PushBack(std::move(mail));
        }

        m_Status = PlayerStatus::Playing;
    }

    void Pause()
    {
        if (m_Status == PlayerStatus::Paused) {
            return;
        }

        m_DecoderMailbox.EmplaceFront(SUSPEND);
        m_RendererMailbox.EmplaceFront(SUSPEND);
        m_BufferMailbox.Wait(m_BufferCount);

        m_Status = PlayerStatus::Paused;
    }

    void Resume()
    {
        m_RendererMailbox.EmplaceBack(PROCEED);

        m_DecoderIndex = m_RendererIndex;
        m_Decoder->SetUnitIndex(m_DecoderIndex);
        m_DecoderMailbox.EmplaceBack(PROCEED);
        while (!m_BufferMailbox.Empty()) {
            auto mail = m_BufferMailbox.Take();
            std::get<TYPE>(mail) = DECODE;
            m_DecoderMailbox.PushBack(std::move(mail));
        }

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
        /*
        if (!m_PauseDecoder) {
            m_PauseDecoder = true;
        }
        m_SemDecoderEnd.Wait();

        m_Decoder->Close();
        */
    }

    void ResumeDecoder()
    {
        /*
        // cout << "data:" << m_UnitBuffers.DataCount() << endl;
        // cout << "free:" << m_UnitBuffers.FreeCount() << endl;

        m_Decoder->Open(m_DecodeFile);
        m_Decoder->SetUnitIndex(m_DecoderIndex);

        m_PauseDecoder = false;
        m_SemWakeDecoder.Post();
        m_SemDecoderBegin.Wait();
        */
    }

    int32_t BitRate() const { return (m_Decoder != nullptr) ? m_Decoder->BitRate() : -1; }

    int32_t SamleRate() const { return (m_Decoder != nullptr) ? m_Decoder->SampleRate() : -1; }

    uint64_t Duration() const { return m_Decoder->Duration(); }

    uint64_t RangeBegin() const { return m_UnitBeg / m_UnitPerMs; }

    uint64_t RangeEnd() const { return m_UnitEnd / m_UnitPerMs; }

    uint64_t RangeDuration() const { return (m_UnitEnd - m_UnitBeg) / m_UnitPerMs; }

    uint64_t OffsetMs() const { return CurrentMs() - RangeBegin(); }

    uint64_t CurrentMs() const { return m_RendererIndex / m_UnitPerMs; }

    enum AudioMode AudioMode() const { return (m_Decoder != nullptr) ? m_Decoder->AudioMode() : AudioMode::None; }

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
    PlayerStatus m_Status = PlayerStatus::Closed;

    std::string m_DecodeFile;

    IDecoder* m_Decoder = nullptr;
    IRenderer* m_Renderer = nullptr;
    std::thread m_DecoderThread;
    std::thread m_RendererThread;
    Mailbox m_DecoderMailbox;
    Mailbox m_RendererMailbox;

    Mailbox m_BufferMailbox;
    int m_BufferCount = 5;
    std::unique_ptr<char[]> m_Buffer;

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
