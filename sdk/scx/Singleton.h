#pragma once

#include <memory>
#include <mutex>

namespace scx {

template<typename T>
class Singleton
{
  public:
    static std::shared_ptr<T> Instance()
    {
        std::call_once(_flag, [] { _instance = std::make_shared<T>(); });
        return _instance;
    }

    static void Cleanup() { _instance.reset(); }

  private:
    static std::once_flag _flag;
    static std::shared_ptr<T> _instance;
};

template<typename T>
std::once_flag Singleton<T>::_flag;

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
}
