#include "PluginManager.h"
#include <ftw.h>
#include <sys/stat.h>
#include <mous/PluginHelper.h>
using namespace std;
using namespace mous;

PluginManager::PluginManager()
{

}

PluginManager::~PluginManager()
{

}

ErrorCode PluginManager::LoadPluginDir(const string& dir)
{
    vector<string> files;
    pFtwFiles = &files;
    ftw(dir.c_str(), &OnFtw, 1);
    pFtwFiles = NULL;

    for (size_t i = 0; i < files.size(); ++i) {
	LoadPlugin(files[i]);
    }

    return MousOk;
}

ErrorCode PluginManager::LoadPlugin(const string& path)
{
    typedef PluginType (*FnGetPluginType)();
    FnGetPluginType fnGetPluginType;
    void* pHandle = NULL;

    pHandle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (pHandle == NULL)
	return MousPluginInvaild;

    fnGetPluginType = (FnGetPluginType)dlsym(pHandle, StrGetPluginType);
    if (fnGetPluginType == NULL) {
	dlclose(pHandle);
	return MousPluginInvaild;
    }

#define MOUS_LOAD_PLUGIN(IPlugin, PluginMap)				    \
PluginAgent<IPlugin>* p = new PluginAgent<IPlugin>();			    \
if (p->Open(path) == MousOk) {						    \
    PluginMap.insert(pair<string, PluginAgent<IPlugin>*>(path, p));	    \
} else {								    \
    p->Close();								    \
    delete p;								    \
}

    PluginType type = fnGetPluginType();
    switch (type) {
	case MousDecoder: {
	    MOUS_LOAD_PLUGIN(IDecoder, m_decoderMap);
	}
	    break;

	case MousEncoder:
	    //MOUS_LOAD_PLUGIN()
	    break;

	case MousRenderer: {
	    MOUS_LOAD_PLUGIN(IRenderer, m_rendererMap);
	}
	    break;

	case MousFilter:
	    break;

	case MousMediaList: {
	    MOUS_LOAD_PLUGIN(IMediaList, m_medialistMap);
	}
	    break;

	default:
	    break;
    }


    dlclose(pHandle);

    return MousOk;

#undef MOUS_LOAD_PLUGIN
}

void PluginManager::UnloadPlugin(const string& path)
{
#define MOUS_UNLOAD_PLUGIN(IPlugin, pluginMap)			    \
{								    \
    std::map<std::string, PluginAgent<IPlugin>*>::iterator iter	    \
	= pluginMap.find(path);					    \
    if (iter != pluginMap.end()) {				    \
	PluginAgent<IPlugin>* p = iter->second;			    \
	p->Close();						    \
	delete p;						    \
	pluginMap.erase(iter);					    \
	return;							    \
    }								    \
}

    MOUS_UNLOAD_PLUGIN(IDecoder, m_decoderMap);
    MOUS_UNLOAD_PLUGIN(IRenderer, m_rendererMap);
    MOUS_UNLOAD_PLUGIN(IMediaList, m_medialistMap);

#undef MOUS_UNLOAD_PLUGIN
}

void PluginManager::UnloadAllPlugins()
{
#define MOUS_UNLOAD_PLUGIN(IPlugin, pluginMap)				\
    for (map<string, PluginAgent<IPlugin>*>::iterator			\
	iter = pluginMap.begin(); iter != pluginMap.end(); ++iter) {	\
	PluginAgent<IPlugin>* p = iter->second;				\
	p->Close();							\
	delete p;							\
    }									\
    pluginMap.clear();

    MOUS_UNLOAD_PLUGIN(IDecoder, m_decoderMap);
    MOUS_UNLOAD_PLUGIN(IRenderer, m_rendererMap);
    MOUS_UNLOAD_PLUGIN(IMediaList, m_medialistMap);

#undef MOUS_UNLOAD_PLUGIN
}

vector<string> PluginManager::GetPluginPath(const PluginType& type)
{
    vector<string> paths;

#define MOUS_GET_PLUGIN_PATH(IPlugin, pluginMap)	\
    paths.reserve(pluginMap.size());			\
    for (map<string, PluginAgent<IPlugin>*>::iterator	\
	iter = pluginMap.begin();			\
	iter != pluginMap.end(); ++iter) {		\
	paths.push_back(iter->first);			\
    }

    switch (type) {
	case MousDecoder: 
	    MOUS_GET_PLUGIN_PATH(IDecoder, m_decoderMap);
	    break;

	case MousEncoder:
	    //MOUS_LOAD_PLUGIN()
	    break;

	case MousRenderer:
	    MOUS_GET_PLUGIN_PATH(IRenderer, m_rendererMap);
	    break;

	case MousFilter:
	    break;

	case MousMediaList:
	    MOUS_GET_PLUGIN_PATH(IMediaList, m_medialistMap);
	    break;

	default:
	    break;
    }

    return paths;

#undef MOUS_GET_PLUGIN_PATH
}

const PluginInfo* PluginManager::GetPluginInfo(const std::string& path)
{
#define MOUS_GET_PLUGININFO(IPlugin, pluginMap)			    \
{								    \
    std::map<std::string, PluginAgent<IPlugin>*>::iterator iter	    \
	= pluginMap.find(path);					    \
    if (iter != pluginMap.end()) {				    \
	PluginAgent<IPlugin>* p = iter->second;			    \
	return p->GetInfo();					    \
    }								    \
}

    MOUS_GET_PLUGININFO(IDecoder, m_decoderMap);
    MOUS_GET_PLUGININFO(IRenderer, m_rendererMap);
    MOUS_GET_PLUGININFO(IMediaList, m_medialistMap);

    return NULL;

#undef MOUS_GET_PLUGININFO

}

vector<IDecoder*> PluginManager::GetDecoders()
{
    vector<IDecoder*> list;
    return list;
}

vector<IRenderer*> PluginManager::GetRenderers()
{
    vector<IRenderer*> list;
    return list;
}

vector<IMediaList*> PluginManager::GetMediaLists()
{
    vector<IMediaList*> list;
    return list;
}

std::vector<std::string>* PluginManager::pFtwFiles = NULL;

int PluginManager::OnFtw(const char* file, const struct stat* s, int)
{
    if (!(s->st_mode & S_IFDIR))
	pFtwFiles->push_back(file);
    return 0;
}
