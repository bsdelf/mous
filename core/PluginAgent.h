#pragma once

#include <core/IPluginAgent.h>

namespace mous {

class PluginAgent: public IPluginAgent
{
    using FnPluginType = EmPluginType (*)(void);
    using FnPluginInfo = const PluginInfo* (*)(void);
    using FnCreateObject = void* (*)(void);
    using FnFreeObject = void (*)(void*);

public:
    explicit PluginAgent(const std::string& path);
    ~PluginAgent();

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
