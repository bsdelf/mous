#pragma once

#include <thread>

#include <core/Conversion.h>
#include <plugin/Decoder.h>
#include <plugin/Encoder.h>

namespace mous {

class Conversion::Impl {
public:
    Impl(const MediaItem& item,
         const std::shared_ptr<Plugin>& decoderPlugin,
         const std::shared_ptr<Plugin>& encoderPlugin)
        : m_Item(item)
        , m_DecoderPlugin(decoderPlugin)
        , m_EncoderPlugin(encoderPlugin)
    {
        // TODO: check availablity
        m_Decoder = std::make_unique<Decoder>(decoderPlugin);
        m_Encoder = std::make_unique<Encoder>(encoderPlugin);
    }

    ~Impl()
    {
        Cancel();
    }

    std::vector<const BaseOption*> DecoderOptions() const
    {
        return m_Decoder ? m_Decoder->GetOptions() : std::vector<const BaseOption*>();
    }

    std::vector<const BaseOption*> EncoderOptions() const
    {
        return m_Encoder ? m_Encoder->GetOptions() : std::vector<const BaseOption*>();
    }

    std::string EncoderFileSuffix() const
    {
        if (!m_Encoder) {
            return {};
        }
        return { m_Encoder->FileSuffix() };
    }

    void Run(const std::string& output)
    {
        m_Progress = 0.0000001;
        m_Finished = false;
        m_Canceled = false;

        m_WorkThread = std::thread([this, output]() {
            ErrorCode err;

            err = m_Decoder->Open(m_Item.url);
            if (err != ErrorCode::Ok) {
                m_Progress = -1;
                m_Finished = true;
                //cout << "Decoder open failed!" << endl;
                return;
            }

            m_Encoder->SetChannels(m_Decoder->Channels());
            m_Encoder->SetSampleRate(m_Decoder->SampleRate());
            m_Encoder->SetBitsPerSample(m_Decoder->BitsPerSample());
            m_Encoder->SetMediaTag(&m_Item.tag);

            err = m_Encoder->OpenOutput(output);
            if (err != ErrorCode::Ok) {
                m_Progress = -1;
                m_Finished = true;
                //cout << "Encoder open failed!" << endl;
                return;
            }

            std::vector<char> unitBuffer(m_Decoder->MaxBytesPerUnit());
            uint32_t unitBufferUsed = 0;
            uint32_t unitCount = 0;

            double unitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();
            uint32_t unitOff = 0, unitBeg = 0, unitEnd = m_Decoder->UnitCount();

            if (m_Item.hasRange) {
                unitBeg = m_Item.msBeg * unitPerMs;
                unitEnd = (m_Item.msEnd != (uint64_t)-1 ? m_Item.msEnd : unitEnd) * unitPerMs;

                if (unitBeg > m_Decoder->UnitCount()) {
                    unitBeg = m_Decoder->UnitCount();
                }

                if (unitEnd > m_Decoder->UnitCount()) {
                    unitEnd = m_Decoder->UnitCount();
                }
            }

            unitOff = unitBeg;
            m_Decoder->SetUnitIndex(unitBeg);

            //cout << unitBeg << "-" << unitEnd << endl;
            while (unitOff < unitEnd && !m_Canceled) {
                m_Decoder->DecodeUnit(unitBuffer.data(), unitBufferUsed, unitCount);
                m_Encoder->Encode(unitBuffer.data(), unitBufferUsed);
                unitOff += unitCount;
                m_Progress = (double)(unitOff-unitBeg) / (unitEnd-unitBeg);
            }

            m_Encoder->Flush();

            m_Encoder->CloseOutput();
            m_Decoder->Close();

            m_Finished = true;
        });
    }

    void Cancel()
    {
        m_Canceled = true;
        if (m_WorkThread.joinable()) {
            m_WorkThread.join();
        }
    }

    double Progress() const
    {
        return m_Progress;
    }

    bool IsFinished() const
    {
        return m_Finished;
    }

private:
    MediaItem m_Item;
    std::shared_ptr<Plugin> m_DecoderPlugin;
    std::shared_ptr<Plugin> m_EncoderPlugin;

    std::thread m_WorkThread;

    std::unique_ptr<Decoder> m_Decoder;
    std::unique_ptr<Encoder> m_Encoder;

    double m_Progress = -1;
    bool m_Finished = true;
    bool m_Canceled = false;
};

}
