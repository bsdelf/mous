#ifndef MOUS_PLUGINMANAGER_H
#define MOUS_PLUGINMANAGER_H

#include <inttypes.h>
#include <map>
#include <vector>
#include <string>
#include <mous/ErrorCode.h>
#include "PluginAgent.h"

struct stat;

namespace mous {

struct PluginInfo;
class IDecoder;
class IRenderer;
class IMediaPack;
class ITagParser;

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    EmErrorCode LoadPluginDir(const std::string& dir);
    EmErrorCode LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& path);
    void UnloadAllPlugins();

    void GetPluginAgents(std::vector<const PluginAgent*>& list, EmPluginType) const;
    void GetPluginPath(std::vector<std::string>& list) const;
    const PluginInfo* PluginInfo(const std::string& path) const;

    //void GetDecoders(std::vector<IPluginAgent*>& list);
    //void GetRenderers(std::vector<IPluginAgent*>& list);
    //void GetMediaPacks(std::vector<IMediaPack*>& list);
    //void GetTagParsers(std::vector<ITagParser*>& list);

private:
    static std::vector<std::string>* gFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:

private:
    std::map<std::string, PluginAgent*> m_PluginMap;
    typedef std::pair<std::string, PluginAgent*> PluginMapPair;
    typedef std::map<std::string, PluginAgent*>::iterator PluginMapIter;
    typedef std::map<std::string, PluginAgent*>::const_iterator PluginMapConstIter;
};

}

#endif
