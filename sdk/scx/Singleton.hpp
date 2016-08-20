#pragma once

#include <pthread.h>
#include <memory>

namespace scx {

    template<typename T>
    class Singleton {
    public:
        static std::shared_ptr<T> Instance() {
            pthread_once(&_ctrl, &Singleton::Init);
            return _instance;
        }

        static void Cleanup() {
            _instance.reset();
        }

    private:
        static void Init() {
            _instance = std::make_shared<T>();
        }

    private:
        static pthread_once_t _ctrl;
        static std::shared_ptr<T> _instance;
    };

    template<typename T>
    pthread_once_t Singleton<T>::_ctrl = PTHREAD_ONCE_INIT;

    template<typename T>
    std::shared_ptr<T> Singleton<T>::_instance = nullptr;

}
