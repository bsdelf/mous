#pragma once

#include <memory>
#include <string>
#include <vector>

#include <util/MediaItem.h>
#include <util/Option.h>
#include <util/Plugin.h>

namespace mous {

class Conversion {
  class Impl;

 public:
  Conversion(const MediaItem& item, const std::shared_ptr<Plugin>& decoderPlugin, const std::shared_ptr<Plugin>& encoderPlugin);
  ~Conversion();

  std::vector<const BaseOption*> DecoderOptions() const;
  std::vector<const BaseOption*> EncoderOptions() const;
  std::string EncoderFileSuffix() const;

  void Run(const std::string& output);
  void Cancel();

  // [0, 1]: progress, < 0: failed
  double Progress() const;

  bool IsFinished() const;

 private:
  std::unique_ptr<Impl> impl;
};

}  // namespace mous
