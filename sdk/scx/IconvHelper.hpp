#ifndef SCX_ICONV_HELPER_HPP
#define SCX_ICONV_HELPER_HPP

#include <errno.h>
#include <iconv.h>
#include <string>

namespace scx {

namespace IconvHelper {

bool ConvFromTo(const std::string& from, const std::string& wanted, 
        const char* originalBuf, size_t originalLen, std::string& convertedContent,
        char* workBuf = NULL, size_t workLen = 0)
{
    typedef size_t (*StdIconv)(iconv_t, const char**, size_t*, char**, size_t*);
    StdIconv std_iconv = (StdIconv)iconv;

    if (from.empty() || wanted.empty())
        return false;
    if (from == wanted)
        return false;

    bool ok = true;
    const char* inbuf = originalBuf;
    size_t inleft = originalLen;

    char* outstart;
    size_t outlen;
    char* outbuf;
    size_t outleft;
    if (originalLen <= workLen) {
        outstart = workBuf;
        outlen = workLen;
    } else {
        outstart = new char[originalLen+4];
        outlen = originalLen+4;
    }
    outbuf = outstart;
    outleft = outlen;

    size_t converted = 0;
    do {
        iconv_t cd = iconv_open(wanted.c_str(), from.c_str());
        if (cd == (iconv_t)-1) {
            ok = false;
            break;
        }

        errno = 0;
        converted = std_iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
        if (converted != (size_t)-1) {
            break;
        } else if (errno != E2BIG) {
            ok = false;
            break;
        }
        inbuf = originalBuf;
        inleft = originalLen;
        if (outstart != workBuf) {
            delete[] outstart;
        }
        outlen = (outlen << 1);
        outstart = new char[outlen];
        outbuf = outstart;
        outleft = outlen;

        iconv_close(cd);
    } while(true);

    if (ok) {
        convertedContent.assign(outstart, outlen-outleft);
    }
    if (outstart != workBuf) {
        delete[] outstart;
    }
    return ok;
}

}
}

#endif
