#ifndef FLAC_DECODER_H
#define FLAC_DECODER_H

#include <plugin/IDecoder.h>
#include <FLAC/stream_decoder.h>
using namespace std;
using namespace mous;

class FlacDecoder: public IDecoder
{
public:
    FlacDecoder();
    virtual ~FlacDecoder();

    virtual vector<string> FileSuffix() const;

    virtual EmErrorCode Open(const string& url);
    virtual void Close();

    virtual bool IsFormatVaild() const;

    virtual EmErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount);
    virtual EmErrorCode SetUnitIndex(uint64_t index);
    virtual uint32_t MaxBytesPerUnit() const;
    virtual uint64_t UnitIndex() const;
    virtual uint64_t UnitCount() const;

    virtual EmAudioMode AudioMode() const;
    virtual int32_t Channels() const;
    virtual int32_t BitsPerSample() const;
    virtual int32_t SampleRate() const;
    virtual int32_t BitRate() const;
    virtual uint64_t Duration() const;

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

private:
    struct decoder_context_t {
        char* buf = nullptr;
        int32_t buf_used = 0;
        int32_t samples_read = 0;
    };

    FLAC__StreamDecoder* m_pDecoder;
    decoder_context_t m_dctx;

    uint64_t m_SampleIndex;
    uint64_t m_SampleCount;

    int32_t m_Channels;
    int32_t m_BitsPerSample;
    int32_t m_SampleRate;
    int32_t m_SamplesPerFrame;
    int32_t m_BitRate;
    uint64_t m_Duration;
};

#endif
