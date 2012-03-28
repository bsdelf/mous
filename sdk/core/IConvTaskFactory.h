#ifndef MOUS_ICONVTASKFACTORY_H
#define MOUS_ICONVTASKFACTORY_H

#include <vector>
#include <string>

namespace mous {

struct MediaItem;
class IConvTask;

class IConvTaskFactory
{
public:
    static IConvTaskFactory* Create();
    static void Free(IConvTaskFactory*);

public:
    virtual ~IConvTaskFactory() { }

    virtual bool CanConvert(const MediaItem* item, std::vector<std::string>& encoders) const = 0;
    virtual IConvTask* CreateTask(const MediaItem* item, const std::string& encoder) const = 0;
};

}

#endif

