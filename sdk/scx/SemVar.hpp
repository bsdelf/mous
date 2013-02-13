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
        ::sem_init(&m_sem, 0, value);
    }

    SemVar(int pshared, int value)
    {
        ::sem_init(&m_sem, pshared, value);
    }

    ~SemVar()
    {
        ::sem_destroy(&m_sem);
    }

    void Post()
    {
        ::sem_post(&m_sem);
    }

    bool TryWait()
    {
        return ::sem_trywait(&m_sem) == 0;
    }

    void Wait()
    {
        ::sem_wait(&m_sem);
    }

    /*
    void TimeWait()
    {
        //return sem_timewait(&m_sem, timeout);
    }
    */

    int Value() const
    {
        int sval = 0;
        ::sem_getvalue(&m_sem, &sval);
        return sval;
    }

    void Clear()
    {
        while (::sem_trywait(&m_sem) == 0);
    }

private:
    mutable sem_t m_sem;
};

}

#endif
