#ifndef SCX_CONDVAR_HPP
#define SCX_CONDVAR_HPP

#include <pthread.h>

namespace scx {

class Mutex;

class CondVar
{
public:
    CondVar()
    {
        pthread_cond_init(&cond, NULL);
    }

    ~CondVar()
    {
        pthread_cond_destroy(&cond);
    }

    int Wait(Mutex* mutex)
    {
        return pthread_cond_wait(&cond, &(mutex->mutex));
    }

    int Signal()
    {
        return pthread_cond_signal(&cond);
    }

    int Broadcast()
    {
        return pthread_cond_broadcast(&cond);
    }

    /*
    void TimeWait()
    {
        //pthread_cond_timewait(&cond, &(m_Mutex.m_Mutex));
    }
    */

private:
    pthread_cond_t cond;
};

}
#endif
