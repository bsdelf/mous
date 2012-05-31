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
    if (factory != NULL)
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
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterDecoderPlugin(agents[i]);
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
    for (size_t i = 0; i < agents.size(); ++i) {
        RegisterEncoderPlugin(agents[i]);
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
    for (size_t i = 0; i < agents.size(); ++i) {
        UnregisterPlugin(agents[i]);
    }
}

void ConvTaskFactory::UnregisterAll()
{
    while (!m_DecAgentMap.empty()) {
        DecAgentMapIter iter = m_DecAgentMap.begin();
        vector<const IPluginAgent*>& list = *(iter->second);
        for (size_t i = 0; i < list.size(); ++i) {
            RemoveDecAgent(list[i]);
        }
    }
    
    m_EncAgentMap.clear();
}

vector<string> ConvTaskFactory::EncoderNames() const
{
    vector<string> list;
    list.reserve(m_EncAgentMap.size());

    EncAgentMapConstIter iter = m_EncAgentMap.begin();
    EncAgentMapConstIter end = m_EncAgentMap.end();
    while (iter != end) {
        list.push_back(iter->first);
        ++iter;
    }

    return list;
}

IConvTask* ConvTaskFactory::CreateTask(const MediaItem& item, const std::string& encoder) const
{
    const IPluginAgent* decAgent = NULL;
    const string& suffix = ToLower(FileHelper::FileSuffix(item.url));
    DecAgentMapConstIter decAgentIter = m_DecAgentMap.find(suffix);
    if (decAgentIter != m_DecAgentMap.end()) {
        vector<const IPluginAgent*> list = *(decAgentIter->second);
        decAgent = list[0];
    }

    const IPluginAgent* encAgent = NULL;
    EncAgentMapConstIter encAgentIter = m_EncAgentMap.find(encoder);
    if (encAgentIter != m_EncAgentMap.end()) {
        encAgent = encAgentIter->second;
    }

    IConvTask* task = NULL;
    if (decAgent != NULL && encAgent != NULL) {
        task = IConvTask::Create(item, decAgent, encAgent);
    }
    return task;
}

void ConvTaskFactory::AddDecAgent(const IPluginAgent* pAgent)
{
    IDecoder* dec = (IDecoder*)pAgent->CreateObject();
    vector<string> suffix = dec->FileSuffix();
    pAgent->FreeObject(dec);

    for (size_t i = 0; i < suffix.size(); ++i) {
        vector<const IPluginAgent*>* agentList = NULL;
        DecAgentMapIter iter = m_DecAgentMap.find(suffix[i]);
        if (iter == m_DecAgentMap.end()) {
            agentList = new vector<const IPluginAgent*>();
            agentList->push_back(pAgent);
            m_DecAgentMap.insert(DecAgentMapPair(suffix[i], agentList));
        } else {
            agentList = iter->second;
            agentList->push_back(pAgent);
        }
    }
}

void ConvTaskFactory::RemoveDecAgent(const IPluginAgent* pAgent)
{
    IDecoder* dec = (IDecoder*)pAgent->CreateObject();
    vector<string> suffix = dec->FileSuffix();
    pAgent->FreeObject(dec);

    for (size_t i = 0; i < suffix.size(); ++i) {
        DecAgentMapIter iter = m_DecAgentMap.find(suffix[i]);
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
    m_EncAgentMap.insert(EncAgentMapPair(pAgent->Info()->name, pAgent));
}

void ConvTaskFactory::RemoveEncAgent(const IPluginAgent* pAgent)
{
    EncAgentMapIter iter = m_EncAgentMap.find(pAgent->Info()->name);
    if (iter != m_EncAgentMap.end())
        m_EncAgentMap.erase(iter);
}
