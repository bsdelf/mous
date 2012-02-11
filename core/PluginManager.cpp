#include "PluginManager.h"
#include <ftw.h>
#include <sys/stat.h>
#include <mous/PluginHelper.h>
#include <mous/IDecoder.h>
#include <mous/IRenderer.h>
#include <mous/IMediaList.h>
#include "PluginAgent.h"
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
    PluginType type = fnGetPluginType();

    IPluginAgent* pAgent = NULL;
    switch (type) {
	case MousDecoder:
	    pAgent = new PluginAgent<IDecoder>(type);
	    break;

	case MousEncoder:
	    break;

	case MousRenderer:
	    pAgent = new PluginAgent<IRenderer>(type);
	    break;

	case MousFilter:
	    break;

	case MousMediaList:
	    pAgent = new PluginAgent<IMediaList>(type);
	    break;

	default:
	    pAgent = NULL;
	    break;
    }

    if (pAgent->Open(path) == MousOk) {
	m_pluginMap.insert(pair<string, IPluginAgent*>(path, pAgent));
    } else {
	pAgent->Close();
	delete pAgent;
    }

    dlclose(pHandle);

    return MousOk;
}

void PluginManager::UnloadPlugin(const string& path)
{
    PluginMapIter iter = m_pluginMap.find(path);
    if (iter != m_pluginMap.end()) {
	IPluginAgent* pAgent = iter->second;
	pAgent->Close();
	delete pAgent;
	m_pluginMap.erase(iter);
    }
}

void PluginManager::UnloadAllPlugins()
{
    for (PluginMapIter iter = m_pluginMap.begin();
	    iter != m_pluginMap.end(); ++iter) {
	IPluginAgent* pAgent = iter->second;
	pAgent->Close();
	delete pAgent;
    }
    m_pluginMap.clear();
}

void PluginManager::GetPluginPath(vector<string>& list)
{
    list.clear();
    list.reserve(m_pluginMap.size());
    for (PluginMapIter iter = m_pluginMap.begin();
	    iter != m_pluginMap.end(); ++iter) {
	list.push_back(iter->first);
    }
}

const PluginInfo* PluginManager::GetPluginInfo(const std::string& path)
{
    PluginMapIter iter = m_pluginMap.find(path);
    return (iter != m_pluginMap.end()) ?
	iter->second->GetInfo() : NULL;
}

void PluginManager::GetDecoders(std::vector<IDecoder*>& list)
{
    return GetPluginsByType(list, MousDecoder);
}

void PluginManager::GetRenderers(std::vector<IRenderer*>& list)
{
    return GetPluginsByType(list, MousRenderer);
}

void PluginManager::GetMediaLists(std::vector<IMediaList*>& list)
{
    return GetPluginsByType(list, MousMediaList);
}

template<typename Super>
void PluginManager::GetPluginsByType(vector<Super*>& list, PluginType type)
{
    list.clear();
    for (PluginMapIter iter = m_pluginMap.begin();
	    iter != m_pluginMap.end(); ++iter) {
	IPluginAgent* pAgent = iter->second;
	if (pAgent->GetType() == type) {
	    PluginAgent<Super>* pd = 
		dynamic_cast<PluginAgent<Super>*>(pAgent);
	    if (pd != NULL) {
		list.push_back(pd->GetPlugin());
	    }
	}
    }
}

std::vector<std::string>* PluginManager::pFtwFiles = NULL;

int PluginManager::OnFtw(const char* file, const struct stat* s, int)
{
    if (!(s->st_mode & S_IFDIR))
	pFtwFiles->push_back(file);
    return 0;
}
