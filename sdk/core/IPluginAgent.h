#pragma once

#include <string>
#include <util/ErrorCode.h>
#include <util/PluginDef.h>

namespace mous {

class IPluginAgent
{
public:
    static IPluginAgent* Create(const std::string&);
    static void Free(IPluginAgent*);

public:
    virtual ~IPluginAgent() { }

    virtual EmPluginType Type() const = 0;
    virtual const PluginInfo* Info() const = 0;
    virtual void* CreateObject() const = 0;
    virtual void FreeObject(void* inf) const = 0;
};

}
