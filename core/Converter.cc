#include <core/Converter.h>
#include "ConverterImpl.h"

namespace mous {

Converter::Converter()
    : impl(std::make_unique<Impl>())
{
}

Converter::~Converter()
{
}

void Converter::LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin)
{
    return impl->LoadDecoderPlugin(plugin);
}

void Converter::LoadEncoderPlugin(const std::shared_ptr<Plugin>& plugin)
{
    return impl->LoadEncoderPlugin(plugin);
}

void Converter::UnloadPlugin(const std::string& path)
{
    return impl->UnloadPlugin(path);
}

void Converter::UnloadPlugin()
{
    return impl->UnloadPlugin();
}

std::vector<std::string> Converter::EncoderNames() const
{
    return impl->EncoderNames();
}

std::shared_ptr<Conversion> Converter::CreateConversion(const MediaItem& item, const std::string& encoder) const
{
    return impl->CreateConversion(item, encoder);
}

}
