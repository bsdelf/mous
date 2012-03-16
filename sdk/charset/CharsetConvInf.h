#ifndef MOUS_CHARSETCONVINF_H
#define MOUS_CHARSETCONVINF_H

#ifdef __cplusplus
extern "C" {
#endif
    void* MousCreateCharsetConv(int bufSize);
    void MousReleaseICharsetConv(void* ptr);
    //TODO: complete c interfaces
#ifdef __cplusplus
}
#endif

#endif
