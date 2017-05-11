#include <string>

#include <core/Plugin.h>

#include "PluginImpl.h"

namespace mous {

Plugin::Plugin(const std::string& path)
    : impl(std::make_unique<Impl>(path))
{
}

Plugin::~Plugin()
{
}

PluginType Plugin::Type() const
{
    return impl->type;
}

const PluginInfo* Plugin::Info() const
{
    return impl->GetPluginInfo();
}

void* Plugin::CreateObject() const
{
    return impl->CreateObject();
}

void Plugin::FreeObject(void* obj) const
{
    return impl->FreeObject(obj);
}

}
