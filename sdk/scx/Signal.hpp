#ifndef SCX_SIGNAL_HPP
#define SCX_SIGNAL_HPP

#include <vector>
#include <memory>
#include <functional>

namespace scx {

template <class S>
class Signal;

template <class R, class... P>
class Signal<R (P...)>
{
public:
    struct Slot
    {
        virtual ~Slot() { }
        virtual char Type() const = 0;
        virtual R Invoke(P...) const = 0;
        virtual const void* Object() const = 0;
        virtual bool EqualTo(const Slot*) const = 0;
    };

public:
    Signal() = default;
    ~Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    const Slot* Connect(R (*p)(P...))
    {
        Slot* slot = new PtrSlot(p);
        m_slots.push_back(std::unique_ptr<Slot>(slot));
        return slot;
    }

    template<typename O>
    const Slot* Connect(R (O::*f)(P...), O* o)
    {
        Slot* slot = new MemSlot<O>(f, o);
        m_slots.push_back(std::unique_ptr<Slot>(slot));
        return slot;
    }

    template<typename O>
    const Slot* Connect(R (O::*f)(P...) const, const O* o)
    {
        Slot* slot = new ConstSlot<O>(f, o);
        m_slots.push_back(std::unique_ptr<Slot>(slot));
        return slot;
    }

    template<typename F>
    const Slot* Connect(const F& f)
    {
        Slot* slot = new FnSlot(f);
        m_slots.push_back(std::unique_ptr<Slot>(slot));
        return slot;
    }

    bool Disconnect(R (*f)(P...))
    {
        PtrSlot slot(f);
        return Disconnect(&slot);
    }

    template<typename O>
    bool Disconnect(R (O::*f)(P...), O* o)
    {
        MemSlot<O> slot(f, o);
        return Disconnect(&slot);
    }

    template<typename O>
    bool Disconnect(R (O::*f)(P...) const, const O* o)
    {
        ConstSlot<O> slot(f, o);
        return Disconnect(&slot);
    }

    bool Disconnect(const void* slot)
    {
        const Slot* ts = static_cast<const Slot*>(slot);
        for (size_t i = 0; i < m_slots.size(); ++i) {
            if (m_slots[i]->EqualTo(ts)) {
                m_slots.erase(m_slots.begin()+i);
                return true;
            }
        }
        return false;
    }

    bool DisconnectObject(const void* o)
    {
        if (o != nullptr) {
            for (size_t i = 0; i < m_slots.size(); ++i) {
                if (m_slots[i]->Object() == o) {
                    m_slots.erase(m_slots.begin()+i);
                    return true;
                }
            }
        }
        return false;
    }

    void Clear()
    {
        m_slots.clear();
    }

    void operator()(P... p) const
    {
        for (size_t i = 0; i < m_slots.size(); ++i) {
            m_slots[i]->Invoke(p...);
        }
    }

    size_t Count() const
    {
        return m_slots.size();
    }

private:
    struct PtrSlot: public Slot
    {
        typedef R (*ptr_t)(P...);

        explicit PtrSlot(ptr_t p):
            ptr(p)
        {
        }

        virtual char Type() const
        {
            return 'p';
        }

        virtual R Invoke(P... p) const
        {
            return ptr(p...);
        }

        virtual const void* Object() const
        {
            return nullptr;
        }

        virtual bool EqualTo(const Slot* slot) const
        {
            if (Type() == slot->Type()) {
                // I think dynamic_cast is unnecessary.
                const PtrSlot* ps = static_cast<const PtrSlot*>(slot);
                return (ptr == ps->ptr);
            }
            return false;
        }
        
    private:
        ptr_t ptr;
    };

    template<typename O>
    struct MemSlot: public Slot
    {
        typedef R (O::*fn_t)(P...);

        MemSlot(fn_t f, O* o):
            fn(f), obj(o)
        {
        }

        virtual char Type() const
        {
            return 'm';
        }

        virtual R Invoke(P... p) const
        {
            return (obj->*fn)(p...);
        }

        virtual const void* Object() const
        {
            return obj;
        }

        virtual bool EqualTo(const Slot* slot) const
        {
            if (Type() == slot->Type()) {
                const MemSlot* ms = static_cast<const MemSlot*>(slot);
                return (obj == ms->obj && fn == ms->fn);
            }
            return false;
        }

    private:
        fn_t fn;
        O* obj;
    };

    template<typename O>
    struct ConstSlot: public Slot
    {
        typedef R (O::*fn_t)(P...) const;

        ConstSlot(fn_t f, const O* o):
            fn(f), obj(o)
        {
        }

        virtual char Type() const
        {
            return 'c';
        }

        virtual R Invoke(P... p) const
        {
            return (obj->*fn)(p...);
        }

        virtual const void* Object() const
        {
            return obj;
        }

        virtual bool EqualTo(const Slot* slot) const
        {
            if (Type() == slot->Type()) {
                const ConstSlot* cs = static_cast<const ConstSlot*>(slot);
                return (obj == cs->obj && fn == cs->fn);
            }
            return false;
        }

    private:
        fn_t fn;
        const O* obj;
    };

    struct FnSlot: public Slot
    {
        typedef std::function<R (P...)> fn_t;

        explicit FnSlot(const fn_t& f):
            fn(f)
        {
        }

        virtual char Type() const
        {
            return 'f';
        }

        virtual R Invoke(P... p) const
        {
            return fn(p...);
        }

        virtual const void* Object() const
        {
            return nullptr;
        }

        virtual bool EqualTo(const Slot* slot) const
        {
            return (slot == this);
        }

    private:
        const fn_t fn;
    };

private:
    std::vector<std::unique_ptr<Slot>> m_slots;
};

}

#endif
