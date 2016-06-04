#include "PluginAgent.h"
#include <dlfcn.h>
using namespace mous;

#include <iostream>
using namespace std;

IPluginAgent* IPluginAgent::Create()
{
    return new PluginAgent;
}

void IPluginAgent::Free(IPluginAgent* ptr)
{
    if (ptr != nullptr)
        delete ptr;
}

PluginAgent::PluginAgent()
{

}

PluginAgent::~PluginAgent()
{
    Close();
}

EmErrorCode PluginAgent::Open(const std::string& path)
{
    m_Handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (m_Handle == nullptr) {
        cout << dlerror() << endl;
        return ErrorCode::PluginFailedToOpen;
    }

    m_FnPluginType = (FnPluginType)dlsym(m_Handle, StrGetPluginType);
    if (m_FnPluginType == nullptr) 
        goto LabelGotoFailed;

    m_FnGetInfo = (FnPluginInfo)dlsym(m_Handle, StrGetPluginInfo);
    if (m_FnGetInfo == nullptr) 
        goto LabelGotoFailed;

    m_FnCreate = (FnCreateObject)dlsym(m_Handle, StrCreateObject);
    if (m_FnCreate == nullptr) 
        goto LabelGotoFailed;

    m_FnFree = (FnFreeObject)dlsym(m_Handle, StrFreeObject);
    if (m_FnCreate == nullptr)
        goto LabelGotoFailed;

    m_Type = m_FnPluginType();

    return ErrorCode::Ok;

LabelGotoFailed:
    dlclose(m_Handle);
    cout << dlerror() << endl;
    return ErrorCode::PluginBadFormat;
}

void PluginAgent::Close()
{
    m_FnGetInfo = nullptr;
    m_FnCreate = nullptr;
    m_FnFree = nullptr;

    if (m_Handle != nullptr) {
        dlclose(m_Handle);
        m_Handle = nullptr;
    }
}

EmPluginType PluginAgent::Type() const
{
    return m_Type;
}

const PluginInfo* PluginAgent::Info() const
{
    return (m_FnGetInfo != nullptr) ? m_FnGetInfo() : nullptr;
}

void* PluginAgent::CreateObject() const
{
    return m_FnCreate();
}

void PluginAgent::FreeObject(void* inf) const
{
    m_FnFree(inf);
}
