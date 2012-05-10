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

    void Plugins(std::vector<const IPluginAgent*>& list, EmPluginType) const;
    void PluginPath(std::vector<std::string>& list) const;
    const PluginInfo* QueryPluginInfo(const std::string& path) const;

private:
    static std::vector<std::string>* gFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:
    std::map<std::string, IPluginAgent*> m_PluginMap;
    typedef std::pair<std::string, IPluginAgent*> PluginMapPair;
    typedef std::map<std::string, IPluginAgent*>::iterator PluginMapIter;
    typedef std::map<std::string, IPluginAgent*>::const_iterator PluginMapConstIter;
};

}

#endif
