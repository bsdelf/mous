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
    using MutexLockGuard = std::lock_guard<std::mutex>;

    using MutexUniqueLock = std::unique_lock<std::mutex>;

    class Task
    {
      public:
        Task() = default;

        template<class F>
        Task(F&& callback, int interval, bool oneshot)
          : _callback(std::forward<F>(callback))
          , _interval(interval)
          , _oneshot(oneshot)
          , _deadline(std::chrono::steady_clock::now() + _interval)
        {
        }

        void Run()
        {
            _callback();
            _deadline += _interval;
        }

        bool Oneshot() const { return _oneshot; }

        auto Deadline() const -> const std::chrono::steady_clock::time_point& { return _deadline; }

        static bool Compare(const std::unique_ptr<Task>& a, const std::unique_ptr<Task>& b)
        {
            return a->Deadline() > b->Deadline();
        }

      private:
        const std::function<void(void)> _callback;
        const std::chrono::milliseconds _interval;
        const bool _oneshot;
        std::chrono::steady_clock::time_point _deadline;
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

        _thread = std::thread([this] {
            while (!_to_exit) {
                MutexUniqueLock active_lock(_active_mutex);

                // wait until deadline or pending actions occur
                {
                    MutexUniqueLock pending_lock(_pending_mutex);

                    auto pred = [this] { return !_pending_actions.empty(); };
                    if (_active_tasks.empty()) {
                        active_lock.unlock();
                        _active_condition.notify_all();
                        _pending_condition.wait(pending_lock, std::move(pred));
                        active_lock.lock();
                    } else {
                        const auto& deadline = _active_tasks.front()->Deadline();
                        _pending_condition.wait_until(pending_lock, deadline, std::move(pred));
                    }

                    if (_to_clear) {
                        _pending_actions.clear();
                        pending_lock.unlock();
                        _pending_condition.notify_all();

                        _active_tasks.clear();
                        active_lock.unlock();
                        _active_condition.notify_all();

                        _to_clear = false;
                        continue;
                    }

                    if (_to_exit) {
                        break;
                    }

                    const bool should_rebuild = !_pending_actions.empty();

                    for (size_t ip = 0; ip < _pending_actions.size(); ++ip) {
                        auto& action = _pending_actions[ip];
                        if (action.new_task) {
                            _active_tasks.push_back(std::move(action.new_task));
                        } else if (action.obsoleted_task) {
                            for (size_t ia = 0; ia < _active_tasks.size(); ++ia) {
                                if (_active_tasks[ia].get() == action.obsoleted_task) {
                                    _active_tasks.erase(_active_tasks.begin() + ia);
                                    break;
                                }
                            }
                        }
                    }

                    _pending_actions.clear();
                    pending_lock.unlock();
                    _pending_condition.notify_all();

                    if (should_rebuild && !std::is_heap(_active_tasks.begin(), _active_tasks.end(), Task::Compare)) {
                        std::make_heap(_active_tasks.begin(), _active_tasks.end(), Task::Compare);
                    }
                }

                // proceed expired tasks
                const auto max = _active_tasks.size();
                for (size_t i = 0; i < max; ++i) {
                    const auto& task = _active_tasks.front();
                    const auto& now = std::chrono::steady_clock::now();
                    if (task->Deadline() > now) {
                        break;
                    }

                    task->Run();

                    std::pop_heap(_active_tasks.begin(), _active_tasks.end() - i, Task::Compare);

                    if (task->Oneshot()) {
                        _active_tasks.erase(_active_tasks.end() - i - 1);
                    }
                }
                if (!std::is_heap(_active_tasks.begin(), _active_tasks.end(), Task::Compare)) {
                    std::make_heap(_active_tasks.begin(), _active_tasks.end(), Task::Compare);
                }
            }
        });
    }

    ~Periodic()
    {
        _to_exit = true;

        // wake up thread
        {
            MutexLockGuard lock(_pending_mutex);
            _pending_actions.emplace_back();
        }
        _pending_condition.notify_all();

        _thread.join();
    }

    template<class F>
    uintptr_t Add(F&& callback, int interval, bool oneshot = false)
    {
        auto new_task = std::make_unique<Task>(std::forward<F>(callback), interval, oneshot);
        auto id = reinterpret_cast<uintptr_t>(new_task.get());

        {
            MutexLockGuard lock(_pending_mutex);
            _pending_actions.emplace_back(std::move(new_task));
        }

        _pending_condition.notify_all();

        return id;
    }

    void Remove(uintptr_t id)
    {
        auto obsoleted_task = reinterpret_cast<Task*>(id);

        {
            MutexLockGuard lock(_pending_mutex);
            _pending_actions.emplace_back(obsoleted_task);
        }

        _pending_condition.notify_all();
    }

    void Clear() { _to_clear = true; }

    void WaitUntilEmpty()
    {
        {
            MutexUniqueLock lock(_pending_mutex);
            _pending_condition.wait(lock, [this] { return _pending_actions.empty(); });
        }

        {
            MutexUniqueLock lock(_active_mutex);
            _active_condition.wait(lock, [this] { return _active_tasks.empty(); });
        }
    }

    size_t Size()
    {
        MutexLockGuard lock(_active_mutex);
        return _active_tasks.size();
    }

    bool Empty()
    {
        MutexLockGuard lock(_active_mutex);
        return _active_tasks.empty();
    }

  private:
    std::thread _thread;
    std::atomic_bool _to_exit{ false };

    std::mutex _active_mutex;
    std::condition_variable _active_condition;
    TaskList _active_tasks;

    std::mutex _pending_mutex;
    std::condition_variable _pending_condition;
    ActionList _pending_actions;

    std::atomic_bool _to_clear{ false };
};

} // namespace scx
