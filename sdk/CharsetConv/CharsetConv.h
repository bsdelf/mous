#ifndef CHARSETCONV_H
#define CHARSETCONV_H

#include <string>

namespace mous {

class nsDetectorWrapper;

class CharsetConv
{
public:
    CharsetConv(int buflen = 1024);
    ~CharsetConv();

    /* Auto detect the given buf, convert to utf8 */
    std::string AutoConv(const char* buf, size_t len);

    /* Auto detect the given buf, convert to the encoding you want */
    std::string AutoConvTo(const char* wanted, const char* buf, size_t len);

private:
    nsDetectorWrapper* mDetector;
    char* const mBuffer;
    size_t mBufferLen;
};

}

#endif
