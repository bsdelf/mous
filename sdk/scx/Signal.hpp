#pragma once

#include <vector>
#include <memory>
#include <functional>

namespace scx {

template<class>
class Signal;

template<class R, class... P>
class Signal<R (P...)> {
private:
    struct Slot;

public:
    Signal() = default;
    ~Signal() = default;
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;

    template<class... Args>
    void Connect(Args... args) {
        Connect(std::move(MakeSlot(args...)));
    }

    void Connect(std::unique_ptr<Slot>&& slot) {
        _slots.emplace_back(std::move(slot));
    }

    template<class... Args>
    bool Disconnect(Args... args) {
        return Disconnect(MakeSlot(args...));
    }

    bool Disconnect(const std::unique_ptr<Slot>& slot) {
        auto ps = static_cast<const Slot*>(slot.get());
        for (size_t i = 0; i < _slots.size(); ++i) {
            if (_slots[i]->EqualTo(ps)) {
                _slots.erase(_slots.begin() + i);
                return true;
            }
        }
        return false;
    }

    bool Disconnect(const void* o) {
        bool y = false;
        _slots.erase(std::remove_if(_slots.begin(), _slots.end(), [&y, o](const auto& slot) {
            if (slot->Object() == o) {
                y = true;
                return true;
            }
            return false;
        }), _slots.end());
        return y;
    }

    void Clear() {
        _slots.clear();
    }

    void operator()(P... p) const {
        for (size_t i = 0; i < _slots.size(); ++i) {
            _slots[i]->Invoke(p...);
        }
    }

    auto Empty() const {
        return _slots.empty();
    }

    auto Size() const {
        return _slots.size();
    }

private:
    std::vector<std::unique_ptr<Slot>> _slots;

    struct Slot {
        virtual ~Slot() { }
        virtual char Type() const = 0;
        virtual R Invoke(P...) const = 0;
        virtual const void* Object() const = 0;
        virtual bool EqualTo(const Slot*) const = 0;
    };

    struct PtrSlot: public Slot {
        typedef R (*ptr_t)(P...);

        explicit PtrSlot(ptr_t p): ptr(p) { }

        virtual char Type() const { return 'p'; }

        virtual R Invoke(P... p) const { return ptr(p...); }

        virtual const void* Object() const { return nullptr; }

        virtual bool EqualTo(const Slot* slot) const {
            if (Type() == slot->Type()) {
                const PtrSlot* ps = static_cast<const PtrSlot*>(slot);
                return (ptr == ps->ptr);
            }
            return false;
        }
        
    private:
        ptr_t ptr;
    };

    template<class O>
    struct MemSlot: public Slot {
        typedef R (O::*fn_t)(P...);

        MemSlot(fn_t f, O* o): fn(f), obj(o) { }

        virtual char Type() const { return 'm'; }

        virtual R Invoke(P... p) const { return (obj->*fn)(p...); }

        virtual const void* Object() const { return obj; }

        virtual bool EqualTo(const Slot* slot) const {
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

    template<class O>
    struct ConstSlot: public Slot {
        typedef R (O::*fn_t)(P...) const;

        ConstSlot(fn_t f, const O* o): fn(f), obj(o) { }

        virtual char Type() const { return 'c'; }

        virtual R Invoke(P... p) const { return (obj->*fn)(p...); }

        virtual const void* Object() const { return obj; }

        virtual bool EqualTo(const Slot* slot) const {
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

    struct FnSlot: public Slot {
        typedef std::function<R (P...)> fn_t;

        explicit FnSlot(const fn_t& f): fn(f) { }

        virtual char Type() const { return 'f'; }

        virtual R Invoke(P... p) const { return fn(p...); }

        virtual const void* Object() const { return nullptr; }

        virtual bool EqualTo(const Slot* slot) const { return (slot == this); }

    private:
        const fn_t fn;
    };

    template<class O>
    static const void* MakeSlot(O* o) {
        return o;
    }

    template<class O>
    static const void* MakeSlot(const O* o) {
        return o;
    }

    static std::unique_ptr<Slot> MakeSlot(R (*p)(P...)) {
        return std::make_unique<PtrSlot>(p);
    }

    template<class O>
    static std::unique_ptr<Slot> MakeSlot(R (O::*f)(P...), O* o) {
        return std::make_unique<MemSlot<O>>(f, o);
    }

    template<class O>
    static std::unique_ptr<Slot> MakeSlot(R (O::*f)(P...) const, const O* o) {
        return std::make_unique<ConstSlot<O>>(f, o);
    }

    template<class F>
    static std::unique_ptr<Slot> MakeSlot(const F& f) {
        return std::make_unique<FnSlot>(f);
    }
};

}

