#include "CharsetConv.h"
using namespace mous;

void* MousCreateCharsetConv(int bufSize)
{
    return new CharsetConv(bufSize);
}

void MousReleaseICharsetConv(void* ptr)
{
    if (ptr != NULL) {
        CharsetConv* conv = static_cast<CharsetConv*>(ptr);
        delete conv;
    }
}
