#pragma once

#include <vector>
#include <string>
#include <memory>
#include <util/ErrorCode.h>
#include <util/PluginDef.h>
#include <core/Plugin.h>

namespace mous {

class PluginManager
{
    class Impl;

public:
    PluginManager();
    ~PluginManager();

    size_t LoadPluginDir(const std::string& dir);
    ErrorCode LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& path);
    void UnloadAll();

    std::vector<const Plugin*> PluginAgents(PluginType) const;
    std::vector<std::string> PluginPaths() const;
    const PluginInfo* QueryPluginInfo(const std::string& path) const;

private:
    std::unique_ptr<Impl> impl;
};

}
