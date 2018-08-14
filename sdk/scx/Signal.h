#pragma once

#include <cinttypes>
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace scx {

namespace signal {

template<class>
class MuteSignal;

template<class R, class... P>
class MuteSignal<R(P...)>
{
    using RawFunction = R (*)(P...);

  private:
    struct Slot;

  public:
    MuteSignal() = default;
    ~MuteSignal() = default;

    MuteSignal(const MuteSignal&) = delete;
    MuteSignal(MuteSignal&&) = delete;
    MuteSignal& operator=(const MuteSignal&) = delete;
    MuteSignal& operator=(MuteSignal&&) = delete;

    template<class... Args>
    uintptr_t Connect(Args&&... args)
    {
        auto slot = MakeSlot(std::forward<Args>(args)...);
        auto id = reinterpret_cast<uintptr_t>(slot.get());
        _slots.push_back(std::move(slot));
        return id;
    }

    bool Disconnect(uintptr_t id)
    {
        for (size_t i = 0; i < _slots.size(); ++i) {
            auto thatId = reinterpret_cast<uintptr_t>(_slots[i].get());
            if (thatId == id) {
                _slots.erase(_slots.begin() + i);
                return true;
            }
        }
        return false;
    }

    bool Disconnect(RawFunction fn)
    {
        bool hit = false;
        EraseIf([&hit, fn](const auto& slot) {
            if (slot->type == SlotType::RawFunction) {
                auto that = static_cast<RawFunctionSlot*>(slot.get());
                if (that->fn == fn) {
                    hit = true;
                    return true;
                }
            }
            return false;
        });
        return hit;
    }

    template<class O>
    bool Disconnect(O* obj)
    {
        bool hit = false;
        EraseIf([&hit, obj](const auto& slot) {
            if (slot->type == SlotType::MemberFunction) {
                auto that = dynamic_cast<MemberFunctionSlot<O>*>(slot.get());
                if (that && that->obj == obj) {
                    hit = true;
                    return true;
                }
            }
            return false;
        });
        return hit;
    }

    template<class F, class O>
    bool Disconnect(F fn, O* obj)
    {
        bool hit = false;
        EraseIf([&hit, fn, obj](const auto& slot) {
            if (slot->type == SlotType::MemberFunction) {
                auto that = dynamic_cast<SignedMemberFunctionSlot<F, O>*>(slot.get());
                if (that && that->obj == obj && that->fn == fn) {
                    hit = true;
                    return true;
                }
            }
            return false;
        });
        return hit;
    }

    void Clear() { _slots.clear(); }

    auto Empty() const { return _slots.empty(); }

    auto Size() const { return _slots.size(); }

  protected:
    std::vector<std::unique_ptr<Slot>> _slots;

  private:
    template<typename Pred>
    void EraseIf(Pred&& pred)
    {
        _slots.erase(std::remove_if(_slots.begin(), _slots.end(), std::forward<Pred>(pred)), _slots.end());
    }

    enum class SlotType : uint8_t
    {
        Function,
        RawFunction,
        MemberFunction
    };

    struct Slot
    {
        explicit Slot(SlotType type)
          : type(type)
        {
        }

        virtual ~Slot() {}
        virtual R Invoke(P...) const = 0;

        const SlotType type;
    };

    template<class F>
    struct FunctionSlot : public Slot
    {
        FunctionSlot(const F& fn)
          : Slot(SlotType::Function)
          , fn(fn)
        {
        }

        FunctionSlot(F&& fn)
          : Slot(SlotType::Function)
          , fn(std::move(fn))
        {
        }

        R Invoke(P... p) const final { return fn(p...); }

        F const fn;
    };

    struct RawFunctionSlot : public Slot
    {
        explicit RawFunctionSlot(RawFunction fn)
          : Slot(SlotType::RawFunction)
          , fn(fn)
        {
        }

        R Invoke(P... p) const final { return fn(p...); }

        RawFunction const fn;
    };

    template<class O>
    struct MemberFunctionSlot : public Slot
    {
        explicit MemberFunctionSlot(O* obj)
          : Slot(SlotType::MemberFunction)
          , obj(obj)
        {
        }

        O* const obj;
    };

    template<class F, class O>
    struct SignedMemberFunctionSlot : public MemberFunctionSlot<O>
    {
        SignedMemberFunctionSlot(F fn, O* obj)
          : MemberFunctionSlot<O>(obj)
          , fn(fn)
        {
        }

        R Invoke(P... p) const final { return (this->obj->*fn)(p...); }

        F const fn;
    };

    static auto MakeSlot(RawFunction f) { return std::make_unique<RawFunctionSlot>(f); }

    template<class F, class O>
    static auto MakeSlot(F f, O* o)
    {
        return std::make_unique<SignedMemberFunctionSlot<F, O>>(f, o);
    }

    template<class F>
    static auto MakeSlot(F&& f)
    {
        return std::make_unique<FunctionSlot<std::decay_t<F>>>(std::forward<F>(f));
    }
};

template<class R>
class LastValueCollector
{
  public:
    void operator+=(const R& val) { this->val = val; }

    void operator+=(R&& val) { this->val = std::move(val); }

    R& operator()() { return val; }

  private:
    R val;
};

} // namespace signal

template<class, template<class> class Collector = signal::LastValueCollector>
class Signal;

template<class R, class... P, template<class> class Collector>
class Signal<R(P...), Collector> : public signal::MuteSignal<R(P...)>
{
  public:
    R operator()(P... p) const
    {
        Collector<R> collector;
        for (size_t i = 0; i < this->_slots.size(); ++i) {
            collector += this->_slots[i]->Invoke(p...);
        }
        return std::move(collector());
    }
};

template<class... P>
class Signal<void(P...)> : public signal::MuteSignal<void(P...)>
{
  public:
    void operator()(P... p) const
    {
        for (size_t i = 0; i < this->_slots.size(); ++i) {
            this->_slots[i]->Invoke(p...);
        }
    }
};

} // namespace scx
