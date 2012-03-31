#include "ConvTaskFactory.h"
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
#include <scx/FileHelper.hpp>
using namespace scx;
using namespace mous;

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
    if (pAgent->GetType() == PluginType::Decoder) {
        AddDecAgent(pAgent);
    }
}

void ConvTaskFactory::RegisterEncoderPlugin(const IPluginAgent* pAgent)
{
    if (pAgent->GetType() == PluginType::Encoder) {
        AddEncAgent(pAgent);
    }
}

void ConvTaskFactory::UnregisterPlugin(const IPluginAgent* pAgent)
{
    switch (pAgent->GetType()) {
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

vector<string> ConvTaskFactory::GetEncoderNames() const
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

IConvTask* ConvTaskFactory::CreateTask(const MediaItem* item, const std::string& encoder) const
{
    const IPluginAgent* decAgent = NULL;
    const string& suffix = FileHelper::FileSuffix(item->url);
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
    if (item != NULL && decAgent != NULL && encAgent != NULL) {
        task = IConvTask::Create(item, decAgent, encAgent);
    }
    return task;
}

void ConvTaskFactory::AddDecAgent(const IPluginAgent* pAgent)
{
    IDecoder* dec = (IDecoder*)pAgent->CreateObject();
    vector<string> suffix = dec->GetFileSuffix();
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
    vector<string> suffix = dec->GetFileSuffix();
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
    m_EncAgentMap.insert(EncAgentMapPair(pAgent->GetInfo()->name, pAgent));
}

void ConvTaskFactory::RemoveEncAgent(const IPluginAgent* pAgent)
{
    EncAgentMapIter iter = m_EncAgentMap.find(pAgent->GetInfo()->name);
    if (iter != m_EncAgentMap.end())
        m_EncAgentMap.erase(iter);
}
