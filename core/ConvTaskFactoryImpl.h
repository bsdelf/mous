#pragma once

#include <unordered_map>

#include <core/ConvTaskFactory.h>
#include <core/ConvTask.h>
#include <core/Plugin.h>
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
#include <util/MediaItem.h>

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>

namespace mous {

class ConvTaskFactory::Impl {
public:
    ~Impl()
    {
        UnregisterAll();
    }

    void RegisterDecoderPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::Decoder) {
            AddDecAgent(pAgent);
        }
    }

    void RegisterDecoderPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent: agents) {
            RegisterDecoderPlugin(agent);
        }
    }

    void RegisterEncoderPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::Encoder) {
            AddEncAgent(pAgent);
        }
    }

    void RegisterEncoderPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent: agents) {
            RegisterEncoderPlugin(agent);
        }
    }

    void UnregisterPlugin(const Plugin* pAgent)
    {
        switch (pAgent->Type()) {
            case PluginType::Decoder:
                RemoveDecAgent(pAgent);
                break;

            case PluginType::Encoder:
                RemoveEncAgent(pAgent);
                break;

            default:
                break;
        }
    }

    void UnregisterPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent: agents) {
            UnregisterPlugin(agent);
        }
    }

    void UnregisterAll()
    {
        while (!indexedDecoderPlugins.empty()) {
            auto iter = indexedDecoderPlugins.begin();
            for (auto agent: (*iter->second)) {
                RemoveDecAgent(agent);
            }
        }
        
        indexedEncoderPlugins.clear();
    }

    std::vector<std::string> EncoderNames() const
    {
        std::vector<std::string> list;
        list.reserve(indexedEncoderPlugins.size());

        for (auto entry: indexedEncoderPlugins) {
            list.push_back(entry.first);
        }

        return list;
    }

    ConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const
    {
        const Plugin* decAgent = nullptr;
        const std::string& suffix = scx::ToLower(scx::FileHelper::FileSuffix(item.url));
        auto decAgentIter = indexedDecoderPlugins.find(suffix);
        if (decAgentIter != indexedDecoderPlugins.end()) {
            std::vector<const Plugin*> list = *(decAgentIter->second);
            decAgent = list[0];
        }

        const Plugin* encAgent = nullptr;
        auto encAgentIter = indexedEncoderPlugins.find(encoder);
        if (encAgentIter != indexedEncoderPlugins.end()) {
            encAgent = encAgentIter->second;
        }

        ConvTask* task = nullptr;
        if (decAgent != nullptr && encAgent != nullptr) {
            task = new ConvTask(item, decAgent, encAgent);
        }
        return task;
    }

private:
    void AddDecAgent(const Plugin* pAgent)
    {
        IDecoder* dec = (IDecoder*)pAgent->CreateObject();
        std::vector<std::string> list = dec->FileSuffix();
        pAgent->FreeObject(dec);

        for (const std::string& suffix: list) {
            std::vector<const Plugin*>* agentList = nullptr;
            auto iter = indexedDecoderPlugins.find(suffix);
            if (iter == indexedDecoderPlugins.end()) {
                agentList = new std::vector<const Plugin*>();
                agentList->push_back(pAgent);
                indexedDecoderPlugins.emplace(suffix, agentList);
            } else {
                agentList = iter->second;
                agentList->push_back(pAgent);
            }
        }
    }

    void RemoveDecAgent(const Plugin* pAgent)
    {
        IDecoder* dec = (IDecoder*)pAgent->CreateObject();
        std::vector<std::string> list = dec->FileSuffix();
        pAgent->FreeObject(dec);

        for (const std::string& suffix: list) {
            auto iter = indexedDecoderPlugins.find(suffix);
            if (iter != indexedDecoderPlugins.end()) {
                std::vector<const Plugin*>& agentList = *(iter->second);
                for (size_t i = 0; i < agentList.size(); ++i) {
                    if (agentList[i] == pAgent) {
                        agentList.erase(agentList.begin()+i);
                        break;
                    }
                }
                if (agentList.empty()) {
                    delete iter->second;
                    indexedDecoderPlugins.erase(iter);
                }
            }
        }
    }

    void AddEncAgent(const Plugin* pAgent)
    {
        indexedEncoderPlugins.emplace(pAgent->Info()->name, pAgent);
    }

    void RemoveEncAgent(const Plugin* pAgent)
    {
        auto iter = indexedEncoderPlugins.find(pAgent->Info()->name);
        if (iter != indexedEncoderPlugins.end())
            indexedEncoderPlugins.erase(iter);
    }

private:
    std::unordered_map<std::string, std::vector<const Plugin*>*> indexedDecoderPlugins;
    std::unordered_map<std::string, const Plugin*> indexedEncoderPlugins;


};

}
