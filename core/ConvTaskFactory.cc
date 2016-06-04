#include "ConvTaskFactory.h"

#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
using namespace mous;

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
using namespace scx;

IConvTaskFactory* IConvTaskFactory::Create()
{
    return new ConvTaskFactory();
}

void IConvTaskFactory::Free(IConvTaskFactory* factory)
{
    if (factory != nullptr)
        delete factory;
}

ConvTaskFactory::ConvTaskFactory()
{
}

ConvTaskFactory::~ConvTaskFactory()
{
    UnregisterAll();
}

void ConvTaskFactory::RegisterDecoderPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::Decoder) {
        AddDecAgent(pAgent);
    }
}

void ConvTaskFactory::RegisterDecoderPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        RegisterDecoderPlugin(agent);
    }
}

void ConvTaskFactory::RegisterEncoderPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->Type() == PluginType::Encoder) {
        AddEncAgent(pAgent);
    }
}

void ConvTaskFactory::RegisterEncoderPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        RegisterEncoderPlugin(agent);
    }
}

void ConvTaskFactory::UnregisterPlugin(const IPluginAgent* pAgent)
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

void ConvTaskFactory::UnregisterPlugin(std::vector<const IPluginAgent*>& agents)
{
    for (auto agent: agents) {
        UnregisterPlugin(agent);
    }
}

void ConvTaskFactory::UnregisterAll()
{
    while (!m_DecAgentMap.empty()) {
        auto iter = m_DecAgentMap.begin();
        for (auto agent: (*iter->second)) {
            RemoveDecAgent(agent);
        }
    }
    
    m_EncAgentMap.clear();
}

vector<string> ConvTaskFactory::EncoderNames() const
{
    vector<string> list;
    list.reserve(m_EncAgentMap.size());

    for (auto entry: m_EncAgentMap) {
        list.push_back(entry.first);
    }

    return list;
}

IConvTask* ConvTaskFactory::CreateTask(const MediaItem& item, const std::string& encoder) const
{
    const IPluginAgent* decAgent = nullptr;
    const string& suffix = ToLower(FileHelper::FileSuffix(item.url));
    auto decAgentIter = m_DecAgentMap.find(suffix);
    if (decAgentIter != m_DecAgentMap.end()) {
        vector<const IPluginAgent*> list = *(decAgentIter->second);
        decAgent = list[0];
    }

    const IPluginAgent* encAgent = nullptr;
    auto encAgentIter = m_EncAgentMap.find(encoder);
    if (encAgentIter != m_EncAgentMap.end()) {
        encAgent = encAgentIter->second;
    }

    IConvTask* task = nullptr;
    if (decAgent != nullptr && encAgent != nullptr) {
        task = IConvTask::Create(item, decAgent, encAgent);
    }
    return task;
}

void ConvTaskFactory::AddDecAgent(const IPluginAgent* pAgent)
{
    IDecoder* dec = (IDecoder*)pAgent->CreateObject();
    vector<string> list = dec->FileSuffix();
    pAgent->FreeObject(dec);

    for (const string& suffix: list) {
        vector<const IPluginAgent*>* agentList = nullptr;
        auto iter = m_DecAgentMap.find(suffix);
        if (iter == m_DecAgentMap.end()) {
            agentList = new vector<const IPluginAgent*>();
            agentList->push_back(pAgent);
            m_DecAgentMap.emplace(suffix, agentList);
        } else {
            agentList = iter->second;
            agentList->push_back(pAgent);
        }
    }
}

void ConvTaskFactory::RemoveDecAgent(const IPluginAgent* pAgent)
{
    IDecoder* dec = (IDecoder*)pAgent->CreateObject();
    vector<string> list = dec->FileSuffix();
    pAgent->FreeObject(dec);

    for (const string& suffix: list) {
        auto iter = m_DecAgentMap.find(suffix);
        if (iter != m_DecAgentMap.end()) {
            vector<const IPluginAgent*>& agentList = *(iter->second);
            for (size_t i = 0; i < agentList.size(); ++i) {
                if (agentList[i] == pAgent) {
                    agentList.erase(agentList.begin()+i);
                    break;
                }
            }
            if (agentList.empty()) {
                delete iter->second;
                m_DecAgentMap.erase(iter);
            }
        }
    }
}

void ConvTaskFactory::AddEncAgent(const IPluginAgent* pAgent)
{
    m_EncAgentMap.emplace(pAgent->Info()->name, pAgent);
}

void ConvTaskFactory::RemoveEncAgent(const IPluginAgent* pAgent)
{
    auto iter = m_EncAgentMap.find(pAgent->Info()->name);
    if (iter != m_EncAgentMap.end())
        m_EncAgentMap.erase(iter);
}
