#include <core/MediaLoader.h>
#include <util/Plugin.h>

#include "MediaLoaderImpl.h"

namespace mous {

MediaLoader::MediaLoader()
    : impl(std::make_unique<Impl>()) {
}

MediaLoader::~MediaLoader() {
}

void MediaLoader::LoadSheetParserPlugin(const std::shared_ptr<Plugin>& plugin) {
  return impl->LoadSheetParserPlugin(plugin);
}

void MediaLoader::LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin) {
  return impl->LoadTagParserPlugin(plugin);
}

void MediaLoader::UnloadPlugin(const std::string& path) {
  return impl->UnloadPlugin(path);
}

void MediaLoader::UnloadPlugin() {
  return impl->UnloadPlugin();
}

std::vector<std::string> MediaLoader::SupportedSuffixes() const {
  return impl->SupportedSuffixes();
}

ErrorCode MediaLoader::LoadMedia(const std::string& path, std::deque<MediaItem>& list) const {
  return impl->LoadMedia(path, list);
}

}  // namespace mous
