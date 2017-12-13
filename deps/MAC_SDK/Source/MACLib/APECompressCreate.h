#pragma once

#include "APECompress.h"

namespace APE
{
class CAPECompressCore;

class CAPECompressCreate
{
public:
    CAPECompressCreate();
    ~CAPECompressCreate();
    
    int InitializeFile(CIO * pIO, const WAVEFORMATEX * pwfeInput, intn nMaxFrames, intn nCompressionLevel, const void * pHeaderData, intn nHeaderBytes);
    int FinalizeFile(CIO * pIO, int nNumberOfFrames, int nFinalFrameBlocks, const void * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes, int nPeakLevel);
    
    int SetSeekByte(int nFrame, int nByteOffset);

    int Start(CIO * pioOutput, const WAVEFORMATEX * pwfeInput, unsigned int nMaxAudioBytes, intn nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const void * pHeaderData = NULL, intn nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
        
    intn GetFullFrameBytes();
    int EncodeFrame(const void * pInputData, intn nInputBytes);

    int Finish(const void * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes);
    
private:    
    CSmartPtr<uint32> m_spSeekTable;
    intn m_nMaxFrames;

    CSmartPtr<CIO> m_spIO;
    CSmartPtr<CAPECompressCore> m_spAPECompressCore;
    
    WAVEFORMATEX m_wfeInput;
    intn m_nCompressionLevel;
    intn m_nSamplesPerFrame;
    intn m_nFrameIndex;
    intn m_nLastFrameBlocks;
};

}
