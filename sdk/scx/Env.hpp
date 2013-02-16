#ifndef SCX_ENV_HPP
#define SCX_ENV_HPP

#include <stdlib.h>
#include <string.h>

#include <string>

namespace scx {

struct Env
{
    constexpr static const char* const Home = "HOME";
    constexpr static const char* const Path = "PATH";
    constexpr static const char* const Lang = "LANG";
    constexpr static const char* const PWD = "PWD";
    constexpr static const char* const LogName = "LOGNAME";
    constexpr static const char* const Term = "TERM";
    constexpr static const char* const Shell = "SHELL";
    constexpr static const char* const TmpDir = "TMPDIR";

    static std::string Get(const std::string& name)
    {
        const char* p = ::getenv(name.c_str());
        return p != nullptr ? std::string(p) : "";
    }

    static bool Set(const std::string& name, std::string& value, bool overwrite = true)
    {
        return ::setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) == 0;
    }

    static bool Put(const std::string& namevalue)
    {
        size_t len = namevalue.size();
        char* buf = new char[len+1];
        buf[len] = '\0';
        ::memcpy(buf, namevalue.data(), len);
        return ::putenv(buf) == 0;
    }

    static bool Unset(const std::string& name)
    {
        return ::unsetenv(name.c_str()) == 0;
    }
};

}

#endif
