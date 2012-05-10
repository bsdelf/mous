#ifndef SCX_SEMVAR_HPP
#define SCX_SEMVAR_HPP

#include <semaphore.h>

namespace scx {

/* POSIX unnamed semphore */
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

    void Post()
    {
        sem_post(&m_Sem);
    }

    bool TryWait()
    {
        return sem_trywait(&m_Sem) == 0;
    }

    void Wait()
    {
        sem_wait(&m_Sem);
    }

    /*
    void TimeWait()
    {
        //return sem_timewait(&m_Sem, timeout);
    }
    */

    int Value() const
    {
        int sval = 0;
        sem_getvalue(&m_Sem, &sval);
        return sval;
    }

    void Clear()
    {
        while (sem_trywait(&m_Sem) == 0);
    }

private:
    mutable sem_t m_Sem;
};

}

#endif
