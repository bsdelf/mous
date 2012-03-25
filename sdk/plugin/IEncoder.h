#ifndef MOUS_IENCODER_H
#define MOUS_IENCODER_H

#include <vector>
#include <inttypes.h>
#include <common/AudioMode.h>
#include <common/ErrorCode.h>
#include <common/Option.h>

namespace mous {

class IEncoder
{
public:
    virtual ~IEncoder() { }

    virtual EmErrorCode EncodeUnit(char* data, uint32_t& used, uint32_t& unitCount) = 0;
    virtual EmErrorCode FlushRest() = 0;

    virtual void SetAudioMode(EmAudioMode mode) const = 0;
    virtual void SetChannels(int32_t channels) const = 0;
    virtual void SetSampleRate(int32_t sampleRate) const = 0;
    virtual void SetBitRate(int32_t bitRate) const = 0;

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<ConstOptionPair>& list) const { return false; };
};

}

#endif
