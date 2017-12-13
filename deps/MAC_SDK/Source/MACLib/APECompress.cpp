#include "All.h"
#include "APECompress.h"
#include IO_HEADER_FILE
#include "APECompressCreate.h"
#include "WAVInputSource.h"

namespace APE
{

CAPECompress::CAPECompress()
{
    m_nBufferHead = 0;
    m_nBufferTail = 0;
    m_nBufferSize = 0;
    m_bBufferLocked = false;
    m_bOwnsOutputIO = false;
    m_pioOutput = NULL;
    m_pBuffer = NULL;

    m_spAPECompressCreate.Assign(new CAPECompressCreate());
}

CAPECompress::~CAPECompress()
{
    SAFE_ARRAY_DELETE(m_pBuffer)

    if (m_bOwnsOutputIO)
    {
        SAFE_DELETE(m_pioOutput)
    }
}

int CAPECompress::Start(const wchar_t * pOutputFilename, const WAVEFORMATEX * pwfeInput, unsigned int nMaxAudioBytes, intn nCompressionLevel, const void * pHeaderData, intn nHeaderBytes)
{
    m_pioOutput = new IO_CLASS_NAME;
    m_bOwnsOutputIO = true;
    
    if (m_pioOutput->Create(pOutputFilename) != 0)
    {
        return ERROR_INVALID_OUTPUT_FILE;
    }
        
    m_spAPECompressCreate->Start(m_pioOutput, pwfeInput, nMaxAudioBytes, nCompressionLevel,
        pHeaderData, nHeaderBytes);
    
    SAFE_ARRAY_DELETE(m_pBuffer)
    m_nBufferSize = m_spAPECompressCreate->GetFullFrameBytes();
    m_pBuffer = new unsigned char [m_nBufferSize];
    memcpy(&m_wfeInput, pwfeInput, sizeof(WAVEFORMATEX));

    return ERROR_SUCCESS;
}

int CAPECompress::StartEx(CIO * pioOutput, const WAVEFORMATEX * pwfeInput, unsigned int nMaxAudioBytes, intn nCompressionLevel, const void * pHeaderData, intn nHeaderBytes)
{
    m_pioOutput = pioOutput;
    m_bOwnsOutputIO = false;

    m_spAPECompressCreate->Start(m_pioOutput, pwfeInput, nMaxAudioBytes, nCompressionLevel,
        pHeaderData, nHeaderBytes);

    SAFE_ARRAY_DELETE(m_pBuffer)
    m_nBufferSize = m_spAPECompressCreate->GetFullFrameBytes();
    m_pBuffer = new unsigned char [m_nBufferSize];
    memcpy(&m_wfeInput, pwfeInput, sizeof(WAVEFORMATEX));

    return ERROR_SUCCESS;
}

intn CAPECompress::GetBufferBytesAvailable()
{
    return m_nBufferSize - m_nBufferTail;
}

int CAPECompress::UnlockBuffer(unsigned int nBytesAdded, bool bProcess)
{
    if (!m_bBufferLocked)
        return ERROR_UNDEFINED;
    
    m_nBufferTail += nBytesAdded;
    m_bBufferLocked = false;
    
    if (bProcess)
    {
        int nResult = ProcessBuffer();
        if (nResult != 0) { return nResult; }
    }
    
    return ERROR_SUCCESS;
}

unsigned char * CAPECompress::LockBuffer(intn * pBytesAvailable)
{
    if (m_pBuffer == NULL) { return NULL; }
    
    if (m_bBufferLocked)
        return NULL;
    
    m_bBufferLocked = true;
    
    if (pBytesAvailable)
        *pBytesAvailable = GetBufferBytesAvailable();
    
    return &m_pBuffer[m_nBufferTail];
}

int CAPECompress::AddData(unsigned char * pData, intn nBytes)
{
    if (m_pBuffer == NULL) return ERROR_INSUFFICIENT_MEMORY;

    intn nBytesDone = 0;
    
    while (nBytesDone < nBytes)
    {
        // lock the buffer
        intn nBytesAvailable = 0;
        unsigned char * pBuffer = LockBuffer(&nBytesAvailable);
        if (pBuffer == NULL || nBytesAvailable <= 0)
            return ERROR_UNDEFINED;
        
        // calculate how many bytes to copy and add that much to the buffer
		intn nBytesToProcess = ape_min(nBytesAvailable, nBytes - nBytesDone);
        memcpy(pBuffer, &pData[nBytesDone], nBytesToProcess);
                        
        // unlock the buffer (fail if not successful)
        int nResult = UnlockBuffer((unsigned int) nBytesToProcess);
        if (nResult != ERROR_SUCCESS)
                return nResult;

        // update our progress
        nBytesDone += nBytesToProcess;
    }

    return ERROR_SUCCESS;
} 

int CAPECompress::Finish(unsigned char * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes)
{
    RETURN_ON_ERROR(ProcessBuffer(true))
    return m_spAPECompressCreate->Finish(pTerminatingData, nTerminatingBytes, nWAVTerminatingBytes);
}

int CAPECompress::Kill()
{
    return ERROR_SUCCESS;
}

int CAPECompress::ProcessBuffer(bool bFinalize)
{
    if (m_pBuffer == NULL) { return ERROR_UNDEFINED; }
    
    try
    {
        // process as much as possible
        intn nThreshold = (bFinalize) ? 0 : m_spAPECompressCreate->GetFullFrameBytes();
        
        while ((m_nBufferTail - m_nBufferHead) >= nThreshold)
        {
            intn nFrameBytes = ape_min(m_spAPECompressCreate->GetFullFrameBytes(), m_nBufferTail - m_nBufferHead);
            
            if (nFrameBytes == 0)
                break;

            int nResult = m_spAPECompressCreate->EncodeFrame(&m_pBuffer[m_nBufferHead], nFrameBytes);
            if (nResult != 0) { return nResult; }
            
            m_nBufferHead += nFrameBytes;
        }
        
        // shift the buffer
        if (m_nBufferHead != 0)
        {
			intn nBytesLeft = m_nBufferTail - m_nBufferHead;
            
            if (nBytesLeft != 0)
                memmove(m_pBuffer, &m_pBuffer[m_nBufferHead], nBytesLeft);
            
            m_nBufferTail -= m_nBufferHead;
            m_nBufferHead = 0;
        }
    }
    catch(...)
    {
        return ERROR_UNDEFINED;
    }
    
    return ERROR_SUCCESS;
}

int CAPECompress::AddDataFromInputSource(CInputSource * pInputSource, unsigned int nMaxBytes, int * pBytesAdded)
{
    // error check the parameters
    if (pInputSource == NULL) return ERROR_BAD_PARAMETER;

    // initialize
    if (pBytesAdded) *pBytesAdded = 0;
        
    // lock the buffer
	intn nBytesAvailable = 0;
    unsigned char * pBuffer = LockBuffer(&nBytesAvailable);
    if ((pBuffer == NULL) || (nBytesAvailable == 0))
        return ERROR_INSUFFICIENT_MEMORY;
    
    // calculate the 'ideal' number of bytes
    unsigned int nBytesRead = 0;

	intn nIdealBytes = m_spAPECompressCreate->GetFullFrameBytes() - (m_nBufferTail - m_nBufferHead);
    if (nIdealBytes > 0)
    {
        // get the data
		intn nBytesToAdd = nBytesAvailable;
        
        if (nMaxBytes > 0)
        {
            if (nBytesToAdd > nMaxBytes) nBytesToAdd = nMaxBytes;
        }

        if (nBytesToAdd > nIdealBytes) nBytesToAdd = nIdealBytes;

        // always make requests along block boundaries
        while ((nBytesToAdd % m_wfeInput.nBlockAlign) != 0)
            nBytesToAdd--;

        intn nBlocksToAdd = nBytesToAdd / m_wfeInput.nBlockAlign;

        // get data
        int nBlocksAdded = 0;
        int nResult = pInputSource->GetData(pBuffer, (int) nBlocksToAdd, &nBlocksAdded);
        if (nResult != 0)
            return ERROR_IO_READ;
        else
            nBytesRead = (nBlocksAdded * m_wfeInput.nBlockAlign);
        
        // store the bytes read
        if (pBytesAdded)
            *pBytesAdded = nBytesRead;
    }
        
    // unlock the data and process
    int nResult = UnlockBuffer(nBytesRead, true);
    if (nResult != 0)
    {
        return nResult;
    }
    
    return ERROR_SUCCESS;
}

}