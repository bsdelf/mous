#pragma once

#include <string>
#include <vector>
#include <core/Plugin.h>

namespace mous {

struct MediaItem;
struct BaseOption;

class IConvTask
{
public:
    // the content pointed by MediaItem* will be copyed
    static IConvTask* Create(const MediaItem&, const Plugin* decAgent, const Plugin* encAgent);
    static void Free(IConvTask*);

public:
    virtual ~IConvTask() { }

    virtual std::vector<const BaseOption*> DecoderOptions() const = 0;
    virtual std::vector<const BaseOption*> EncoderOptions() const = 0;
    virtual const char* EncoderFileSuffix() const = 0;

    virtual void Run(const std::string& output) = 0;
    virtual void Cancel() = 0;

    // percent [0, 1], failed < 0
    virtual double Progress() const = 0;
    virtual bool IsFinished() const = 0;
};

}
