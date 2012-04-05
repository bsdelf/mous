#ifndef FAACENCODER_H
#define FAACENCODER_H

#include <plugin/IEncoder.h>
#include <stdio.h>
#include <faac.h>
#include <mp4v2/mp4v2.h>
using namespace std;
using namespace mous;

class FaacEncoder: public IEncoder
{
public:
    FaacEncoder();
    virtual ~FaacEncoder();

    virtual const char* GetFileSuffix() const;

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode FlushRest();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual bool GetOptions(std::vector<const BaseOption*>& list) const;

private:
    RangedIntOption m_Quality;
    EnumedIntOption m_BitRate;

    string m_FileName;

    MP4FileHandle m_Mp4File;
    MP4TrackId m_Mp4Track;

    faacEncHandle m_EncHandle;
    unsigned long m_SampleRate;
    unsigned int m_Channels;
    unsigned long m_InputSamples;
    unsigned long m_MaxOutputBytes;

    int32_t m_BitsPerSample;

    char* m_InputBuffer;
    int m_InputBufferSize;
    int m_InputBufferUsed;
    char* m_OutputBuffer;
    int m_OutputBufferSize;
    int m_OutputBufferUsed;
};

#endif
