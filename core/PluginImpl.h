#pragma once

#include <dlfcn.h>

#include <stdexcept>
#include <string>

namespace mous {

using FnPluginType = EmPluginType (*)(void);
using FnPluginInfo = const PluginInfo* (*)(void);
using FnCreateObject = void* (*)(void);
using FnFreeObject = void (*)(void*);

class Plugin::Impl
{
  public:
    void* lib = nullptr;
    FnPluginType GetPluginType = nullptr;
    FnPluginInfo GetPluginInfo = nullptr;
    FnCreateObject CreateObject = nullptr;
    FnFreeObject FreeObject = nullptr;
    EmPluginType type = PluginType::None;

    explicit Impl(const std::string& path)
    {
        std::string what;

        lib = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (!lib) {
            goto RaiseException;
        }

        GetPluginType = reinterpret_cast<FnPluginType>(dlsym(lib, StrGetPluginType));
        if (!GetPluginType) {
            goto CleanupAndRaise;
        }

        GetPluginInfo = reinterpret_cast<FnPluginInfo>(dlsym(lib, StrGetPluginInfo));
        if (!GetPluginInfo) {
            goto CleanupAndRaise;
        }

        CreateObject = reinterpret_cast<FnCreateObject>(dlsym(lib, StrCreateObject));
        if (!CreateObject) {
            goto CleanupAndRaise;
        }

        FreeObject = reinterpret_cast<FnFreeObject>(dlsym(lib, StrFreeObject));
        if (!CreateObject) {
            goto CleanupAndRaise;
        }

        type = GetPluginType();
        return;

    RaiseException:
        what = dlerror();
        throw std::runtime_error(what);

    CleanupAndRaise:
        what = dlerror();
        dlclose(lib);
        throw std::runtime_error(what);
    }

    ~Impl() { dlclose(lib); }
};
}
