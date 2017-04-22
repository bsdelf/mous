#pragma once

#include <memory>
#include <vector>

#include <core/Plugin.h>
#include <util/MediaItem.h>
#include <util/Option.h>

namespace mous {

class ConvTask
{
    class Impl;

public:
    explicit ConvTask(const MediaItem& item, const Plugin* decoderPlugin, const Plugin* encoderPlugin);
    ~ConvTask();

    std::vector<const BaseOption*> DecoderOptions() const;
    std::vector<const BaseOption*> EncoderOptions() const;
    const char* EncoderFileSuffix() const;

    void Run(const std::string& output);
    void Cancel();

    // [0, 1]: progress, < 0: failed
    double Progress() const;

    bool IsFinished() const;

private:
    std::unique_ptr<Impl> impl;
};

}
