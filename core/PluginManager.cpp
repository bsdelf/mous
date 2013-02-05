#include "PluginManager.h"
#include <core/IPluginAgent.h>
using namespace std;
using namespace mous;

#include "scx/Dir.hpp"
#include "scx/FileInfo.hpp"
using namespace scx;

IPluginManager* IPluginManager::Create()
{
    return new PluginManager;
}

void IPluginManager::Free(IPluginManager* ptr)
{
    if (ptr != NULL)
        delete ptr;
}

PluginManager::PluginManager()
{

}

PluginManager::~PluginManager()
{

}

size_t PluginManager::LoadPluginDir(const string& dir)
{
    vector<string> files = Dir::ListDir(dir);

    for (size_t i = 0; i < files.size(); ++i) {
        if (files[i].substr(0, 3) == "lib") {
            string full = dir + "/" + files[i];
            FileInfo info(full);
            if (info.Type() == FileType::Regular)
                LoadPlugin(full);
        }
    }

    return m_PluginMap.size();
}

EmErrorCode PluginManager::LoadPlugin(const string& path)
{
    IPluginAgent* pAgent = IPluginAgent::Create();
    EmErrorCode ret = pAgent->Open(path);
    if (ret == ErrorCode::Ok) {
        m_PluginMap.insert(PluginMapPair(path, pAgent));
    } else {
        IPluginAgent::Free(pAgent);
    }

    return ret;
}

void PluginManager::UnloadPlugin(const string& path)
{
    PluginMapIter iter = m_PluginMap.find(path);
    if (iter != m_PluginMap.end()) {
        IPluginAgent* pAgent = iter->second;
        pAgent->Close();
        IPluginAgent::Free(pAgent);
        m_PluginMap.erase(iter);
    }
}

void PluginManager::UnloadAll()
{
    for (PluginMapIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        IPluginAgent* pAgent = iter->second;
        pAgent->Close();
        IPluginAgent::Free(pAgent);
    }
    m_PluginMap.clear();
}

vector<const IPluginAgent*> PluginManager::PluginAgents(EmPluginType type) const
{
    vector<const IPluginAgent*> list;
    list.reserve(m_PluginMap.size());
    for (PluginMapConstIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        IPluginAgent* pAgent = iter->second;
        if (pAgent->Type() == type) {
            list.push_back(pAgent);
        }
    }
    return list;
}

vector<string> PluginManager::PluginPaths() const
{
    vector<string> list;
    list.reserve(m_PluginMap.size());
    for (PluginMapConstIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        list.push_back(iter->first);
    }
    return list;
}

const PluginInfo* PluginManager::QueryPluginInfo(const std::string& path) const
{
    PluginMapConstIter iter = m_PluginMap.find(path);
    return (iter != m_PluginMap.end()) ?
        iter->second->Info() : NULL;
}
