#pragma once

#include "../APEDecompress.h"
#include "UnMAC.h"

namespace APE
{

class CAPEDecompressOld : public IAPEDecompress
{
public:
    CAPEDecompressOld(int * pErrorCode, CAPEInfo * pAPEInfo, int nStartBlock = -1, int nFinishBlock = -1);
    ~CAPEDecompressOld();

    int GetData(char * pBuffer, intn nBlocks, intn * pBlocksRetrieved);
    int Seek(intn nBlockOffset);

    intn GetInfo(APE_DECOMPRESS_FIELDS Field, intn nParam1 = 0, intn nParam2 = 0);
    
protected:
    // buffer
    CSmartPtr<char> m_spBuffer;
    intn m_nBufferTail;
    
    // file info
    intn m_nBlockAlign;
    intn m_nCurrentFrame;

    // start / finish information
    intn m_nStartBlock;
    intn m_nFinishBlock;
    intn m_nCurrentBlock;
    bool m_bIsRanged;

    // decoding tools    
    CUnMAC m_UnMAC;
    CSmartPtr<CAPEInfo> m_spAPEInfo;
    
    bool m_bDecompressorInitialized;
    int InitializeDecompressor();
};

}

