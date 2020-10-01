#include <map>
#include <string>
#include "gstdl_jwsmtp.h"
#include <gstdl_systeminfo.h>

extern "C"
{
    #include "gtl_core.h"
    #include "gtl_profile.h"

    // <function, <time in us (max about 71min for unsigned long), call counter>>
    std::map<std::string, std::pair<unsigned long long, unsigned long> > sFunctionsCosts;

    // return total diff (secs and ns) in ns
    long long diff2nsec(struct timespec start, struct timespec end)
    {
        return ((long long)(end.tv_sec-start.tv_sec)*1000000000
                + (long long)(end.tv_nsec) - (long long)(start.tv_nsec));
    }

    struct timespec diff2timespec(struct timespec start, struct timespec end)
    {
        struct timespec temp;
        if ((end.tv_nsec-start.tv_nsec)<0)
        {
            temp.tv_sec = end.tv_sec-start.tv_sec-1;
            temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
        }
        else
        {
            temp.tv_sec = end.tv_sec-start.tv_sec;
            temp.tv_nsec = end.tv_nsec-start.tv_nsec;
        }
        return temp;
    }

    // lInc is supposed to be in nanosec (needs 64-bits parameter, as with 32-bits unsigned, max would be about 4.26")
    void IncFunc(const char* lFN, unsigned long long lInc)
    {
        lInc=lInc<1000?1:lInc/1000;
        //printf("IncFunc: %s %llu us\n", lFN, lInc);
        std::string lFNS(lFN);
        if (sFunctionsCosts.find(lFNS)!=sFunctionsCosts.end())
        {
            // first is total time
            sFunctionsCosts[lFN].first+=lInc;
            // second is number of calls
            sFunctionsCosts[lFN].second++;
        }
        else
        {
            std::pair<unsigned long long, unsigned long> lCost;
            lCost.first=lInc;
            lCost.second=1;
            sFunctionsCosts[lFN]=lCost;
        }
    }

// todo : add a log at each GTL_PROFILE_END ?
// GTL_LOG(6, "%s lasts %ld sec %ld microsec", __func__, lDiff.tv_sec, lDiff.tv_nsec/1000);

    int gtl_EstimateCPUFreq()
    {
        CGSystemInfo lSI;
        return lSI.EstimateCPUFreq();
    }

    int gtl_LogPerfInfo(int lSev)
    {
        // On my 8core Dell Vostro in debug build:
        // gtl_beginjob use about 10usec (about 20usec when messages)
        // gtl_PrivateInit lasts 300msec
        // gtl_init 350msec
        // gtl_test less than 1msec

        unsigned long long lTotal=0; // usec: max 71min for unsigned long
        double lMean;

        for (std::map<std::string, std::pair<unsigned long long, unsigned long> >::iterator it=sFunctionsCosts.begin();
             it!=sFunctionsCosts.end(); it++)
        {
            std::string lFN=(*it).first;
            lMean = (double)((*it).second.first)/(double)((*it).second.second);
#if defined(__MINGW32__)
            GTL_LOG(lSev, (char*)"Function %s used total %I64u usec and has been called %lu times (mean of %g usec per call)",
                    (char*)lFN.c_str(), (*it).second.first, (*it).second.second, lMean );
#else
            GTL_LOG(lSev, (char*)"Function %s used total %llu usec and has been called %lu times (mean of %g usec per call)",
                    (char*)lFN.c_str(), (*it).second.first, (*it).second.second, lMean );
#endif
            lTotal+=(*it).second.first;
        }

#if defined(__MINGW32__)
        GTL_LOG(lSev, (char*)"Gross total time spend in GTL: %I64u usec", lTotal);
#else
        GTL_LOG(lSev, (char*)"Gross total time spend in GTL: %llu usec", lTotal);
#endif
        return 0;
    }

    int gtl_SendEmail()
    {
        //jwsmtp::mailer lMailer; // adds 500ko to GTL lib and provokes a _fstat64 not located in msvcrt.dll under NT
        /*
        lMailer.setsubject(std::string("Hello GTL"));
        lMailer.setmessage(std::string("Hello this is the email send from GTL"));
        lMailer.setserver(std::string("mail.galaxysemi.com"));
        lMailer.setsender(std::string("william.tambellini@galaxysemi.com"));
        lMailer.addrecipient(std::string("william.tambellini@galaxysemi.com"));
        lMailer.send();
        GTL_LOG(5, (char*)"Send email response: %s", lMailer.response().c_str() );
        if (lMailer.response().substr(0,3)==std::string("250"))
            return 0;
        */
        return -1;
    }

    /*
    // gcc has -p option to profile using these 2 callbacks:
    void __cyg_profile_func_enter(void* p_this_fn, void* p_call_site)
    {
        //printf("func_enter %p\n", p_this_fn?p_this_fn:0);
    }
    void __cyg_profile_func_exit(void* p_this_fn, void* p_call_site)
    {
    }
    */

}
