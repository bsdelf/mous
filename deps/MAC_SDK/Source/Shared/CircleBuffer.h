#pragma once

namespace APE
{

class CCircleBuffer  
{
public:
    // construction / destruction
    CCircleBuffer();
    virtual ~CCircleBuffer();

    // create the buffer
    void CreateBuffer(intn nBytes, intn nMaxDirectWriteBytes);

    // query
	intn MaxAdd();
	intn MaxGet();

    // direct writing
    __forceinline unsigned char * GetDirectWritePointer()
    {
        // return a pointer to the tail -- note that it will always be safe to write
        // at least m_nMaxDirectWriteBytes since we use an end cap region
        return &m_pBuffer[m_nTail];
    }

    __forceinline void UpdateAfterDirectWrite(intn nBytes)
    {
        // update the tail
        m_nTail += nBytes;

        // if the tail enters the "end cap" area, set the end cap and loop around
        if (m_nTail >= (m_nTotal - m_nMaxDirectWriteBytes))
        {
            m_nEndCap = m_nTail;
            m_nTail = 0;
        }
    }

    // get data
    intn Get(unsigned char * pBuffer, intn nBytes);

    // remove / empty
    void Empty();
	intn RemoveHead(intn nBytes);
	intn RemoveTail(intn nBytes);

private:
    intn m_nTotal;
    intn m_nMaxDirectWriteBytes;
    intn m_nEndCap;
    intn m_nHead;
    intn m_nTail;
    unsigned char * m_pBuffer;
};

}
