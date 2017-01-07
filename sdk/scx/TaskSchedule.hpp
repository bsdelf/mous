#pragma once

#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include <deque>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace scx {

class TaskSchedule
{
private:
    struct Task
    {
        Task() = default;

        explicit Task(const std::function<void (void)>& c):
            callback(c)
        { }

        volatile bool canceled = false;
        volatile bool oneshot = true;
        const std::function<void (void)> callback;

        struct timeval interval = { 0, 0 };
        struct timeval attime = { 0, 0 };

        static bool IsEarlier(const Task* a, const Task* b)
        {
            return timercmp(&a->attime, &b->attime, <);
        }

        static bool NotLater(const Task* a, const Task* b)
        {
            return timercmp(&a->attime, &b->attime, <=);
        }
    };

    typedef std::deque<Task*> TaskList;

public:
    TaskSchedule() = default;
    TaskSchedule(const TaskSchedule&) = delete;
    TaskSchedule& operator=(const TaskSchedule&) = delete;

    ~TaskSchedule()
    {
        Stop();
        Clear();
    }

    void Start()
    {
        // avoid callback storm
        m_pendings.insert(m_pendings.begin(), m_tasks.begin(), m_tasks.end());
        m_tasks.clear();

        m_work = true;
        m_cancel = false;

        m_thread = std::thread([this] {
            Task dummy;
            while (m_work) {
                struct timeval tv = { 0L, 2L };
                if (::select(0, nullptr, nullptr, nullptr, &tv) == 0) {
                    // pick up expired tasks
                    dummy.attime = CurrentTimeVal();
                    auto end = std::lower_bound(
                        m_tasks.begin(), m_tasks.end(), &dummy, Task::NotLater);
                    TaskList expired(m_tasks.begin(), end);
                    m_tasks.erase(m_tasks.begin(), end);

                    for (Task* task: expired) {
                        if (!m_work)
                            return;

                        if (!task->canceled && !m_cancel)
                            task->callback();

                        if (task->oneshot || task->canceled || m_cancel) {
                            delete task;
                            --m_count;
                            std::lock_guard<std::mutex> elocker(m_emutex);
                            if (m_tasks.empty() && m_pendings.empty())
                                m_econd.notify_all();
                        } else {
                            RefreshTask(task);
                            InsertTask(task);
                        }
                    }
                } else {
                    // fatal error occurs
                    break;
                }

                // take all pendings
                std::lock_guard<std::mutex> plocker(m_pmutex);
                for (Task* task: m_pendings) {
                    RefreshTask(task);
                    InsertTask(task);
                }
                m_pendings.clear();
            }
        });
    }

    void Stop()
    {
        m_work = false;
        if (m_thread.joinable())
            m_thread.join();
    }

    bool IsRunning() const
    {
        return m_work;
    }

    long Add(const std::function<void (void)>& fn, int ms, bool oneshot = false)
    {
        Task* task = new Task(fn);
        task->oneshot = oneshot;
        task->interval.tv_sec = ms / 1000L;
        task->interval.tv_usec = ms % 1000L * 1000L;

        ++m_count;

        m_pmutex.lock();
        m_pendings.push_back(task);
        m_pmutex.unlock();

        return reinterpret_cast<long>(task);
    }

    /* async cancel(destroy) task */
    void Cancel(long key)
    {
        Task* task = reinterpret_cast<Task*>(key);
        if (task != nullptr) {
            task->canceled = true;
        }
    }

    /* async cancel(destroy) tasks */
    void Cancel()
    {
        m_cancel = true;
    }

    /* wait for all tasks be destroyed */
    void Wait()
    {
        std::unique_lock<std::mutex> locker(m_emutex);
        while (!m_tasks.empty() || !m_pendings.empty())
            m_econd.wait(locker);
    }

    /* destroy all tasks immediately */
    void Clear()
    {
        bool work = IsRunning();
        Stop();
        for (Task* task: m_tasks)
            delete task;
        for (Task* task: m_pendings)
            delete task;
        if (work)
            Start();
    }

    /* count of tasks still alive(not destroyed yet) */
    int Count() const
    {
        return m_count;
    }

private:
    static inline struct timeval CurrentTimeVal()
    {
#ifdef __MACH__
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
        return { mts.tv_sec, mts.tv_nsec / 1000 };
#else
        struct timespec ts = { 0L, 0L };
        ::clock_gettime(CLOCK_MONOTONIC, &ts);
        return { ts.tv_sec, ts.tv_nsec / 1000L };
#endif
    }

    void inline RefreshTask(Task* task)
    {
        struct timeval tv = CurrentTimeVal();
        timeradd(&tv, &task->interval, &task->attime);
    }

    void inline InsertTask(Task* task)
    {
        auto pos = std::lower_bound(
            m_tasks.begin(), m_tasks.end(), task, Task::IsEarlier);
        m_tasks.insert(pos, task);
    }

private:
    volatile bool m_work = false;
    volatile bool m_cancel = false;

    std::thread m_thread;
    std::atomic_int m_count;
    TaskList m_tasks;
    TaskList m_pendings;
    std::mutex m_pmutex;

    std::mutex m_emutex;
    std::condition_variable m_econd;
};

}

