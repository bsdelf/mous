#ifndef SCX_TASKSCHEDULE_HPP
#define SCX_TASKSCHEDULE_HPP

#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#include <deque>
#include <algorithm>
#include <mutex>
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
        explicit Task(const std::function<void (void)>& _invoke):
            canceled(false),
            invoke(_invoke)
        { }

        std::mutex mtx;
        bool canceled;

        std::function<void (void)> invoke;
        bool oneshot;
        struct timeval intval;
        struct timeval attime;
    };

    typedef std::deque<Task*> TaskList;

    struct TaskCmp
    {
        bool operator()(const Task* a, const Task* b) const
        {
            return timercmp(&a->attime, &b->attime, <);
        }
    };

public:
    TaskSchedule():
        m_Work(false)
    { }

    ~TaskSchedule()
    {
        Stop(true);
    }

    // be careful with storm
    bool Start(bool reset = true)
    {
        if (reset) {
            m_Pending.insert(m_Pending.begin(), m_TaskList.begin(), m_TaskList.end());
            m_TaskList.clear();
        }
            
        m_Work = true;
        return pthread_create(
                &m_ThreadId, nullptr, 
                &TaskSchedule::OnThread, static_cast<void*>(this)) == 0;
    }

    // synchronized stop
    void Stop(bool clear = false)
    {
        m_Work = false;
        pthread_join(m_ThreadId, nullptr);

        if (clear) {
            for (size_t i = 0; i < m_TaskList.size(); ++i) {
                delete m_TaskList[i];
            }
            m_TaskList.clear();

            for (size_t i = 0; i < m_Pending.size(); ++i) {
                delete m_Pending[i];
            }
            m_Pending.clear();
        }
    }

    long Schedule(const std::function<void (void)>& fn, int ms, bool oneshot = false)
    {
        Task* task = new Task(fn);
        task->oneshot = oneshot;
        task->intval.tv_sec = ms / 1000L;
        task->intval.tv_usec = ms % 1000L * 1000L;
        task->attime.tv_sec = 0;
        task->attime.tv_usec = 0;

        std::unique_lock<std::mutex> locker(m_PendingMutex);
        m_Pending.push_back(task);
        locker.unlock();

        return reinterpret_cast<long>(task);
    }

    void Cancel(long key)
    {
        Task* task = reinterpret_cast<Task*>(key);
        if (task != nullptr) {
            std::lock_guard<std::mutex> locker(task->mtx);
            task->canceled = true;
        }
    }

    size_t Count() const
    {
        return m_TaskList.size();
    }

private:
    static inline struct timeval CurrentTimeVal()
    {
        struct timespec ts = { 0L, 0L };
#ifdef __MACH__
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
#else
        clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
        struct timeval tv = { ts.tv_sec, ts.tv_nsec / 1000L };
        return tv;
    }

    void inline UpdateTask(Task* task)
    {
        struct timeval tv = CurrentTimeVal();
        timeradd(&tv, &task->intval, &task->attime);
    }

    void inline InsertTask(Task* task)
    {
        TaskList::iterator pos = std::lower_bound(
                m_TaskList.begin(), m_TaskList.end(), task, TaskCmp());
        m_TaskList.insert(pos, task);
    }

    void DoOnThread()
    {
        while (m_Work) {
            struct timeval tv = { 0L, 2L };
            if (select(0, nullptr, nullptr, nullptr, &tv) == 0) {
                struct timeval tv = CurrentTimeVal();

                size_t n = 0;
                for (n = 0; n < m_TaskList.size(); ++n) {
                    Task* task = m_TaskList[n];
                    if (timercmp(&task->attime, &tv, >))
                        break;
                }

                if (n > 0) {
                    TaskList todoList(m_TaskList.begin(), m_TaskList.begin() + n);
                    m_TaskList.erase(m_TaskList.begin(), m_TaskList.begin() + n);
                    for (size_t i = 0; i < n && m_Work; ++i) {
                        Task* task = todoList[i];

                        std::lock_guard<std::mutex> locker(task->mtx);

                        if (!task->canceled)
                            task->invoke();

                        if (task->oneshot || task->canceled) {
                            delete task;
                        } else {
                            UpdateTask(task);
                            InsertTask(task);
                        }
                    }
                }
            } else {
                break;
            }

            std::lock_guard<std::mutex> locker(m_PendingMutex);

            if (!m_Pending.empty()) {
                Task* task = m_Pending.front();
                UpdateTask(task);
                InsertTask(task);
                m_Pending.pop_front();
            }
        }
    }

    static void* OnThread(void* pThis)
    {
        TaskSchedule* ule = static_cast<TaskSchedule*>(pThis);
        ule->DoOnThread();
        return nullptr;
    }

private:
    bool m_Work;
    pthread_t m_ThreadId;
    TaskList m_TaskList;
    TaskList m_Pending;
    mutable std::mutex m_PendingMutex;
};

}

#endif
