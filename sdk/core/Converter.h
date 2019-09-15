#pragma once

#include <memory>
#include <string>
#include <vector>

#include <core/Conversion.h>
#include <util/MediaItem.h>
#include <util/Plugin.h>

namespace mous {

class Converter {
  class Impl;

 public:
  Converter();
  ~Converter();

  void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin);
  void LoadEncoderPlugin(const std::shared_ptr<Plugin>& plugin);
  void UnloadPlugin(const std::string& path);
  void UnloadPlugin();

  std::vector<std::string> EncoderNames() const;
  std::shared_ptr<Conversion> CreateConversion(const MediaItem& item, const std::string& encoder) const;

 private:
  std::unique_ptr<Impl> impl;
};

}  // namespace mous
