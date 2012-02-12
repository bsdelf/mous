#ifndef SCX_CONV_HPP
#define SCX_CONV_HPP

#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
using namespace std;

namespace scx {

template<typename num_t>
static inline num_t StrToNum(const string& str) 
{
    stringstream stream;
    stream << str;
    num_t num;
    stream >> dec >> num;
    return num;
}

template<typename num_t>
static inline string NumToStr(const num_t& num, streamsize precision = 0)
{
    stringstream stream;
    stream.setf(ios::fixed, ios::floatfield);
    stream.precision(precision);
    stream << dec << num;
    return stream.str();
}

template<typename num_t>
static inline string NumHToStr(const num_t& num, streamsize precision = 0)
{
    stringstream stream;
    stream.setf(ios::fixed, ios::floatfield);
    stream.precision(precision);
    stream << hex << num;
    return stream.str();
}

template<typename num_t>
static inline string NumOToStr(const num_t& num, streamsize precision = 0)
{
    stringstream stream;
    stream.setf(ios::fixed, ios::floatfield);
    stream.precision(precision);
    stream << oct << num;
    return stream.str();
}

static inline string ToLower(const string& str)
{
    string lower;
    lower.resize(str.size());
    transform(str.begin(), str.end(), lower.begin(), (int (*)(int))tolower);
    return lower;
}

static inline string ToUpper(const string& str)
{
    string upper;
    upper.resize(str.size());
    transform(str.begin(), str.end(), upper.begin(), (int (*)(int))toupper);
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

#endif
