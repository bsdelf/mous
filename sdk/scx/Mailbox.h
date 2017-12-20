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
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.push_back(std::forward<T>(mail));
        condition_.notify_all();
    }

    template<typename... Args>
    void EmplaceBack(Args&&... args)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.emplace_back(std::forward<Args>(args)...);
        condition_.notify_all();
    }

    template<typename T>
    void PushFront(T&& mail)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.push_front(std::forward<T>(mail));
        condition_.notify_all();
    }

    template<typename... Args>
    void EmplaceFront(Args&&... args)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.emplace_front(std::forward<Args>(args)...);
        condition_.notify_all();
    }

    auto Take()
    {
        std::unique_lock<std::mutex> locker(mutex_);
        condition_.wait(locker, [this] { return not queue_.empty(); });
        auto mail = std::move(queue_.front());
        queue_.pop_front();
        return mail;
    }

    auto TryTake()
    {
        std::unique_lock<std::mutex> locker(mutex_);
        if (queue_.empty()) {
            return Mail();
        }
        auto mail = std::move(queue_.front());
        queue_.pop_front();
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
        std::unique_lock<std::mutex> locker(mutex_);
        condition_.wait(locker, [this, n] { return queue_.size() >= n; });
    }

    void Clear()
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.clear();
    }

    void Shrink()
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.shrink_to_fit();
    }

    auto Size() const { return queue_.size(); }

    auto Empty() const { return queue_.empty(); }

  private:
    std::mutex mutex_;
    std::condition_variable condition_;
    std::deque<Mail> queue_;
};
}
