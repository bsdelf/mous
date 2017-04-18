#pragma once

#include <vector>
#include <string>

namespace mous {

struct MediaItem;
class IConvTask;
class Plugin;

class IConvTaskFactory
{
public:
    static IConvTaskFactory* Create();
    static void Free(IConvTaskFactory*);

public:
    virtual ~IConvTaskFactory() { }

    virtual void RegisterDecoderPlugin(const Plugin* pAgent) = 0;
    virtual void RegisterDecoderPlugin(std::vector<const Plugin*>& agents) = 0;

    virtual void RegisterEncoderPlugin(const Plugin* pAgent) = 0;
    virtual void RegisterEncoderPlugin(std::vector<const Plugin*>& agents) = 0;

    virtual void UnregisterPlugin(const Plugin* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const Plugin*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual std::vector<std::string> EncoderNames() const = 0;
    virtual IConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const = 0;
};

}
