#include "All.h"
#include "Assembly.h"

extern "C"
{
#ifdef ENABLE_ASSEMBLY
  #ifdef ARCH_X86
    void Adapt_x86_mmx(short * pM, const short * pAdapt, int nDirection, int nOrder);
    int CalculateDotProduct_x86_mmx(const short * pA, const short * pB, int nOrder);
  #elif ARCH_X86_64
    void Adapt_x86_64_mmx(short * pM, const short * pAdapt, int nDirection, int nOrder);
    int CalculateDotProduct_x86_64_mmx(const short * pA, const short * pB, int nOrder);
    #endif
#endif

    void Adapt(short * pM, const short * pAdapt, int nDirection, int nOrder)
    {
#ifdef ENABLE_ASSEMBLY
    #ifdef ARCH_X86
        return Adapt_x86_mmx(pM, pAdapt, nDirection, nOrder);
    #elif ARCH_X86_64
        return Adapt_x86_64_mmx(pM, pAdapt, nDirection, nOrder);
    #endif
#endif
    }

    int CalculateDotProduct(const short * pA, const short * pB, int nOrder)
    {
#ifdef ENABLE_ASSEMBLY
    #ifdef ARCH_X86
        return CalculateDotProduct_x86_mmx(pA, pB, nOrder);
    #elif ARCH_X86_64
        return CalculateDotProduct_x86_64_mmx(pA, pB, nOrder);
    #endif
#endif
        return 0;
    }

    BOOL GetMMXAvailable()
    {
        BOOL ok = FALSE;
#ifdef ENABLE_ASSEMBLY
    #ifdef ARCH_X86
        ok = TRUE;
        printf("#mmx x86 ok\n");
    #elif ARCH_X86_64
        ok = TRUE;
        printf("#mmx amd64 ok\n");
    #endif
#endif
        return ok;
    }
};
