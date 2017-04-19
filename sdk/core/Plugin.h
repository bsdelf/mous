#pragma once

#include <memory>
#include <util/PluginDef.h>

namespace mous {

class PluginPrivate;

class Plugin
{
    friend PluginPrivate;

public:
    explicit Plugin(const std::string& path);
    ~Plugin();

    EmPluginType Type() const;
    const PluginInfo* Info() const;
    void* CreateObject() const;
    void FreeObject(void* inf) const;

private:
    std::unique_ptr<PluginPrivate> d;
};

}
