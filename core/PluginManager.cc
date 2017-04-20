#include <core/PluginManager.h>
#include <core/Plugin.h>

#include "PluginManagerImpl.h"

namespace mous {

PluginManager::PluginManager()
    : impl(std::make_unique<Impl>())
{

}

PluginManager::~PluginManager()
{

}

size_t PluginManager::LoadPluginDir(const std::string& dir)
{
    return impl->LoadPluginDir(dir);
}

EmErrorCode PluginManager::LoadPlugin(const std::string& path)
{
    return impl->LoadPlugin(path);
}

void PluginManager::UnloadPlugin(const std::string& path)
{
    return impl->UnloadPlugin(path);
}

void PluginManager::UnloadAll()
{
    return impl->UnloadAll();
}

std::vector<const Plugin*> PluginManager::PluginAgents(EmPluginType type) const
{
    return impl->PluginAgents(type);
}

std::vector<std::string> PluginManager::PluginPaths() const
{
    return impl->PluginPaths();
}

const PluginInfo* PluginManager::QueryPluginInfo(const std::string& path) const
{
    return impl->QueryPluginInfo(path);
}

}
