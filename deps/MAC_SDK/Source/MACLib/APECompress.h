#pragma once

#include "MACLib.h"

namespace APE
{
class CAPECompressCreate;

/*************************************************************************************************
CAPECompress - uses the CAPECompressHub to provide a simpler compression interface (with buffering, etc)
*************************************************************************************************/
class CAPECompress : public IAPECompress
{
public:
    CAPECompress();
    ~CAPECompress();

    // start encoding
    int Start(const wchar_t * pOutputFilename, const WAVEFORMATEX * pwfeInput, unsigned int nMaxAudioBytes, intn nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const void * pHeaderData = NULL, intn nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
    int StartEx(CIO * pioOutput, const WAVEFORMATEX * pwfeInput, unsigned int nMaxAudioBytes, intn nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const void * pHeaderData = NULL, intn nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
    
    // add data / compress data

    // allows linear, immediate access to the buffer (fast)
	intn GetBufferBytesAvailable();
    int UnlockBuffer(unsigned int nBytesAdded, bool bProcess = true);
    unsigned char * LockBuffer(intn * pBytesAvailable);
    
    // slower, but easier than locking and unlocking (copies data)
    int AddData(unsigned char * pData, intn nBytes);
    
    // use a CIO (input source) to add data
    int AddDataFromInputSource(CInputSource * pInputSource, unsigned int nMaxBytes = 0, int * pBytesAdded = NULL);
    
    // finish / kill
    int Finish(unsigned char * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes);
    int Kill();
    
private:    
    int ProcessBuffer(bool bFinalize = false);
    
    CSmartPtr<CAPECompressCreate> m_spAPECompressCreate;

    intn m_nBufferHead;
    intn m_nBufferTail;
    intn m_nBufferSize;
    unsigned char * m_pBuffer;
    bool m_bBufferLocked;

    CIO * m_pioOutput;
    bool m_bOwnsOutputIO;
    WAVEFORMATEX m_wfeInput;
};

}
