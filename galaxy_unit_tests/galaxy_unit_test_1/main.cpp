/*
    GalaxySemi Unit test 1 : simulate a tester client to connect to a GTM server sending random test results
*/
//#include <random> // Would have been usefull to easily generate random normal distributions but needs C++11
#include <math.h>
// Following include adds 15" to exececution on Dell Vostro i5 @2.4GHz (LTBG-VOSTRO)
// (29" -> 44")
//#include <algorithm> // for std::min()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> // needed for the time() function
#include <time.h>
// unistd also available on mingw (even if partial) but not on msvc nor borland
#if !defined(__BORLANDC__) && !defined(_MSC_VER)
    #include <unistd.h> // not available on BCB5
#endif

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif
#include <sys/stat.h>

// Found on http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
#ifdef __MACH__
#include <mach/mach_time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
int clock_gettime(int /*clk_id*/, struct timespec *t){
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#endif

// chrono seems to be usefull for any thread sleep function
// but available only in C++2011: needs -std=c++0x or -std=gnu++0x compiler options.
// is there any C++2011 define ?
//#include <chrono>

#define GETSTRING(x) #x
#define RAND_BETWEEN(MINI, MAXI) std::min((MINI), (MAXI))+(abs((MAXI)-(MINI)))*((double)rand()/(double)RAND_MAX)

// standard GTL is a 100% C API
extern "C"
{
    #include <gtl_core.h>
    // deprecated gtl functions
    extern int d_gtl_clear_messages_stack();
}

int  sNumOfSites=4;
int  sNumOfRuns=1000;
long sNumOfMsecPerRun=200;

struct timespec diff2ts(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0)
    {   // more than 1sec ?
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else
    {   // less than 1 sec ?
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

long long diff2nsec(struct timespec start, struct timespec end)
{
    long long lDiff = (long long)(end.tv_sec-start.tv_sec)*1000000000
            + (long long)(end.tv_nsec) - (long long)(start.tv_nsec);
    return lDiff;
}

/* wait n millisec
*/
void wait(long num_msec)
{
    struct timespec lTS1, lTS2, lTS3;
    clock_gettime(CLOCK_MONOTONIC, &lTS1);
    do
    {
        clock_gettime(CLOCK_MONOTONIC, &lTS2);
        lTS3=diff2ts(lTS1, lTS2);
    }
    while (lTS3.tv_sec*1000+lTS3.tv_nsec/1000000<num_msec);

    //#if !defined(__BORLANDC__) && !defined(_MSC_VER)
    #if 0
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

int getFileSize(const char *szFileName)
{
    struct  stat attributes;

    /* Get File Size */
    if(stat(szFileName, &attributes) == -1)
    {
        return -1;
    }

    return attributes.st_size;
}

/*======================================================================*/
/* Copy a file (using fopen() function)                                 */
/*                                                                      */
/* Parameters : 														*/
/*	szSourceFile : name of source file to copy							*/
/*	szDestFile	 : name of destination file								*/
/*	nOverWrite   : 0 if destination file should not be overwritten if	*/
/*				   it exists											*/
/*	nUseTemp	 : not 0 if the function should use a temporary file to */
/*				   copy to and then rename the destination file to		*/
/*				   szDestFile											*/
/*																		*/
/* Return : 1 if copy sucessfull, 0 else                                */
/*======================================================================*/
int copyFile(const char *szSourceFile, const char *szDestFile)
{
    FILE	*fpSourceFile, *fpDestFile;
    char	szTempDestFile[512], *ptBuffer;
    int		nFileSize, nBytes;

    /* Check if files have different names */
    if(!strcmp(szSourceFile, szDestFile))
        return 0;

    /* Get size of the file to copy */
    if((nFileSize = getFileSize(szSourceFile)) == -1)
        return 0;

    /* Check if destination file exists */
    fpDestFile = fopen(szDestFile, "r");
    if(fpDestFile)
    {
        fclose(fpDestFile);
        if(remove(szDestFile))
            return 0;
    }

    /* Allocate memory for transfer buffer */
    ptBuffer = (char *)malloc(nFileSize*sizeof(char));
    if(!ptBuffer)
        return 0;

    /* Change destination filename if nUseTemp */
    strcpy(szTempDestFile, szDestFile);

    /* Open source file */
    fpSourceFile = fopen(szSourceFile, "rb");
    if(!fpSourceFile)
    {
        free(ptBuffer);
        return 0;
    }

    /* Open destination file */
    fpDestFile = fopen(szTempDestFile, "wb");
    if(!fpDestFile)
    {
        free(ptBuffer);
        fclose(fpSourceFile);
        return 0;
    }

    /* Copy the file */
    nBytes = fread(ptBuffer, sizeof(char), nFileSize, fpSourceFile);
    if(nBytes != nFileSize)
    {
        free(ptBuffer);
        fclose(fpSourceFile);
        fclose(fpDestFile);
        return 0;
    }
    nBytes = fwrite(ptBuffer, sizeof(char), nFileSize, fpDestFile);
    if(nBytes != nFileSize)
    {
        free(ptBuffer);
        fclose(fpSourceFile);
        fclose(fpDestFile);
        return 0;
    }

    /* Cleanup nUseTemp */
    free(ptBuffer);
    fclose(fpSourceFile);
    fclose(fpDestFile);

    return 1;
}

/******************************************************************************!
 * \fn unlinkSqlite
 ******************************************************************************/
void unlinkSqlite()
{
    char lRes[1024] = "?";
    gtl_get((char*) GTL_KEY_TEMP_FOLDER, lRes);
    strcat(lRes, "/gtl.sqlite");
    unlink(lRes);
}

/******************************************************************************!
 * \fn unlinkSqlite
 ******************************************************************************/
void copyRefSqlite()
{
    char lDest[1024] = "?";
    char lSrc[1024] = "./gtl.sqlite.ref";
    gtl_get((char*) GTL_KEY_TEMP_FOLDER, lDest);
    strcat(lDest, "/gtl.sqlite");
    copyFile(lSrc, lDest);
}

// Retest function to be called before gtl open or during the test program for "on the fly" retest
int Retest(char *lDesiredLimits, char* lRetestHbins)
{
    printf("Retest using %s limits...\n", lDesiredLimits?lDesiredLimits:"?");
    gtl_set((char*)GTL_KEY_DESIRED_LIMITS, lDesiredLimits); // tighest ? largest ?
    gtl_set((char*)GTL_KEY_RETEST_HBINS, (char*)lRetestHbins);
    int lRes=gtl_command((char*)"retest");
    if (lRes!=0)
    {
        printf("gtl retest command failed: %d\n", lRes);
        gtl_command((char*)GTL_COMMAND_CLOSE);
        return lRes;
    }
    return lRes;
}

int FlushMessages()
{
    char lNumMsg[1024]="?";
    gtl_get(GTL_KEY_NUM_OF_MESSAGES_IN_STACK, lNumMsg);
    int n=0;
    int r=sscanf(lNumMsg, "%d", &n);
    if (r!=1)
        return -1;
    if (n==0)
        return 0;
    printf("Now %d messages in stack:\n", n);
    for (int i=0; i<n; i++)
    {
        int severity=0, messageID=0;
        char message_sev_string[1024]="?";
        char message_id_string[1024]="?";
        char message[GTL_MESSAGE_STRING_SIZE]="?"; // currently 1 message is 1024 chars

        int r=gtl_command((char*)GTL_COMAND_POP_FIRST_MESSAGE);
        if (r==0)
        {
            gtl_get(GTL_KEY_CURRENT_MESSAGE, message);
            gtl_get(GTL_KEY_CURRENT_MESSAGE_SEV, message_sev_string);
            sscanf(message_sev_string, "%d", &severity);
            gtl_get(GTL_KEY_CURRENT_MESSAGE_ID, message_id_string);
            sscanf(message_id_string, "%d", &messageID);
            printf("%d\t %d\t %s\n", severity, messageID, message);
        }
        else
        {
            printf("Failed to pop message: %d\n", r);
            return r;
        }
    }
    gtl_command((char*)GTL_COMMAND_CLEAR_MESSAGES_STACK);
    d_gtl_clear_messages_stack(); // test to check funtion still usable even if disappeared in the header
    return 0;
}

// should return a normal distributed sample between mini and maxi
double GenerateANormalDitribSample(double mini, double maxi)
{
    double sum=0;
    unsigned i=0;
    for (i=0; i<12; i++)
    {
        sum+=((double)rand())/(double)RAND_MAX;
    }
    return mini+(sum/12.)*(fabs(maxi-mini));
}

int main(int argc, char** argv)
{
    printf("Usage: NumberOfSite NumberOfRun RunDurationInMSec Retest(ms, otf or none) "\
           "TempSqliteMode (delete, copyref, leave) (currently %d args)\n", argc);

    srand(69); // with this seed and recipe and default args, we should expect 185 PAT at least on win32
    int lRes=0;

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
        long run_duration=atol(argv[3]);
        if (run_duration>=0)
            sNumOfMsecPerRun=run_duration;
    }

    char lRetestType[255]="";
    strcpy(lRetestType, (char*)"none"); // "otf" (on the fly) or "ms" (multisession) or "none"
    if (argc>4 && argv[4])
    {
        strncpy(lRetestType, argv[4], 255);
    }

    char lTempSqliteMode[255]="";
    strcpy(lTempSqliteMode, (char*)"leave"); // "delete" or "copyref" or "leave"
    if (argc>5 && argv[5])
    {
        strncpy(lTempSqliteMode, argv[5], 255);
    }

    // What to do with temp sqlite file?
    if (strcmp("delete", lTempSqliteMode)==0)
        // Remove temp sqlite file.
        unlinkSqlite();
    else if (strcmp("copyref", lTempSqliteMode)==0)
        // Copy ref sqlite file
        copyRefSqlite();


    char lGTLTF[1024]="?";
    gtl_get(GTL_KEY_TEMP_FOLDER, lGTLTF);
    printf("GTL temp folder found = %s\n", lGTLTF);

    if (lGTLTF[0] == '\0')
    {
        gtl_set(GTL_KEY_TEMP_FOLDER, (char*)".");
        gtl_get(GTL_KEY_TEMP_FOLDER, lGTLTF);
        printf("Forced GTL temp folder to %s\n", lGTLTF);
    }

    if (strcmp("ms", lRetestType)==0) // multi session
    {
        // multi session Retest
        lRes=Retest((char*) "last", // 'widest' 'tightest' or 'last'
                    (char*) "all fail");
        if (lRes!=0)
        {
            printf("Retest command failed: %d", lRes);
            exit(EXIT_FAILURE);
        }
    }

    printf("NumOfSites=%d NumOfRuns=%d NumOfMsecPerRun=%ld TempSqliteMode=%s\n",
           sNumOfSites, sNumOfRuns, sNumOfMsecPerRun, lTempSqliteMode);

    // Reconnection mode
    char lRM[256]="";
    gtl_get((char*)GTL_KEY_RECONNECTION_MODE, lRM);
    printf("Reconnection mode %s\n", lRM);
    gtl_set((char*)GTL_KEY_RECONNECTION_MODE, (char*)"on");

    // Let's check GTL version
    char lLV[255]="?";
    gtl_get((char*)GTL_KEY_LIB_VERSION, lLV);
    printf("GTL version: %s\n", lLV);

    // Mandatory: set output folder
    int r=gtl_set((char*)GTL_KEY_OUTPUT_FOLDER, (char*)".");
    if (r!=0)
    {
        printf("gtl_set output folder failed: %d\n", r);
        return EXIT_FAILURE;
    }

    // Mandatory: Setting the number of sites expected for the session
    char lNumOfSitesString[255]="";
    sprintf(lNumOfSitesString, "%d", sNumOfSites);
    r=gtl_set((char*)GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, lNumOfSitesString);
    if (r!=0)
    {
        printf("Failed to set num of sites: %d\n", r);
        exit(EXIT_FAILURE);
    }
    // Mandatory: set site numbers
    char lSiteNumbersString[2048]="";
    char lSiteNumberString[4];
    for (int i = 0; i < sNumOfSites && i < 255; ++i)
    {
        // Max number of sites is anyway today 255
        sprintf(lSiteNumberString, "%d ", i);
        strcat(lSiteNumbersString, lSiteNumberString);
    }
    r=gtl_set((char*)GTL_KEY_SITES_NUMBERS, lSiteNumbersString);
    if (r!=0)
    {
        printf("Failed to set sites numbers: %d", r);
        exit(EXIT_FAILURE);
    }

    // Setting the max mem for GTL messages :
    // currently a message is 1032 octet... so 30000 bytes will allow 30 messages max in the stack
    gtl_set((char*)GTL_KEY_MAX_MESSAGES_STACK_SIZE, (char*)"90064");

    //gtl_set((char*)"gtm_communication_mode", (char*)"synchronous");

    // Just to check the current lib state
    char lLibState[1024]="?";
    gtl_get(GTL_KEY_LIB_STATE, lLibState);
    printf("Current lib state (should be 0)=%s\n", lLibState); // Should be 0

    // using new API: gtl_set/get
    gtl_set((char*)GTL_KEY_OUTPUT_FILENAME, (char*)"gtl.sqlite"); // could be done later
    gtl_set((char*)GTL_KEY_DATA_FILENAME, (char*)"none");
    gtl_set((char*)GTL_KEY_CONFIG_FILE, (char*)"gtl_tester.conf" ); // needed for gtl_open
    gtl_set((char*)GTL_KEY_RECIPE_FILE, (char*)"unit_test_recipe.json"); // csv recipe is deprecated
    gtl_set((char*)GTL_KEY_STATION_NUMBER, (char*)"6969");
    gtl_set((char*)GTL_KEY_PRODUCT_ID, (char*)"ProductA"); // mandatory
    gtl_set((char*)GTL_KEY_LOT_ID, (char*)"Lot3"); // mandatory
    gtl_set((char*)GTL_KEY_SUBLOT_ID, (char*)"SublotX"); // mandatory
    gtl_set((char*)GTL_KEY_AUX_FILE, (char*)"/tmp/MySuperbAuxFile.aux");
    gtl_set((char*)GTL_KEY_FACILITY_ID, (char*)"My superb facility");
    gtl_set((char*)GTL_KEY_FAMILY_ID, (char*)"My big family");
    gtl_set((char*)GTL_KEY_FLOW_ID, (char*)"Flow 66");
    gtl_set((char*)GTL_KEY_FLOOR_ID, (char*)"Base floor");
    gtl_set((char*)GTL_KEY_JOB_NAME, (char*)"unit test");
    gtl_set((char*)GTL_KEY_JOB_REVISION, (char*)"4.0");
    gtl_set((char*)GTL_KEY_MODE_CODE, (char*)"D");
    gtl_set((char*)GTL_KEY_OPERATOR_NAME, (char*)"Kate");
    gtl_set((char*)GTL_KEY_RETEST_CODE, (char*)"N");
    gtl_set((char*)GTL_KEY_ROM_CODE, (char*)"987654321");
    gtl_set((char*)GTL_KEY_SPEC_NAME, (char*)"My superb specs");
    gtl_set((char*)GTL_KEY_SPEC_VERSION, (char*)"specs v1");
    gtl_set((char*)GTL_KEY_SETUP_ID, (char*)"setup21");
    gtl_set((char*)GTL_KEY_TEST_CODE, (char*)"FINAL");
    gtl_set((char*)GTL_KEY_TESTER_NAME, (char*)"my tester"); // mandatory
    gtl_set((char*)GTL_KEY_TESTER_TYPE, (char*)"GStester"); // mandatory
    gtl_set((char*)GTL_KEY_TEST_TEMPERATURE, (char*)"0");
    gtl_set((char*)GTL_KEY_USER_TXT, (char*)"My not too long user text");

    printf("gtl open...\n");
    r=gtl_command((char*)"open");
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl open failed: %d\n", r);
        FlushMessages();
        gtl_command((char*)"close"); // just to be sure to close any output, logs...
        if (r == GTL_CORE_ERR_GTM_INIT)
        {
            // Usefull just to test the init of GTL. Without server
            return EXIT_SUCCESS;
        }
        else
        {
            return EXIT_FAILURE;
        }
    }

    // Should be GTL_STATE_ENABLED
    gtl_get((char*)GTL_KEY_LIB_STATE, lLibState);
    printf("Current lib state (should be %d)=%s\n", GTL_STATE_ENABLED, lLibState);

    char lSplitlotID[256]="?";
    gtl_get(GTL_KEY_SPLITLOT_ID, lSplitlotID);
    printf("Current splitlot ID: %s\n", lSplitlotID);

    if (strcmp(lLibState, "1")!=0)
    {
        printf("error : GTL not initialized: '%s'\n", lLibState);
        return EXIT_FAILURE;
    }

    gtl_command((char*)"-gtl_debugon");
    gtl_command((char*)"-gtl_testlist");

    unsigned int lNumOfPAT=0;
    //float ratio=0;
    struct timespec lStartOfBatch, lEndOfBatch;
    int             lGetBatchTimeStatus=0;
    lGetBatchTimeStatus = clock_gettime(CLOCK_MONOTONIC, &lStartOfBatch);

    time_t start_t;
    // This is the number of seconds since midnight Jan 1, 1970. NOTE: This is also defined in non-ISO sys/types.h.
    time(&start_t);

    float expected_time_for_the_whole_lot=((float)sNumOfRuns*(float)sNumOfMsecPerRun)/1000.0;
    printf("Expected time for the whole lot: %g sec\n", expected_time_for_the_whole_lot);

    struct timespec lStartOfRun, lEndOfRun;
    int             lGetRunTimeStatus=0;

    for (int iRun=0; iRun<sNumOfRuns; iRun++)
    {
        lGetRunTimeStatus = clock_gettime(CLOCK_MONOTONIC, &lStartOfRun);

        printf("Run %d out of %d... \r", iRun+1, sNumOfRuns);

        // On the fly retest: add an option
        if (strcmp("otf", lRetestType)==0)
            if ( (iRun==(int)(((float)sNumOfRuns)/3.0f)) || (iRun==(int)(((float)sNumOfRuns)/1.33f)) )
            {
                lRes=Retest((char*)"last", (char*)"all fail");
                gtl_get(GTL_KEY_SPLITLOT_ID, lSplitlotID);
                printf("Current splitlot ID is now: %s\n", lSplitlotID);
            }

        lRes=gtl_command((char*)"beginjob");
        if (lRes!=GTL_CORE_ERR_OKAY)
        {
            printf("\ngtl_beginjob failed: %d\n", lRes);
            // either just lost the GTM connection, either just offline and cannot reconnect
            if ( (lRes!=GTL_CORE_ERR_GTM_COMM) && (lRes!=GTL_CORE_ERR_GTM_INIT) )
                return EXIT_FAILURE;
            printf("Let's continue in offline mode...\n");
            gtl_set((char*)GTL_KEY_RECONNECTION_MODE, (char*)"on");
        }

        for (int i=0; i<sNumOfSites; i++)
        {
            // todo: mutate get_site_state() to gtl set/get commands
            //gtl_get("site state");
            // PTR
            //r=gtl_test(i, 2001, (char*)"PTR test1", ((double)rand()/(double)RAND_MAX) * 110.);   // from 0 to 100
            r=gtl_test(i, 2001, (char*)"PTR test1", GenerateANormalDitribSample(0.,100.) ); // +RAND_BETWEEN(0.,0.1)
            if (r!=GTL_CORE_ERR_OKAY)
                GTL_LOG(3, (char*)"gtl_test failed: %d\n", r);

            r=gtl_test(i, 2002, (char*)"PTR test2",  ((double)rand()/(double)RAND_MAX) * 2.); // from 0 to 2
            if (r!=GTL_CORE_ERR_OKAY)
                GTL_LOG(3, (char*)"gtl_test failed: %d\n", r);

            r=gtl_test(i, 2003, (char*)"PTR test3",  ((double)rand()/(double)RAND_MAX) * 2.); // from 0 to 2
            if (r!=GTL_CORE_ERR_OKAY)
                GTL_LOG(3, (char*)"gtl_test failed: %d\n", r);

#if 0
            r=gtl_test(i, 2004, (char*)"PTR test4",  ((double)rand()/(double)RAND_MAX) * 2.); // from 0 to 2
            if (r!=GTL_CORE_ERR_OKAY)
                GTL_LOG(3, (char*)"gtl_test failed: %d\n", r);
#endif


            // MPR
            int lMPRindexes[]={0, 1, 2};
            double lMPR[3];
            for (int q=0; q<3; q++)
              lMPR[q]=((double)rand()/(double)RAND_MAX) * 2.; // from 0 to 2
            r=gtl_mptest(i, 3001, (char*)"MPR test1", lMPR, lMPRindexes, 3);

			if (r!=GTL_CORE_ERR_OKAY)
            {
                printf("gtl_mptest failed:%d\n", r);
                GTL_LOG(3, (char*)"gtl_mptest failed: %d\n", r);
                return EXIT_FAILURE;
            }

            // Site binning
            int lOriginalBin=(int) (((double)rand()/(double)RAND_MAX) * 8.);
            int lNewBin=lOriginalBin;
            char lPartID[1024]="";
            sprintf(lPartID, "run%d.site%d", iRun, i);

            // in unit test recipe, good bins are 1 to 7
            lRes=gtl_binning(i, lOriginalBin, lOriginalBin, &lNewBin, &lNewBin, lPartID);
            if (lRes!=GTL_CORE_ERR_OKAY) // always test if gtl_binning failed or not
            {
                printf("gtl_binning failed: %d\n", lRes);
                gtl_command((char*)GTL_COMMAND_CLOSE);
                return EXIT_FAILURE;
            }
            if (lOriginalBin!=lNewBin)
            {
                lNumOfPAT++;
            }
        }

        r=gtl_command((char*)"endjob");
        if (r!=GTL_CORE_ERR_OKAY)
        {
            printf("gtl_endjob failed: %d\n", r);
            gtl_get(GTL_KEY_LIB_STATE, lLibState);
            int lLibStateNb=-1;
            sscanf(lLibState, "%d", &lLibStateNb);
            if ( (r==GTL_CORE_ERR_FAILED_TO_SEND_RESULTS)
                 || (lLibStateNb!=GTL_STATE_ENABLED) )
                return EXIT_FAILURE;
        }

        r=FlushMessages();
        if (r!=0)
        {
            printf("Flush messages failed: %d", r);
            return 0;
        }

        // Let's wait to ensure we match specified NumOfMsecPerRun parameter. Usually a run is not less than 500ms.
        if((lGetRunTimeStatus == 0) && (clock_gettime(CLOCK_MONOTONIC, &lEndOfRun) == 0))
        {
            long lRunDuration = (long)(diff2nsec(lStartOfRun, lEndOfRun)/1000000);
            if(lRunDuration < sNumOfMsecPerRun)
                wait(sNumOfMsecPerRun-lRunDuration);    // usual sleep functions does not work on win7...

        }

        // let's compute progress ratio
        /*
        ratio=(float)iRun/(float)sNumOfRuns;
        // let's wait if too fast
        time_t current;
        time(&current);
        int current_elapsed_time = (current-start_t);
        while ( ( (int) (ratio*(float)expected_time_for_the_whole_lot) ) > current_elapsed_time )
        {
            // we are too fast, let s wait max 1sec
            time(&current);
            current_elapsed_time=current-start_t;
        }
        */
    }

    time_t end_t;
    time(&end_t);

    if((lGetBatchTimeStatus == 0) && (clock_gettime(CLOCK_MONOTONIC, &lEndOfBatch) == 0))
    {
        float lBatchNsecs = (float)diff2nsec(lStartOfBatch, lEndOfBatch);
        printf("\nSession of %d runs done in %g secs, i.e. %d msec per run\n", sNumOfRuns, lBatchNsecs/1000000000.0,
               (int)((lBatchNsecs/1000000.0)/(float)sNumOfRuns)
               );
    }
    else
    {
        printf("\nSession of %d runs done in %f secs, i.e. %d msec per run\n",
               sNumOfRuns, difftime(end_t, start_t),
               (int) ((difftime(end_t, start_t) * 1000) / (float) sNumOfRuns));
    }

    printf("End of lot: number of outliers found: %d\n\n", lNumOfPAT);
    if (lNumOfPAT==0)
        printf("warning: at least few PAT should have been done...\n");

    // Testing query command
    gtl_set(GTL_KEY_QUERY, (char*)"select * from ft_ptest_rollinglimits");
    gtl_set(GTL_KEY_QUERY_OUTPUT_FILE, (char*)"limits.csv");
    gtl_command((char*)GTL_COMMAND_QUERY);

    //printf("Expected results :\n");
    //printf("NumberOfSites\t NumOfRuns\tRunduration\t Num of PATed dies\n");
    // 10ms per run is too fast in order to have stable results.

    char lSiteState[1024]="?";
    gtl_set((char*)GTL_KEY_DESIRED_SITE_NB, (char*)"0");
    gtl_get((char*)GTL_KEY_SITE_STATE, lSiteState);
    printf("Site 0 is in state: %s (should be DPAT:1)\n", lSiteState);

    // if enough run, state should be GTL_STATE_ENABLED
    gtl_get(GTL_KEY_LIB_STATE, lLibState);
    printf("Lib state (shoud be %d)=%s\n", GTL_STATE_ENABLED, lLibState );

#ifndef QT_NO_DEBUG
    //printf("Press a key to endlot...\n");
    //int k=getchar();
    //printf("Key %d pressed...\n", k);
#endif

#ifndef QT_NO_DEBUG
    //printf("Press a key to gtl close...\n");
    //int k=getchar();
    //printf("Key %d pressed...\n", k);
#endif

    r=gtl_command((char*)GTL_COMMAND_CLOSE);
    if (r!=GTL_CORE_ERR_OKAY)
    {
        printf("gtl close failed: %d\n", r);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
