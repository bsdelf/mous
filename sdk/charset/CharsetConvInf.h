#ifndef MOUS_CHARSETCONVINF_H
#define MOUS_CHARSETCONVINF_H

#ifdef __cplusplus
extern "C" {
#endif
    void* MousCreateCharsetConv(int bufSize);
    void MousReleaseICharsetConv(void* ptr);
#ifdef __cplusplus
}
#endif

#endif
