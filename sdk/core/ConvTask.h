#pragma once

#include <memory>
#include <vector>
#include <string>

#include <util/Plugin.h>
#include <util/MediaItem.h>
#include <util/Option.h>

namespace mous {

class ConvTask
{
    class Impl;

public:
    ConvTask(const MediaItem& item, const std::shared_ptr<Plugin>& decoderPlugin, const std::shared_ptr<Plugin>& encoderPlugin);
    ~ConvTask();

    std::vector<const BaseOption*> DecoderOptions() const;
    std::vector<const BaseOption*> EncoderOptions() const;
    std::string EncoderFileSuffix() const;

    void Run(const std::string& output);
    void Cancel();

    // [0, 1]: progress, < 0: failed
    double Progress() const;

    bool IsFinished() const;

private:
    std::unique_ptr<Impl> impl;
};

}
