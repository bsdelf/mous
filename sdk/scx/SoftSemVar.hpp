#ifndef SCX_SOFTSEMVAR_HPP
#define SCX_SOFTSEMVAR_HPP

#include <mutex>
#include <condition_variable>

namespace scx {

class SoftSemVar
{
public:
    explicit SoftSemVar(int value = 0):
        m_Value(value)
    {
    }

    ~SoftSemVar()
    {
    }

    void Post(int n = 1)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_Value += n;
        m_CondVar.notify_all();
    }

    bool TryWait(int n = 1)
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        if (n > m_Value)
            return false;
        m_Value -= n;
        return true;
    }

    void Wait(int n = 1)
    {
        std::unique_lock<std::mutex> locker(m_Mutex);
        while (n > m_Value)
            m_CondVar.wait(locker);
        m_Value -= n;
    }

    int Value() const
    {
        std::lock_guard<std::mutex> locker(*const_cast<std::mutex*>(&m_Mutex));
        return m_Value;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> locker(m_Mutex);
        m_Value = 0;
    }

private:
    int m_Value;
    std::mutex m_Mutex;
    std::condition_variable m_CondVar;
};

}

#endif
