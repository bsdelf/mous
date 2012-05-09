#ifndef SCX_ENV_HPP
#define SCX_ENV_HPP

#include <stdlib.h>

#include <string>

namespace scx {

namespace Env {

const char* const Home = "HOME";
const char* const Path = "PATH";
const char* const Lang = "LANG";
const char* const PWD = "PWD";
const char* const LogName = "LOGNAME";
const char* const Term = "TERM";
const char* const Shell = "SHELL";
const char* const TmpDir = "TMPDIR";

static inline std::string GetEnv(const std::string& name)
{
    char* p = getenv(name.c_str());
    return p != NULL ? std::string(p) : "";
}

static inline bool SetEnv(const std::string& name, std::string& value, bool overwrite = true)
{
    return setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0) == 0;
}

static inline bool PutEnv(const std::string& namevalue)
{
    size_t len = namevalue.size();
    char* buf = new char[len+1];
    buf[len] = '\0';
    memcpy(buf, namevalue.data(), len);
    return putenv(buf) == 0;
}

};

}

#endif
