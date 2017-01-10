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

    virtual const char* FileSuffix() const;

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode Flush();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual void SetMediaTag(const MediaTag* tag);
    virtual std::vector<const BaseOption*> Options() const;

private:
    //size_t WavReadFloat32();
    void WriteToolVersion();
    void UpdateMediaTag();

private:
    RangedIntOption m_OptQuality;
    RangedIntOption m_OptBitRate;
    GroupedOption m_OptVbrOrAbr;

    BooleanOption m_OptTns;
    BooleanOption m_OptMidSide;
    BooleanOption m_OptOptimize;

    string m_FileName;

    MP4FileHandle m_Mp4File;
    MP4TrackId m_Mp4Track;
    uint64_t m_TotalSamples;
    uint64_t m_EncodedSamples;
    unsigned int m_FrameSize;
    unsigned int m_DelaySamples;

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

    const MediaTag* m_MediaTag;

    //float* m_FloatBuffer;
};

#endif
