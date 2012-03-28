#ifndef MOUS_CONVTASKFACTORY_H
#define MOUS_CONVTASKFACTORY_H

#include <core/IConvTaskFactory.h>
#include <core/IConvTask.h>

namespace mous {

class ConvTaskFactory: public IConvTaskFactory
{
public:
    ConvTaskFactory();
    virtual ~ConvTaskFactory();

    virtual bool CanConvert(const MediaItem* item, std::vector<std::string>& encoders) const;
    virtual IConvTask* CreateTask(const MediaItem* item, const std::string& encoder) const;
};

}

#endif
