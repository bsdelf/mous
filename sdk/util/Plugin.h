#pragma once

#include <dlfcn.h>
#include <util/PluginDef.h>
#include <utility>

namespace mous {

class Plugin {
  using GetPluginInfo = const PluginInfo* (*)(void);

 public:
  friend void swap(Plugin& a, Plugin& b) {
    using std::swap;
    swap(a.path_, b.path_);
    swap(a.handle_, b.handle_);
    swap(a.type_, b.type_);
    swap(a.name_, b.name_);
    swap(a.desc_, b.desc_);
    swap(a.version_, b.version_);
  }

  Plugin() = default;

  Plugin(const Plugin&) = delete;

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
      ::dlclose(handle);
      return;
    }
    path_ = path;
    handle_ = handle;
    type_ = info->type;
    name_ = info->name;
    desc_ = info->desc;
    version_ = info->version;
  }

  Plugin(Plugin&& that)
      : Plugin() {
    swap(*this, that);
  }

  ~Plugin() {
    if (handle_) {
      ::dlclose(handle_);
    }
  }

  Plugin& operator=(Plugin that) {
    swap(*this, that);
    return *this;
  }

  explicit operator bool() const {
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

}  // namespace mous
