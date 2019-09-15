#pragma once

#include <memory>
#include <string>

#include <plugin/TagParser.h>
#include <util/Plugin.h>

namespace mous {

class TagParserFactory {
  class Impl;

 public:
  TagParserFactory();
  ~TagParserFactory();

  void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin);
  void UnloadPlugin(const std::string& path);
  void UnloadPlugin();

  std::shared_ptr<TagParser> CreateParser(const std::string& fileName) const;

 private:
  std::unique_ptr<Impl> impl;
};

}  // namespace mous
