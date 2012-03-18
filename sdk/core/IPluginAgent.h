#ifndef MOUS_IPLUGINAGENT_H
#define MOUS_IPLUGINAGENT_H

#include <string>
#include <common/ErrorCode.h>
#include <common/PluginDef.h>

namespace mous {

class IPluginAgent
{
public:
    static IPluginAgent* Create(EmPluginType type);
    static void Free(IPluginAgent*);

public:
    virtual ~IPluginAgent() { }

    virtual EmPluginType GetType() const = 0;

    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    virtual const PluginInfo* GetInfo() const = 0;
    virtual void* CreateObject() const = 0;
    virtual void FreeObject(void* inf) const = 0;
};

}
#endif
