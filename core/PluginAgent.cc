#include <dlfcn.h>

#include <string>
#include <stdexcept>

#include "PluginAgent.h"
using namespace mous;

IPluginAgent* IPluginAgent::Create(const std::string& path)
{
    return new PluginAgent(path);
}

void IPluginAgent::Free(IPluginAgent* ptr)
{
    if (ptr != nullptr) {
        delete ptr;
    }
}

PluginAgent::PluginAgent(const std::string& path)
{
    std::string what;

    m_Handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!m_Handle) {
        goto RaiseException;
    }

    m_FnPluginType = (FnPluginType)dlsym(m_Handle, StrGetPluginType);
    if (!m_FnPluginType) {
        goto CleanupAndRaise;
    }

    m_FnGetInfo = (FnPluginInfo)dlsym(m_Handle, StrGetPluginInfo);
    if (!m_FnGetInfo) {
        goto CleanupAndRaise;
    }

    m_FnCreate = (FnCreateObject)dlsym(m_Handle, StrCreateObject);
    if (!m_FnCreate) {
        goto CleanupAndRaise;
    }

    m_FnFree = (FnFreeObject)dlsym(m_Handle, StrFreeObject);
    if (!m_FnCreate) {
        goto CleanupAndRaise;
    }

    m_Type = m_FnPluginType();
    return;

RaiseException:
    what = dlerror();
    throw std::runtime_error(what);

CleanupAndRaise:
    what = dlerror();
    dlclose(m_Handle);
    throw std::runtime_error(what);
}

PluginAgent::~PluginAgent()
{
    dlclose(m_Handle);
}

EmPluginType PluginAgent::Type() const
{
    return m_Type;
}

const PluginInfo* PluginAgent::Info() const
{
    return m_FnGetInfo();
}

void* PluginAgent::CreateObject() const
{
    return m_FnCreate();
}

void PluginAgent::FreeObject(void* obj) const
{
    m_FnFree(obj);
}
