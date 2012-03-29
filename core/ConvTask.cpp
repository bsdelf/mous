#include "ConvTask.h"
using namespace mous;

IConvTask* IConvTask::Create(const MediaItem* item, IDecoder* decoder, IEncoder* encoder)
{
    return new ConvTask(item, decoder, encoder);
}

void IConvTask::Free(IConvTask* task)
{
    if (task != NULL)
        delete task;
}

ConvTask::ConvTask(const MediaItem* item, IDecoder* decoder, IEncoder* encoder):
    m_Item(*item),
    m_Decoder(decoder),
    m_Encoder(encoder)
{
}

ConvTask::~ConvTask()
{
    if (m_Decoder != NULL)
        delete m_Decoder;
    if (m_Encoder != NULL)
        delete m_Encoder;
}

bool ConvTask::GetDecoderOptions(std::vector<const BaseOption*>& list) const
{
    return m_Decoder->GetOptions(list);
}

bool ConvTask::GetEncoderOptions(std::vector<const BaseOption*>& list) const
{
    return m_Encoder->GetOptions(list);
}

EmErrorCode ConvTask::Run()
{
    return ErrorCode::Ok;
}

void ConvTask::Cancel()
{
}

int ConvTask::GetProgress() const
{
    return -1;
}
