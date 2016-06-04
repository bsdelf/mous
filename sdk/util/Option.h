#pragma once

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
    Boolean,

    EnumedInt,
    EnumedFloat,
    EnumedString,

    RangedInt,
    RangedFloat,

    Grouped
};
}
typedef OptionType::e EmOptionType;

inline const char* ToString(EmOptionType type)
{
    switch (type) {
        case OptionType::None:
            return "None";

        case OptionType::Int:
            return "Int";

        case OptionType::Float:
            return "Float";

        case OptionType::String:
            return "String";

        case OptionType::Boolean:
            return "Boolean";

        case OptionType::EnumedInt:
            return "EnumedInt";

        case OptionType::EnumedFloat:
            return "EnumedFloat";

        case OptionType::EnumedString:
            return "EnumedString";

        case OptionType::RangedInt:
            return "RangedInt";

        case OptionType::RangedFloat:
            return "RangedFloat";

        case OptionType::Grouped:
            return "Grouped";
    }
    return "";
}

struct BaseOption
{
    const EmOptionType type;
    std::string desc;

    explicit BaseOption(EmOptionType _type):
        type(_type)
    {
    }
};

struct IntOption: public BaseOption
{
    int32_t defaultVal;
    mutable int32_t userVal; 

    IntOption(): BaseOption(OptionType::Int) {}
};

struct FloatOption: public BaseOption
{
    int32_t defaultVal;
    mutable double userVal; 

    int point;  // double = defaultVal/point eg. 123/10=12.3

    FloatOption(): BaseOption(OptionType::Float) {}
};

struct StringOption: public BaseOption
{
    std::string defaultVal;
    mutable std::string userVal; 

    StringOption(): BaseOption(OptionType::String) {}
};

struct BooleanOption: public BaseOption
{
    std::string detail;
    bool defaultChoice;
    mutable bool userChoice;

    BooleanOption(): BaseOption(OptionType::Boolean) {}
};

struct EnumedIntOption: public BaseOption
{
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 

    EnumedIntOption(): BaseOption(OptionType::EnumedInt) {}
};

struct EnumedFloatOption: public BaseOption
{
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 

    int point;  // double = defaultVal/point eg. 123/10=12.3

    EnumedFloatOption(): BaseOption(OptionType::EnumedFloat) {}
};

struct EnumedStringOption: public BaseOption
{
    std::vector<std::string> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 

    EnumedStringOption(): BaseOption(OptionType::EnumedString) {}
};

struct RangedIntOption: public BaseOption
{
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 

    RangedIntOption(): BaseOption(OptionType::RangedInt) {}
};

struct RangedFloatOption: public BaseOption
{
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 

    int point;  // double = defaultVal/point eg. 123/10=12.3

    RangedFloatOption(): BaseOption(OptionType::RangedFloat) {}
};

struct GroupedOption: public BaseOption
{
    std::vector<std::pair<std::string, std::vector<BaseOption*> > > groups;
    int defaultUse;
    mutable int userUse;

    GroupedOption(): BaseOption(OptionType::Grouped) { }
};

}
