#ifndef MOUS_CONVTASKFACTORY_H
#define MOUS_CONVTASKFACTORY_H

#include <map>
#include <common/MediaItem.h>
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
#include <core/IConvTaskFactory.h>
#include <core/IConvTask.h>
#include <core/IPluginAgent.h>
#include <scx/LPVBuffer.hpp>
using namespace scx;
using namespace std;

namespace mous {

class ConvTaskFactory: public IConvTaskFactory
{
public:
    ConvTaskFactory();
    virtual ~ConvTaskFactory();

    virtual void RegisterPluginAgent(const IPluginAgent* pAgent);
    virtual void UnregisterPluginAgent(const IPluginAgent* pAgent);
    virtual void UnregisterAll();

    virtual vector<string> GetEncoderNames() const;
    virtual IConvTask* CreateTask(const MediaItem* item, const std::string& encoder) const;

private:
    void AddDecAgent(const IPluginAgent* pAgent);
    void RemoveDecAgent(const IPluginAgent* pAgent);
    void AddEncAgent(const IPluginAgent* pAgent);
    void RemoveEncAgent(const IPluginAgent* pAgent);

private:
    map<string, vector<const IPluginAgent*>*> m_DecAgentMap;
    typedef pair<string, vector<const IPluginAgent*>*> DecAgentMapPair;
    typedef map<string, vector<const IPluginAgent*>*>::iterator DecAgentMapIter;
    typedef map<string, vector<const IPluginAgent*>*>::const_iterator DecAgentMapConstIter;

    map<string, const IPluginAgent*> m_EncAgentMap;
    typedef pair<string, const IPluginAgent*> EncAgentMapPair;
    typedef map<string, const IPluginAgent*>::iterator EncAgentMapIter;
    typedef map<string, const IPluginAgent*>::const_iterator EncAgentMapConstIter;

};

}

#endif
