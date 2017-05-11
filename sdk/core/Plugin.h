#pragma once

#include <memory>
#include <util/PluginDef.h>

namespace mous {

class Plugin
{
    class Impl;

public:
    explicit Plugin(const std::string& path);
    ~Plugin();

    PluginType Type() const;
    const PluginInfo* Info() const;
    void* CreateObject() const;
    void FreeObject(void* inf) const;

private:
    std::unique_ptr<Impl> impl;
};

}
