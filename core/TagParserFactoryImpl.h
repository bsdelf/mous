#pragma once

#include <scx/FileHelper.h>
#include <map>

namespace mous {

class TagParserFactory::Impl {
 public:
  ~Impl() {
    UnloadPlugin();
  }

  void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin) {
    auto parser = std::make_shared<TagParser>(plugin);
    if (!parser || !*parser) {
      return;
    }
    const auto& suffixList = parser->FileSuffix();
    for (const std::string& suffix : suffixList) {
      auto iter = parsers_.find(suffix);
      if (iter == parsers_.end()) {
        parsers_.emplace(suffix, parser);
      }
    }
  }

  void UnloadPlugin(const std::string&) {
    // TODO
  }

  void UnloadPlugin() {
    parsers_.clear();
  }

  std::shared_ptr<TagParser> CreateParser(const std::string& fileName) const {
    const std::string& suffix = scx::FileHelper::FileSuffix(fileName);
    auto iter = parsers_.find(suffix);
    if (iter == parsers_.end()) {
      iter = parsers_.find("*");
      if (iter == parsers_.end()) {
        return {};
      }
    }
    return std::make_shared<TagParser>(iter->second->GetPlugin());
  }

 private:
  std::map<std::string, std::shared_ptr<TagParser>> parsers_;
};

}  // namespace mous
