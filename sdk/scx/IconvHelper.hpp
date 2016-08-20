#pragma once

#include <errno.h>
#include <iconv.h>
#include <string>
#include <vector>

namespace scx {

namespace IconvHelper {

static inline bool ConvFromTo(const std::string& from, const std::string& wanted, 
        const char* srcBuf, size_t srcLen, std::string& dest, std::vector<char>& workBuf)
{
    typedef size_t (*StdIconv)(iconv_t, const char**, size_t*, char**, size_t*);
    StdIconv std_iconv = (StdIconv)iconv;

    if (from.empty() || wanted.empty())
        return false;
    if (from == wanted)
        return false;

    bool ok = true;
    const char* inBuf = srcBuf;
    size_t inLeft = srcLen;

    char* outBuf;
    size_t outLeft;
    if (srcLen > workBuf.size()) {
        workBuf.resize(srcLen*3+4);
    }
    outBuf = &workBuf[0];
    outLeft = workBuf.size();

    size_t converted = 0;
    do {
        iconv_t cd = iconv_open(wanted.c_str(), from.c_str());
        if (cd == (iconv_t)-1) {
            ok = false;
            break;
        }

        errno = 0;
        converted = std_iconv(cd, &inBuf, &inLeft, &outBuf, &outLeft);
        if (converted != (size_t)-1) {
            break;
        } else if (errno != E2BIG) {
            ok = false;
            break;
        }
        inBuf = srcBuf;
        inLeft = srcLen;

        workBuf.resize(workBuf.size() * 2);
        outBuf = &workBuf[0];
        outLeft = workBuf.size();

        iconv_close(cd);
    } while(true);

    if (ok) {
        dest.assign(&workBuf[0], workBuf.size() - outLeft);
    }

    return ok;
}

static inline bool ConvFromTo(const std::string& from, const std::string& wanted, 
        const char* srcBuf, size_t srcLen, std::string& dest)
{
    std::vector<char> workBuf;
    return ConvFromTo(from, wanted, srcBuf, srcLen, dest, workBuf);
}


}
}

