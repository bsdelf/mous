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
        pthread_mutex_init(&mutex, NULL);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
    }

    int Lock()
    {
        return pthread_mutex_lock(&mutex);
    }

    int Unlock()
    {
        return pthread_mutex_unlock(&mutex);
    }

    int TryLock()
    {
        return pthread_mutex_trylock(&mutex);
    }

private:
    pthread_mutex_t mutex;
};

class MutexLocker
{
public:
    explicit MutexLocker(Mutex* mutex):
        m_Mutex(mutex)
    {
        m_Mutex->Lock();
    }

    ~MutexLocker()
    {
        Unlock();
    }

    void Unlock()
    {
        m_Mutex->Unlock();
    }

    void Relock()
    {
        m_Mutex->Lock();
    }

private:
    Mutex* m_Mutex;
};

}

#endif
