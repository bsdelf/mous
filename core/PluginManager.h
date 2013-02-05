#ifndef MOUS_PLUGINMANAGER_H
#define MOUS_PLUGINMANAGER_H

#include <map>
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

    std::vector<const IPluginAgent*> PluginAgents(EmPluginType) const;
    std::vector<std::string> PluginPaths() const;
    const PluginInfo* QueryPluginInfo(const std::string& path) const;

private:
    std::map<std::string, IPluginAgent*> m_PluginMap;
    typedef std::pair<std::string, IPluginAgent*> PluginMapPair;
    typedef std::map<std::string, IPluginAgent*>::iterator PluginMapIter;
    typedef std::map<std::string, IPluginAgent*>::const_iterator PluginMapConstIter;
};

}

#endif
