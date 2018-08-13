#include <core/ConvTaskFactory.h>
#include "ConvTaskFactoryImpl.h"

namespace mous {

ConvTaskFactory::ConvTaskFactory()
    : impl(std::make_unique<Impl>())
{
}

ConvTaskFactory::~ConvTaskFactory()
{
}

void ConvTaskFactory::LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin)
{
    return impl->LoadDecoderPlugin(plugin);
}

void ConvTaskFactory::LoadEncoderPlugin(const std::shared_ptr<Plugin>& plugin)
{
    return impl->LoadEncoderPlugin(plugin);
}

void ConvTaskFactory::UnloadPlugin(const std::string& path)
{
    return impl->UnloadPlugin(path);
}

void ConvTaskFactory::UnloadPlugin()
{
    return impl->UnloadPlugin();
}

std::vector<std::string> ConvTaskFactory::EncoderNames() const
{
    return impl->EncoderNames();
}

ConvTask* ConvTaskFactory::CreateTask(const MediaItem& item, const std::string& encoder) const
{
    return impl->CreateTask(item, encoder);
}

}
