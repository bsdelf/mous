#ifndef MP3DECODER_H
#define MP3DECODER_H

#include <mous/IDecoder.h>
#include <mous/PluginHelper.h>
#include "mpg123.h"
using namespace std;
using namespace mous;

class Mp3Decoder: public IDecoder {
public:
    Mp3Decoder();
    virtual ~Mp3Decoder();

    virtual const std::vector<std::string> GetFileSuffix() const;

    virtual ErrorCode Open(const std::string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual ErrorCode ReadUnit();
    virtual ErrorCode SetUnitIndex(uint32_t index);
    virtual uint32_t GetUnitIndex() const;
    virtual uint32_t GetUnitCount() const;
    virtual uint32_t GetMsPerUnit() const;

    virtual AudioMode GetAudioMode() const;
    virtual uint32_t GetBitRate() const;
    virtual uint32_t GetSampleRate() const;
    virtual uint64_t GetDuration() const;

private:
    mpg123_handle* m_pHandle;
    uint32_t m_unitIndex;
    uint32_t m_unitCount;
    uint32_t m_msPerUnit;
    uint64_t m_duration;
};

#endif
