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

    virtual void GetFileSuffix(std::vector<std::string>& list) const;

    virtual ErrorCode Open(const std::string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual ErrorCode ReadUnit(char* data, uint32_t& used);
    virtual ErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t GetMaxBytesPerUnit() const;
    virtual uint64_t GetUnitIndex() const;
    virtual uint64_t GetUnitCount() const;

    virtual AudioMode GetAudioMode() const;
    virtual uint32_t GetChannels() const;
    virtual uint32_t GetBitRate() const;
    virtual uint32_t GetSampleRate() const;
    virtual uint64_t GetDuration() const;

private:
    mpg123_handle* m_pHandle;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_UnitIndex;
    uint64_t m_UnitCount;

    uint32_t m_Channels;
    uint32_t m_BitRate;
    uint32_t m_SampleRate;
    uint64_t m_Duration;
};

#endif
