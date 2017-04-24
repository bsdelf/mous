#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

namespace scx {

template<typename... Contents>
class Mailbox
{
  public:
    using Mail = std::tuple<Contents..., std::weak_ptr<Mailbox>>;

  public:
    Mailbox() = default;
    ~Mailbox() = default;

    Mailbox(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;

    Mailbox& operator=(const Mailbox&) = delete;
    Mailbox& operator=(Mailbox&&) = delete;

    template<typename T>
    void PushBack(T&& mail)
    {
        std::lock_guard<std::mutex> locker(_mutex);
        _queue.emplace_front(std::forward<T>(mail));
        _condition.notify_all();
    }

    template<typename T>
    void PushFront(T&& mail)
    {
        std::lock_guard<std::mutex> locker(_mutex);
        _queue.emplace_back(std::forward<T>(mail));
        _condition.notify_all();
    }

    auto Take()
    {
        std::unique_lock<std::mutex> locker(_mutex);
        _condition.wait(locker, [this] { return not _queue.empty(); });
        auto mail = std::move(_queue.back());
        _queue.pop_back();
        return mail;
    }

    auto TryTake()
    {
        std::unique_lock<std::mutex> locker(_mutex);
        if (_queue.empty()) {
            return Mail();
        }
        auto mail = std::move(_queue.back());
        _queue.pop_back();
        return mail;
    }

    template<typename T>
    auto ExchangeBack(T&& mail)
    {
        auto box = std::make_shared<Mailbox>();
        std::get<std::tuple_size<Mail>::value - 1>(mail) = box;
        PushBack(std::forward<T>(mail));
        return box->Take();
    }

    template<typename T>
    auto ExchangeFront(T&& mail)
    {
        auto box = std::make_shared<Mailbox>();
        std::get<std::tuple_size<Mail>::value - 1>(mail) = box;
        PushFront(std::forward<T>(mail));
        return box->Take();
    }

    void Wait(size_t n)
    {
        std::unique_lock<std::mutex> locker(_mutex);
        _condition.wait(locker, [this, n] { return _queue.size() >= n; });
    }

    void Clear()
    {
        std::lock_guard<std::mutex> locker(_mutex);
        _queue.clear();
    }

    void Shrink()
    {
        std::lock_guard<std::mutex> locker(_mutex);
        _queue.shrink_to_fit();
    }

    auto Size() const { return _queue.size(); }

    auto Empty() const { return _queue.empty(); }

  private:
    std::mutex _mutex;
    std::condition_variable _condition;
    std::deque<Mail> _queue;
};
}
