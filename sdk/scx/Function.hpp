#ifndef SCX_FUNCTION_HPP
#define SCX_FUNCTION_HPP

#include <cstddef>
#include <cassert>

namespace scx {

namespace FunctionType {
enum e {
    None,
    Pointer,
    Member
};
}
typedef FunctionType::e EmFunctionType;

/* Function base class */
template<typename signature>
class FunctionBase;

template<typename ret_t>
class FunctionBase<ret_t (void)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(void) const = 0;
    virtual void* GetReceiver() const = 0;
};

template<typename ret_t,
     typename arg_t>
class FunctionBase<ret_t (arg_t)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(arg_t) const = 0;
    virtual void* GetReceiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t>
class FunctionBase<ret_t (arg1_t, arg2_t)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t) const = 0;
    virtual void* GetReceiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t>
class FunctionBase<ret_t (arg1_t, arg2_t, arg3_t)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t) const = 0;
    virtual void* GetReceiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t>
class FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t, arg4_t) const = 0;
    virtual void* GetReceiver() const = 0;
};

template<typename ret_t,
     typename arg1_t,
     typename arg2_t,
     typename arg3_t,
     typename arg4_t,
     typename arg5_t>
class FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
public:
    virtual ~FunctionBase() { };
    virtual FunctionBase* Clone() const = 0;
    virtual ret_t operator()(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t) const = 0;
    virtual void* GetReceiver() const = 0;
};

/* Function mem class */
template<typename signature>
class FunctionMem;

#define SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(argts...) \
    typedef ret_t (recv_t::*fn_t)(argts);               \
    typedef FunctionBase<ret_t (argts)> FunctionBase_t; \
public:\
    FunctionMem(fn_t fn, recv_t* recv): \
        m_Function(fn),                 \
        m_Receiver(recv) {              \
    }                                   \
    \
    FunctionBase_t* Clone() const {                     \
        return new FunctionMem(m_Function, m_Receiver); \
    }                                                   \
    \
    void* GetReceiver() const { return m_Receiver; }\
    \
private:                    \
    fn_t m_Function;        \
    recv_t* m_Receiver

template<typename ret_t, typename recv_t>
class FunctionMem<ret_t (recv_t*)>: public FunctionBase<ret_t (void)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(void);

public:
    ret_t operator()(void) const
    {
        return (m_Receiver->*m_Function)();
    }
};

template<typename ret_t, typename recv_t,
     typename arg_t>
class FunctionMem<ret_t (recv_t*, arg_t)>: public FunctionBase<ret_t (arg_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return (m_Receiver->*m_Function)(arg);
    }
};

template<typename ret_t, typename recv_t,
     typename arg1_t,
     typename arg2_t>
class FunctionMem<ret_t (recv_t*, arg1_t, arg2_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(arg1_t, arg2_t);

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
class FunctionMem<ret_t (recv_t*, arg1_t, arg2_t, arg3_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(arg1_t, arg2_t, arg3_t);

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
class FunctionMem<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(arg1_t, arg2_t, arg3_t, arg4_t);

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
class FunctionMem<ret_t (recv_t*, arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        return (m_Receiver->*m_Function)(arg1, arg2, arg3, arg4, arg5);
    }
};

#undef SCX_FUNCTION_COPY_FUNCTION_MEM_COMMON

/* Function ptr class */
template<typename signature>
class FunctionPtr;

#define SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(argts...)     \
    typedef ret_t (*fn_t)(argts);                           \
    typedef FunctionBase<ret_t (argts)> FunctionBase_t;     \
public:\
    FunctionPtr(fn_t fn):   \
        m_Function(fn)      \
    {                       \
    }                       \
    \
    FunctionBase_t* Clone() const {         \
        return new FunctionPtr(m_Function); \
    }                                       \
    \
    void* GetReceiver() const   \
    {                           \
        return NULL;            \
    }                           \
    \
private:\
    fn_t m_Function
 
template<typename ret_t,
typename arg_t>
class FunctionPtr<ret_t (arg_t)>: public FunctionBase<ret_t (arg_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(arg_t);

public:
    ret_t operator()(arg_t arg) const
    {
        return m_Function(arg);
    }
};

template<typename ret_t>
class FunctionPtr<ret_t (void)>: public FunctionBase<ret_t (void)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(void);

public:
    ret_t operator()(void) const
    {
        return m_Function();
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t>
class FunctionPtr<ret_t (arg1_t, arg2_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(arg1_t, arg2_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2) const {
        return m_Function(arg1, arg2);
    }
};

template<typename ret_t,
typename arg1_t,
typename arg2_t,
typename arg3_t>
class FunctionPtr<ret_t (arg1_t, arg2_t, arg3_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(arg1_t, arg2_t, arg3_t);

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
class FunctionPtr<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(arg1_t, arg2_t, arg3_t, arg4_t);

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
class FunctionPtr<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>:
    public FunctionBase<ret_t (arg1_t, arg2_t, arg3_t, arg4_t, arg5_t)>
{
    SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON(arg1_t, arg2_t, arg3_t, arg4_t, arg5_t);

public:
    ret_t operator()(arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4, arg5_t arg5) const
    {
        return m_Function(arg1, arg2, arg3, arg4, arg5);
    }
};

#undef SCX_FUNCTION_COPY_FUNCTION_PTR_COMMON

/* Function wrapper */
template<typename signature>
class Function;

#define SCX_FUNCTION_COPY_FUNCTION_COMMON_VOID              \
public:\
    template<typename fn_t, typename recv_t>                    \
    Function(fn_t fn, recv_t recv) {                            \
        m_PtrFn = new FunctionMem<ret_t (recv_t)>(fn, recv);    \
        m_FnType = FunctionType::Member;                        \
    }                                                           \
    \
    Function(ret_t (*fn)(void)) {                       \
        m_PtrFn = new FunctionPtr<ret_t (void)>(fn);    \
        m_FnType = FunctionType::Pointer;               \
    }                                                   \
    \
    template<typename fn_t, typename recv_t>                    \
    void Bind(fn_t fn, recv_t recv) {                           \
        if (m_PtrFn != NULL) delete m_PtrFn;                    \
        m_PtrFn = new FunctionMem<ret_t (recv_t)>(fn, recv);    \
        m_FnType = FunctionType::Member;                        \
    }                                                           \
    \
    void Bind(ret_t (*fn)(void)) {                      \
        if (m_PtrFn != NULL) delete m_PtrFn;            \
        m_PtrFn = new FunctionPtr<ret_t (void)>(fn);    \
        m_FnType = FunctionType::Pointer;               \
    }                                                   \
    \
    SCX_FUNCTION_COPY_FUNCTION_COMMON(void)

#define SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID(argts...)   \
public:\
    template<typename fn_t, typename recv_t>                            \
    Function(fn_t fn, recv_t recv) {                                    \
        m_PtrFn = new FunctionMem<ret_t (recv_t, argts)>(fn, recv);     \
        m_FnType = FunctionType::Member;                                \
    }                                                                   \
    \
    Function(ret_t (*fn)(argts)) {                      \
        m_PtrFn = new FunctionPtr<ret_t (argts)>(fn);   \
        m_FnType = FunctionType::Pointer;               \
    }                                                   \
    \
    template<typename fn_t, typename recv_t>                        \
    void Bind(fn_t fn, recv_t recv) {                               \
        if (m_PtrFn != NULL) delete m_PtrFn;                        \
        m_PtrFn = new FunctionMem<ret_t (recv_t, argts)>(fn, recv); \
        m_FnType = FunctionType::Member;                            \
    }                                                               \
    \
    void Bind(ret_t (*fn)(argts)) {                     \
        if (m_PtrFn != NULL) delete m_PtrFn;            \
        m_PtrFn = new FunctionPtr<ret_t (argts)>(fn);   \
        m_FnType = FunctionType::Pointer;               \
    }                                                   \
    \
    SCX_FUNCTION_COPY_FUNCTION_COMMON(argts)

#define SCX_FUNCTION_COPY_FUNCTION_COMMON(argts...)     \
    typedef FunctionBase<ret_t (argts)> FunctionBase_t; \
    \
public:\
    Function():                         \
        m_PtrFn(NULL),                  \
        m_FnType(FunctionType::None)    \
    { }                                 \
    \
    Function(const Function& f) {       \
        m_PtrFn = f.m_PtrFn->Clone();   \
        m_FnType = f.m_FnType;          \
    }                                   \
    \
    ~Function() {                               \
        if (m_PtrFn != NULL) delete m_PtrFn;    \
    }                                           \
    \
    Function& operator=(const Function& f) {        \
        if (&f != this) {                           \
            if (m_PtrFn != NULL) delete m_PtrFn;    \
            m_PtrFn = f.m_PtrFn->Clone();           \
            m_FnType = f.m_FnType;                  \
        }                                           \
        return *this;                               \
    }                                               \
    \
    EmFunctionType GetType() const {    \
        return m_FnType;                \
    }                                   \
    \
    void* GetReceiver() const {         \
        return m_PtrFn->GetReceiver();  \
    }                                   \
    \
private:\
    FunctionBase_t* m_PtrFn;    \
    EmFunctionType m_FnType
        
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

#undef SCX_FUNCTION_COPY_FUNCTION_COMMON_VOID
#undef SCX_FUNCTION_COPY_FUNCTION_COMMON
#undef SCX_FUNCTION_COPY_FUNCTION_COMMON_NVOID
}

#endif
