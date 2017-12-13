#pragma once

#ifdef ENABLE_SSE_ASSEMBLY
	#include <emmintrin.h> // SSE 2
#endif

namespace APE
{

#include "RollBuffer.h"
#define NN_WINDOW_ELEMENTS    512

class CNNFilter
{
public:
    CNNFilter(intn nOrder, intn nShift, intn nVersion);
    ~CNNFilter();

    int Compress(int nInput);
    int Decompress(int nInput);
    void Flush();

private:
    intn m_nOrder;
	intn m_nShift;
	intn m_nVersion;
	intn m_nRunningAverage;
    APE::CRollBuffer<short> m_rbInput;
    APE::CRollBuffer<short> m_rbDeltaM;
    short * m_paryM;
	#ifdef ENABLE_SSE_ASSEMBLY
		bool m_bSSEAvailable;
	#endif

    __forceinline short GetSaturatedShortFromInt(int nValue) const
    {
        return short((nValue == short(nValue)) ? nValue : (nValue >> 31) ^ 0x7FFF);
    }

    __forceinline void Adapt(short * pM, short * pAdapt, int nDirection, intn nOrder);
    __forceinline intn CalculateDotProduct(short * pA, short * pB, intn nOrder);
    
	#ifdef ENABLE_SSE_ASSEMBLY
		__forceinline void AdaptSSE(short * pM, short * pAdapt, int nDirection, intn nOrder);
		__forceinline intn CalculateDotProductSSE(short * pA, short * pB, intn nOrder);
	#endif
    
	#if !defined(_MSC_VER) && defined(ENABLE_SSE_ASSEMBLY)
        typedef union __attribute__ ((aligned (16))) __oword {
            __m128i m128i;
            int8_t m128i_i8[16];
            int16_t m128i_i16[8];
            int32_t m128i_i32[4];
            int64_t m128i_i64[2];
            uint8_t m128i_u8[16];
            uint16_t m128i_u16[8];
            uint32_t m128i_u32[4];
            uint64_t m128i_u64[2];
        } __oword;
    #endif
};

}
