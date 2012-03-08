#include "CharsetConv.h"
#include "uchardect/src/nscore.h"
#include "uchardect/src/nsUniversalDetector.h"
#include <iostream>
#include <iconv.h>
using namespace std;
using namespace mous;

namespace mous {

class nsDetectorWrapper: public nsUniversalDetector
{
public:
    nsDetectorWrapper():
        nsUniversalDetector(NS_FILTER_ALL)
    {
    }

    virtual ~nsDetectorWrapper()
    {
    }

    const char* GetCharset() const
    {
        return mCharset.c_str();
    }

    virtual void Reset()
    {
        nsUniversalDetector::Reset();
        mCharset.clear();
    }

private:
    virtual void Report(const char* charset)
    {
        mCharset.assign(charset);
    }

private:
    string mCharset;
};

}

CharsetConv::CharsetConv(int buflen):
    mDetector(new nsDetectorWrapper),
    mBuffer(new char[buflen]),
    mBufferLen(buflen)
{
}

CharsetConv::~CharsetConv()
{
    if (mDetector != NULL)
        delete mDetector;
    if (mBuffer != NULL)
        delete[] mBuffer;
}

string CharsetConv::AutoConv(const char* buf, size_t len)
{
    return AutoConvTo("UTF-8", buf, len);
}

string CharsetConv::AutoConvTo(const char* wanted, const char* buf, size_t len)
{
    string outcontent;
    int ret = mDetector->HandleData(buf, (PRUint32)len);
    if (ret == NS_OK) {
        mDetector->DataEnd();
        const char* set = mDetector->GetCharset();
        cout << "set:" << set << endl;
        if (*set != '\0') {
            const char* inbuf = buf;
            size_t inleft = len;

            char* outstart;
            size_t outlen;
            char* outbuf;
            size_t outleft;
            if (len <= mBufferLen) {
                outstart = mBuffer;
                outlen = mBufferLen;
            } else {
                outstart = new char[len+4];
                outlen = len;
            }
            outbuf = outstart;
            outleft = outlen;

            size_t converted = 0;
            bool failed = false;
            do {
                iconv_t cd = iconv_open(wanted, set);
                if (cd == (iconv_t)-1) {
                    failed = true;
                    break;
                }

                errno = 0;
                converted = iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
                if (converted != (size_t)-1) {
                    cout << "done" << endl;
                    break;
                } else if (errno != E2BIG) {
                    cout << strerror(errno) << endl;
                    failed = true;
                    break;
                }
                inbuf = buf;
                inleft = len;
                delete[] outstart;
                outlen = (outlen << 1);
                outstart = new char[outlen];
                outbuf = outstart;
                outleft = outlen;

                iconv_close(cd);
            } while(true);

            if (!failed) {
                outcontent.assign(outstart, outlen-outleft);
            }
            if (outstart != mBuffer) {
                delete[] outstart;
            }

        } else {
        }
        mDetector->Reset();
    }
    return outcontent;
}
