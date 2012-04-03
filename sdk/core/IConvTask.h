#ifndef MOUS_ICONVTASK_H
#define MOUS_ICONVTASK_H

#include <string>
#include <vector>
#include <core/IPluginAgent.h>

namespace mous {

struct MediaItem;
struct BaseOption;

class IConvTask
{
public:
    // the content pointed by MediaItem* will be copyed
    static IConvTask* Create(const MediaItem*, const IPluginAgent* decAgent, const IPluginAgent* encAgent);
    static void Free(IConvTask*);

public:
    virtual ~IConvTask() { }

    virtual bool GetDecoderOptions(std::vector<const BaseOption*>& list) const = 0;
    virtual bool GetEncoderOptions(std::vector<const BaseOption*>& list) const = 0;
    virtual const char* GetEncoderFileSuffix() const = 0;

    virtual void Run(const std::string& output) = 0;
    virtual void Cancel() = 0;

    // percent [0, 1], failed < 0
    virtual double GetProgress() const = 0;
    virtual bool IsFinished() const = 0;
};

}

#endif
