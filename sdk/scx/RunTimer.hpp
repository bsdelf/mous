#ifndef SCX_RUNTIMER_HPP
#define SCX_RUNTIMER_HPP

#include <iostream>
#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
using namespace std;

#define SCX_BENCH(tag, times, loops, i, j,	    \
	onLoop,					    \
	beforeStart, afterStart,		    \
	beforeStop, afterStop)			    \
{					            \
    scx::RunTimer scx_timer;			    \
    double scx_sum = 0;				    \
    for (int i = 0; i < times; ++i) {		    \
	beforeStart;				    \
	scx_timer.Start();			    \
	afterStart;				    \
	for (int j = 0; j < loops; ++j) {	    \
	    onLoop;				    \
	}					    \
	beforeStop;				    \
	scx_timer.Stop();			    \
	scx_sum += scx_timer.DiffMS();		    \
	afterStop;				    \
    }						    \
    cout << tag << (scx_sum / times) << endl;	    \
}; class __END__

#define SCX_BENCH0(tag, times, count, onLoop)	\
    SCX_BENCH(tag, times, count, i, j,		\
	    onLoop,				\
	    , ,					\
	    , )			    

#define SCX_BENCH1(tag, times, count, onLoop,	\
	beforeStart)				\
    SCX_BENCH(tag, times, count, i, j,		\
	    onLoop,				\
	    beforeStart, ,			\
	    , )

#define SCX_BENCH4(tag, times, count, onLoop,	\
	afterStop)				\
    SCX_BENCH(tag, times, count, i, j,		\
	    onLoop,				\
	    , ,					\
	    , afterStop)

#define SCX_BENCH14(tag, times, count, onLoop,	\
	beforeStart, afterStop)			\
    SCX_BENCH(tag, times, count, i, j,		\
	    onLoop,				\
	    beforeStart, ,			\
	    , afterStop)


namespace scx {

class RunTimer {
public:
    void Start() {
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	mBegin.tv_sec = mts.tv_sec;
	mBegin.tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_MONOTONIC_PRECISE, &mBegin);
#endif
    }

    void Stop() {
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	mEnd.tv_sec = mts.tv_sec;
	mEnd.tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_MONOTONIC_PRECISE, &mEnd);
#endif
    }

    void PrintMS() const {
	cout << (time_t)DiffMS() << endl;
    }

    void PrintUS() const {
	cout << (time_t)DiffUS() << endl;
    }

    double DiffMS() const {
	return DiffUS() / 1000.f;
    }

    double DiffUS() const {
	return (mEnd.tv_sec - mBegin.tv_sec)*1000000.f + (mEnd.tv_nsec - mBegin.tv_nsec)/1000.f;
    }

private:
    timespec mBegin;
    timespec mEnd;
};

}

#endif
