#pragma once

#include <memory>
#include <vector>
#include <string>

#include <core/Plugin.h>
#include <core/IConvTask.h>
#include <util/MediaItem.h>

namespace mous {

class ConvTaskFactory
{
    class Impl;

public:
    ConvTaskFactory();
    ~ConvTaskFactory();

    void RegisterDecoderPlugin(const Plugin* pAgent);
    void RegisterDecoderPlugin(std::vector<const Plugin*>& agents);

    void RegisterEncoderPlugin(const Plugin* pAgent);
    void RegisterEncoderPlugin(std::vector<const Plugin*>& agents);

    void UnregisterPlugin(const Plugin* pAgent);
    void UnregisterPlugin(std::vector<const Plugin*>& agents);
    void UnregisterAll();

    std::vector<std::string> EncoderNames() const;
    IConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const;

private:
    std::unique_ptr<Impl> impl;
};

}
