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

void ConvTaskFactory::RegisterDecoderPlugin(const Plugin* plugin)
{
    return impl->RegisterDecoderPlugin(plugin);
}

void ConvTaskFactory::RegisterDecoderPlugin(std::vector<const Plugin*>& plugins)
{
    return impl->RegisterDecoderPlugin(plugins);
}

void ConvTaskFactory::RegisterEncoderPlugin(const Plugin* plugin)
{
    return impl->RegisterEncoderPlugin(plugin);
}

void ConvTaskFactory::RegisterEncoderPlugin(std::vector<const Plugin*>& plugins)
{
    return impl->RegisterEncoderPlugin(plugins);
}

void ConvTaskFactory::UnregisterPlugin(const Plugin* plugin)
{
    return impl->UnregisterPlugin(plugin);
}

void ConvTaskFactory::UnregisterPlugin(std::vector<const Plugin*>& plugins)
{
    return impl->UnregisterPlugin(plugins);
}

void ConvTaskFactory::UnregisterAll()
{
    return impl->UnregisterAll();
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
