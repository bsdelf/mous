#pragma once

#include <dlfcn.h>
#include <memory>
#include <util/PluginDef.h>

namespace mous {

class Plugin {
    using FuncPluginInfo = const PluginInfo* (*)(void);

  public:
    Plugin() = default;
    Plugin(const Plugin&) = delete;
    Plugin& operator=(const Plugin&) = delete;

    Plugin(const std::string& path, int mode = RTLD_LAZY | RTLD_GLOBAL) {
        handle_ = dlopen(path.c_str(), mode);
        if (!handle_) {
            return;
        }
        func_plugin_info_ = Symbol<FuncPluginInfo>(StrGetPluginInfo);
        if (!func_plugin_info_) {
            dlclose(handle_);
            handle_ = nullptr;
            return;
        }
        const auto info = func_plugin_info_();
        path_ = path;
        type_ = info->type;
        name_ = info->name;
        desc_ = info->desc;
        version_ = info->version;
    }

    Plugin(Plugin&& that) {
        handle_ = that.handle_;
        that.handle_ = nullptr;
    }

    ~Plugin() {
        if (handle_) {
            dlclose(handle_);
            handle_ = nullptr;
        }
    }

    Plugin& operator=(Plugin&& that) {
        handle_ = that.handle_;
        that.handle_ = nullptr;
        return *this;
    }

    explicit operator bool () const {
        return handle_;
    }

    auto Symbol(const std::string& name) const {
        return dlsym(handle_, name.c_str());
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
        return std::string(dlerror());
    }

  private:
    std::string path_;
    void* handle_ = nullptr;
    FuncPluginInfo func_plugin_info_ = nullptr;
    PluginType type_ = PluginType::None;
    std::string name_;
    std::string desc_;
    uint32_t version_;
};

}
