#pragma once

#include <errno.h>
#include <iconv.h>
#include <string>
#include <vector>

namespace scx {

namespace IconvHelper {

inline bool
ConvFromTo(const std::string& srcEncoding,
           const std::string& destEncoding,
           const char* srcBuf,
           size_t srcLen,
           std::string& dest,
           std::vector<char>& workBuf)
{
    if (srcEncoding.empty() || destEncoding.empty() || srcEncoding == destEncoding)
        return false;

    using iconv_fn = size_t (*)(iconv_t, const char**, size_t*, char**, size_t*);
    const iconv_fn do_iconv = (iconv_fn)iconv;

    bool ok = true;
    const char* inBuf = srcBuf;
    size_t inLeft = srcLen;

    char* outBuf;
    size_t outLeft;
    if (srcLen > workBuf.size()) {
        workBuf.resize(srcLen * 3 + 4);
    }
    outBuf = workBuf.data();
    outLeft = workBuf.size();

    do {
        iconv_t cd = iconv_open(destEncoding.c_str(), srcEncoding.c_str());
        if (cd == (iconv_t)-1) {
            ok = false;
            break;
        }

        errno = 0;
        size_t nbytes = do_iconv(cd, &inBuf, &inLeft, &outBuf, &outLeft);
        if (nbytes != (size_t)-1) {
            break;
        } else if (errno != E2BIG) {
            ok = false;
            break;
        }
        inBuf = srcBuf;
        inLeft = srcLen;

        workBuf.resize(workBuf.size() * 2);
        outBuf = workBuf.data();
        outLeft = workBuf.size();

        iconv_close(cd);
    } while (true);

    if (ok) {
        dest.assign(workBuf.data(), workBuf.size() - outLeft);
    }

    return ok;
}

inline bool
ConvFromTo(const std::string& srcEncoding, const std::string& destEncoding, const char* srcBuf, size_t srcLen, std::string& dest)
{
    std::vector<char> workBuf;
    return ConvFromTo(srcEncoding, destEncoding, srcBuf, srcLen, dest, workBuf);
}
}
}
