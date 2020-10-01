/*
    Unit test 1 : simulate a tester client to connect to a GTM server sending random test result
*/
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // needed for the time() function
#include <time.h> // on linux only ?
// unistd also available on mingw (even if partial)
#if !defined(__BORLANDC__) && !defined(_MSC_VER)
    #include <unistd.h> // not available on BCB5
#endif

#ifdef WIN32
    #include <windows.h>
    #include <winsock2.h>
#endif

// chrono seems to be usefull for any thread sleep function
// but available only in C++2011: needs -std=c++0x or -std=gnu++0x compiler options.
// is there any C++2011 define ?
//#include <chrono>

extern "C"
{
    #include <gtl_core.h>
}

int sNumOfSites=5;
int sNumOfRuns=5000;
int sNumOfMsecPerRun=10;

/* wait n msec : despite all trials, does not work on Windows.
*/
void wait(long num_msec)
{
    // It seems there is no cross platform C sleep or wait method.
#if !defined(__BORLANDC__) && !defined(_MSC_VER)
    // trying the select() function : time in microseconds
    struct timeval tv;
    tv.tv_sec = num_msec / 1000000;
    tv.tv_usec = num_msec % 1000000;
    int n=select(0, NULL, NULL, NULL, &tv); // last arg is a timeout. Does not seem to work on Windows7
    if (n==-1)
    {
        // trying usleep : known to work at least on Linux
        n=usleep((unsigned int)(num_msec*1000) ); // does not seem to work on WIN32
        if (n==-1) // usleep failed
        {
            #ifdef WIN32
                Sleep((DWORD)num_msec); // Sleep does not seem to work neither on mingw32...
            #else
                // what else ?
            #endif
        }
    }
#endif
}

int main(int argc, char** argv)
{
    printf("Usage: NumberOfSite NumberOfRun RunDurationInMSec (currently %d args)\n", argc);

    srand(69); // with this seed and recipe and default args, we should expect n PAT for default args

    if (argc>1 && argv[1])
    {
        int ns=atoi(argv[1]);
        sNumOfSites=ns;
    }

    if (argc>2 && argv[2])
    {
        int nr=atoi(argv[2]);
        if (nr>0)
            sNumOfRuns=nr;
    }

    if (argc>3 && argv[3])
    {
        int run_duration=atoi(argv[3]);
        if (run_duration>0)
            sNumOfMsecPerRun=run_duration;
    }

    // getcwd not found on Win32
	//char cwd[1024]=""; getcwd(cwd, 1024);
    //printf("cwd: %s\n", cwd); // or getcwd(...) ? get_current_dir_name()
    printf("NumOfSites=%d NumOfRuns=%d NumOfMsecPerRun=%d\n", sNumOfSites, sNumOfRuns, sNumOfMsecPerRun);

    // Reconnection mode
    char lRM[256]="?";
    int lResult=gtl_get((char*)GTL_KEY_RECONNECTION_MODE, lRM);
    printf("Reconnection mode %s\n", lRM);
    // Just to check :
    lResult=gtl_set((char*)GTL_KEY_RECONNECTION_MODE, (char*)"on");

    char lLV[255]="?";
    gtl_get((char*)GTL_KEY_LIB_VERSION, lLV);
    printf("GTL version : %s\n", lLV);
    char lTM[255]="?";
    int r=gtl_get((char*)GTL_KEY_TRACEABILITY_MODE, lTM);
    printf("GTL traceability mode= %s\n", lTM);

    r=gtl_set((char*)GTL_KEY_OUTPUT_FOLDER, (char*)".");
    if (r!=0)
    {
        printf("gtl_set output folder failed: %d\n", r);
        return EXIT_FAILURE;
    }
    gtl_set((char*)GTL_KEY_OUTPUT_FILENAME, (char*)"gtl.sqlite");
    gtl_set((char*)GTL_KEY_DATA_FILENAME, (char*)"none");
    gtl_set((char*)"gtm_communication_mode", (char*)"synchronous");
    //gtl_set((char*)"http_server", (char*)"on");
    printf("Current lib state (should be 0)=%d\n", gtl_get_lib_state()); // Should be 0

    // Sites number
    int lSitesNumbers[256];
    for (int i=0; i<256; i++)
        lSitesNumbers[i]=i;

    printf("gtl_set_node_info...\n");
    r=gtl_set_node_info(69, (char*)"my hostid", (char*)"my hostname", (char*)"my node", (char*)"UnitTest",
                            (char*)"my tester exec", (char*)"my job", (char*)"?", (char*)"?", (char*)"?");
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl_set_node_info failed: %d", r);
        return EXIT_FAILURE;
    }

    printf("gtl_set_prod_info...\n");
    r=gtl_set_prod_info((char*)"operator1", (char*)"job rev 1", (char*)"lot 2", (char*)"sublot 2", (char*)"product A");
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl_set_prod_info failed: %d", r);
        return EXIT_FAILURE;
    }

    printf("gtl_init...\n");
    // currently a message is 1032 octet...
    //  max9260quad_b.csv
    r=gtl_init((char*)"gtl_tester.conf", (char*)"max9260quad_mprtest.csv", sNumOfSites, lSitesNumbers, 20064);
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl_init_lib failed: %d\n", r);
        //printf("Press enter to gtl_close...\n");
        //getchar();  // Wait until user hits "enter"
        printf("gtl_close...\n");
        gtl_close(); // just to be sure to close any output, logs...
        //printf("Press enter to quit...\n");
        //getchar();  // Wait until user hits "enter"
        return EXIT_FAILURE;
    }

    // Should be GTL_STATE_ENABLED
    printf("Current lib state (should be %d)=%d\n", GTL_STATE_ENABLED, gtl_get_lib_state());

    if (gtl_get_lib_state()==GTL_STATE_NOT_INITIALIZED)
    {
        printf("error : GTL disabled\n");
        return EXIT_FAILURE;
    }

    int lRes=0;
    unsigned int lNumOfPAT=0;
    float ratio=0;
    time_t start_t;
    time(&start_t); // This is the number of seconds since midnight Jan 1, 1970. NOTE: This is also defined in non-ISO sys/types.h.

    int expected_time_for_the_whole_lot=((float)sNumOfRuns) * (((float)sNumOfMsecPerRun)/1000.f);
    printf("Expected time for the whole lot: %d sec\n", expected_time_for_the_whole_lot);

    for (int iRun=0; iRun<sNumOfRuns; iRun++)
    {
        printf("run %d : %.1f %%... \r", iRun, ratio*100);

        lRes=gtl_beginjob();
        if (lRes!=GTL_CORE_ERR_OKAY)
        {
            printf("\ngtl_beginjob failed: %d\n", lRes);
            if (lRes!=GTL_CORE_ERR_GTM_COMM)
                return EXIT_FAILURE;
            printf("Lets continue in offline mode...\n");
        }

        for (int i=0; i<sNumOfSites; i++)
        {
            double lTR=((double)rand()/(double)RAND_MAX) * 205./1000.;
            r=gtl_test(i, 171000001, (char*)"VDDA/hs41__vdda", lTR );
            if (r!=GTL_CORE_ERR_OKAY)
            {
                printf((char*)"gtl_test failed: %d\n", r);
            }

            r=gtl_test(i, 176001035, (char*)"IOS__DOUT__1V7__DCS__H - 1",  -(((double)rand()/(double)RAND_MAX)) * 10./1000.-0.005);
            if (r!=GTL_CORE_ERR_OKAY)
            {
                printf((char*)"gtl_test failed: %d\n", r);
            }

            r=gtl_test(i, 176001035, (char*)"IOS__DOUT__1V7__DCS__H - 2",  -(((double)rand()/(double)RAND_MAX)) * 10./1000.);
            if (r!=GTL_CORE_ERR_OKAY)
            {
                printf((char*)"gtl_test failed: %d\n", r);
            }

            //r=gtl_test(i, 176001035, (char*)"IOS__DOUT__1V7__DCS__H - 003", -(((double)rand()/(double)RAND_MAX)) * 20./1000.);   // from 0 to 100

            /*
            r=gtl_test(i, 3002, (char*)"test 2_;", ((double)rand()/(double)RAND_MAX) * 2.); // from 0 to 2
            r=gtl_test(i, 3003, (char*)"test 3_;", ((double)rand()/(double)RAND_MAX) * 2.); // from 0 to 2
            */

            // Site binning
            int lOriginalBin=(int) ( ((double)rand()/(double)RAND_MAX) * 2. + 1.);
            int lNewBin=lOriginalBin;
            char lPartID[1024]="";
            sprintf(lPartID, "run%d.site%d", iRun, i);

            lRes=gtl_binning(i, lOriginalBin, lOriginalBin, &lNewBin, &lNewBin, lPartID);
            if (lRes!=GTL_CORE_ERR_OKAY) // always test if gtl_binning failed or not
            {
                printf("gtl_binning failed: %d\n", lRes);
                gtl_close();
                return EXIT_FAILURE;
            }
            if (lOriginalBin!=lNewBin)
                lNumOfPAT++;

        }

        r=gtl_endjob();
        if (r!=GTL_CORE_ERR_OKAY)
        {
            printf("gtl_endjob failed: %d\n", r);
            if (r==GTL_CORE_ERR_FAILED_TO_SEND_RESULTS && gtl_get_lib_state()!=GTL_STATE_OFFLINE)
                return EXIT_FAILURE;
        }

        static int n=0;
        if (n!=gtl_get_number_messages_in_stack())
        {
            n=gtl_get_number_messages_in_stack();
            printf("Now %d messages in stack\n", n);
            for (int i=0; i<n; i++)
            {
                int severity=0, messageID=0;
                char message[GTL_MESSAGE_STRING_SIZE]="?"; // currently 1024 chars

                int r=gtl_pop_first_message(&severity, message, &messageID);
                if (r==0)
                    printf("%d\t %d\t %s\n", severity, messageID, message);
                else
                {
                    printf("Failed to pop message: %d\n", r);
                    return EXIT_FAILURE;
                }
            }
            //gtl_clear_messages_stack(); // no more needed if you pop all messages
        }

        // Let s wait: 100 ms ? very short, too fast test program. Usually a run is not less than 500ms.
        //wait(9000); // usual sleep functions does not work on win7...

        // let's compute progress ratio
        ratio=(float)iRun/(float)sNumOfRuns;
        // let s wait if too fast
        time_t current;
        time(&current);
        int current_elapsed_time = (current-start_t);
        while ( ( (int) (ratio*(float)expected_time_for_the_whole_lot) ) > current_elapsed_time )
        {
            // we are too fast, let s wait max 1sec
            time(&current);
            current_elapsed_time=current-start_t;
        }
    }

    time_t end_t;
    time(&end_t);
    printf("\nSession of %d runs done in %ld secs, i.e. %d msec per run\n", sNumOfRuns, (end_t-start_t),
           (int)(((float)(end_t-start_t)*1000)/(float)sNumOfRuns)
           );

    printf("End of lot: number of PAT done: %d\n\n", lNumOfPAT);
    if (lNumOfPAT==0)
        printf("warning: at least few PAT should have been done...\n");

    printf("Expected results :\n");
    printf("NumberOfSites\t NumOfRuns\tRunduration\t Num of PATed dies\n");
    // 10ms per run is too fast in order to have stable results.
    //printf("5                5000           10        \t  1512                DEFAULT ?\n");
    /*
    1                1000           10          0
    2                1000           10          0
    5                5000           20          1571
    1                1000           20          49
    */

    printf("1                1000           100        \t 62\n");
    printf("2                1000           20         \t 97 ? 120 ? 123 ? : variable because run duration is too short\n");
    printf("2                1000           40         \t 129\n");
    printf("2                1000           50         \t 118? 130? run duration too short\n");
    printf("2                1000           60         \t 131\n");
    printf("2                1000           100        \t 130\n");
    printf("4                1000           100        \t 255\n");

    /*
    2                5000           200         629
    2                3000           100         ?
    2                3000           200         367
    */

    // if enough run, state should be GTL_STATE_ENABLED
    printf("Lib state (shoud be %d)=%d\n", GTL_STATE_ENABLED, gtl_get_lib_state());

    //gtl_command((char*)"-gtl_status");

#ifndef QT_NO_DEBUG
    //printf("Press a key to endlot...\n");
    //int k=getchar();
    //printf("Key %d pressed...\n", k);
#endif

    /* Deprecated
    r=gtl_endlot();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl_endlot failed: %d\n", r);
        return EXIT_FAILURE;
    }
    */

#ifndef QT_NO_DEBUG
    //printf("Press a key to gtl_close...\n");
    //k=getchar();
    //printf("Key %d pressed...\n", k);
#endif

    r=gtl_close();
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl_close failed: %d\n", r);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
