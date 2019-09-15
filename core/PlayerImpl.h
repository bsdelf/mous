#pragma once

#include <assert.h>
#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <map>
#include <thread>
using namespace std;

#include <scx/BlockingQueue.h>
#include <scx/Conv.h>
#include <scx/FileHelper.h>
using namespace scx;

#include <plugin/Decoder.h>
#include <plugin/FormatProbe.h>
#include <plugin/Output.h>
#include <util/Plugin.h>

namespace mous {

struct UnitBuffer {
  uint32_t used;
  uint32_t unitCount;
  char data[];
};

struct Mail {
  size_t action;
  UnitBuffer* buffer;

  Mail(size_t action, UnitBuffer* buffer)
      : action(action), buffer(buffer) {
  }
};

using Mailbox = scx::BlockingQueue<Mail>;

enum : std::size_t {
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

class Player::Impl {
 public:
  Impl() {
    m_DecoderThread = std::thread([this]() {
      int status = IDLE;
      int quit = false;

      while (!quit) {
        auto mail = m_DecoderMailbox.Take();

        const int opcode = status | mail.action;

        switch (opcode) {
          case IDLE | QUIT:
          case RUNNING | QUIT: {
            quit = true;
            break;
          }

          case IDLE | PROCEED: {
            status = RUNNING;
            break;
          }

          case IDLE | DECODE: {
            m_BufferMailbox.PushBack(std::move(mail));
            break;
          }

          case IDLE | SUSPEND:
          case RUNNING | SUSPEND: {
            status = IDLE;
            break;
          }

          case RUNNING | DECODE: {
            if (m_DecoderIndex >= m_UnitEnd) {
              m_BufferMailbox.PushBack(std::move(mail));
              break;
            }
            auto& buf = mail.buffer;
            m_Decoder->DecodeUnit(buf->data, buf->used, buf->unitCount);
            m_DecoderIndex += buf->unitCount;
            if (m_DecoderIndex >= m_UnitEnd) {
              status = IDLE;
            }
            mail.action = OUTPUT;
            m_OutputMailbox.PushBack(std::move(mail));
            break;
          }

          default: {
            // unexpected
            break;
          }
        }
      }
    });

    m_OutputThread = std::thread([this]() {
      int status = IDLE;
      int quit = false;

      while (!quit) {
        auto mail = m_OutputMailbox.Take();

        const int opcode = status | mail.action;

        switch (opcode) {
          case IDLE | QUIT:
          case RUNNING | QUIT: {
            quit = true;
            break;
          }

          case IDLE | PROCEED: {
            status = RUNNING;
            break;
          }

          case IDLE | OUTPUT: {
            m_BufferMailbox.PushBack(std::move(mail));
            break;
          }

          case IDLE | SUSPEND:
          case RUNNING | SUSPEND: {
            status = IDLE;
            break;
          }

          case RUNNING | OUTPUT: {
            if (m_OutputIndex >= m_UnitEnd) {
              m_BufferMailbox.PushBack(std::move(mail));
              break;
            }
            auto& buf = mail.buffer;
            if (m_Output->Write(buf->data, buf->used) != ErrorCode::Ok) {
              // avoid busy write
              const int64_t delay = buf->unitCount / m_UnitPerMs * 1e6;
              std::this_thread::sleep_for(std::chrono::nanoseconds(delay));
            }
            m_OutputIndex += buf->unitCount;
            if (m_OutputIndex >= m_UnitEnd) {
              status = IDLE;
              std::thread([this]() { m_SigFinished(); }).detach();
            }
            mail.action = DECODE;
            m_DecoderMailbox.PushBack(std::move(mail));
            break;
          }

          default: {
            // unexpected
            break;
          }
        }
      }
    });
  }

  ~Impl() {
    Close();

    m_DecoderMailbox.EmplaceFront(QUIT, nullptr);
    m_OutputMailbox.EmplaceFront(QUIT, nullptr);

    if (m_DecoderThread.joinable()) {
      m_DecoderThread.join();
    }
    if (m_OutputThread.joinable()) {
      m_OutputThread.join();
    }

    m_BufferMailbox.Clear();

    UnloadPlugin();
  }

  PlayerStatus Status() const {
    return m_Status;
  }

  void LoadFormatProbePlugin(const std::shared_ptr<Plugin>& plugin) {
    auto probe = std::make_shared<FormatProbe>(plugin);
    if (!probe || !*probe) {
      return;
    }
    const auto suffixes = probe->FileSuffix();
    for (const auto& suffix : suffixes) {
      const auto& str = ToLower(suffix);
      auto iter = m_Probes.find(str);
      if (iter == m_Probes.end()) {
        m_Probes.emplace(suffix, probe);
      }
    }
  }

  void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin) {
    auto decoder = std::make_shared<Decoder>(plugin);
    if (!decoder || !*decoder) {
      return;
    }
    const auto& suffixes = decoder->FileSuffix();
    const auto& encodings = decoder->Encodings();
    for (const auto& suffix : suffixes) {
      const auto& str = ToLower(suffix);
      const auto key = "suffix/" + str;
      const auto iter = m_Decoders.find(key);
      if (iter == m_Decoders.end()) {
        m_Decoders.emplace(key, decoder);
      }
    }
    for (const auto& encoding : encodings) {
      const auto& str = ToLower(encoding);
      const auto key = "encoding/" + str;
      const auto iter = m_Decoders.find(key);
      if (iter == m_Decoders.end()) {
        m_Decoders.emplace(key, decoder);
      }
    }
  }

  void LoadOutputPlugin(const std::shared_ptr<Plugin>& plugin) {
    m_Output = std::make_unique<Output>(plugin);
    if (!m_Output || !*m_Output) {
      return;
    }
    m_Output->Open();
  }

  void UnloadPlugin(const std::string&) {
    // TODO
  }

  void UnloadPlugin() {
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

    m_Probes.clear();
  }

  vector<string> SupportedSuffixes() const {
    vector<string> list;
    list.reserve(m_Decoders.size());
    std::string prefix("suffix/");
    for (const auto& entry : m_Decoders) {
      const auto pos = entry.first.find(prefix);
      if (pos == std::string::npos) {
        continue;
      }
      auto&& suffix = entry.first.substr(prefix.size());
      list.push_back(std::move(suffix));
    }
    return list;
  }

  int BufferCount() const {
    return m_BufferCount;
  }

  void SetBufferCount(int count) {
    m_BufferCount = count;
  }

  int Volume() const {
    return m_Output ? m_Output->GetVolume() : -1;
  }

  void SetVolume(int level) {
    if (m_Output) {
      m_Output->SetVolume(level);
    }
  }

  ErrorCode Open(const string& path) {
    std::shared_ptr<Decoder> decoder;
    {
      const auto& suffix = ToLower(FileHelper::FileSuffix(path));
      // detect encoding
      std::string encoding;
      {
        auto iter = m_Probes.find(suffix);
        if (iter != m_Probes.end()) {
          encoding = iter->second->Probe(path);
        }
      }
      if (encoding.size() > 0) {
        const auto iter = m_Decoders.find("encoding/" + encoding);
        if (iter != m_Decoders.end()) {
          decoder = iter->second;
        }
      }
      if (!decoder) {
        auto iter = m_Decoders.find("suffix/" + suffix);
        if (iter != m_Decoders.end()) {
          decoder = iter->second;
        }
      }
    }
    if (!decoder) {
      return ErrorCode::PlayerNoDecoder;
    }
    ErrorCode err = decoder->Open(path);
    if (err != ErrorCode::Ok) {
      return err;
    }

    if (!m_Output) {
      decoder->Close();
      return ErrorCode::PlayerNoOutput;
    }
    int32_t channels = decoder->Channels();
    int32_t sampleRate = decoder->SampleRate();
    int32_t bitsPerSample = decoder->BitsPerSample();
    // TODO: add log
    err = m_Output->Setup(channels, sampleRate, bitsPerSample);
    if (err != ErrorCode::Ok) {
      decoder->Close();
      printf("Failed to set output, error: %u, channels: %d, sample rate: %d, bits: %d\n", err, channels, sampleRate, bitsPerSample);
      return err;
    }

    // TODO: add log for buffer size and count
    const uint32_t maxBytesPerUnit = decoder->MaxBytesPerUnit();
    m_BufferMailbox.Clear();
    m_Buffer = std::make_unique<char[]>(m_BufferCount * (sizeof(UnitBuffer) + maxBytesPerUnit));
    for (int i = 0; i < m_BufferCount; ++i) {
      auto ptr = m_Buffer.get() + (sizeof(UnitBuffer) + maxBytesPerUnit) * i;
      auto buf = reinterpret_cast<UnitBuffer*>(ptr);
      m_BufferMailbox.EmplaceBack(0, buf);
    }
    m_UnitPerMs = (double)decoder->UnitCount() / decoder->Duration();
    m_Status = PlayerStatus::Stopped;
    m_DecodeFile = path;
    m_Decoder = decoder;
    return ErrorCode::Ok;
  }

  void Close() {
    if (m_Status == PlayerStatus::Closed) {
      return;
    }

    Pause();

    m_Decoder->Close();
    m_Decoder = nullptr;
    m_DecodeFile.clear();

    m_Status = PlayerStatus::Closed;
  }

  string FileName() const {
    return m_DecodeFile;
  }

  void Play() {
    uint64_t beg = 0;
    uint64_t end = m_Decoder->UnitCount();
    PlayRange(beg, end);
  }

  void Play(uint64_t msBegin, uint64_t msEnd) {
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

  void PlayRange(uint64_t beg, uint64_t end) {
    m_UnitBeg = beg;
    m_UnitEnd = end;

    m_OutputIndex = m_UnitBeg;
    m_OutputMailbox.EmplaceBack(PROCEED, nullptr);

    m_DecoderIndex = m_UnitBeg;
    m_Decoder->SetUnitIndex(m_UnitBeg);
    m_DecoderMailbox.EmplaceBack(PROCEED, nullptr);
    while (!m_BufferMailbox.Empty()) {
      auto mail = m_BufferMailbox.Take();
      mail.action = DECODE;
      m_DecoderMailbox.PushBack(std::move(mail));
    }

    m_Status = PlayerStatus::Playing;
  }

  void Pause() {
    if (m_Status == PlayerStatus::Paused) {
      return;
    }

    m_DecoderMailbox.EmplaceFront(SUSPEND, nullptr);
    m_OutputMailbox.EmplaceFront(SUSPEND, nullptr);
    m_BufferMailbox.Wait(m_BufferCount);

    m_Status = PlayerStatus::Paused;
  }

  void Resume() {
    m_OutputMailbox.EmplaceBack(PROCEED, nullptr);

    m_DecoderIndex = m_OutputIndex;
    m_Decoder->SetUnitIndex(m_DecoderIndex);
    m_DecoderMailbox.EmplaceBack(PROCEED, nullptr);
    while (!m_BufferMailbox.Empty()) {
      auto mail = m_BufferMailbox.Take();
      mail.action = DECODE;
      m_DecoderMailbox.PushBack(std::move(mail));
    }

    m_Status = PlayerStatus::Playing;
  }

  void SeekTime(uint64_t msPos) {
    switch (m_Status) {
      case PlayerStatus::Playing: {
        Pause();
        DoSeekTime(msPos);
        Resume();
        break;
      }

      case PlayerStatus::Paused:
      case PlayerStatus::Stopped: {
        DoSeekTime(msPos);
        break;
      }

      default: {
        break;
      }
    }
  }

  void SeekPercent(double percent) {
    uint64_t unit = m_UnitBeg + (m_UnitEnd - m_UnitBeg) * percent;

    switch (m_Status) {
      case PlayerStatus::Playing: {
        Pause();
        DoSeekUnit(unit);
        Resume();
        break;
      }

      case PlayerStatus::Paused:
      case PlayerStatus::Stopped: {
        DoSeekUnit(unit);
        break;
      }

      default: {
        break;
      }
    }
  }

  void DoSeekTime(uint64_t msPos) {
    uint64_t unitPos = std::min((uint64_t)(m_UnitPerMs * msPos), m_Decoder->UnitCount());
    DoSeekUnit(unitPos);
  }

  void DoSeekUnit(uint64_t unit) {
    if (unit < m_UnitBeg) {
      unit = m_UnitBeg;
    } else if (unit > m_UnitEnd) {
      unit = m_UnitEnd;
    }

    m_Decoder->SetUnitIndex(unit);

    m_DecoderIndex = unit;
    m_OutputIndex = unit;
  }

  void PauseDecoder() {
    /*
        if (!m_PauseDecoder) {
            m_PauseDecoder = true;
        }
        m_SemDecoderEnd.Wait();

        m_Decoder->Close();
        */
  }

  void ResumeDecoder() {
    /*
        m_Decoder->Open(m_DecodeFile);
        m_Decoder->SetUnitIndex(m_DecoderIndex);

        m_PauseDecoder = false;
        m_SemWakeDecoder.Post();
        m_SemDecoderBegin.Wait();
        */
  }

  int32_t BitRate() const {
    return m_Decoder ? m_Decoder->BitRate() : -1;
  }

  int32_t SamleRate() const {
    return m_Decoder ? m_Decoder->SampleRate() : -1;
  }

  uint64_t Duration() const {
    return m_Decoder->Duration();
  }

  uint64_t RangeBegin() const {
    return m_UnitBeg / m_UnitPerMs;
  }

  uint64_t RangeEnd() const {
    return m_UnitEnd / m_UnitPerMs;
  }

  uint64_t RangeDuration() const {
    return (m_UnitEnd - m_UnitBeg) / m_UnitPerMs;
  }

  uint64_t OffsetMs() const {
    return CurrentMs() - RangeBegin();
  }

  uint64_t CurrentMs() const {
    return m_OutputIndex / m_UnitPerMs;
  }

  enum AudioMode AudioMode() const {
    return m_Decoder ? m_Decoder->AudioMode() : AudioMode::None;
  }

  Signal<void(void)>* SigFinished() {
    return &m_SigFinished;
  }

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
  std::map<std::string, std::shared_ptr<FormatProbe>> m_Probes;

  scx::Signal<void(void)> m_SigFinished;
};
}  // namespace mous
