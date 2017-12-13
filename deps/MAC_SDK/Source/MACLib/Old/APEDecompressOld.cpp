#include "All.h"

#ifdef APE_BACKWARDS_COMPATIBILITY

#include "UnMAC.h"
#include "APEDecompressOld.h"
#include "../APEInfo.h"

namespace APE
{

CAPEDecompressOld::CAPEDecompressOld(int * pErrorCode, CAPEInfo * pAPEInfo, int nStartBlock, int nFinishBlock)
{
    *pErrorCode = ERROR_SUCCESS;

    // open / analyze the file
    m_spAPEInfo.Assign(pAPEInfo);

    // version check (this implementation only works with 3.92 and earlier files)
    if (GetInfo(APE_INFO_FILE_VERSION) > 3920)
    {
        *pErrorCode = ERROR_UNDEFINED;
        return;
    }

    // create the buffer
    m_nBlockAlign = (int)GetInfo(APE_INFO_BLOCK_ALIGN);
    
    // initialize other stuff
    m_nBufferTail = 0;
    m_bDecompressorInitialized = false;
    m_nCurrentFrame = 0;
    m_nCurrentBlock = 0;

    // set the "real" start and finish blocks
    m_nStartBlock = (nStartBlock < 0) ? 0 : ape_min(nStartBlock, (int)GetInfo(APE_INFO_TOTAL_BLOCKS));
    m_nFinishBlock = (nFinishBlock < 0) ? (int)GetInfo(APE_INFO_TOTAL_BLOCKS) : ape_min(nFinishBlock, (int)GetInfo(APE_INFO_TOTAL_BLOCKS));
    m_bIsRanged = (m_nStartBlock != 0) || (m_nFinishBlock != (int)GetInfo(APE_INFO_TOTAL_BLOCKS));
}

CAPEDecompressOld::~CAPEDecompressOld()
{

}

int CAPEDecompressOld::InitializeDecompressor()
{
    // check if we have anything to do
    if (m_bDecompressorInitialized)
        return ERROR_SUCCESS;

    // initialize the decoder
    RETURN_ON_ERROR(m_UnMAC.Initialize(this))

    intn nMaximumDecompressedFrameBytes = m_nBlockAlign * GetInfo(APE_INFO_BLOCKS_PER_FRAME);
    intn nTotalBufferBytes = ape_max(65536, (nMaximumDecompressedFrameBytes + 16) * 2);
    m_spBuffer.Assign(new char [nTotalBufferBytes], true);
    if (m_spBuffer == NULL)
        return ERROR_INSUFFICIENT_MEMORY;

    // update the initialized flag
    m_bDecompressorInitialized = true;

    // seek to the beginning
    return Seek(0);
}

int CAPEDecompressOld::GetData(char * pBuffer, intn nBlocks, intn * pBlocksRetrieved)
{
    if (pBlocksRetrieved) *pBlocksRetrieved = 0;

    RETURN_ON_ERROR(InitializeDecompressor())
    
    // cap
    intn nBlocksUntilFinish = m_nFinishBlock - m_nCurrentBlock;
    nBlocks = ape_min(nBlocks, nBlocksUntilFinish);

    intn nBlocksRetrieved = 0;

    // fulfill as much of the request as possible
    intn nTotalBytesNeeded = nBlocks * m_nBlockAlign;
	intn nBytesLeft = nTotalBytesNeeded;
	intn nBlocksDecoded = 1;

    while (nBytesLeft > 0 && nBlocksDecoded > 0)
    {
        // empty the buffer
		intn nBytesAvailable = m_nBufferTail;
		intn nIntialBytes = ape_min(nBytesLeft, nBytesAvailable);
        if (nIntialBytes > 0)
        {
            memcpy(&pBuffer[nTotalBytesNeeded - nBytesLeft], &m_spBuffer[0], nIntialBytes);
            
            if ((m_nBufferTail - nIntialBytes) > 0)
                memmove(&m_spBuffer[0], &m_spBuffer[nIntialBytes], m_nBufferTail - nIntialBytes);
                
            nBytesLeft -= nIntialBytes;
            m_nBufferTail -= nIntialBytes;

        }

        // decode more
        if (nBytesLeft > 0)
        {
            nBlocksDecoded = m_UnMAC.DecompressFrame((unsigned char *) &m_spBuffer[m_nBufferTail], (int32) m_nCurrentFrame++, 0);
            if (nBlocksDecoded == -1)
            {
                return -1;
            }
            m_nBufferTail += (nBlocksDecoded * m_nBlockAlign);
        }
    }
    
    nBlocksRetrieved = (nTotalBytesNeeded - nBytesLeft) / m_nBlockAlign;

    // update the position
    m_nCurrentBlock += nBlocksRetrieved;

    if (pBlocksRetrieved) *pBlocksRetrieved = nBlocksRetrieved;
    
    return ERROR_SUCCESS;
}

int CAPEDecompressOld::Seek(intn nBlockOffset)
{
    RETURN_ON_ERROR(InitializeDecompressor())

    // use the offset
    nBlockOffset += m_nStartBlock;
    
    // cap (to prevent seeking too far)
    if (nBlockOffset >= m_nFinishBlock)
        nBlockOffset = m_nFinishBlock - 1;
    if (nBlockOffset < m_nStartBlock)
        nBlockOffset = m_nStartBlock;

    // flush the buffer
    m_nBufferTail = 0;
    
    // seek to the perfect location
    intn nBaseFrame = nBlockOffset / GetInfo(APE_INFO_BLOCKS_PER_FRAME);
	intn nBlocksToSkip = nBlockOffset % GetInfo(APE_INFO_BLOCKS_PER_FRAME);
	intn nBytesToSkip = nBlocksToSkip * m_nBlockAlign;
        
    // skip necessary blocks
	intn nMaximumDecompressedFrameBytes = m_nBlockAlign * (int)GetInfo(APE_INFO_BLOCKS_PER_FRAME);
    char *pTempBuffer = new char [nMaximumDecompressedFrameBytes + 16];
    ZeroMemory(pTempBuffer, nMaximumDecompressedFrameBytes + 16);
    
    m_nCurrentFrame = nBaseFrame;

    intn nBlocksDecoded = m_UnMAC.DecompressFrame((unsigned char *) pTempBuffer, (int32) m_nCurrentFrame++, 0);
    
    if (nBlocksDecoded == -1)
    {
        return -1;
    }
    
	intn nBytesToKeep = (nBlocksDecoded * m_nBlockAlign) - nBytesToSkip;
    memcpy(&m_spBuffer[m_nBufferTail], &pTempBuffer[nBytesToSkip], nBytesToKeep);
    m_nBufferTail += nBytesToKeep;
    
    delete [] pTempBuffer;
    
    m_nCurrentBlock = nBlockOffset;
    
    return ERROR_SUCCESS;
}

intn CAPEDecompressOld::GetInfo(APE_DECOMPRESS_FIELDS Field, intn nParam1, intn nParam2)
{
    intn nRetVal = 0;
    bool bHandled = true;

    switch (Field)
    {
    case APE_DECOMPRESS_CURRENT_BLOCK:
        nRetVal = m_nCurrentBlock - m_nStartBlock;
        break;
    case APE_DECOMPRESS_CURRENT_MS:
    {
        int nSampleRate = (int)m_spAPEInfo->GetInfo(APE_INFO_SAMPLE_RATE, 0, 0);
        if (nSampleRate > 0)
            nRetVal = int((double(m_nCurrentBlock) * double(1000)) / double(nSampleRate));
        break;
    }
    case APE_DECOMPRESS_TOTAL_BLOCKS:
        nRetVal = m_nFinishBlock - m_nStartBlock;
        break;
    case APE_DECOMPRESS_LENGTH_MS:
    {
        int nSampleRate = (int)m_spAPEInfo->GetInfo(APE_INFO_SAMPLE_RATE, 0, 0);
        if (nSampleRate > 0)
            nRetVal = int((double(m_nFinishBlock - m_nStartBlock) * double(1000)) / double(nSampleRate));
        break;
    }
    case APE_DECOMPRESS_CURRENT_BITRATE:
        nRetVal = (int)GetInfo(APE_INFO_FRAME_BITRATE, m_nCurrentFrame);
        break;
    case APE_DECOMPRESS_AVERAGE_BITRATE:
    {
        if (m_bIsRanged)
        {
            // figure the frame range
            const intn nBlocksPerFrame = GetInfo(APE_INFO_BLOCKS_PER_FRAME);
			intn nStartFrame = m_nStartBlock / nBlocksPerFrame;
			intn nFinishFrame = (m_nFinishBlock + nBlocksPerFrame - 1) / nBlocksPerFrame;

            // get the number of bytes in the first and last frame
			intn nTotalBytes = (GetInfo(APE_INFO_FRAME_BYTES, nStartFrame) * (m_nStartBlock % nBlocksPerFrame)) / nBlocksPerFrame;
            if (nFinishFrame != nStartFrame)
                nTotalBytes += (GetInfo(APE_INFO_FRAME_BYTES, nFinishFrame) * (m_nFinishBlock % nBlocksPerFrame)) / nBlocksPerFrame;

            // get the number of bytes in between
            const intn nTotalFrames = (int)GetInfo(APE_INFO_TOTAL_FRAMES);
            for (intn nFrame = nStartFrame + 1; (nFrame < nFinishFrame) && (nFrame < nTotalFrames); nFrame++)
                nTotalBytes += (int)GetInfo(APE_INFO_FRAME_BYTES, nFrame);

            // figure the bitrate
            int nTotalMS = int((double(m_nFinishBlock - m_nStartBlock) * double(1000)) / double(GetInfo(APE_INFO_SAMPLE_RATE)));
            if (nTotalMS != 0)
                nRetVal = (nTotalBytes * 8) / nTotalMS;
        }
        else
        {
            nRetVal = (int)GetInfo(APE_INFO_AVERAGE_BITRATE);
        }

        break;
    }
    default:
        bHandled = false;
    }

    if (!bHandled && m_bIsRanged)
    {
        bHandled = true;

        switch (Field)
        {
        case APE_INFO_WAV_HEADER_BYTES:
            nRetVal = sizeof(WAVE_HEADER);
            break;
        case APE_INFO_WAV_HEADER_DATA:
        {
            char * pBuffer = (char *) nParam1;
            int nMaxBytes = (int)nParam2;
            
            if (sizeof(WAVE_HEADER) > nMaxBytes)
            {
                nRetVal = -1;
            }
            else
            {
                WAVEFORMATEX wfeFormat; GetInfo(APE_INFO_WAVEFORMATEX, (intn) &wfeFormat, 0);
                WAVE_HEADER WAVHeader; FillWaveHeader(&WAVHeader, 
                    (m_nFinishBlock - m_nStartBlock) * GetInfo(APE_INFO_BLOCK_ALIGN), 
                    &wfeFormat, 0);
                memcpy(pBuffer, &WAVHeader, sizeof(WAVE_HEADER));
                nRetVal = 0;
            }
            break;
        }
        case APE_INFO_WAV_TERMINATING_BYTES:
            nRetVal = 0;
            break;
        case APE_INFO_WAV_TERMINATING_DATA:
            nRetVal = 0;
            break;
        default:
            bHandled = false;
        }
    }

    if (!bHandled)
        nRetVal = m_spAPEInfo->GetInfo(Field, nParam1, nParam2);

    return nRetVal;
}

}

#endif // #ifdef APE_BACKWARDS_COMPATIBILITY
