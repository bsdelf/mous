#include <core/TagParserFactory.h>
#include "TagParserFactoryImpl.h"

namespace mous {

TagParserFactory::TagParserFactory()
    : impl(std::make_unique<Impl>())
{
}

TagParserFactory::~TagParserFactory()
{
}

void TagParserFactory::RegisterTagParserPlugin(const Plugin* plugin)
{
    return impl->RegisterTagParserPlugin(plugin);
}

void TagParserFactory::RegisterTagParserPlugin(std::vector<const Plugin*>& plugins)
{
    return impl->RegisterTagParserPlugin(plugins);
}

void TagParserFactory::UnregisterPlugin(const Plugin* plugin)
{
    return impl->UnregisterPlugin(plugin);
}

void TagParserFactory::UnregisterPlugin(std::vector<const Plugin*>& plugins)
{
    return impl->UnregisterPlugin(plugins);
}

void TagParserFactory::UnregisterAll()
{
    return impl->UnregisterAll();
}

ITagParser* TagParserFactory::CreateParser(const std::string& fileName) const
{
    return impl->CreateParser(fileName);
}

void TagParserFactory::FreeParser(ITagParser* parser) const
{
    return impl->FreeParser(parser);
}

}
