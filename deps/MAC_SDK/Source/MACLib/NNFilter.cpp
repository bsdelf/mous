#include "All.h"
#include "GlobalFunctions.h"
#include "NNFilter.h"
#ifdef ENABLE_SSE_ASSEMBLY
	#include <emmintrin.h> // SSE 2
#endif

namespace APE
{

CNNFilter::CNNFilter(intn nOrder, intn nShift, intn nVersion)
{
    if ((nOrder <= 0) || ((nOrder % 16) != 0)) throw(1);
    m_nOrder = nOrder;
    m_nShift = nShift;
    m_nVersion = nVersion;
    
#ifdef ENABLE_SSE_ASSEMBLY
    m_bSSEAvailable = GetSSEAvailable();
#endif

    m_rbInput.Create(NN_WINDOW_ELEMENTS, m_nOrder);
    m_rbDeltaM.Create(NN_WINDOW_ELEMENTS, m_nOrder);
    m_paryM = (short *) AllocateAligned(intn(sizeof(short)) * m_nOrder, 16); // align for possible SSE usage
}

CNNFilter::~CNNFilter()
{
    if (m_paryM != NULL)
    {
        FreeAligned(m_paryM);
        m_paryM = NULL;
    }
}

void CNNFilter::Flush()
{
    memset(&m_paryM[0], 0, m_nOrder * sizeof(short));
    m_rbInput.Flush();
    m_rbDeltaM.Flush();
    m_nRunningAverage = 0;
}

int CNNFilter::Compress(int nInput)
{
    // convert the input to a short and store it
    m_rbInput[0] = GetSaturatedShortFromInt(nInput);

    // figure a dot product
    intn nDotProduct;
#ifdef ENABLE_SSE_ASSEMBLY
    if (m_bSSEAvailable)
       nDotProduct = CalculateDotProductSSE(&m_rbInput[-m_nOrder], &m_paryM[0], m_nOrder);
    else
#endif
        nDotProduct = CalculateDotProduct(&m_rbInput[-m_nOrder], &m_paryM[0], m_nOrder);

    // calculate the output
    int nOutput = int(nInput) - int((int(nDotProduct) + (1 << (int(m_nShift) - 1))) >> int(m_nShift));

    // adapt
#ifdef ENABLE_SSE_ASSEMBLY
    if (m_bSSEAvailable)
        AdaptSSE(&m_paryM[0], &m_rbDeltaM[-m_nOrder], nOutput, m_nOrder);
    else
#endif
        Adapt(&m_paryM[0], &m_rbDeltaM[-m_nOrder], nOutput, m_nOrder);

    int nTempABS = abs(nInput);

    if (nTempABS > (m_nRunningAverage * 3))
        m_rbDeltaM[0] = ((nInput >> 25) & 64) - 32;
    else if (nTempABS > (m_nRunningAverage * 4) / 3)
        m_rbDeltaM[0] = ((nInput >> 26) & 32) - 16;
    else if (nTempABS > 0)
        m_rbDeltaM[0] = ((nInput >> 27) & 16) - 8;
    else
        m_rbDeltaM[0] = 0;

    m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

    m_rbDeltaM[-1] >>= 1;
    m_rbDeltaM[-2] >>= 1;
    m_rbDeltaM[-8] >>= 1;
        
    // increment and roll if necessary
    m_rbInput.IncrementSafe();
    m_rbDeltaM.IncrementSafe();

    return nOutput;
}

int CNNFilter::Decompress(int nInput)
{
    // figure a dot product
    intn nDotProduct;
#ifdef ENABLE_SSE_ASSEMBLY
    if (m_bSSEAvailable)
        nDotProduct = CalculateDotProductSSE(&m_rbInput[-m_nOrder], &m_paryM[0], m_nOrder);
    else
#endif
		nDotProduct = CalculateDotProduct(&m_rbInput[-m_nOrder], &m_paryM[0], m_nOrder);

    // adapt
#ifdef ENABLE_SSE_ASSEMBLY
    if (m_bSSEAvailable)
        AdaptSSE(&m_paryM[0], &m_rbDeltaM[-m_nOrder], nInput, m_nOrder);
    else
#endif
		Adapt(&m_paryM[0], &m_rbDeltaM[-m_nOrder], nInput, m_nOrder);

    // store the output value
    int nOutput = nInput + ((int(nDotProduct) + (1 << (m_nShift - 1))) >> m_nShift);

    // update the input buffer
    m_rbInput[0] = GetSaturatedShortFromInt(nOutput);

    if (m_nVersion >= 3980)
    {
        int nTempABS = abs(nOutput);

        if (nTempABS > (m_nRunningAverage * 3))
            m_rbDeltaM[0] = ((nOutput >> 25) & 64) - 32;
        else if (nTempABS > (m_nRunningAverage * 4) / 3)
            m_rbDeltaM[0] = ((nOutput >> 26) & 32) - 16;
        else if (nTempABS > 0)
            m_rbDeltaM[0] = ((nOutput >> 27) & 16) - 8;
        else
            m_rbDeltaM[0] = 0;

        m_nRunningAverage += (nTempABS - m_nRunningAverage) / 16;

        m_rbDeltaM[-1] >>= 1;
        m_rbDeltaM[-2] >>= 1;
        m_rbDeltaM[-8] >>= 1;
    }
    else
    {
        m_rbDeltaM[0] = (nOutput == 0) ? 0 : ((nOutput >> 28) & 8) - 4;
        m_rbDeltaM[-4] >>= 1;
        m_rbDeltaM[-8] >>= 1;
    }

    // increment and roll if necessary
    m_rbInput.IncrementSafe();
    m_rbDeltaM.IncrementSafe();
    
    return nOutput;
}

void CNNFilter::Adapt(short * pM, short * pAdapt, int nDirection, intn nOrder)
{
    nOrder >>= 4;

    if (nDirection < 0) 
    {    
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ += *pAdapt++;)
        }
    }
    else if (nDirection > 0)
    {
        while (nOrder--)
        {
            EXPAND_16_TIMES(*pM++ -= *pAdapt++;)
        }
    }
}

intn CNNFilter::CalculateDotProduct(short * pA, short * pB, intn nOrder)
{
    intn nDotProduct = 0;
    nOrder >>= 4;

    while (nOrder--)
    {
        EXPAND_16_TIMES(nDotProduct += *pA++ * *pB++;)
    }
    
    return nDotProduct;
}

#ifdef ENABLE_SSE_ASSEMBLY

void CNNFilter::AdaptSSE(short * pM, short * pAdapt, int nDirection, intn nOrder)
{
    // we require that pM is aligned, allowing faster loads and stores
    ASSERT((size_t(pM) % 16) == 0);

    if (nDirection < 0) 
    {    
        for (int z = 0; z < nOrder; z += 8)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_add_epi16(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
    else if (nDirection > 0)
    {
        for (int z = 0; z < nOrder; z += 8)
        {
            __m128i sseM = _mm_load_si128((__m128i *) &pM[z]);
            __m128i sseAdapt = _mm_loadu_si128((__m128i *) &pAdapt[z]);
            __m128i sseNew = _mm_sub_epi16(sseM, sseAdapt);
            _mm_store_si128((__m128i *) &pM[z], sseNew);
        }
    }
}

intn CNNFilter::CalculateDotProductSSE(short * pA, short * pB, intn nOrder)
{
    // we require that pB is aligned, allowing faster loads
    ASSERT((size_t(pB) % 16) == 0);

    // loop
    __m128i sseSum = _mm_setzero_si128();
    for (int z = 0; z < nOrder; z += 8)
    {
        __m128i sseA = _mm_loadu_si128((__m128i *) &pA[z]);
        __m128i sseB = _mm_load_si128((__m128i *) &pB[z]);
        __m128i sseDotProduct = _mm_madd_epi16(sseA, sseB);
        sseSum = _mm_add_epi32(sseSum, sseDotProduct);
    }

    // build output
	intn nDotProduct = 0;
#ifndef _MSC_VER
    __oword my__oword;
    my__oword.m128i = sseSum;
    nDotProduct = my__oword.m128i_i32[0] + my__oword.m128i_i32[1] + my__oword.m128i_i32[2] + my__oword.m128i_i32[3];
#else
    nDotProduct = sseSum.m128i_i32[0] + sseSum.m128i_i32[1] + sseSum.m128i_i32[2] + sseSum.m128i_i32[3];
#endif

    // TODO: SSE4 instructions might help performance of the horizontal add, for example:
    //int nDotProduct = _mm_extract_epi32(sseSum, 0) + _mm_extract_epi32(sseSum, 1) + _mm_extract_epi32(sseSum, 2) + _mm_extract_epi32(sseSum, 3);

    return nDotProduct;
}

#endif

}