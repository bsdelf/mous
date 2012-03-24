#ifndef MOUS_OPTION_H
#define MOUS_OPTION_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <utility>

namespace mous {

namespace OptionType {
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
typedef OptionType::e EmOptionType;

struct OptionInt
{
    std::string desc;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct OptionFloat
{
    std::string desc;
    double defaultVal;
    mutable double userVal; 
};

struct OptionString
{
    std::string desc;
    std::string defaultVal;
    mutable std::string userVal; 
};

struct OptionEnumedInt
{
    std::string desc;
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct OptionEnumedFloat
{
    std::string desc;
    std::vector<double> enumedtVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct OptionEnumedString
{
    std::string desc;
    std::vector<std::string> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct OptionRangedInt
{
    std::string desc;
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct OptionRangedFloat
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
    typedef std::pair<void*, EmOptionType> OptionPair;
    typedef std::pair<const void*, EmOptionType> ConstOptionPair;

public:
    virtual ~OptionProvider() { }

    // use my facility, reimplement these
    virtual bool GetOptions(std::vector<ConstOptionPair>& list) const { return false; }
    virtual bool PickOptions() const { return false; }
    virtual bool PickOption(size_t index) const { return false; }

    // use getopt(), reimplement these
    virtual const char* GetUsage() const { return NULL; }
    virtual bool SetOptions(int argc, const char* argv[]) const { return false; }
};

}

#endif

