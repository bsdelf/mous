#pragma once

#include <dlfcn.h>
#include <memory>
#include <util/PluginDef.h>

namespace mous {

class Plugin {
    using FuncPluginType = PluginType (*)(void);
    using FuncPluginInfo = const PluginInfo* (*)(void);
    using FuncCreateObject = void* (*)(void);
    using FuncFreeObject = void (*)(void*);

  public:
    Plugin() = default;
    Plugin(const Plugin&) = delete;
    Plugin& operator=(const Plugin&) = delete;

    Plugin(const std::string& path, int mode = RTLD_LAZY | RTLD_GLOBAL) {
        handle_ = dlopen(path.c_str(), mode);
        if (!handle_) {
            return;
        }
        funcPluginType_ = Symbol<FuncPluginType>(StrGetPluginType);
        if (!funcPluginType_) {
            goto Cleanup;
        }
        funcPluginInfo_ = Symbol<FuncPluginInfo>(StrGetPluginInfo);
        if (!funcPluginInfo_) {
            goto Cleanup;
        }
        funcCreateObject_ = Symbol<FuncCreateObject>(StrCreateObject);
        if (!funcCreateObject_) {
            goto Cleanup;
        }
        funcFreeObject_ = Symbol<FuncFreeObject>(StrFreeObject);
        if (!funcFreeObject_) {
            goto Cleanup;
        }
        type_ = funcPluginType_();
        path_ = path;
        return;

    Cleanup:
        dlclose(handle_);
        handle_ = nullptr;
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

    auto Info() const {
        return funcPluginInfo_();
    }

    template<class T>
    T CreateObject() const {
        void* data = funcCreateObject_();
        return static_cast<T>(data);
    }

    void FreeObject(void* data) const {
        funcFreeObject_(data);
    }

    static inline auto LatestError() {
        return std::string(dlerror());
    }

  private:
    void* handle_ = nullptr;
    FuncPluginType funcPluginType_ = nullptr;
    FuncPluginInfo funcPluginInfo_ = nullptr;
    FuncCreateObject funcCreateObject_ = nullptr;
    FuncFreeObject funcFreeObject_ = nullptr;
    PluginType type_ = PluginType::None;
    std::string path_;
};

}
