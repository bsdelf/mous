#pragma once

#include <unordered_map>
#include <util/MediaItem.h>
#include <core/IConvTaskFactory.h>
#include <core/IConvTask.h>
#include <core/Plugin.h>
using namespace std;

namespace mous {

class ConvTaskFactory: public IConvTaskFactory
{
public:
    ConvTaskFactory();
    virtual ~ConvTaskFactory();

    virtual void RegisterDecoderPlugin(const Plugin* pAgent);
    virtual void RegisterDecoderPlugin(std::vector<const Plugin*>& agents);

    virtual void RegisterEncoderPlugin(const Plugin* pAgent);
    virtual void RegisterEncoderPlugin(std::vector<const Plugin*>& agents);

    virtual void UnregisterPlugin(const Plugin* pAgent);
    virtual void UnregisterPlugin(std::vector<const Plugin*>& agents);
    virtual void UnregisterAll();

    virtual vector<string> EncoderNames() const;
    virtual IConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const;

private:
    void AddDecAgent(const Plugin* pAgent);
    void RemoveDecAgent(const Plugin* pAgent);
    void AddEncAgent(const Plugin* pAgent);
    void RemoveEncAgent(const Plugin* pAgent);

private:
    unordered_map<string, vector<const Plugin*>*> m_DecAgentMap;
    unordered_map<string, const Plugin*> m_EncAgentMap;

};

}
