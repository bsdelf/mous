#include "ConvTask.h"
#include <iostream>
using namespace mous;

IConvTask* IConvTask::Create(const MediaItem& item, const IPluginAgent* decAgent, const IPluginAgent* encAgent)
{
    return new ConvTask(item, decAgent, encAgent);
}

void IConvTask::Free(IConvTask* task)
{
    if (task != NULL)
        delete task;
}

ConvTask::ConvTask(const MediaItem& item, const IPluginAgent* decAgent, const IPluginAgent* encAgent):
    m_Item(item),
    m_DecAgent(decAgent),
    m_EncAgent(encAgent),
    m_Progress(-1),
    m_Finished(true),
    m_Canceled(false)
{
    m_Decoder = (IDecoder*)m_DecAgent->CreateObject();
    m_Encoder = (IEncoder*)m_EncAgent->CreateObject();
}

ConvTask::~ConvTask()
{
    Cancel();

    m_DecAgent->FreeObject(m_Decoder);
    m_EncAgent->FreeObject(m_Encoder);
}

bool ConvTask::DecoderOptions(std::vector<const BaseOption*>& list) const
{
    return m_Decoder != NULL ? m_Decoder->Options(list) : false;
}

bool ConvTask::EncoderOptions(std::vector<const BaseOption*>& list) const
{
    return m_Encoder != NULL ? m_Encoder->Options(list) : false;
}

const char* ConvTask::EncoderFileSuffix() const
{
    return m_Encoder != NULL ? m_Encoder->FileSuffix() : NULL;
}

void ConvTask::Run(const string& output)
{
    m_Progress = 0.0000001;
    m_Finished = false;
    m_Canceled = false;
    m_WorkThread.Run(Function<void (const string&)>(&ConvTask::DoConvert, this), output);
}

void ConvTask::Cancel()
{
    m_Canceled = true;
    m_WorkThread.Join();
}

double ConvTask::Progress() const
{
    return m_Progress;
}

bool ConvTask::IsFinished() const
{
    return m_Finished;
}

void ConvTask::DoConvert(const string& output)
{
    EmErrorCode err;

    err = m_Decoder->Open(m_Item.url);
    if (err != ErrorCode::Ok) {
        m_Progress = -1;
        m_Finished = true;
        cout << "Decoder open failed!" << endl;
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
        cout << "Encoder open failed!" << endl;
        return;
    }

    char* unitBuffer = new char[m_Decoder->MaxBytesPerUnit()];
    uint32_t unitBufferUsed = 0;
    uint32_t unitCount = 0;

    double unitPerMs = (double)m_Decoder->UnitCount() / m_Decoder->Duration();
    uint32_t unitOff = 0, unitBeg = 0, unitEnd = m_Decoder->UnitCount();

    if (m_Item.hasRange) {
        unitBeg = m_Item.msBeg * unitPerMs;
        unitEnd = (m_Item.msEnd != (uint64_t)-1 ? m_Item.msEnd : unitEnd) * unitPerMs;

        if (unitBeg > m_Decoder->UnitCount())
            unitBeg = m_Decoder->UnitCount();

        if (unitEnd > m_Decoder->UnitCount())
            unitEnd = m_Decoder->UnitCount();
    }

    unitOff = unitBeg;
    m_Decoder->SetUnitIndex(unitBeg);

    cout << unitBeg << "-" << unitEnd << endl;
    while (unitOff < unitEnd && !m_Canceled) {
        m_Decoder->DecodeUnit(unitBuffer, unitBufferUsed, unitCount);
        m_Encoder->Encode(unitBuffer, unitBufferUsed);
        unitOff += unitCount;
        m_Progress = (double)(unitOff-unitBeg) / (unitEnd-unitBeg);
    }

    m_Encoder->FlushRest();

    m_Encoder->CloseOutput();
    m_Decoder->Close();

    delete[] unitBuffer;

    m_Finished = true;
}
