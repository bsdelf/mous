#ifndef SCX_FAKESEMVAR_HPP
#define SCX_FAKESEMVAR_HPP

#include "Mutex.hpp"
#include "CondVar.hpp"

namespace scx {

class FakeSemVar
{
public:
    explicit FakeSemVar(int value = 0):
        m_Value(value)
    {
    }

    ~FakeSemVar()
    {
    }

    void Post(int n = 1)
    {
        MutexLocker mlocker(&m_Mutex);
        m_Value += n;
        m_CondVar.Broadcast();
    }

    bool TryWait(int n = 1)
    {
        MutexLocker mlocker(&m_Mutex);
        if (n > m_Value)
            return false;
        m_Value -= n;
        return true;
    }

    void Wait(int n = 1)
    {
        MutexLocker mlocker(&m_Mutex);
        while (n > m_Value)
            m_CondVar.Wait(&m_Mutex);
        m_Value -= n;
    }

    int GetValue() const
    {
        MutexLocker mlocker(const_cast<Mutex*>(&m_Mutex));
        return m_Value;
    }

    void Clear()
    {
        MutexLocker mlocker(&m_Mutex);
        m_Value = 0;
    }

private:
    int m_Value;
    Mutex m_Mutex;
    CondVar m_CondVar;
};

}

#endif
