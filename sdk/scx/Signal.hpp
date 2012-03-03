#ifndef SCX_SIGNAL_HPP
#define SCX_SIGNAL_HPP

#include <vector>
#include "Function.hpp"

namespace scx {

/* Signal class, not support copy now!! */
template<typename signature>
class Signal;

#define SCX_SIGNAL_COPY_SIGNAL_COMMON                   \
public:                                                 \
    ~Signal()                                           \
    {                                                   \
        for (size_t i = 0; i < m_Slots.size(); ++i) {   \
            delete m_Slots[i];                          \
        }                                               \
    }                                                   \
    \
    template<typename fn_t, typename pv_t>          \
    const Function_t* Connect(fn_t fn, pv_t pv)     \
    {                                               \
        Function_t* pslot = new Function_t(fn, pv); \
        m_Slots.push_back(pslot);                   \
        return pslot;                               \
    }                                               \
    \
    template<typename fn_t>                         \
    const Function_t* Connect(fn_t fn)              \
    {                                               \
        Function_t* pslot = new Function_t(fn);     \
        m_Slots.push_back(pslot);                   \
        return pslot;                               \
    }                                               \
    \
    void Connect(Signal* sig)                       \
    {                                               \
        m_Signals.push_back(sig);                   \
    }                                               \
    \
    void Disconnect(const Function_t* fn)               \
    {                                                   \
        for (size_t i = 0; i < m_Slots.size(); ++i) {   \
            if (m_Slots[i] == fn) {                     \
                delete fn;                              \
                m_Slots.erase(m_Slots.begin() + i);     \
                break;                                  \
            }                                           \
        }                                               \
    }                                                   \
    \
    void Disconnect(const Signal* sig)                  \
    {                                                   \
        for (size_t i = 0; i < m_Signals.size(); ++i) { \
            if (m_Signals[i] == sig) {                  \
                m_Signals.erase(m_Signals.begin() + i); \
                break;                                  \
            }                                           \
        }                                               \
    }                                                   \
    \
    template<typename recv_t>                           \
    void DisconnectReceiver(recv_t* recv)               \
    {                                                   \
        for (int i = m_Slots.size()-1; i >= 0; --i) {   \
            if ((m_Slots[i]->GetReceiver()) == recv) {  \
                delete m_Slots[i];                      \
                m_Slots.erase(m_Slots.begin() + i);     \
            }                                           \
        }                                               \
    }                                                   \
    \
    void Reserve(const size_t size)     \
    {                                   \
        m_Slots.Reserve(size);          \
        m_Signals.Reserve(size);        \
    }                                   \
    \
private:                                \
    std::vector<Function_t*> m_Slots;   \
    std::vector<Signal*> m_Signals

template<typename ret_t, typename arg_t>
class Signal<ret_t (arg_t)>
{
typedef Function<ret_t (arg_t)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(arg_t arg) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])(arg);
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])(arg);
        }
    }
};

template<typename ret_t>
class Signal<ret_t (void)>
{
typedef Function<ret_t (void)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(void) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])();
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])();
        }
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t>
class Signal<ret_t (arg1_t, arg2_t)>
{
typedef Function<ret_t (arg1_t, arg2_t)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(arg1_t arg1, arg2_t arg2) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])(arg1, arg2);
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])(arg1, arg2);
        }
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t>
class Signal<ret_t (arg1_t, arg2_t, arg3_t)>
{
typedef Function<ret_t (arg1_t, arg2_t, arg3_t)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])(arg1, arg2, arg3);
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])(arg1, arg2, arg3);
        }
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t>
class Signal<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
typedef Function<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])(arg1, arg2, arg3, arg4);
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])(arg1, arg2, arg3, arg4);
        }
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t,
typename arg5_t>
class Signal<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
typedef Function<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)> Function_t;

SCX_SIGNAL_COPY_SIGNAL_COMMON;

public:
    void operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        for (size_t i = 0; i < m_Slots.size(); ++i) {
            (*m_Slots[i])(arg1, arg2, arg3, arg4, arg5);
        }

        for (size_t i = 0; i < m_Signals.size(); ++i) {
            (*m_Signals[i])(arg1, arg2, arg3, arg4, arg5);
        }
    }
};

#undef SCX_SIGNAL_COPY_SIGNAL_COMMON

}

#endif
