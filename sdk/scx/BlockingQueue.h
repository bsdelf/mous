#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>

namespace scx {

template<typename T>
class BlockingQueue
{
  public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue(BlockingQueue&&) = delete;

    BlockingQueue& operator=(const BlockingQueue&) = delete;
    BlockingQueue& operator=(BlockingQueue&&) = delete;

    template<typename Arg>
    void PushBack(Arg&& data)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.push_back(std::forward<Arg>(data));
        condition_.notify_all();
    }

    template<typename... Args>
    void EmplaceBack(Args&&... args)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.emplace_back(std::forward<Args>(args)...);
        condition_.notify_all();
    }

    template<typename Arg>
    void PushFront(Arg&& data)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        queue_.push_front(std::forward<Arg>(data));
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
        auto data = std::move(queue_.front());
        queue_.pop_front();
        return data;
    }

    auto TryTake()
    {
        std::unique_lock<std::mutex> locker(mutex_);
        if (queue_.empty()) {
            return T{};
        }
        auto data = std::move(queue_.front());
        queue_.pop_front();
        return data;
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
    std::deque<T> queue_;
};
}
