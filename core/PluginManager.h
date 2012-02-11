#ifndef MOUS_PLUGINMANAGER_H
#define MOUS_PLUGINMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <mous/ErrorCode.h>
#include <mous/PluginHelper.h>
struct stat;

namespace mous {

class PluginInfo;
class IPluginAgent;
class IDecoder;
class IRenderer;
class IMediaList;

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    ErrorCode LoadPluginDir(const std::string& dir);
    ErrorCode LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& path);
    void UnloadAllPlugins();

    void GetPluginPath(std::vector<std::string>& list);
    const PluginInfo* GetPluginInfo(const std::string& path);

    void GetDecoders(std::vector<IDecoder*>& list);
    void GetRenderers(std::vector<IRenderer*>& list);
    void GetMediaLists(std::vector<IMediaList*>& list);

private:
    static std::vector<std::string>* pFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:
    template<typename Super>
    void GetPluginsByType(std::vector<Super*>& list, PluginType);

private:
    std::map<std::string, IPluginAgent*> m_pluginMap;
    typedef std::map<std::string, IPluginAgent*>::iterator PluginMapIter;
};

}

#endif
