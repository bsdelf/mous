#pragma once

#include <string>
#include <vector>
#include <core/Plugin.h>
#include <plugin/ITagParser.h>

namespace mous {

class ITagParserFactory
{
public:
    static ITagParserFactory* Create();
    static void Free(ITagParserFactory*);

public:
    virtual ~ITagParserFactory() { }

    virtual void RegisterTagParserPlugin(const Plugin* pAgent) = 0;
    virtual void RegisterTagParserPlugin(std::vector<const Plugin*>& agents) = 0;

    virtual void UnregisterPlugin(const Plugin* pAgent) = 0;
    virtual void UnregisterPlugin(std::vector<const Plugin*>& agents) = 0;
    virtual void UnregisterAll() = 0;

    virtual ITagParser* CreateParser(const std::string& fileName) const = 0;
    virtual void FreeParser(ITagParser* parser) const = 0;
};

}
