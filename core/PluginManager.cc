#include <core/PluginManager.h>
using namespace std;

#include "scx/Dir.hpp"
#include "scx/FileInfo.hpp"
using namespace scx;

#include <unordered_map>
namespace mous {

class PluginManagerPrivate
{
public:
    std::unordered_map<std::string, Plugin*> indexedPlugins;
};

PluginManager::PluginManager()
    : d(std::make_unique<PluginManagerPrivate>())
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

    return d->indexedPlugins.size();
}

EmErrorCode PluginManager::LoadPlugin(const string& path)
{
    d->indexedPlugins[path] = new Plugin(path);
    return ErrorCode::Ok;
}

void PluginManager::UnloadPlugin(const string& path)
{
    auto iter = d->indexedPlugins.find(path);
    if (iter != d->indexedPlugins.end()) {
        Plugin* pAgent = iter->second;
        delete pAgent;
        d->indexedPlugins.erase(iter);
    }
}

void PluginManager::UnloadAll()
{
    for (auto entry: d->indexedPlugins) {
        Plugin* pAgent = entry.second;
        delete pAgent;
    }
    d->indexedPlugins.clear();
}

vector<const Plugin*> PluginManager::PluginAgents(EmPluginType type) const
{
    vector<const Plugin*> list;
    list.reserve(d->indexedPlugins.size());
    for (auto entry: d->indexedPlugins) {
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
    list.reserve(d->indexedPlugins.size());
    for (auto entry: d->indexedPlugins) {
        list.push_back(entry.first);
    }
    return list;
}

const PluginInfo* PluginManager::QueryPluginInfo(const std::string& path) const
{
    auto iter = d->indexedPlugins.find(path);
    return (iter != d->indexedPlugins.end()) ? iter->second->Info() : nullptr;
}

}
