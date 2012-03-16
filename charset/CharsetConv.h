#ifndef MOUS_CHARSETCONV_H
#define MOUS_CHARSETCONV_H

#include <string>
#include <charset/ICharsetConv.h>

namespace mous {

class nsDetectorWrapper;

class CharsetConv: public ICharsetConv
{
public:
    CharsetConv(int buflen = 1024);
    ~CharsetConv();

public:
    std::string Probe(const char* buf, size_t len);

    /* Auto detect the given buf, convert to utf8. */
    bool AutoConv(const char* buf, size_t len, std::string& content);

    /* Auto detect the given buf, convert to the encoding you want. */
    bool AutoConvTo(const std::string& wanted, 
            const char* buf, size_t len, std::string& content);

    /* If converted return true, otherwise return false(failed/unnecessary). */
    bool ConvFromTo(const std::string& from, const std::string& wanted,
            const char* buf, size_t len, std::string& content);

private:
    nsDetectorWrapper* mDetector;
    char* const mBuffer;
    size_t mBufferLen;
};

}

#endif
