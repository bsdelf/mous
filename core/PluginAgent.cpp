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
    if (ptr != NULL)
        delete ptr;
}

PluginAgent::PluginAgent():
    m_Handle(NULL),
    m_FnGetInfo(NULL),
    m_FnCreate(NULL),
    m_FnFree(NULL),
    m_Type(PluginType::None)
{

}

PluginAgent::~PluginAgent()
{
    Close();
}

EmErrorCode PluginAgent::Open(const std::string& path)
{
    m_Handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (m_Handle == NULL) {
        cout << dlerror() << endl;
        return ErrorCode::PluginFailedToOpen;
    }

    m_FnPluginType = (FnPluginType)dlsym(m_Handle, StrGetPluginType);
    if (m_FnPluginType == NULL) 
        goto LabelGotoFailed;

    m_FnGetInfo = (FnPluginInfo)dlsym(m_Handle, StrGetPluginInfo);
    if (m_FnGetInfo == NULL) 
        goto LabelGotoFailed;

    m_FnCreate = (FnCreateObject)dlsym(m_Handle, StrCreateObject);
    if (m_FnCreate == NULL) 
        goto LabelGotoFailed;

    m_FnFree = (FnFreeObject)dlsym(m_Handle, StrFreeObject);
    if (m_FnCreate == NULL)
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
    m_FnGetInfo = NULL;
    m_FnCreate = NULL;
    m_FnFree = NULL;

    if (m_Handle != NULL) {
        dlclose(m_Handle);
        m_Handle = NULL;
    }
}

EmPluginType PluginAgent::Type() const
{
    return m_Type;
}

const PluginInfo* PluginAgent::Info() const
{
    return (m_FnGetInfo != NULL) ? m_FnGetInfo() : NULL;
}

void* PluginAgent::CreateObject() const
{
    return m_FnCreate();
}

void PluginAgent::FreeObject(void* inf) const
{
    m_FnFree(inf);
}
