#pragma once

#include <assert.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <thread>
using namespace std;

#include <scx/Conv.h>
#include <scx/FileHelper.h>
#include <scx/Mailbox.h>
using namespace scx;

#include <util/Plugin.h>
#include <plugin/Decoder.h>
#include <plugin/Output.h>

namespace mous {

struct UnitBuffer
{
    uint32_t used;
    uint32_t unitCount;
    char data[];
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
    OUTPUT = 1u << 5,
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

                            std::get<TYPE>(mail) = OUTPUT;
                            m_OutputMailbox.PushBack(std::move(mail));

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

        m_OutputThread = std::thread([this]() {
            int status = IDLE;
            int quit = false;

            while (!quit) {
                auto mail = m_OutputMailbox.Take();

                const int opcode = status | std::get<TYPE>(mail);

                switch (opcode) {
                    case IDLE | QUIT:
                    case RUNNING | QUIT: {
                        quit = true;
                    } break;

                    case IDLE | PROCEED: {
                        status = RUNNING;
                    } break;

                    case IDLE | OUTPUT: {
                        std::get<TYPE>(mail) = 0;
                        m_BufferMailbox.PushBack(std::move(mail));
                    } break;

                    case IDLE | SUSPEND:
                    case RUNNING | SUSPEND: {
                        status = IDLE;
                    } break;

                    case RUNNING | OUTPUT: {
                        auto& buf = std::get<DATA>(mail);

                        if (m_OutputIndex < m_UnitEnd) {
                            if (m_Output->Write(buf->data, buf->used) != ErrorCode::Ok) {
                                // avoid busy write
                                const int64_t delay = buf->unitCount / m_UnitPerMs * 1e6;
                                std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
                            }
                            m_OutputIndex += buf->unitCount;

                            std::get<TYPE>(mail) = DECODE;
                            m_DecoderMailbox.PushBack(std::move(mail));

                            if (m_OutputIndex >= m_UnitEnd) {
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

        m_DecoderMailbox.EmplaceFront(QUIT, nullptr, std::weak_ptr<Mailbox>());
        m_OutputMailbox.EmplaceFront(QUIT, nullptr, std::weak_ptr<Mailbox>());

        if (m_DecoderThread.joinable()) {
            m_DecoderThread.join();
        }
        if (m_OutputThread.joinable()) {
            m_OutputThread.join();
        }

        m_BufferMailbox.Clear();

        UnloadPlugin();
    }

    PlayerStatus Status() const { return m_Status; }

    void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto decoder = std::make_shared<Decoder>(plugin);
        if (!decoder || !*decoder) {
            return;
        }
        const vector<string>& list = decoder->FileSuffix();
        for (const string& item : list) {
            const string& suffix = ToLower(item);
            auto iter = m_Decoders.find(suffix);
            if (iter == m_Decoders.end()) {
                m_Decoders.emplace(suffix, decoder);
            }
        }
    }

    void LoadOutputPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        m_Output = std::make_unique<Output>(plugin);
        if (!m_Output || !*m_Output) {
            return;
        }
        m_Output->Open();
    }

    void UnloadPlugin(const std::string&)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        Close();

        if (m_Decoder) {
            m_Decoder->Close();
        }
        m_Decoder.reset();
        m_Decoders.clear();

        if (m_Output) {
            m_Output->Close();
            m_Output.reset();
        }
    }

    vector<string> SupportedSuffixes() const
    {
        vector<string> list;
        list.reserve(m_Decoders.size());
        for (const auto& entry : m_Decoders) {
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

    int Volume() const {
        return m_Output ? m_Output->GetVolume() : -1;
    }

    void SetVolume(int level)
    {
        if (m_Output) {
            m_Output->SetVolume(level);
        }
    }

    ErrorCode Open(const string& path)
    {
        if (!m_Output) {
            return ErrorCode::PlayerNoOutput;
        }

        string suffix = ToLower(FileHelper::FileSuffix(path));
        auto iter = m_Decoders.find(suffix);
        if (iter == m_Decoders.end()) {
            return ErrorCode::PlayerNoDecoder;
        }
        m_Decoder = iter->second;

        ErrorCode err = m_Decoder->Open(path);
        if (err != ErrorCode::Ok) {
            return err;
        }
        m_DecodeFile = path;

        // TODO: add log for buffer size and count
        const uint32_t maxBytesPerUnit = m_Decoder->MaxBytesPerUnit();
        m_BufferMailbox.Clear();
        m_Buffer = std::make_unique<char[]>(m_BufferCount * (sizeof(UnitBuffer) + maxBytesPerUnit));
        for (int i = 0; i < m_BufferCount; ++i) {
            auto ptr = m_Buffer.get() + (sizeof(UnitBuffer) + maxBytesPerUnit) * i;
            auto buf = reinterpret_cast<UnitBuffer*>(ptr);
            m_BufferMailbox.EmplaceBack(0, buf, std::weak_ptr<Mailbox>());
        }

        m_UnitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();

        int32_t channels = m_Decoder->Channels();
        int32_t samleRate = m_Decoder->SampleRate();
        int32_t bitsPerSamle = m_Decoder->BitsPerSample();
        // TODO: add log
        err = m_Output->Setup(channels, samleRate, bitsPerSamle);
        if (err != ErrorCode::Ok) {
            cout << "FATAL: failed to set output:" << static_cast<uint8_t>(err) << endl;
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

        // TODO: add log for range and total
        PlayRange(beg, end);
    }

    void PlayRange(uint64_t beg, uint64_t end)
    {
        m_UnitBeg = beg;
        m_UnitEnd = end;

        m_OutputIndex = m_UnitBeg;
        m_OutputMailbox.EmplaceBack(PROCEED, nullptr, std::weak_ptr<Mailbox>());

        m_DecoderIndex = m_UnitBeg;
        m_Decoder->SetUnitIndex(m_UnitBeg);
        m_DecoderMailbox.EmplaceBack(PROCEED, nullptr, std::weak_ptr<Mailbox>());
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

        m_DecoderMailbox.EmplaceFront(SUSPEND, nullptr, std::weak_ptr<Mailbox>());
        m_OutputMailbox.EmplaceFront(SUSPEND, nullptr, std::weak_ptr<Mailbox>());
        m_BufferMailbox.Wait(m_BufferCount);

        m_Status = PlayerStatus::Paused;
    }

    void Resume()
    {
        m_OutputMailbox.EmplaceBack(PROCEED, nullptr, std::weak_ptr<Mailbox>());

        m_DecoderIndex = m_OutputIndex;
        m_Decoder->SetUnitIndex(m_DecoderIndex);
        m_DecoderMailbox.EmplaceBack(PROCEED, nullptr, std::weak_ptr<Mailbox>());
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
        m_OutputIndex = unit;
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

    uint64_t CurrentMs() const { return m_OutputIndex / m_UnitPerMs; }

    enum AudioMode AudioMode() const { return (m_Decoder != nullptr) ? m_Decoder->AudioMode() : AudioMode::None; }

    Signal<void(void)>* SigFinished() { return &m_SigFinished; }

  private:
    PlayerStatus m_Status = PlayerStatus::Closed;

    std::string m_DecodeFile;

    std::shared_ptr<Decoder> m_Decoder;
    std::unique_ptr<Output> m_Output;
    std::thread m_DecoderThread;
    std::thread m_OutputThread;
    Mailbox m_DecoderMailbox;
    Mailbox m_OutputMailbox;

    Mailbox m_BufferMailbox;
    int m_BufferCount = 5;
    std::unique_ptr<char[]> m_Buffer;

    uint64_t m_UnitBeg = 0;
    uint64_t m_UnitEnd = 0;

    uint64_t m_DecoderIndex = 0;
    uint64_t m_OutputIndex = 0;

    double m_UnitPerMs = 0;

    std::map<std::string, std::shared_ptr<Decoder>> m_Decoders;

    scx::Signal<void(void)> m_SigFinished;
};
}
