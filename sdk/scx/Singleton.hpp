#ifndef SCX_SINGLETON_HPP
#define SCX_SINGLETON_HPP

#include <pthread.h>

namespace scx {

template<typename T>
class Singleton {
public:
    static T& Instance() {
	pthread_once(&control, &Singleton::Init);
	return *ptrInstance;
    }

private:
    static void Init() {
	ptrInstance = new T;
    }

private:
    static pthread_once_t control;
    static T* ptrInstance;
};

template<typename T>
pthread_once_t Singleton<T>::control = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::ptrInstance = NULL;

}
#endif
