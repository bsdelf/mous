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
    void UnloadAllPlugins();

    void GetPluginAgents(std::vector<const IPluginAgent*>& list, EmPluginType) const;
    void GetPluginPath(std::vector<std::string>& list) const;
    const PluginInfo* GetPluginInfo(const std::string& path) const;

    //void GetDecoders(std::vector<IPluginAgent*>& list);
    //void GetRenderers(std::vector<IPluginAgent*>& list);
    //void GetMediaPacks(std::vector<IMediaPack*>& list);
    //void GetTagParsers(std::vector<ITagParser*>& list);

private:
    static std::vector<std::string>* gFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:

private:
    std::map<std::string, IPluginAgent*> m_PluginMap;
    typedef std::pair<std::string, IPluginAgent*> PluginMapPair;
    typedef std::map<std::string, IPluginAgent*>::iterator PluginMapIter;
    typedef std::map<std::string, IPluginAgent*>::const_iterator PluginMapConstIter;
};

}

#endif
