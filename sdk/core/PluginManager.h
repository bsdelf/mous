#pragma once

#include <vector>
#include <string>
#include <memory>
#include <util/ErrorCode.h>
#include <util/PluginDef.h>
#include <core/Plugin.h>

struct stat;

namespace mous {

struct PluginInfo;
/*
class IDecoder;
class IRenderer;
class IMediaPack;
class ITagParser;
*/

class PluginManagerPrivate;

class PluginManager
{
    friend PluginManagerPrivate;

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
    std::unique_ptr<PluginManagerPrivate> d;
};

}
