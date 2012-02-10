#ifndef MOUS_PLUGINMANAGER_H
#define MOUS_PLUGINMANAGER_H

#include <map>
#include <vector>
#include <string>
#include <mous/IDecoder.h>
#include <mous/IRenderer.h>
#include <mous/IMediaList.h>
#include "PluginAgent.h"

struct stat;

namespace mous {

class PluginManager
{
public:
    PluginManager();
    ~PluginManager();

    ErrorCode LoadPluginDir(const std::string& dir);
    ErrorCode LoadPlugin(const std::string& path);
    void UnloadPlugin(const std::string& path);
    void UnloadAllPlugins();

    std::vector<std::string> GetPluginPath(const PluginType& type);
    const PluginInfo* GetPluginInfo(const std::string& path);
    std::vector<IDecoder*> GetDecoders();
    std::vector<IRenderer*> GetRenderers();
    std::vector<IMediaList*> GetMediaLists();

private:
    static std::vector<std::string>* pFtwFiles;
    static int OnFtw(const char* file, const struct stat* s, int);

private:
    std::map<std::string, PluginAgent<IDecoder>*> m_decoderMap;
    //std::map<std::string, PluginAgent<IEncoder>*> m_encoderMap;
    std::map<std::string, PluginAgent<IRenderer>*> m_rendererMap;
    std::map<std::string, PluginAgent<IMediaList>*> m_medialistMap;

    typedef std::map<std::string, PluginAgent<IDecoder>*>::iterator DecoderMapIter;
    typedef std::map<std::string, PluginAgent<IRenderer>*>::iterator RendererMapIter;
    typedef std::map<std::string, PluginAgent<IMediaList>*>::iterator MediaListMapIter;
};

}

#endif
