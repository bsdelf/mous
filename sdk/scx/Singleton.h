#pragma once

#include <memory>
#include <mutex>

namespace scx {

template <typename T>
class Singleton {
 public:
  static std::shared_ptr<T> Instance() {
    std::call_once(flag_, [] { instance_ = std::make_shared<T>(); });
    return instance_;
  }

  static void Cleanup() {
    instance_.reset();
  }

 private:
  static std::once_flag flag_;
  static std::shared_ptr<T> instance_;
};

template <typename T>
std::once_flag Singleton<T>::flag_;

template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ = nullptr;
}  // namespace scx
