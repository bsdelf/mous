#ifndef MOUS_PLUGINAGENT_H
#define MOUS_PLUGINAGENT_H

#include <core/IPluginAgent.h>
#include <dlfcn.h>

namespace mous {

class PluginAgent: public IPluginAgent
{
    typedef const PluginInfo* (*FnPluginInfo)(void);
    typedef void* (*FnCreateObject)(void);
    typedef void (*FnFreeObject)(void*);

public:
    explicit PluginAgent(EmPluginType type):
        m_Type(type),
        m_pHandle(NULL),
        m_fnGetInfo(NULL),
        m_fnCreate(NULL),
        m_fnFree(NULL)
    {

    }

    ~PluginAgent()
    {
        Close();
    }

    EmPluginType GetType() const
    {
        return m_Type;
    }

    EmErrorCode Open(const std::string& path)
    {
        m_pHandle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (m_pHandle == NULL)
            return ErrorCode::MgrBadFormat;

        m_fnGetInfo = (FnPluginInfo)dlsym(m_pHandle, StrGetPluginInfo);
        if (m_fnGetInfo == NULL)
            return ErrorCode::MgrBadFormat;

        m_fnCreate = (FnCreateObject)dlsym(m_pHandle, StrCreateObject);
        if (m_fnCreate == NULL)
            return ErrorCode::MgrBadFormat;

        m_fnFree = (FnFreeObject)dlsym(m_pHandle, StrFreeObject);
        if (m_fnCreate == NULL)
            return ErrorCode::MgrBadFormat;

        return ErrorCode::Ok;
    }

    void Close()
    {
        m_fnGetInfo = NULL;
        m_fnCreate = NULL;
        m_fnFree = NULL;

        if (m_pHandle != NULL) {
            dlclose(m_pHandle);
            m_pHandle = NULL;
        }
    }

    const PluginInfo* GetInfo() const
    {
        return (m_fnGetInfo != NULL) ? m_fnGetInfo() : NULL;
    }

    void* CreateObject() const
    {
        return m_fnCreate();
    }

    void FreeObject(void* inf) const
    {
        m_fnFree(inf);
    }

private:
    const EmPluginType m_Type;

    void* m_pHandle;

    FnPluginInfo m_fnGetInfo;
    FnCreateObject m_fnCreate;
    FnFreeObject m_fnFree;
};

}
#endif
