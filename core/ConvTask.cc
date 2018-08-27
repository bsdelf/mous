#include <core/ConvTask.h>

#include "ConvTaskImpl.h"

namespace mous {

ConvTask::ConvTask(const MediaItem& item, 
                   const std::shared_ptr<Plugin>& decoderPlugin,
                   const std::shared_ptr<Plugin>& encoderPlugin):
    impl(std::make_unique<Impl>(item, decoderPlugin, encoderPlugin))
{
}

ConvTask::~ConvTask()
{
}

std::vector<const BaseOption*> ConvTask::DecoderOptions() const
{
    return impl->DecoderOptions();
}

std::vector<const BaseOption*> ConvTask::EncoderOptions() const
{
    return impl->EncoderOptions();
}

std::string ConvTask::EncoderFileSuffix() const
{
    return impl->EncoderFileSuffix();
}

void ConvTask::Run(const std::string& output)
{
    return impl->Run(output);
}

void ConvTask::Cancel()
{
    return impl->Cancel();
}

double ConvTask::Progress() const
{
    return impl->Progress();
}

bool ConvTask::IsFinished() const
{
    return impl->IsFinished();
}

}
