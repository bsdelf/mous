#pragma once

#include <functional>
#include <memory>
#include <strings.h>
#include <scx/Conv.h>
#include <scx/Dir.h>
#include <scx/FileInfo.h>
#include <util/PluginDef.h>
#include <util/Plugin.h>

namespace mous {

class PluginScanner {
public:
    PluginScanner() {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < nCallbacks_; ++i) {
            callbacks_[i] = [](const std::shared_ptr<Plugin>&) {};
        }
    }

    // NOTE: be careful with overwrite
    PluginScanner& OnPlugin(PluginType type, std::function<void (const std::shared_ptr<Plugin>& plugin)> callback) {
        const auto bits = scx::ToUnderlying(type);
        for (int k = 0; k < nCallbacks_; ++k) {
            const auto mask = 1u << k;
            if (bits & mask) {
                callbacks_[k] = callback;
            }
        }
        return *this;
    }

    int Scan(const std::string& path) const {
        int count = 0;
        const auto& files = scx::Dir::ListDir(path);
        for (size_t i = 0; i < files.size(); ++i) {
            const auto& file = files[i];
            if (file.compare(0, 3, "lib") != 0) {
                continue;
            }           
            std::string full = path + "/" + file;
            if (scx::FileInfo(full).Type() != scx::FileType::Regular) {
                continue;
            }
            auto plugin = std::make_shared<Plugin>(full);
            if (!*plugin) {
                continue;
            }
            const auto bits = scx::ToUnderlying(plugin->Type());
            for (int k = 0; k < nCallbacks_; ++k) {
                const auto mask = 1u << k;
                if (bits & mask) {
                    callbacks_[k](plugin);
                }
            }
            ++count;
        }
        return count;
    }

private:
    static constexpr int nCallbacks_ = 32;
    std::function<void (const std::shared_ptr<Plugin>&)> callbacks_[nCallbacks_];
};

}
