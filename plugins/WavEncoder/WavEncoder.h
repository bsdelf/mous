#ifndef WAVENCODER_H
#define WAVENCODER_H

#include <plugin/IEncoder.h>
#include <fstream>
using namespace std;
using namespace mous;

#pragma pack(push, 1)

struct WavHeader
{
    // RIFF chunk
    char riffId[4];
    uint32_t lenAfterRiff;
    char riffType[4];

    // format chunk
    char formatId[4];
    uint32_t formatChunkLen;
    uint16_t formatTag;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t avgBytesPerSec;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    // data chunk
    char dataId[4];
    uint32_t dataChunkLen;
};

#pragma pack(pop)

class WavEncoder
{
public:
    WavEncoder();
    virtual ~WavEncoder();

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode FlushRest();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const { return false; };

private:
    static void InitWavHeader(WavHeader* header);

private:
    fstream m_OutputFile;
    WavHeader m_WavHeader;
};

#endif

