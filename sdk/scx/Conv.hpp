#pragma once

#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>

namespace scx {

template<typename num_t>
static inline num_t StrToNum(const std::string& str, std::ios_base&(*base)(std::ios_base&) = std::dec) {
    std::stringstream stream;
    stream << str;
    num_t num;
    stream >> base >> num;
    return num;
}

template<typename num_t>
static inline std::string NumToStr(const num_t& num, std::streamsize precision = 0, std::ios_base&(*base)(std::ios_base&) = std::dec) {
    std::stringstream stream;
    stream.setf(std::ios::fixed, std::ios::floatfield);
    stream.precision(precision);
    stream << base << num;
    return stream.str();
}

static inline std::string ToLower(const std::string& str) {
    std::string lower;
    lower.resize(str.size());
    std::transform(str.begin(), str.end(), lower.begin(), (int (*)(int))tolower);
    return lower;
}

static inline std::string ToUpper(const std::string& str) {
    std::string upper;
    upper.resize(str.size());
    std::transform(str.begin(), str.end(), upper.begin(), (int (*)(int))toupper);
    return upper;
}

/*
static inline bool IsLower(const string& str)
{
    return true;
}

static inline bool IsUpper(const string& str)
{
    return true;
}
*/

};

