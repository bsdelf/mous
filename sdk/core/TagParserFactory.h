#pragma once

#include <memory>
#include <string>

#include <util/Plugin.h>
#include <plugin/ITagParser.h>

namespace mous {

class TagParserFactory
{
    class Impl;

public:
    TagParserFactory();
    ~TagParserFactory();

    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin);
    void UnloadPlugin(const std::string& path);
    void UnloadPlugin();

    ITagParser* CreateParser(const std::string& fileName) const;
    void FreeParser(ITagParser* parser) const;

private:
    std::unique_ptr<Impl> impl;
};

}
