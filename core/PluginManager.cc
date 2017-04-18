#include "PluginManager.h"
#include <core/Plugin.h>
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
    if (ptr != nullptr)
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
    const auto& files = Dir::ListDir(dir);

    for (size_t i = 0; i < files.size(); ++i) {
        if (files[i].substr(0, 3) == "lib") {
            string full = dir + "/" + files[i];
            if (FileInfo(full).Type() == FileType::Regular) {
                LoadPlugin(full);
            }
        }
    }

    return m_PluginMap.size();
}

EmErrorCode PluginManager::LoadPlugin(const string& path)
{
    m_PluginMap[path] = new Plugin(path);
    return ErrorCode::Ok;
}

void PluginManager::UnloadPlugin(const string& path)
{
    auto iter = m_PluginMap.find(path);
    if (iter != m_PluginMap.end()) {
        Plugin* pAgent = iter->second;
        delete pAgent;
        m_PluginMap.erase(iter);
    }
}

void PluginManager::UnloadAll()
{
    for (auto entry: m_PluginMap) {
        Plugin* pAgent = entry.second;
        delete pAgent;
    }
    m_PluginMap.clear();
}

vector<const Plugin*> PluginManager::PluginAgents(EmPluginType type) const
{
    vector<const Plugin*> list;
    list.reserve(m_PluginMap.size());
    for (auto entry: m_PluginMap) {
        Plugin* pAgent = entry.second;
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
    for (auto entry: m_PluginMap) {
        list.push_back(entry.first);
    }
    return list;
}

const PluginInfo* PluginManager::QueryPluginInfo(const std::string& path) const
{
    auto iter = m_PluginMap.find(path);
    return (iter != m_PluginMap.end()) ? iter->second->Info() : nullptr;
}
