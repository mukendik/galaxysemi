#ifndef GTL_PROFILE_H
#define GTL_PROFILE_H

#include <time.h> // timespec
#if defined(__MACH__)
# include <mach/mach_time.h>
#endif

//! \brief Profiling/performance measurement
struct tagGTL_PerfInfo
{
    long mTestTotalTime;
    unsigned mTotalNumOfTestCall;
    long mMPTestTotalTime;
    unsigned mTotalNumOfMPTestCall;
    long mBinningTotalTime;
    unsigned mTotalNumOfBinningCall;
    long mBeginJobTotalTime;
    unsigned mTotalNumOfBeginJobCall;
};
typedef struct tagGTL_PerfInfo GTL_PerfInfo;
extern GTL_PerfInfo gtl_stPerfInfo;

// todo : test struct timespec lTS1;
#define CONCAT(A, B) A ## B
#define RESOLVE_MACRO(x) x
#define GET_STRING(x) CONCAT( , x)
// gettimeofday(&lTS1, 0) is supposed to give nsec resolution but no way on win32...
// OS X does not have clock_gettime, but use clock_get_time
// http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
// todo: try:
    //clock_serv_t cclock; mach_timespec_t mts; host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    //clock_get_time(cclock, &mts); mach_port_deallocate(mach_task_self(),cclock); lTS1.tv_sec = mts.tv_sec;
    //lTS1.tv_nsec = mts.tv_nsec;

// function to increment the total time for function lFN
extern void IncFunc(const char* lFN, unsigned long long lInc);
extern long long diff2nsec(struct timespec start, struct timespec end);
extern struct timespec diff2timespec(struct timespec start, struct timespec end);
extern int gtl_LogPerfInfo(int lSev);
extern int gtl_EstimateCPUFreq();

#define PREPEND_PERFINFO(funcname) GET_STRING(funcname)TotalTime


#if defined(__CYGWIN__) && !defined(__MINGW32__)
    #define GTL_PROFILE_START
#elif defined(__MACH__)
    #define GTL_PROFILE_START unsigned long long lTS1 = mach_absolute_time();
#else
    #define GTL_PROFILE_START struct timespec lTS1; clock_gettime(CLOCK_MONOTONIC, &lTS1);
#endif

// todo : add a log at each GTL_PROFILE_END ?
// GTL_LOG(6, "%s lasts %ld sec %ld microsec", __func__, lDiff.tv_sec, lDiff.tv_nsec/1000);

// && defined(GTLDEBUG)

#if defined(__CYGWIN__) && !defined(__MINGW32__)
    #define GTL_PROFILE_STOP
#elif defined(__MACH__)
    #define GTL_PROFILE_STOP unsigned long long lTS2 = mach_absolute_time(); \
        long long result_in_ns = lTS2 - lTS1; \
        IncFunc( __func__, (unsigned long long) result_in_ns);
#else
    #define GTL_PROFILE_STOP  struct timespec lTS2; \
        clock_gettime(CLOCK_MONOTONIC, &lTS2); \
        long long result_in_ns=diff2nsec(lTS1, lTS2); \
        IncFunc( __func__, (unsigned long long)result_in_ns);
#endif

/* // timeval implementation: buggy: in win32, usec = 900000 most of the time
#define GTL_PROFILE_START struct timeval lTV1; gettimeofday(&lTV1, 0);
#define GTL_PROFILE_STOP  struct timeval lTV2; gettimeofday(&lTV2, 0); \
    struct timeval lDiff; timeval_subtract(&lDiff, &lTV1, &lTV2); \
    GTL_LOG(6, "%s lasts %ld sec %ld usec", __func__, lDiff.tv_sec, lDiff.tv_usec);
    //RESOLVE_MACRO(gtl_stPerfInfo.mTestTotalTime__func__+=lTV2.tv_usec-lTV1.tv_usec;
*/

/* C++
#define GTL_PROFILE_START clock_t lStartClock=clock();
#define GTL_PROFILE_STOP  clock_t lEndClock=clock(); \
    GTL_LOG(5, "%s last %ld msec", __func__, lEndClock-lStartClock );
    //gtl_stPerfInfo.mTotalTime__func__+=...
*/

#endif // GTL_PROFILE_H
