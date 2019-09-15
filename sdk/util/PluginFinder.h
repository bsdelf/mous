#pragma once

#include <scx/Conv.h>
#include <scx/Dir.h>
#include <scx/FileInfo.h>
#include <strings.h>
#include <util/Plugin.h>
#include <util/PluginDef.h>
#include <deque>
#include <functional>
#include <memory>

namespace mous {

class PluginFinder {
 public:
  PluginFinder() {
    Reset();
  }

  void Reset() {
    for (int i = 0; i < ncallbacks_; ++i) {
      callbacks_[i] = [](const auto&) {};
    }
  }

  // NOTE: be careful with overwrite
  PluginFinder& OnPlugin(PluginType type, std::function<void(const std::shared_ptr<Plugin>& plugin)> callback) {
    const auto bits = scx::ToUnderlying(type);
    for (int k = 0; k < ncallbacks_; ++k) {
      const auto mask = 1u << k;
      if (bits & mask) {
        callbacks_[k] = callback;
      }
    }
    return *this;
  }

  void Run(const std::string& path) const {
    std::deque<std::string> fifo{path};
    while (!fifo.empty()) {
      std::string head{std::move(fifo.front())};
      fifo.pop_front();
      auto type = scx::FileInfo(head).Type();
      switch (type) {
        case scx::FileType::Regular: {
          auto plugin = std::make_shared<Plugin>(head);
          if (*plugin) {
            const auto bits = scx::ToUnderlying(plugin->Type());
            for (int k = 0; k < ncallbacks_; ++k) {
              const auto mask = 1u << k;
              if (bits & mask) {
                callbacks_[k](plugin);
              }
            }
          }
          break;
        }
        case scx::FileType::Directory: {
          const auto& files = scx::Dir::ListDir(head);
          for (size_t i = 0; i < files.size(); ++i) {
            const auto& file = files[i];
            if (file == "." || file == "..") {
              continue;
            }
            fifo.emplace_back(head + "/" + file);
          }
          break;
        }
        default: {
          break;
        }
      }
    }
  }

 private:
  static constexpr int ncallbacks_ = 32;
  std::function<void(const std::shared_ptr<Plugin>&)> callbacks_[ncallbacks_];
};

}  // namespace mous
