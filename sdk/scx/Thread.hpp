#ifndef SCX_THREAD_HPP
#define SCX_THREAD_HPP

#include <pthread.h>
#include <signal.h>
#include "Function.hpp"

namespace scx {

class Thread
{
public:
    Thread()
    {
    }
    
    ~Thread()
    {
    }

    int Join() const
    {
        return pthread_join(m_thread_id, NULL);
    }

    int Cancel() const
    {
        return pthread_cancel(m_thread_id);
    }

    int Kill(int sig) const
    {
        return pthread_kill(m_thread_id, sig);
    }

    int Detach() const
    {
        return pthread_detach(m_thread_id);
    }

    pthread_t GetId() const
    {
        return m_thread_id;
    }

private:
    template<typename fn_t, typename arg_t>
    struct Params1
    {
        fn_t fn;
        arg_t arg;

        explicit Params1(const fn_t& f, const arg_t& a):
            fn(f),
            arg(a)
        { }
    };

    template<typename fn_t, 
         typename arg1_t, 
         typename arg2_t>
    struct Params2
    {
        fn_t fn;
        arg1_t arg1; arg2_t arg2;

        explicit Params2(const fn_t& f, 
                const arg1_t& a1, const arg2_t& a2):
            fn(f),
            arg1(a1), arg2(a2)
        { }
    };

    template<typename fn_t, 
         typename arg1_t, 
         typename arg2_t,
         typename arg3_t>
    struct Params3
    {
        fn_t fn;
        arg1_t arg1; arg2_t arg2; arg3_t arg3;

        explicit Params3(const fn_t& f, 
                const arg1_t& a1, const arg2_t& a2, const arg3_t& a3):
            fn(f),
            arg1(a1), arg2(a2), arg3(a3)
        { }
    };

    template<typename fn_t, 
         typename arg1_t, 
         typename arg2_t,
         typename arg3_t,
         typename arg4_t>
    struct Params4
    {
        fn_t fn;
        arg1_t arg1; arg2_t arg2;
        arg3_t arg3; arg4_t arg4;

        explicit Params4(
                const fn_t& f, 
                const arg1_t& a1, const arg2_t& a2, 
                const arg3_t& a3, const arg4_t& a4):
            fn(f),
            arg1(a1), arg2(a2),
            arg3(a3), arg4(a4)
        { }
    };

    template<typename fn_t, 
         typename arg1_t, 
         typename arg2_t,
         typename arg3_t,
         typename arg4_t,
         typename arg5_t>
    struct Params5
    {
        fn_t fn;
        arg1_t arg1; arg2_t arg2;
        arg3_t arg3; arg4_t arg4;
        arg5_t arg5;

        explicit Params5(const fn_t& f, 
                const arg1_t& a1, const arg2_t& a2, 
                const arg3_t& a3, const arg4_t& a4, const arg5_t& a5):
            fn(f),
            arg1(a1), arg2(a2),
            arg3(a3), arg4(a4),
            arg5(a5)
        { }
    };

public:
/* for void function */
#define SCX_THREAD_COPY_RUN_VOID                    \
    typedef void* (*pth_fn_t)(void*);               \
    typedef void (*mem_fn_t)(fn_t*);                \
    mem_fn_t mem_fn = &Thread::RunFunction;         \
    fn_t* p_fn = new fn_t(fn);                      \
    int ret = pthread_create(&m_thread_id,          \
        NULL, (pth_fn_t)mem_fn, p_fn);              \
    if (ret != 0)                                   \
        delete p_fn 

    template<typename fn_t>
    Thread(fn_t fn)
    {
        SCX_THREAD_COPY_RUN_VOID;
    }
    
    template<typename fn_t>
    int Run(fn_t fn)
    {
        SCX_THREAD_COPY_RUN_VOID;
        return ret;
    }

/* for multi arguments function */
#define SCX_THREAD_COPY_RUN_MILTI_ARGS              \
    typedef void* (*pth_fn_t)(void*);               \
    typedef void (*mem_fn_t)(params_t*);            \
    mem_fn_t mem_fn = &Thread::RunFunction;         \
    int ret = pthread_create(&m_thread_id,          \
        NULL, (pth_fn_t)mem_fn, p_params);          \
    if (ret != 0)                                   \
        delete p_params

/* for 1 argument */
#define SCX_THREAD_COPY_INIT_1ARG                   \
    typedef Params1<fn_t, arg_t> params_t;          \
    params_t* p_params =                            \
        new params_t(fn, arg)

    template<typename fn_t, typename arg_t>
    Thread(fn_t fn, arg_t arg)
    {
        SCX_THREAD_COPY_INIT_1ARG;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
    }

    template<typename fn_t, typename arg_t>
    int Run(fn_t fn, arg_t arg)
    {
        SCX_THREAD_COPY_INIT_1ARG;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
        return ret;
    }

/* for 2 arguments */
#define SCX_THREAD_COPY_INIT_2ARGS                      \
    typedef Params2<fn_t, arg1_t, arg2_t> params_t;     \
    params_t* p_params =                                \
        new params_t(fn, arg1, arg2)

    template<typename fn_t, typename arg1_t, typename arg2_t>
    Thread(fn_t fn, arg1_t arg1, arg2_t arg2)
    {
        SCX_THREAD_COPY_INIT_2ARGS;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
    }

    template<typename fn_t, typename arg1_t, typename arg2_t>
    int Run(fn_t fn, arg1_t arg1, arg2_t arg2)
    {
        SCX_THREAD_COPY_INIT_2ARGS;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
        return ret;
    }

/* for 3 arguments */
#define SCX_THREAD_COPY_INIT_3ARGS                              \
    typedef Params3<fn_t, arg1_t, arg2_t, arg3_t> params_t;     \
    params_t* p_params =                                        \
        new params_t(fn, arg1, arg2, arg3)

    template<typename fn_t, typename arg1_t, typename arg2_t, typename arg3_t>
    Thread(fn_t fn, arg1_t arg1, arg2_t arg2, arg3_t arg3)
    {
        SCX_THREAD_COPY_INIT_3ARGS;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
    }

    template<typename fn_t, typename arg1_t, typename arg2_t, typename arg3_t>
    int Run(fn_t fn, arg1_t arg1, arg2_t arg2, arg3_t arg3)
    {
        SCX_THREAD_COPY_INIT_3ARGS;
        SCX_THREAD_COPY_RUN_MILTI_ARGS;
        return ret;
    }

private:
    template<typename fn_t>
    static void RunFunction(fn_t* p_fn) 
    {
        (*p_fn)();
        delete p_fn;
    }

    template<typename fn_t, typename arg_t>
    static void RunFunction(Params1<fn_t, arg_t>* p_params) 
    {
        (p_params->fn)(p_params->arg);
        delete p_params;
    }

    template<typename fn_t,
         typename arg1_t,
         typename arg2_t>
    static void RunFunction(Params2<fn_t, arg1_t, arg2_t>* p_params) 
    {
        (p_params->fn)(p_params->arg1, p_params->arg2);
        delete p_params;
    }
    
    template<typename fn_t,
         typename arg1_t,
         typename arg2_t,
         typename arg3_t>
    static void RunFunction(Params3<fn_t, arg1_t, arg2_t, arg3_t>* p_params) 
    {
        (p_params->fn)(p_params->arg1, p_params->arg2, p_params->arg3);
        delete p_params;
    }

private:
    pthread_t m_thread_id;
};

#undef SCX_THREAD_COPY_RUN_VOID
#undef SCX_THREAD_COPY_RUN_MILTI_ARGS
#undef SCX_THREAD_COPY_INIT_1ARG
#undef SCX_THREAD_COPY_INIT_2ARGS
#undef SCX_THREAD_COPY_INIT_3ARGS
}

#endif
