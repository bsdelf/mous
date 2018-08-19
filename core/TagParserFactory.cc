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

void TagParserFactory::LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin)
{
    return impl->LoadTagParserPlugin(plugin);
}

void TagParserFactory::UnloadPlugin(const std::string& path)
{
    return impl->UnloadPlugin(path);
}

void TagParserFactory::UnloadPlugin()
{
    return impl->UnloadPlugin();
}

std::shared_ptr<TagParser> TagParserFactory::CreateParser(const std::string& fileName) const
{
    return impl->CreateParser(fileName);
}

}
