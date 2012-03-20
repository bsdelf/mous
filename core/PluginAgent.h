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

    EmPluginType GetType() const;
    const PluginInfo* GetInfo() const;
    void* CreateObject() const;
    void FreeObject(void* inf) const;

private:
    void* m_Handle;

    FnPluginType m_FnPluginType;
    FnPluginInfo m_FnGetInfo;
    FnCreateObject m_FnCreate;
    FnFreeObject m_FnFree;

    EmPluginType m_Type;
};

}
#endif
