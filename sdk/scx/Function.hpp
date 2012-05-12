#ifndef SCX_FUNCTION_HPP
#define SCX_FUNCTION_HPP

#include <cstddef>
#include <cassert>

namespace scx {

namespace FunctionType {
enum e {
    None,
    Free,
    Member,
    ConstMember
};
}
typedef FunctionType::e EmFunctionType;

/* Function base class */
template<typename signature>
class IFunction;

template<typename ret_t>
class IFunction<ret_t (void)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(void) const = 0;
    virtual void* Receiver() const = 0;
};

template<typename ret_t,
     typename arg_t>
class IFunction<ret_t (arg_t)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(arg_t) const = 0;
    virtual void* Receiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t>
class IFunction<ret_t (arg1_t, arg2_t)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t) const = 0;
    virtual void* Receiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t>
class IFunction<ret_t (arg1_t, arg2_t, arg3_t)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t) const = 0;
    virtual void* Receiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t>
class IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t, arg4_t) const = 0;
    virtual void* Receiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t,
     typename arg5_t>
class IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
public:
    virtual ~IFunction() { };
    virtual IFunction* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t) const = 0;
    virtual void* Receiver() const = 0;
};


#define SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(Class, qualify, argts...) \
    typedef ret_t (recv_t::*fn_t)(argts) qualify;       \
    typedef IFunction<ret_t (argts)> ifunction_t;       \
public:\
    Class(ret_t (recv_t::*fn)(argts) qualify, recv_t* recv):\
        m_Function(fn),         \
        m_Receiver(recv)        \
    { }\
    \
    ifunction_t* Clone() const\
    {\
        return new Class(m_Function, m_Receiver); \
    }\
    \
    void* Receiver() const\
    {\
        return const_cast<void*>(static_cast<const void*>(m_Receiver)); \
    }\
    \
private:\
    const fn_t m_Function;        \
    recv_t* m_Receiver

/*==== Member Function ====*/
template<typename signature>
class MemberFunction;

template<typename ret_t, typename recv_t>
class MemberFunction<ret_t (recv_t*)>: public IFunction<ret_t (void)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , void);

public:
    ret_t operator()(void) const
    {
        return (m_Receiver->*m_Function)();
    }
};

template<typename ret_t, typename recv_t,
     typename arg_t>
class MemberFunction<ret_t (recv_t*, arg_t)>: public IFunction<ret_t (arg_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return (m_Receiver->*m_Function)(arg);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t>
class MemberFunction<ret_t (recv_t*, arg1_t, arg2_t)>:
    public IFunction<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , arg1_t, arg2_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t>
class MemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , arg1_t, arg2_t, arg3_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t>
class MemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , arg1_t, arg2_t, arg3_t, arg4_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3, arg4);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t,
     typename arg5_t>
class MemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(MemberFunction, , arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3, arg4, arg5);
    }
};

/*==== Const Member Function ====*/
template<typename signature>
class ConstMemberFunction;

template<typename ret_t, typename recv_t>
class ConstMemberFunction<ret_t (recv_t*)>: public IFunction<ret_t (void)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, void);

public:
    ret_t operator()(void) const
    {
        return (m_Receiver->*m_Function)();
    }
};

template<typename ret_t, typename recv_t,
     typename arg_t>
class ConstMemberFunction<ret_t (recv_t*, arg_t)>: public IFunction<ret_t (arg_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return (m_Receiver->*m_Function)(arg);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t>
class ConstMemberFunction<ret_t (recv_t*, arg1_t, arg2_t)>:
    public IFunction<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, arg1_t, arg2_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t>
class ConstMemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, arg1_t, arg2_t, arg3_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t>
class ConstMemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, arg1_t, arg2_t, arg3_t, arg4_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3, arg4);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t,
     typename arg5_t>
class ConstMemberFunction<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON(ConstMemberFunction, const, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3, arg4, arg5);
    }
};


#undef SCX_FUNCTION_COPY_MEM_FUNCTION_COMMON

/*==== Free Function ====*/
#define SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(argts...) \
    typedef ret_t (*fn_t)(argts);                   \
    typedef IFunction<ret_t (argts)> ifunction_t;   \
public:\
    FreeFunction(fn_t fn):  \
        m_Function(fn)      \
    { }                     \
    \
    ifunction_t* Clone() const\
    {\
        return new FreeFunction(m_Function); \
    }\
    \
    void* Receiver() const\
    {\
        return NULL;    \
    }\
    \
private:\
    const fn_t m_Function
 
template<typename signature>
class FreeFunction;

template<typename ret_t,
typename arg_t>
class FreeFunction<ret_t (arg_t)>: public IFunction<ret_t (arg_t)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return m_Function(arg);
    }
};

template<typename ret_t>
class FreeFunction<ret_t (void)>: public IFunction<ret_t (void)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(void);

public:
    ret_t operator()(void) const
    {
        return m_Function();
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t>
class FreeFunction<ret_t (arg1_t, arg2_t)>:
    public IFunction<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(arg1_t, arg2_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2) const
    {
        return m_Function(arg1, arg2);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t>
class FreeFunction<ret_t (arg1_t, arg2_t, arg3_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(arg1_t, arg2_t, arg3_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3) const
    {
        return m_Function(arg1, arg2, arg3);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t>
class FreeFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(arg1_t, arg2_t, arg3_t, arg4_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) const
    {
        return m_Function(arg1, arg2, arg3, arg4);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t,
typename arg5_t>
class FreeFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>:
    public IFunction<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        return m_Function(arg1, arg2, arg3, arg4, arg5);
    }
};

#undef SCX_FUNCTION_COPY_FREE_FUNCTION_COMMON

/*==== Function wrapper ====*/
template<typename signature>
class Function;

#define SCX_FUNCTION_COPY_FUNCTION_COMMON_VOID \
public:\
    template<class recv_t>\
    Function(ret_t (recv_t::*fn)(void), recv_t* recv)\
    {\
        m_PtrFn = new MemberFunction<ret_t (recv_t*)>(fn, recv);    \
        m_FnType = FunctionType::Member;                            \
    }\
    \
    template<class recv_t>\
    Function(ret_t (recv_t::*fn)(void) const, const recv_t* recv)\
    {\
        m_PtrFn = new ConstMemberFunction<ret_t (const recv_t*)>(fn, recv); \
        m_FnType = FunctionType::ConstMember;                               \
    }\
    \
    Function(ret_t (*fn)(void))\
    {\
        m_PtrFn = new FreeFunction<ret_t (void)>(fn);   \
        m_FnType = FunctionType::Free;                  \
    }\
    \
    template<class recv_t>\
    void Bind(ret_t (recv_t::*fn)(void), recv_t* recv)\
    {\
        if (m_PtrFn != NULL)                                    \
            delete m_PtrFn;                                     \
        m_PtrFn = new MemberFunction<ret_t (recv_t*)>(fn, recv);\
        m_FnType = FunctionType::Member;                        \
    }\
    \
    template<class recv_t>\
    void Bind(ret_t (recv_t::*fn)(void) const, const recv_t* recv)\
    {\
        if (m_PtrFn != NULL)                                                \
            delete m_PtrFn;                                                 \
        m_PtrFn = new ConstMemberFunction<ret_t (const recv_t*)>(fn, recv); \
        m_FnType = FunctionType::ConstMember;                               \
    }\
    \
    void Bind(ret_t (*fn)(void))\
    {\
        if (m_PtrFn != NULL)                            \
            delete m_PtrFn;                             \
        m_PtrFn = new FreeFunction<ret_t (void)>(fn);   \
        m_FnType = FunctionType::Free;                  \
    }\
    \
    SCX_FUNCTION_COPY_FUNCTION_COMMON(void)

#define SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(argts...) \
public:\
    template<class recv_t>\
    Function(ret_t (recv_t::*fn)(argts), recv_t* recv)\
    {\
        m_PtrFn = new MemberFunction<ret_t (recv_t*, argts)>(fn, recv); \
        m_FnType = FunctionType::Member;                                \
    }\
    \
    template<class recv_t>\
    Function(ret_t (recv_t::*fn)(argts) const, const recv_t* recv)\
    {\
        m_PtrFn = new ConstMemberFunction<ret_t (const recv_t*, argts)>(fn, recv);  \
        m_FnType = FunctionType::ConstMember;                                       \
    }\
    \
    Function(ret_t (*fn)(argts))\
    {\
        m_PtrFn = new FreeFunction<ret_t (argts)>(fn);  \
        m_FnType = FunctionType::Free;                  \
    }\
    \
    template<class recv_t>\
    void Bind(ret_t (recv_t::*fn)(argts), recv_t* recv)\
    {\
        if (m_PtrFn != NULL)                                            \
            delete m_PtrFn;                                             \
        m_PtrFn = new MemberFunction<ret_t (recv_t*, argts)>(fn, recv); \
        m_FnType = FunctionType::Member;                                \
    }\
    \
    template<class recv_t>\
    void Bind(ret_t (recv_t::*fn)(argts) const, const recv_t* recv)\
    {\
        if (m_PtrFn != NULL)                                                        \
            delete m_PtrFn;                                                         \
        m_PtrFn = new ConstMemberFunction<ret_t (const recv_t*, argts)>(fn, recv);  \
        m_FnType = FunctionType::ConstMember;                                       \
    }\
    \
    void Bind(ret_t (*fn)(argts))\
    {\
        if (m_PtrFn != NULL)                            \
            delete m_PtrFn;                             \
        m_PtrFn = new FreeFunction<ret_t (argts)>(fn);  \
        m_FnType = FunctionType::Free;                  \
    }\
    \
    SCX_FUNCTION_COPY_FUNCTION_COMMON(argts)

#define SCX_FUNCTION_COPY_FUNCTION_COMMON(argts...) \
    typedef IFunction<ret_t (argts)> ifunction_t; \
    \
public:\
    Function():\
        m_PtrFn(NULL),                  \
        m_FnType(FunctionType::None)    \
    { }                                 \
    \
    Function(const Function& f)\
    {\
        m_PtrFn = f.m_PtrFn->Clone();   \
        m_FnType = f.m_FnType;          \
    }\
    \
    ~Function()\
    {\
        if (m_PtrFn != NULL)    \
            delete m_PtrFn;     \
    }\
    \
    Function& operator=(const Function& f)\
    {\
        if (&f != this) {                   \
            if (m_PtrFn != NULL)            \
                delete m_PtrFn;             \
            m_PtrFn = f.m_PtrFn->Clone();   \
            m_FnType = f.m_FnType;          \
        }                                   \
        return *this;                       \
    }\
    \
    EmFunctionType Type() const\
    {\
        return m_FnType;            \
    }\
    \
    void* Receiver() const\
    {\
        return m_PtrFn->Receiver();  \
    }\
    \
private:\
    ifunction_t* m_PtrFn;    \
    EmFunctionType m_FnType
 
template<typename ret_t>
class Function<ret_t (void)>
{
    SCX_FUNCTION_COPY_FUNCTION_COMMON_VOID;

public:
    ret_t operator()(void) const
    {
        return (*m_PtrFn)();
    }
};
       
template<typename ret_t, typename arg_t>
class Function<ret_t (arg_t)>
{ 
    SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return (*m_PtrFn)(arg);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t>
class Function<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(arg1_t, arg2_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2) const
    {
        return (*m_PtrFn)(arg1, arg2);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t>
class Function<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(arg1_t, arg2_t, arg3_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3) const
    {
        return (*m_PtrFn)(arg1, arg2, arg3);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t>
class Function<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(arg1_t, arg2_t, arg3_t, arg4_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) const
    {
        return (*m_PtrFn)(arg1, arg2, arg3, arg4);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t,
typename arg4_t,
typename arg5_t>
class Function<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const {
        return (*m_PtrFn)(arg1, arg2, arg3, arg4, arg5);
    }
};

#undef SCX_FUNCTION_COPY_FUNCTION_COMMON
#undef SCX_FUNCTION_COPY_FUNCTION_COMMON_VOID
#undef SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID
}

#endif
