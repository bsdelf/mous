#ifndef SCX_COND_VAR_HPP
#define SCX_COND_VAR_HPP

#include <pthread.h>

namespace scx {

class Mutex;

class CondVar
{
public:
    explicit CondVar(Mutex& refMutex):
	m_refMutex(refMutex)
    {
	pthread_cond_init(&m_Cond, NULL);
    }

    ~CondVar()
    {
	pthread_cond_destroy(&m_Cond);
    }

    int Wait()
    {
	return pthread_cond_wait(&m_Cond, &(m_refMutex.m_Mutex));
    }

    int Signal()
    {
	return pthread_cond_signal(&m_Cond);
    }

    int SignalAll()
    {
	return pthread_cond_broadcast(&m_Cond);
    }

    void TimeWait()
    {
	//pthread_cond_timewait(&m_Cond, &(m_refMutex.m_Mutex));
    }

private:
    pthread_cond_t m_Cond;
    Mutex& m_refMutex;
};

}
#endif
