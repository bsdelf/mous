#ifndef MOUS_CONVTASKFACTORY_H
#define MOUS_CONVTASKFACTORY_H

#include <unordered_map>
#include <util/MediaItem.h>
#include <core/IConvTaskFactory.h>
#include <core/IConvTask.h>
#include <core/IPluginAgent.h>
using namespace std;

namespace mous {

class ConvTaskFactory: public IConvTaskFactory
{
public:
    ConvTaskFactory();
    virtual ~ConvTaskFactory();

    virtual void RegisterDecoderPlugin(const IPluginAgent* pAgent);
    virtual void RegisterDecoderPlugin(std::vector<const IPluginAgent*>& agents);

    virtual void RegisterEncoderPlugin(const IPluginAgent* pAgent);
    virtual void RegisterEncoderPlugin(std::vector<const IPluginAgent*>& agents);

    virtual void UnregisterPlugin(const IPluginAgent* pAgent);
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents);
    virtual void UnregisterAll();

    virtual vector<string> EncoderNames() const;
    virtual IConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const;

private:
    void AddDecAgent(const IPluginAgent* pAgent);
    void RemoveDecAgent(const IPluginAgent* pAgent);
    void AddEncAgent(const IPluginAgent* pAgent);
    void RemoveEncAgent(const IPluginAgent* pAgent);

private:
    unordered_map<string, vector<const IPluginAgent*>*> m_DecAgentMap;
    typedef pair<string, vector<const IPluginAgent*>*> DecAgentMapPair;

    unordered_map<string, const IPluginAgent*> m_EncAgentMap;
    typedef pair<string, const IPluginAgent*> EncAgentMapPair;

};

}

#endif
