#ifndef MOUS_ICHARSETCONV_H
#define MOUS_ICHARSETCONV_H

#include <string>
#include "CharsetConvInf.h"

namespace mous {

class ICharsetConv
{
public:
    virtual ~ICharsetConv() { }

    virtual std::string Probe(const char* buf, size_t len) = 0;

    /* Auto detect the given buf, convert to utf8. */
    virtual bool AutoConv(const char* buf, size_t len, std::string& content) = 0;

    /* Auto detect the given buf, convert to the encoding you want. */
    virtual bool AutoConvTo(const std::string& wanted, 
            const char* buf, size_t len, std::string& content) = 0;

    /* If converted return true, otherwise return false(failed/unnecessary). */
    virtual bool ConvFromTo(const std::string& from, const std::string& wanted, 
            const char* buf, size_t len, std::string& content) = 0;
};

}

#endif
