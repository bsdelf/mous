#ifndef MOUS_PLUGINAGENT_H
#define MOUS_PLUGINAGENT_H

#include <core/IPluginAgent.h>

namespace mous {

class PluginAgent: public IPluginAgent
{
    typedef EmPluginType (*FnPluginType)(void);
    typedef const PluginInfo* (*FnPluginInfo)(void);
    typedef void* (*FnCreateObject)(void);
    typedef void (*FnFreeObject)(void*);

public:
    PluginAgent();
    ~PluginAgent();

    EmErrorCode Open(const std::string& path);
    void Close();

    EmPluginType Type() const;
    const PluginInfo* Info() const;
    void* CreateObject() const;
    void FreeObject(void* inf) const;

private:
    void* m_Handle = nullptr;

    FnPluginType m_FnPluginType = nullptr;
    FnPluginInfo m_FnGetInfo = nullptr;
    FnCreateObject m_FnCreate = nullptr;
    FnFreeObject m_FnFree = nullptr;

    EmPluginType m_Type = PluginType::None;
};

}
#endif
