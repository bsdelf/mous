#ifndef MOUS_ARGVDEF_H
#define MOUS_ARGVDEF_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <utility>

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

typedef std::pair<void*, EmArgvType> ArgvPair;
typedef std::pair<const void*, EmArgvType> ArgvConstPair;

struct ArgvInt
{
    std::string desc;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct ArgvFloat
{
    std::string desc;
    double defaultVal;
    mutable double userVal; 
};

struct ArgvString
{
    std::string desc;
    std::string defaultVal;
    mutable std::string userVal; 
};

struct ArgvEnumedInt
{
    std::string desc;
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvEnumedFloat
{
    std::string desc;
    std::vector<double> enumedtVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvEnumedString
{
    std::string desc;
    std::vector<std::string> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct ArgvRangedInt
{
    std::string desc;
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct ArgvRangedFloat
{
    std::string desc;
    double min;
    double max;
    double defaultVal;
    mutable double userVal; 
};

class OptionProvider
{
public:
    virtual ~OptionProvider() { }

    // use my facility, implement these
    virtual bool GetOptions(std::vector<ArgvConstPair>& list) const { return false; }
    virtual bool PickOptions() { return false; }
    virtual bool PickOption(size_t index) { return false; }

    // if you want to use getopt(), implement these
    virtual const char* GetUsage() const { return NULL; }
    virtual bool SetOptions(const char* opts) { return false; }
};

}

#endif

