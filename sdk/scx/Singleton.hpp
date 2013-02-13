#ifndef SCX_SINGLETON_HPP
#define SCX_SINGLETON_HPP

#include <pthread.h>

namespace scx {

template<typename T>
class Singleton
{
public:
    static T* Instance()
    {
        pthread_once(&control, &Singleton::Init);
        return sInstance;
    }

    static void Cleanup()
    {
        if (sInstance != nullptr) {
            delete sInstance;
            sInstance = nullptr;
        }
    }

private:
    static void Init()
    {
        sInstance = new T;
    }

private:
    static pthread_once_t control;
    static T* sInstance;
};

template<typename T>
pthread_once_t Singleton<T>::control = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::sInstance = nullptr;

}
#endif
