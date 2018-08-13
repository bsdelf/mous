#pragma once

#include <memory>
#include <vector>
#include <string>

#include <util/Plugin.h>
#include <util/MediaItem.h>
#include <core/ConvTask.h>

namespace mous {

class ConvTaskFactory
{
    class Impl;

public:
    ConvTaskFactory();
    ~ConvTaskFactory();

    void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin);
    void LoadEncoderPlugin(const std::shared_ptr<Plugin>& plugin);
    void UnloadPlugin(const std::string& path);
    void UnloadPlugin();

    std::vector<std::string> EncoderNames() const;
    ConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const;

private:
    std::unique_ptr<Impl> impl;
};

}
