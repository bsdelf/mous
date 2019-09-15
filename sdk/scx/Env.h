#pragma once

#include <stdlib.h>
#include <string.h>

#include <string>

namespace scx {

struct Env {
  static bool Has(const std::string& name) {
    return ::getenv(name.c_str()) != nullptr;
  }

  static std::string Get(const std::string& name) {
    const char* p = ::getenv(name.c_str());
    return (p != nullptr) ? p : "";
  }

  static bool Set(const std::string& name, const std::string& value, bool overwrite = true) {
    return ::setenv(name.c_str(), value.c_str(), overwrite) == 0;
  }

  static bool Unset(const std::string& name) {
    return ::unsetenv(name.c_str()) == 0;
  }
};
}  // namespace scx
