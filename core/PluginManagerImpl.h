#pragma once

#include "scx/Dir.hpp"
#include "scx/FileInfo.hpp"
using namespace scx;

#include <unordered_map>

namespace mous {
class PluginManager::Impl
{
  public:
    size_t LoadPluginDir(const std::string& dir)
    {
        const auto& files = Dir::ListDir(dir);

        for (size_t i = 0; i < files.size(); ++i) {
            if (files[i].compare(0, 3, "lib") == 0) {
                std::string full = dir + "/" + files[i];
                if (FileInfo(full).Type() == FileType::Regular) {
                    LoadPlugin(full);
                }
            }
        }

        return indexedPlugins.size();
    }

    EmErrorCode LoadPlugin(const std::string& path)
    {
        indexedPlugins[path] = new Plugin(path);
        return ErrorCode::Ok;
    }

    void UnloadPlugin(const std::string& path)
    {
        auto iter = indexedPlugins.find(path);
        if (iter != indexedPlugins.end()) {
            Plugin* pAgent = iter->second;
            delete pAgent;
            indexedPlugins.erase(iter);
        }
    }

    void UnloadAll()
    {
        for (auto entry : indexedPlugins) {
            Plugin* pAgent = entry.second;
            delete pAgent;
        }
        indexedPlugins.clear();
    }

    std::vector<const Plugin*> PluginAgents(EmPluginType type) const
    {
        std::vector<const Plugin*> list;
        list.reserve(indexedPlugins.size());
        for (auto entry : indexedPlugins) {
            Plugin* pAgent = entry.second;
            if (pAgent->Type() == type) {
                list.push_back(pAgent);
            }
        }
        return list;
    }

    std::vector<std::string> PluginPaths() const
    {
        std::vector<std::string> list;
        list.reserve(indexedPlugins.size());
        for (auto entry : indexedPlugins) {
            list.push_back(entry.first);
        }
        return list;
    }

    const PluginInfo* QueryPluginInfo(const std::string& path) const
    {
        auto iter = indexedPlugins.find(path);
        return (iter != indexedPlugins.end()) ? iter->second->Info() : nullptr;
    }

  private:
    std::unordered_map<std::string, Plugin*> indexedPlugins;
};
}
