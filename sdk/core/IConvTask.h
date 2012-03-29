#ifndef MOUS_ICONVTASK_H
#define MOUS_ICONVTASK_H

#include <vector>
#include <util/ErrorCode.h>

namespace mous {

struct MediaItem;
class IDecoder;
class IEncoder;
struct BaseOption;

class IConvTask
{
public:
    // the content pointed by MediaItem* will be copyed
    static IConvTask* Create(const MediaItem*, IDecoder*, IEncoder*);
    static void Free(IConvTask*);

public:
    virtual ~IConvTask() { }

    virtual bool GetDecoderOptions(std::vector<const BaseOption*>& list) const = 0;
    virtual bool GetEncoderOptions(std::vector<const BaseOption*>& list) const = 0;

    virtual EmErrorCode Run() = 0;
    virtual void Cancel() = 0;

    // percent [0, 100], failed < 0, done > 100
    virtual int GetProgress() const = 0;
};

}

#endif
