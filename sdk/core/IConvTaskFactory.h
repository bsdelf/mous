#ifndef MOUS_ICONVTASKFACTORY_H
#define MOUS_ICONVTASKFACTORY_H

#include <vector>
#include <string>

namespace mous {

struct MediaItem;
class IConvTask;
class IPluginAgent;

class IConvTaskFactory
{
public:
    static IConvTaskFactory* Create();
    static void Free(IConvTaskFactory*);

public:
    virtual ~IConvTaskFactory() { }

    virtual void RegisterDecoderPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterDecoderPlugin(std::vector<const IPluginAgent*>& agents) = 0;

    virtual void RegisterEncoderPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterEncoderPlugin(std::vector<const IPluginAgent*>& agents) = 0;

    virtual void UnregisterPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual std::vector<std::string> EncoderNames() const = 0;
    virtual IConvTask* CreateTask(const MediaItem* item, const std::string& encoder) const = 0;
};

}

#endif

