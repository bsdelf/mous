#pragma once

#include <unordered_map>
#include <core/IPluginManager.h>

struct stat;

namespace mous {

struct PluginInfo;
class IDecoder;
class IRenderer;
class IMediaPack;
class ITagParser;

class PluginManager: public IPluginManager
{
public:
    PluginManager();
    ~PluginManager();

    size_t LoadPluginDir(const std::string& dir);
    EmErrorCode LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& path);
    void UnloadAll();

    std::vector<const Plugin*> PluginAgents(EmPluginType) const;
    std::vector<std::string> PluginPaths() const;
    const PluginInfo* QueryPluginInfo(const std::string& path) const;

private:
    std::unordered_map<std::string, Plugin*> m_PluginMap;
};

}
