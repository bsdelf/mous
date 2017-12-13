/*****************************************************************************************
UnMAC.h
Copyright (C) 2000-2018 by Matthew T. Ashland   All Rights Reserved.

Methods for decompressing or verifying APE files

Notes:
    -none
*****************************************************************************************/

#pragma once

#include "../BitArray.h"
#include "../UnBitArrayBase.h"

namespace APE
{

class CAntiPredictor;
class CPrepare;
class CAPEDecompressCore;
class CPredictorBase;
class IPredictorDecompress;
class IAPEDecompress;

/*****************************************************************************************
CUnMAC class... a class that allows decoding on a frame-by-frame basis
*****************************************************************************************/
class CUnMAC 
{
public:
    
    // construction/destruction
    CUnMAC();
    ~CUnMAC();

    // functions
    int Initialize(IAPEDecompress *pAPEDecompress);
    int Uninitialize();
    intn DecompressFrame(unsigned char *pOutputData, int32 FrameIndex, int CPULoadBalancingFactor = 0);

    int SeekToFrame(intn FrameIndex);
    
private:

    // data members
    bool m_bInitialized;
    int m_LastDecodedFrameIndex;
    IAPEDecompress * m_pAPEDecompress;
    CPrepare * m_pPrepare;

    CAPEDecompressCore * m_pAPEDecompressCore;

    // functions
    void GenerateDecodedArrays(intn nBlocks, intn nSpecialCodes, intn nFrameIndex, intn nCPULoadBalancingFactor);
    void GenerateDecodedArray(int *Input_Array, uint32 Number_of_Elements, intn Frame_Index, CAntiPredictor *pAntiPredictor, intn CPULoadBalancingFactor = 0);

    int CreateAntiPredictors(int nCompressionLevel, int nVersion);

    intn DecompressFrameOld(unsigned char *pOutputData, int32 FrameIndex, int CPULoadBalancingFactor);
    uint32 CalculateOldChecksum(int *pDataX, int *pDataY, intn nChannels, intn nBlocks);

public:
    
    int m_nBlocksProcessed;
    unsigned int m_nCRC;
    unsigned int m_nStoredCRC;
    WAVEFORMATEX m_wfeInput;
};

}