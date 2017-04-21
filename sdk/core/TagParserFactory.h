#pragma once

#include <memory>
#include <string>

#include <core/Plugin.h>
#include <plugin/ITagParser.h>

namespace mous {

class TagParserFactory
{
    class Impl;

public:
    TagParserFactory();
    ~TagParserFactory();

    void RegisterTagParserPlugin(const Plugin* pAgent);
    void RegisterTagParserPlugin(std::vector<const Plugin*>& agents);

    void UnregisterPlugin(const Plugin* pAgent);
    void UnregisterPlugin(std::vector<const Plugin*>& agents);
    void UnregisterAll();

    ITagParser* CreateParser(const std::string& fileName) const;
    void FreeParser(ITagParser* parser) const;

private:
    std::unique_ptr<Impl> impl;
};

}
