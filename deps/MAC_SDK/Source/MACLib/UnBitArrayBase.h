#pragma once

namespace APE
{

class IAPEDecompress;
class CIO;

struct UNBIT_ARRAY_STATE
{
    uint32 k;
    uint32 nKSum;
};

enum DECODE_VALUE_METHOD
{
    DECODE_VALUE_METHOD_UNSIGNED_INT,
    DECODE_VALUE_METHOD_UNSIGNED_RICE,
    DECODE_VALUE_METHOD_X_BITS
};

class CUnBitArrayBase
{
public:
    // construction / destruction
    CUnBitArrayBase(intn nFurthestReadByte);
    virtual ~CUnBitArrayBase();
    
    // functions
    virtual int FillBitArray();
    virtual int FillAndResetBitArray(intn nFileLocation = -1, intn nNewBitIndex = 0);
        
    virtual void GenerateArray(int * pOutputArray, int nElements, intn nBytesRequired = -1) {}
    virtual unsigned int DecodeValue(DECODE_VALUE_METHOD DecodeMethod, int nParam1 = 0, int nParam2 = 0) { return 0; }
    
    virtual void AdvanceToByteBoundary();
	virtual bool EnsureBitsAvailable(uint32 nBits, bool bThrowExceptionOnFailure);

    virtual int DecodeValueRange(UNBIT_ARRAY_STATE & BitArrayState) { return 0; }
    virtual void FlushState(UNBIT_ARRAY_STATE & BitArrayState) { }
    virtual void FlushBitArray() { }
    virtual void Finalize() { }
    
protected:
    virtual int CreateHelper(CIO * pIO, intn nBytes, intn nVersion);
    virtual uint32 DecodeValueXBits(uint32 nBits);
    
    uint32 m_nElements;
    uint32 m_nBytes;
    uint32 m_nBits;
    uint32 m_nGoodBytes;
    
    intn m_nVersion;
    CIO * m_pIO;
    intn m_nFurthestReadByte;

    uint32 m_nCurrentBitIndex;
    uint32 * m_pBitArray;
};

CUnBitArrayBase * CreateUnBitArray(IAPEDecompress * pAPEDecompress, intn nVersion);

}
