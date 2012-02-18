#ifndef MOUS_PLUGINMANAGER_H
#define MOUS_PLUGINMANAGER_H

#include <inttypes.h>
#include <map>
#include <vector>
#include <string>
#include <mous/ErrorCode.h>
#include <mous/PluginHelper.h>
struct stat;

namespace mous {

struct PluginInfo;
class IPluginAgent;
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

    void GetPluginPath(std::vector<std::string>& list);
    const PluginInfo* GetPluginInfo(const std::string& path);
    const PluginInfo* GetPluginInfo(const void* vp);

    void GetDecoders(std::vector<IDecoder*>& list);
    void GetRenderers(std::vector<IRenderer*>& list);
    void GetMediaPacks(std::vector<IMediaPack*>& list);
    void GetTagParsers(std::vector<ITagParser*>& list);

    void* GetVpPlugin(const std::string& path, EmPluginType& type);

private:
    static std::vector<std::string>* pFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:
    template<typename Super>
    void GetPluginsByType(std::vector<Super*>& list, EmPluginType);

private:
    std::map<std::string, IPluginAgent*> m_PluginMap;
    typedef std::pair<std::string, IPluginAgent*> PluginMapPair;
    typedef std::map<std::string, IPluginAgent*>::iterator PluginMapIter;
};

}

#endif
