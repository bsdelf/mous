#include "PluginAgent.h"
using namespace mous;

IPluginAgent* IPluginAgent::Create(EmPluginType type)
{
    return new PluginAgent(type);
}

void IPluginAgent::Free(IPluginAgent* ptr)
{
    if (ptr != NULL)
        delete ptr;
}


