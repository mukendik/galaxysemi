#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h> // for clock_....
#include <limits.h>
//#include <limits> // C++
#include <math.h>
#include <gtl_core.h>
#include <../gtl-core/gtl_profile.h>

extern int gtl_LogPerfInfo(int lSev);

int time_test()
{
    printf("time() test\n");

    GTL_PROFILE_START

    time_t start_time;
    time(&start_time);
    time_t current_time;
    time(&current_time);

    while( current_time-start_time<3)
    {
       time(&current_time);
    }

    GTL_PROFILE_STOP

    printf("According to time(), %f secs\n",
           difftime(current_time, start_time));
    printf("According to GTL profiler, %llu ns \n", result_in_ns );

    unsigned long long result_in_sec=result_in_ns/1000000000;
    return (result_in_sec>3 || result_in_sec<2 )?-1:1;
}

void printf_test()
{
    printf("printf test:\n");
    GTL_PROFILE_START
    printf("My super greedy printf test. On my win32 debug, usually about 200us\n");
    GTL_PROFILE_STOP
    printf("According to GTL profiler, %llu ns \n", result_in_ns );
}

void fopen_test()
{
    printf("fopen test on an unexisting file. Usually 100us on win32...\n");
    GTL_PROFILE_START
    FILE* f=fopen("some_unexisting_file_for_sure.xxx", "r");
    if (f)
    {
        fclose(f); // hardly believable but anyway
    }
    GTL_PROFILE_STOP
    printf("According to GTL profiler, %llu ns \n", result_in_ns );
}

void clock_test()
{
    printf("\nclock() test: CLOCKS_PER_SEC=%ld sizeof(clock_t)=%zd\n",
           (long) CLOCKS_PER_SEC, sizeof(clock_t));
    clock_t start, end;
    int temp = 1024;
    start = clock();
    for (int i = 0; i<2500000 ; i++)
    {
        temp+=temp;
    }
    end = clock();
    printf("clock() test: %ld\n", end-start);
}

void sqrt_test()
{
    printf("sqrt test: usually about 1us on my win32 2GHz CPU, but up to 4us on our CentOS linux64 QA machine...\n");
    double a;
    GTL_PROFILE_START
    a = sqrt(99.);
    GTL_PROFILE_STOP
    if (a < 0)
    {  
        printf("error\n");
    }
    printf("According to GTL profiler: %llu ns \n", result_in_ns );
}

void rand_test()
{
    printf("rand test: usually about 1us on win32...\n");
    GTL_PROFILE_START
    rand();
    GTL_PROFILE_STOP
    printf("According to GTL profiler, %llu ns \n", result_in_ns );
}

void addition_test()
{
    printf("Double addition test: usually 1us on win32...\n");
    double a;
    GTL_PROFILE_START
        a = 1.5 + 1.5;
    GTL_PROFILE_STOP
    if (a < 0)
    {
        printf("error\n");
    }
    printf("According to GTL profiler: %llu ns \n", result_in_ns );
}

int main(int argc, char *argv[])
{
    printf("GTL profiler unit test 1 (argc=%d argv[0]=%s):\n", argc, argv[0]);

    printf("sizeof timespec (8bytes on win32) : %zd\n",
           sizeof(struct timespec));
    printf("sizeof time_t: %zd\n", sizeof(time_t));
    printf("min and max long: %ld %ld\n", LONG_MIN, LONG_MAX);
    printf("max ulong: %lu\n", ULONG_MAX);

    #ifdef __STDC__
        printf("__STDC__ defined.\n");
    #endif

    #ifdef __cplusplus
        printf("Compiled with __cplusplus\n");
    #endif

    #ifdef __STDC_VERSION__
        printf("__STDC_VERSION__ defined.\n");
    #endif
    printf("max ulonglong: %llu\n", ULLONG_MAX); // under linux ULLONG_MAX ? ULONG_LONG_MAX = windows

    #ifdef _POSIX
        printf("_POSIX defined\n");
    #endif

    #ifdef _TIMESPEC_DEFINED
        printf("_TIMESPEC_DEFINED\n");
    #endif

    //clock_test(); // dont understand why win32 returns 8...

    printf("\n");
    if (time_test()<0)
    {
        printf("time test failed\n");
        return EXIT_FAILURE;
    }

    printf("\n");
    printf_test();

    printf("\n");
    fopen_test();

    printf("\n");
    rand_test();

    printf("\n");
    sqrt_test();

    printf("\n");
    addition_test();

    printf("Success\n");

    return EXIT_SUCCESS;
}
