#include "PluginManager.h"
#include <ftw.h>
#include <sys/stat.h>
#include <plugin/PluginHelper.h>
#include <plugin/IDecoder.h>
#include <plugin/IRenderer.h>
#include <plugin/IMediaPack.h>
#include <iostream>
using namespace std;
using namespace mous;

PluginManager::PluginManager()
{

}

PluginManager::~PluginManager()
{

}

EmErrorCode PluginManager::LoadPluginDir(const string& dir)
{
    vector<string> files;
    gFtwFiles = &files;
    ftw(dir.c_str(), &OnFtw, 1);
    gFtwFiles = NULL;

    for (size_t i = 0; i < files.size(); ++i) {
        LoadPlugin(files[i]);
    }

    return ErrorCode::Ok;
}

EmErrorCode PluginManager::LoadPlugin(const string& path)
{
    typedef EmPluginType (*FnPluginType)();
    FnPluginType fnPluginType;
    void* pHandle = NULL;

    pHandle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (pHandle == NULL) {
        cout << dlerror() << endl;
        return ErrorCode::MgrFailedToOpen;
    }

    fnPluginType = (FnPluginType)dlsym(pHandle, StrGetPluginType);
    if (fnPluginType == NULL) {
        dlclose(pHandle);
        return ErrorCode::MgrBadFormat;
    }
    EmPluginType type = fnPluginType();

    PluginAgent* pAgent = new PluginAgent(type);
    /*
       switch (type) {
       case PluginType::Decoder:
       pAgent = new PluginAgent<IDecoder>(type);
       break;

       case PluginType::Encoder:
       break;

       case PluginType::Renderer:
       pAgent = new PluginAgent<IRenderer>(type);
       break;

       case PluginType::MediaPack:
       pAgent = new PluginAgent<IMediaPack>(type);
       break;

       case PluginType::TagParser:
       pAgent = new PluginAgent<ITagParser>(type);
       break;

       case PluginType::Filter:
       break;

       default:
       pAgent = NULL;
       break;
       }
       */

    if (pAgent->Open(path) == ErrorCode::Ok) {
        m_PluginMap.insert(PluginMapPair(path, pAgent));
    } else {
        pAgent->Close();
        delete pAgent;
    }

    dlclose(pHandle);

    return ErrorCode::Ok;
}

void PluginManager::UnloadPlugin(const string& path)
{
    PluginMapIter iter = m_PluginMap.find(path);
    if (iter != m_PluginMap.end()) {
        PluginAgent* pAgent = iter->second;
        pAgent->Close();
        delete pAgent;
        m_PluginMap.erase(iter);
    }
}

void PluginManager::UnloadAllPlugins()
{
    for (PluginMapIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        PluginAgent* pAgent = iter->second;
        pAgent->Close();
        delete pAgent;
    }
    m_PluginMap.clear();
}

void PluginManager::GetPluginAgents(vector<const PluginAgent*>& list, EmPluginType type) const
{
    list.clear();
    for (PluginMapConstIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        PluginAgent* pAgent = iter->second;
        if (pAgent->GetType() == type) {
            list.push_back(pAgent);
        }
    }
}

void PluginManager::GetPluginPath(vector<string>& list) const
{
    list.clear();
    list.reserve(m_PluginMap.size());
    for (PluginMapConstIter iter = m_PluginMap.begin();
            iter != m_PluginMap.end(); ++iter) {
        list.push_back(iter->first);
    }
}

const PluginInfo* PluginManager::GetPluginInfo(const std::string& path) const
{
    PluginMapConstIter iter = m_PluginMap.find(path);
    return (iter != m_PluginMap.end()) ?
        iter->second->GetInfo() : NULL;
}

vector<string>* PluginManager::gFtwFiles = NULL;

int PluginManager::OnFtw(const char* file, const struct stat* s, int)
{
    if (!(s->st_mode & S_IFDIR))
        gFtwFiles->push_back(file);
    return 0;
}
