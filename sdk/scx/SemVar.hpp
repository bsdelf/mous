#ifndef SCX_SEMVAR_HPP
#define SCX_SEMVAR_HPP

#ifdef USE_POSIX_SEMAPHORE // POSIX unnamed semaphore
#include <semaphore.h> 
#else
#include <mutex>
#include <condition_variable>
#endif

namespace scx {

class SemVar
{
public:
    SemVar(const SemVar&) = delete;
    SemVar& operator=(const SemVar&) = delete;

    explicit SemVar(int value = 0)
    {
#ifdef USE_POSIX_SEMAPHORE
        ::sem_init(&m_sem, 0, value);
#else
        m_value = value;
#endif
    }

#ifdef USE_POSIX_SEMAPHORE
    SemVar(int pshared, int value)
    {
        ::sem_init(&m_sem, pshared, value);
    }
#endif

    ~SemVar()
    {
#ifdef USE_POSIX_SEMAPHORE
        ::sem_destroy(&m_sem);
#endif
    }

    void Post(int n = 1)
    {
#ifdef USE_POSIX_SEMAPHORE
        while (n-- > 0)
            ::sem_post(&m_sem);
#else
        std::lock_guard<std::mutex> locker(m_vmtx);
        m_value += n;
        m_vcond.notify_all();
#endif
    }

    bool TryWait(int n = 1)
    {
#ifdef USE_POSIX_SEMAPHORE
        while (n > 0 && ::sem_trywait(&m_sem) == 0) {
            --n;
        }
        return (n == 0);
#else
        std::lock_guard<std::mutex> locker(m_vmtx);
        if (n > m_value)
            return false;
        m_value -= n;
        return true;
#endif
    }

    void Wait(int n = 1)
    {
#ifdef USE_POSIX_SEMAPHORE
        while (n-- > 0) {
            ::sem_wait(&m_sem);
        }
#else
        std::unique_lock<std::mutex> locker(m_vmtx);
        while (n > m_value)
            m_vcond.wait(locker);
        m_value -= n;
#endif
    }

    /*
    void TimeWait()
    {
        //return sem_timewait(&m_sem, timeout);
    }
    */

    int Value() const
    {
#ifdef USE_POSIX_SEMAPHORE
        int sval = 0;
        ::sem_getvalue(&m_sem, &sval);
        return sval;
#else
        std::lock_guard<std::mutex> locker(*const_cast<std::mutex*>(&m_vmtx));
        return m_value;
#endif
    }

    void Clear()
    {
#ifdef USE_POSIX_SEMAPHORE
        while (::sem_trywait(&m_sem) == 0);
#else
        std::lock_guard<std::mutex> locker(m_vmtx);
        m_value = 0;
#endif
    }

private:
#ifdef USE_POSIX_SEMAPHORE
    mutable sem_t m_sem;
#else
    int m_value = 0;
    std::mutex m_vmtx;
    std::condition_variable m_vcond;
#endif
};

}

#endif
