#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>
#include <vector>

namespace scx {

class Periodic
{
  private:
    // TODO: use std::scoped_lock in the future
    using MutexLockGuard = std::lock_guard<std::mutex>;

    using MutexUniqueLock = std::unique_lock<std::mutex>;

    class Task
    {
      public:
        Task() = default;

        template<class F>
        Task(F&& callback, int interval, bool oneshot)
          : callback_(std::forward<F>(callback))
          , interval_(interval)
          , oneshot_(oneshot)
          , deadline_(std::chrono::steady_clock::now() + interval_)
        {
        }

        void Run()
        {
            callback_();
            deadline_ += interval_;
        }

        bool Oneshot() const { return oneshot_; }

        auto Deadline() const -> const std::chrono::steady_clock::time_point& { return deadline_; }

        static bool Compare(const std::unique_ptr<Task>& a, const std::unique_ptr<Task>& b)
        {
            return a->Deadline() > b->Deadline();
        }

      private:
        const std::function<void(void)> callback_;
        const std::chrono::milliseconds interval_;
        const bool oneshot_;
        std::chrono::steady_clock::time_point deadline_;
    };

    using TaskList = std::vector<std::unique_ptr<Task>>;

    // TODO: consider to use std::variant in the future
    struct Action
    {
        Action(std::unique_ptr<Task>&& new_task)
          : new_task(std::move(new_task))
        {
        }
        Action(Task* obsoleted_task)
          : obsoleted_task(obsoleted_task)
        {
        }

        Action(Action&& that)
        {
            new_task = std::move(that.new_task);
            obsoleted_task = that.obsoleted_task;
        }

        Action() = default;

        std::unique_ptr<Task> new_task;
        Task* obsoleted_task = nullptr;
    };

    using ActionList = std::vector<Action>;

  public:
    Periodic(Periodic&&) = delete;
    Periodic(const Periodic&) = delete;

    Periodic& operator=(Periodic&&) = delete;
    Periodic& operator=(const Periodic&) = delete;

    Periodic()
    {
        using namespace std::chrono_literals;

        thread_ = std::thread([this] {
            while (!to_exit_) {
                // wait until deadline or pending actions occur
                {
                    MutexUniqueLock pending_lock(pending_mutex_);

                    auto pred = [this] { return !pending_actions_.empty(); };

                    MutexUniqueLock active_lock(active_mutex_);
                    if (active_tasks_.empty()) {
                        active_lock.unlock();
                        active_condition_.notify_all();
                        pending_condition_.wait(pending_lock, std::move(pred));
                    } else {
                        const auto& deadline = active_tasks_.front()->Deadline();
                        active_lock.unlock();
                        pending_condition_.wait_until(pending_lock, deadline, std::move(pred));
                    }
                }

                // clear pending actions and active tasks
                if (to_clear_) {
                    pending_mutex_.lock();
                    pending_actions_.clear(); // noexcept
                    pending_mutex_.unlock();
                    pending_condition_.notify_all();

                    active_mutex_.lock();
                    active_tasks_.clear(); // noexcept
                    active_mutex_.unlock();
                    active_condition_.notify_all();

                    to_clear_ = false;
                    continue;
                }

                // exit after potential clear
                if (to_exit_) {
                    break;
                }

                MutexLockGuard lock(active_mutex_);

                // proceed pending actions
                {
                    bool should_rebuild = false;

                    {
                        MutexLockGuard lock(pending_mutex_);

                        for (size_t ip = 0; ip < pending_actions_.size(); ++ip) {
                            auto& action = pending_actions_[ip];
                            if (action.new_task) {
                                should_rebuild = true;
                                active_tasks_.push_back(std::move(action.new_task));
                            } else if (action.obsoleted_task) {
                                for (size_t ia = 0; ia < active_tasks_.size(); ++ia) {
                                    if (active_tasks_[ia].get() == action.obsoleted_task) {
                                        should_rebuild = true;
                                        active_tasks_.erase(active_tasks_.begin() + ia);
                                        break;
                                    }
                                }
                            }
                        }

                        pending_actions_.clear();
                    }

                    pending_condition_.notify_all();

                    if (should_rebuild && !std::is_heap(active_tasks_.begin(), active_tasks_.end(), Task::Compare)) {
                        std::make_heap(active_tasks_.begin(), active_tasks_.end(), Task::Compare);
                    }
                }

                // proceed expired tasks
                const auto max = active_tasks_.size();

                for (size_t i = 0; i < max; ++i) {
                    const auto& task = active_tasks_.front();

                    if (task->Deadline() > std::chrono::steady_clock::now()) {
                        break;
                    }

                    task->Run();

                    std::pop_heap(active_tasks_.begin(), active_tasks_.end() - i, Task::Compare);

                    if (task->Oneshot()) {
                        active_tasks_.erase(active_tasks_.end() - i - 1);
                    }
                }

                if (!std::is_heap(active_tasks_.begin(), active_tasks_.end(), Task::Compare)) {
                    std::make_heap(active_tasks_.begin(), active_tasks_.end(), Task::Compare);
                }
            }
        });
    }

    ~Periodic()
    {
        to_exit_ = true;

        // wake up thread
        {
            MutexLockGuard lock(pending_mutex_);
            pending_actions_.emplace_back();
        }
        pending_condition_.notify_all();

        thread_.join();
    }

    template<class F>
    uintptr_t Add(F&& callback, int interval, bool oneshot = false)
    {
        auto new_task = std::make_unique<Task>(std::forward<F>(callback), interval, oneshot);
        auto id = reinterpret_cast<uintptr_t>(new_task.get());

        {
            MutexLockGuard lock(pending_mutex_);
            pending_actions_.emplace_back(std::move(new_task));
        }

        pending_condition_.notify_all();

        return id;
    }

    void Remove(uintptr_t id)
    {
        auto obsoleted_task = reinterpret_cast<Task*>(id);

        {
            MutexLockGuard lock(pending_mutex_);
            pending_actions_.emplace_back(obsoleted_task);
        }

        pending_condition_.notify_all();
    }

    void Clear() { to_clear_ = true; }

    void WaitUntilEmpty()
    {
        {
            MutexUniqueLock lock(pending_mutex_);
            pending_condition_.wait(lock, [this] { return pending_actions_.empty(); });
        }

        {
            MutexUniqueLock lock(active_mutex_);
            active_condition_.wait(lock, [this] { return active_tasks_.empty(); });
        }
    }

    size_t Size()
    {
        MutexLockGuard lock(active_mutex_);
        return active_tasks_.size();
    }

    bool Empty()
    {
        MutexLockGuard lock(active_mutex_);
        return active_tasks_.empty();
    }

  private:
    std::thread thread_;
    std::atomic_bool to_exit_{ false };

    std::mutex active_mutex_;
    std::condition_variable active_condition_;
    TaskList active_tasks_;

    std::mutex pending_mutex_;
    std::condition_variable pending_condition_;
    ActionList pending_actions_;

    std::atomic_bool to_clear_{ false };
};

} // namespace scx
