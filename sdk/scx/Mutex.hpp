#ifndef SCX_MUTEX_HPP
#define SCX_MUTEX_HPP

#include <pthread.h>

namespace scx {

class CondVar;

class Mutex
{
friend class CondVar;

public:
    Mutex()
    {
	pthread_mutex_init(&m_Mutex, NULL);
    }

    ~Mutex()
    {
	pthread_mutex_destroy(&m_Mutex);
    }

    int Lock()
    {
	return pthread_mutex_lock(&m_Mutex);
    }

    int Unlock()
    {
	return pthread_mutex_unlock(&m_Mutex);
    }

    int TryLock()
    {
	return pthread_mutex_trylock(&m_Mutex);
    }

private:
    pthread_mutex_t m_Mutex;
};

}

#endif
