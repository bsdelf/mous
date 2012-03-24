#ifndef MOUS_ARGVDEF_H
#define MOUS_ARGVDEF_H

#include <inttypes.h>
#include <string>
#include <vector>

namespace mous {

namespace ArgvType {
enum e {
    None = 0,

    Int,            // int32_t
    Float,          // double
    String,

    EnumedInt,
    EnumedFloat,
    EnumedString,

    RangedInt,
    RangedFloat
};
}
typedef ArgvType::e EmArgvType;

struct ArgvBase
{
    EmArgvType type;
    std::string desc;
};

struct ArgvInt: public ArgvBase
{
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct ArgvFloat: public ArgvBase
{
    double defaultVal;
    mutable double userVal; 
};

struct ArgvString: public ArgvBase
{
    std::string defaultVal;
    mutable std::string userVal; 
};

struct ArgvEnumedInt: public ArgvBase
{
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvEnumedFloat: public ArgvBase
{
    std::vector<double> enumedtVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvEnumedString: public ArgvBase
{
    std::vector<std::string> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvRangedInt: public ArgvBase
{
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct ArgvRangedFloat: public ArgvBase
{
    double min;
    double max;
    double defaultVal;
    mutable double userVal; 
};

#undef MOUS_COMMON_ARGVS
}

#endif

