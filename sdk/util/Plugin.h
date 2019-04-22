#pragma once

#include <dlfcn.h>
#include <memory>
#include <utility>
#include <util/PluginDef.h>

namespace mous {

class Plugin {
    using GetPluginInfo = const PluginInfo* (*)(void);

  public:
    Plugin() = default;
    Plugin(const Plugin&) = delete;
    Plugin& operator=(const Plugin&) = delete;

    Plugin(const std::string& path, int mode = RTLD_LAZY | RTLD_GLOBAL) {
        auto handle = ::dlopen(path.c_str(), mode);
        if (!handle) {
            return;
        }
        auto get_plugin_info = reinterpret_cast<GetPluginInfo>(::dlsym(handle, StrGetPluginInfo));
        if (!get_plugin_info) {
            ::dlclose(handle);
            return;
        }
        const auto info = get_plugin_info();
        if (!info) {
            return;
        }
        path_ = path;
        handle_ = handle;
        type_ = info->type;
        name_ = info->name;
        desc_ = info->desc;
        version_ = info->version;
    }

    Plugin(Plugin&& that): Plugin() {
        std::swap(*this, that);
    }

    ~Plugin() {
        Clear();
    }

    Plugin& operator=(Plugin&& that) {
        Clear();
        std::swap(*this, that);
        return *this;
    }

    void Clear() {
        if (handle_) {
            ::dlclose(handle_);
            handle_ = nullptr;
        }
        path_.clear();
        type_ = PluginType::None;
        name_.clear();
        desc_.clear();
        version_ = 0;
    }

    explicit operator bool () const {
        return handle_;
    }

    auto Symbol(const std::string& name) const {
        return ::dlsym(handle_, name.c_str());
    }

    template <class T>
    T Symbol(const std::string& name) const {
        return reinterpret_cast<T>(Symbol(name));
    }
    
    auto Path() const {
        return path_;
    }

    auto Type() const {
        return type_;
    }

    auto Name() const {
        return name_;
    }

    auto Description() const {
        return desc_;
    }

    auto Version() const {
        return version_;
    }

    static inline auto LatestError() {
        return std::string(::dlerror());
    }

  private:
    std::string path_;
    void* handle_ = nullptr;
    PluginType type_ = PluginType::None;
    std::string name_;
    std::string desc_;
    uint32_t version_;
};

}
