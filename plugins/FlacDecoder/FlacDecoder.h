#ifndef FLAC_DECODER_H
#define FLAC_DECODER_H

#include <mous/IDecoder.h>
#include <FLAC/stream_decoder.h>
using namespace std;
using namespace mous;

class FlacDecoder: public IDecoder
{
public:
    FlacDecoder();
    virtual ~FlacDecoder();

    virtual void GetFileSuffix(vector<string>& list) const;

    virtual EmErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual EmErrorCode ReadUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual EmErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t GetMaxBytesPerUnit() const;
    virtual uint64_t GetUnitIndex() const;
    virtual uint64_t GetUnitCount() const;

    virtual EmAudioMode GetAudioMode() const;
    virtual int32_t GetChannels() const;
    virtual int32_t GetBitsPerSample() const;
    virtual int32_t GetSampleRate() const;
    virtual int32_t GetBitRate() const;
    virtual uint64_t GetDuration() const;

private:
    static FLAC__StreamDecoderWriteStatus WriteCallback(
	    const FLAC__StreamDecoder *decoder, 
	    const FLAC__Frame *frame, 
	    const FLAC__int32 * const buffer[], 
	    void *client_data);

    static void ErrorCallback(
	    const FLAC__StreamDecoder *decoder, 
	    FLAC__StreamDecoderErrorStatus status, 
	    void *client_data);

    static char* gBuf;
    static int32_t gBufLen;
    static int32_t gSamplesRead;

private:
    FLAC__StreamDecoder* m_pDecoder;

    uint32_t m_MaxBytesPerUnit;
    uint64_t m_SampleIndex;
    uint64_t m_SampleCount;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    int32_t m_BitRate;
    uint64_t m_Duration;
};

#endif
