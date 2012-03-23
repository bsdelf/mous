#include "CharsetConv.h"
#include "uchardect/src/nscore.h"
#include "uchardect/src/nsUniversalDetector.h"
#include <cstring>
#include <iconv.h>
#include <iostream>
using namespace std;
using namespace mous;

namespace mous {

class nsDetectorWrapper: public nsUniversalDetector
{
public:
    nsDetectorWrapper():
        //nsUniversalDetector(NS_FILTER_ALL)
        //nsUniversalDetector(NS_FILTER_CJK)
        nsUniversalDetector(NS_FILTER_CHINESE_SIMPLIFIED)
    {
    }

    virtual ~nsDetectorWrapper()
    {
    }

    string GetCharset() const
    {
        return mCharset;
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

string CharsetConv::Probe(const char* buf, size_t len)
{
    /*
    //#include <unicode/ucsdet.h>
    //-licuio
    int max = -1;
    int maxi = -1;

    UErrorCode uerr = U_ZERO_ERROR;
    int32_t found = 1;
    UCharsetDetector* udec = ucsdet_open(&uerr);
    ucsdet_setText(udec, buf, len, &uerr);
    const UCharsetMatch** match = ucsdet_detectAll(udec, &found, &uerr);
    for (int i = 0; i < found; ++i) {
        int conf = ucsdet_getConfidence(match[i], &uerr);
        if (conf > max) {
            max = conf;
            maxi = i;
        }
        cout << ucsdet_getName(match[i], &uerr) << '\t';
        cout << conf << endl;
    }
    cout << found << endl;
    ucsdet_close(udec);
    
    if (maxi != -1)
        return ucsdet_getName(match[maxi], &uerr);
    else
        return "";
    */
 
    string charset;
    int ret = mDetector->HandleData(buf, (PRUint32)len);
    mDetector->DataEnd();
    if (ret == NS_OK) {
        charset.assign(mDetector->GetCharset());
    }
    mDetector->Reset();
    return charset;
}

bool CharsetConv::AutoConv(const char* buf, size_t len, string& content)
{
    return AutoConvTo("UTF-8", buf, len, content);
}

bool CharsetConv::AutoConvTo(const string& wanted, const char* buf, size_t len, string& content)
{
    string charset(Probe(buf, len));
    if (!charset.empty()) {
        return ConvFromTo(charset, wanted, buf, len, content);
    } else {
        return false;
    }
}

bool CharsetConv::ConvFromTo(const string& from, const string& wanted, const char* buf, size_t len, string& content)
{
    if (from.empty() || wanted.empty())
        return false;
    if (from == wanted)
        return false;

    cout << "set:" << from << endl;
    bool ok = true;
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
        outlen = len+4;
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
        converted = iconv(cd, &inbuf, &inleft, &outbuf, &outleft);
        if (converted != (size_t)-1) {
            cout << "done" << endl;
            break;
        } else if (errno != E2BIG) {
            cout << strerror(errno) << endl;
            ok = false;
            break;
        }
        inbuf = buf;
        inleft = len;
        if (outstart != mBuffer) {
            delete[] outstart;
        }
        outlen = (outlen << 1);
        outstart = new char[outlen];
        outbuf = outstart;
        outleft = outlen;

        iconv_close(cd);
    } while(true);

    if (ok) {
        content.assign(outstart, outlen-outleft);
    }
    if (outstart != mBuffer) {
        delete[] outstart;
    }
    return ok;
}
