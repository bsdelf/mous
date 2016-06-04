#pragma once

#include <string>
#include <vector>
#include <core/IPluginAgent.h>
#include <plugin/ITagParser.h>

namespace mous {

class ITagParserFactory
{
public:
    static ITagParserFactory* Create();
    static void Free(ITagParserFactory*);

public:
    virtual ~ITagParserFactory() { }

    virtual void RegisterTagParserPlugin(const IPluginAgent* pAgent) = 0;
    virtual void RegisterTagParserPlugin(std::vector<const IPluginAgent*>& agents) = 0;

    virtual void UnregisterPlugin(const IPluginAgent* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const IPluginAgent*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual ITagParser* CreateParser(const std::string& fileName) const = 0;
    virtual void FreeParser(ITagParser* parser) const = 0;
};

}
