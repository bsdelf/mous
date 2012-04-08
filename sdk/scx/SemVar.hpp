#ifndef SCX_SEM_VAR_HPP
#define SCX_SEM_VAR_HPP

#include <semaphore.h>

namespace scx {

class SemVar
{
public:
    explicit SemVar(int value = 0)
    {
        sem_init(&m_Sem, 0, value);
    }

    SemVar(int pshared, int value)
    {
        sem_init(&m_Sem, pshared, value);
    }

    ~SemVar()
    {
        sem_destroy(&m_Sem);
    }

    int Post()
    {
        return sem_post(&m_Sem);
    }

    int TryWait()
    {
        return sem_trywait(&m_Sem);
    }

    int Wait()
    {
        return sem_wait(&m_Sem);
    }

    void TimeWait()
    {
        //return sem_timewait(&m_Sem, timeout);
    }

    int GetValue() const
    {
        int sval = 0;
        sem_getvalue(&m_Sem, &sval);
        return sval;
    }

    void Clear() const
    {
        while (sem_trywait(&m_Sem) == 0);
    }

private:
    mutable sem_t m_Sem;
};

}

#endif
