#include "ConvTaskFactory.h"
using namespace mous;

IConvTaskFactory* IConvTaskFactory::Create()
{
    return new ConvTaskFactory();
}

void IConvTaskFactory::Free(IConvTaskFactory* factory)
{
    if (factory != NULL)
        delete factory;
}

ConvTaskFactory::ConvTaskFactory()
{
}

ConvTaskFactory::~ConvTaskFactory()
{
}

bool ConvTaskFactory::CanConvert(const MediaItem* item, std::vector<std::string>& encoders) const
{
    encoders.clear();
    return true;
}

IConvTask* ConvTaskFactory::CreateTask(const MediaItem* item, const std::string& encoder) const
{
    return IConvTask::Create(item, NULL, NULL);
}

