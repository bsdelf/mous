#pragma once

#include <iostream>
#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#define SCX_BENCH(tag, times, loops, i, j, onLoop, beforeStart, afterStart, beforeStop, afterStop)                             \
    {                                                                                                                          \
        scx::RunTimer scx_timer;                                                                                               \
        double scx_sum = 0;                                                                                                    \
        for (int i = 0; i < times; ++i) {                                                                                      \
            beforeStart;                                                                                                       \
            scx_timer.Start();                                                                                                 \
            afterStart;                                                                                                        \
            for (int j = 0; j < loops; ++j) {                                                                                  \
                onLoop;                                                                                                        \
            }                                                                                                                  \
            beforeStop;                                                                                                        \
            scx_timer.Stop();                                                                                                  \
            scx_sum += scx_timer.DiffMS();                                                                                     \
            afterStop;                                                                                                         \
        }                                                                                                                      \
        std::cout << tag << (scx_sum / times) << "ms" << std::endl;                                                            \
    };                                                                                                                         \
    class __END__

#define SCX_BENCH0(tag, times, count, onLoop) SCX_BENCH(tag, times, count, i, j, onLoop, , , , )

#define SCX_BENCH1(tag, times, count, onLoop, beforeStart) SCX_BENCH(tag, times, count, i, j, onLoop, beforeStart, , , )

#define SCX_BENCH4(tag, times, count, onLoop, afterStop) SCX_BENCH(tag, times, count, i, j, onLoop, , , , afterStop)

#define SCX_BENCH134(tag, times, count, onLoop, beforeStart, beforeStop, afterStop)                                            \
    SCX_BENCH(tag, times, count, i, j, onLoop, beforeStart, , beforeStop, afterStop)

#define SCX_BENCH14(tag, times, count, onLoop, beforeStart, afterStop)                                                         \
    SCX_BENCH(tag, times, count, i, j, onLoop, beforeStart, , , afterStop)

#define SCX_BENCH13(tag, times, count, onLoop, beforeStart, beforeStop)                                                        \
    SCX_BENCH(tag, times, count, i, j, onLoop, beforeStart, , beforeStop, )

namespace scx {

class RunTimer
{
  public:
    RunTimer()
      : m_Started(false)
    {
    }

    void Start()
    {
        CurrentTimeSpec(m_Begin);
        m_Started = true;
    }

    void Stop()
    {
        CurrentTimeSpec(m_End);
        m_Started = false;
    }

    void PrintMS() const { std::cout << (time_t)DiffMS() << std::endl; }

    void PrintUS() const { std::cout << (time_t)DiffUS() << std::endl; }

    double DiffMS() const { return DiffUS() / 1000.f; }

    double DiffUS() const { return (m_End.tv_sec - m_Begin.tv_sec) * 1000000.f + (m_End.tv_nsec - m_Begin.tv_nsec) / 1000.f; }

    double DiffCurrentMS() const { return DiffCurrentUS() / 1000.f; }

    double DiffCurrentUS() const
    {
        struct timespec end;
        CurrentTimeSpec(end);
        return (end.tv_sec - m_Begin.tv_sec) * 1000000.f + (end.tv_nsec - m_Begin.tv_nsec) / 1000.f;
    }

    bool Started() const { return m_Started; }

  public:
    static void CurrentTimeSpec(struct timespec& ts)
    {
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
    }

  private:
    timespec m_Begin;
    timespec m_End;
    bool m_Started;
};
}
