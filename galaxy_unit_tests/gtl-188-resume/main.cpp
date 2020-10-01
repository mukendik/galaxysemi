/******************************************************************************!
 * \file main.cpp
 * \brief Simulate a tester client to connect to a GTM server sending
          random test results. Used by scenario 188, not by unit tests pass
 ******************************************************************************/
// Would have been usefull to easily generate
// random normal distributions but needs C++11
//#include <random>
#include <math.h>
#include <algorithm>  // for std::min()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>  // needed for the time() function
#include <time.h>
// unistd also available on mingw (even if partial) but not on msvc nor borland
#if ! defined(__BORLANDC__) && ! defined(_MSC_VER)
    #include <unistd.h>  // not available on BCB5
#endif

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
#endif

// standard GTL is a 100% C API
extern "C"
{
    #include <gtl_core.h>
    // deprecated gtl functions
    extern int d_gtl_clear_messages_stack();
}

int gNumOfSites = 4;
int gNumOfRuns = 1000;
int gNumOfMsecPerRun = 200;
char gRes[1024] = "?";

/******************************************************************************!
 * \fn GenerateANormalDitribSample
 * \brief Should return a normal distributed sample between mini and maxi
 ******************************************************************************/
double GenerateANormalDitribSample(double mini, double maxi)
{
    double sum = 0;
    unsigned i = 0;
    for (i = 0; i < 12; i++)
    {
        sum += ((double) rand()) / (double) RAND_MAX;
    }
    return mini + (sum / 12.) * (fabs(maxi - mini));
}

/******************************************************************************!
 * \fn gtlClose
 ******************************************************************************/
void gtlClose()
{
    int r = gtl_command((char*) GTL_COMMAND_CLOSE);
    if (r != GTL_CORE_ERR_OKAY)
    {
        printf("gtl close failed: %d\n", r);
        exit(EXIT_FAILURE);
    }
}

/******************************************************************************!
 * \fn gtlOpen
 ******************************************************************************/
void gtlOpen()
{
    int r;

    gtl_get(GTL_KEY_LIB_STATE, gRes);
    if (*gRes != '0')
    {
        printf("gtl state failed\n");
        exit(EXIT_FAILURE);
    }

    r = gtl_command((char*) "open");
    if (r != GTL_CORE_ERR_OKAY)
    {
        printf("gtl open failed: %d\n", r);
        exit(EXIT_FAILURE);
    }

    gtl_get((char*) GTL_KEY_LIB_STATE, gRes);
    if (*gRes != '1')
    {
        printf("gtl state failed\n");
        gtlClose();
        exit(EXIT_FAILURE);
    }
}

/******************************************************************************!
 * \fn gtlInit
 ******************************************************************************/
void gtlInit()
{
    gtl_set((char*) GTL_KEY_OUTPUT_FOLDER, (char*) ".");
    gtl_set((char*) GTL_KEY_CONFIG_FILE, (char*) "gtl_tester.conf");
    gtl_set((char*) GTL_KEY_OUTPUT_FILENAME, (char*) "gtl.sqlite");
    gtl_set((char*) GTL_KEY_DATA_FILENAME, (char*) "none");
    gtl_set((char*) GTL_KEY_RECIPE_FILE, (char*) "unit_test_recipe.json");
    gtl_set((char*) GTL_KEY_STATION_NUMBER, (char*) "6969");
    gtl_set((char*) GTL_KEY_PRODUCT_ID, (char*) "ProductA");
    gtl_set((char*) GTL_KEY_LOT_ID, (char*) "Lot2");
    gtl_set((char*) GTL_KEY_SUBLOT_ID, (char*) "SublotX");
    gtl_set((char*) GTL_KEY_AUX_FILE, (char*) "/tmp/MySuperbAuxFile.aux");
    gtl_set((char*) GTL_KEY_FACILITY_ID, (char*) "My superb facility");
    gtl_set((char*) GTL_KEY_FAMILY_ID, (char*) "My big family");
    gtl_set((char*) GTL_KEY_FLOW_ID, (char*) "Flow 66");
    gtl_set((char*) GTL_KEY_FLOOR_ID, (char*) "Base floor");
    gtl_set((char*) GTL_KEY_JOB_NAME, (char*) "unit test");
    gtl_set((char*) GTL_KEY_JOB_REVISION, (char*) "4.0");
    gtl_set((char*) GTL_KEY_MODE_CODE, (char*) "D");
    gtl_set((char*) GTL_KEY_OPERATOR_NAME, (char*) "Kate");
    gtl_set((char*) GTL_KEY_RETEST_CODE, (char*) "N");
    gtl_set((char*) GTL_KEY_ROM_CODE, (char*) "987654321");
    gtl_set((char*) GTL_KEY_SPEC_NAME, (char*) "My superb specs");
    gtl_set((char*) GTL_KEY_SPEC_VERSION, (char*) "specs v1");
    gtl_set((char*) GTL_KEY_SETUP_ID, (char*) "setup21");
    gtl_set((char*) GTL_KEY_TEST_CODE, (char*) "FINAL");
    gtl_set((char*) GTL_KEY_TESTER_NAME, (char*) "my tester");
    gtl_set((char*) GTL_KEY_TESTER_TYPE, (char*) "GStester");
    gtl_set((char*) GTL_KEY_TEST_TEMPERATURE, (char*) "0");
    gtl_set((char*) GTL_KEY_USER_TXT, (char*) "My not too long user text");
}

/******************************************************************************!
 * \fn runOnce
 ******************************************************************************/
int runOnce(int lRun)
{
    int r;

    r = gtl_command((char*) "beginjob");
    if (r != GTL_CORE_ERR_OKAY)
    {
        printf("\ngtl_beginjob failed: %d\n", r);
        // Either just lost the GTM connection,
        // either just offline and cannot reconnect
        if ((r != GTL_CORE_ERR_GTM_COMM) &&
            (r != GTL_CORE_ERR_GTM_INIT))
        {
            return EXIT_FAILURE;
        }
        printf("Let's continue in offline mode...\n");
        gtl_set((char*) GTL_KEY_RECONNECTION_MODE, (char*) "on");
    }

    for (int i = 0; i < gNumOfSites; i++)
    {
        // Todo: mutate get_site_state() to gtl set/get commands
        // gtl_get("site state");
        // PTR
        // r=gtl_test(i, 2001, (char*)"PTR test1",
        //   ((double)rand()/(double)RAND_MAX) * 110.);  // from 0 to 100

        r = gtl_test(i,
                     2001,
                     (char*) "PTR test1",
                     GenerateANormalDitribSample(0., 100.));
        if (r != GTL_CORE_ERR_OKAY)
        {
            GTL_LOG(3, (char*) "gtl_test failed: %d\n", r);
        }

        // From 0 to 2
        r = gtl_test(i, 2002, (char*) "PTR test2",
                     ((double) rand() / (double) RAND_MAX) * 2.);
        if (r != GTL_CORE_ERR_OKAY)
        {
            GTL_LOG(3, (char*) "gtl_test failed: %d\n", r);
        }

        // From 0 to 2
        r = gtl_test(i, 2003, (char*) "PTR test3",
                     ((double) rand() / (double) RAND_MAX) * 2.);
        if (r != GTL_CORE_ERR_OKAY)
        {
            GTL_LOG(3, (char*) "gtl_test failed: %d\n", r);
        }

        // MPR
        int lMPRindexes[] = {
            0, 1, 2
        };
        double lMPR[3];
        for (int q = 0; q < 3; q++)
        {
            // From 0 to 2
            lMPR[q] = ((double) rand() / (double) RAND_MAX) * 2.;
        }
        r = gtl_mptest(i, 3001, (char*) "MPR test1", lMPR, lMPRindexes, 3);

        if (r != GTL_CORE_ERR_OKAY)
        {
            printf("gtl_mptest failed:%d\n", r);
            GTL_LOG(3, (char*) "gtl_mptest failed: %d\n", r);
            return EXIT_FAILURE;
        }

        // Site binning
        int lOriginalBin =
            (int) (((double) rand() / (double) RAND_MAX) * 8.);
        int lNewBin = lOriginalBin;
        char lPartID[1024] = "";
        sprintf(lPartID, "run%d.site%d", lRun, i);

        // In unit test recipe, good bins are 1 to 7
        r = gtl_binning(i,
                           lOriginalBin,
                           lOriginalBin,
                           &lNewBin,
                           &lNewBin,
                           lPartID);
        // Always test if gtl_binning failed or not
        if (r != GTL_CORE_ERR_OKAY)
        {
            printf("gtl_binning failed: %d\n", r);
            gtl_command((char*) GTL_COMMAND_CLOSE);
            return EXIT_FAILURE;
        }
    }

    r = gtl_command((char*) "endjob");
    if (r != GTL_CORE_ERR_OKAY)
    {
        printf("gtl_endjob failed: %d\n", r);
        char lLibState[1024] = "?";
        gtl_get(GTL_KEY_LIB_STATE, lLibState);
        int lLibStateNb = -1;
        sscanf(lLibState, "%d", &lLibStateNb);
        if (r == GTL_CORE_ERR_FAILED_TO_SEND_RESULTS ||
            lLibStateNb != GTL_STATE_ENABLED)
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

/******************************************************************************!
 * \fn runOnceAndWait
 ******************************************************************************/
int runOnceAndWait(int lRun, int gNumOfRuns, time_t start_t,
                   int expected_time_for_the_whole_lot)
{
    float ratio;

    if (runOnce(lRun) == EXIT_FAILURE)
    {
        printf("gtl run failed: run=%d\n", lRun);
        gtlClose();
        exit(EXIT_FAILURE);
    }

    // Let's compute progress ratio
    ratio = (float) lRun / (float) gNumOfRuns;
    // Let's wait if too fast
    time_t current;
    time(&current);
    int current_elapsed_time = current - start_t;
    while (((int) (ratio * (float) expected_time_for_the_whole_lot)) >
           current_elapsed_time)
    {
        // We are too fast, let s wait max 1 sec
        time(&current);
        current_elapsed_time = current - start_t;
    }

    return EXIT_SUCCESS;
}

/******************************************************************************!
 * \fn run
 * \brief Runs <lNum> run and make sure the state is <lExp>
 ******************************************************************************/
void run(int lNum, int lExp)
{
    gtl_set((char*) GTL_KEY_DESIRED_SITE_NB, (char*) "0");

    if (lNum == 1) {
        runOnce(0);
        gtl_get((char*) GTL_KEY_SITE_STATE, gRes);
        if ((lExp == GTL_SITESTATE_BASELINE && *gRes != '0') ||
            (lExp == GTL_SITESTATE_DPAT && *gRes != '1'))
        {
            printf("gtl get GTL_KEY_SITE_STATE failed: run=%d, res=%s\n",
                   0, gRes);
            gtlClose();
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        time_t start_t;
        int lRun;
        int expected_time_for_the_whole_lot =
            (int) (((float) gNumOfRuns) *
                   (((float) gNumOfMsecPerRun) / 1000.f));

        time(&start_t);

        for (lRun = 0; lRun < lNum; lRun++)
        {
            runOnceAndWait(lRun, lNum, start_t,
                           expected_time_for_the_whole_lot);
            gtl_get((char*) GTL_KEY_SITE_STATE, gRes);
            if ((lExp == GTL_SITESTATE_BASELINE && *gRes == '0') ||
                (lExp == GTL_SITESTATE_DPAT && *gRes == '1'))
            {
                break;
            }
        }
        if (lRun == lNum)
        {
            printf("gtl get GTL_KEY_SITE_STATE failed: run=%d, res=%s\n",
                   lRun, gRes);
            gtlClose();
            exit(EXIT_FAILURE);
        }
    }
}

/******************************************************************************!
 * \fn unlinkSqlite
 ******************************************************************************/
void unlinkSqlite()
{
    gtl_get((char*) GTL_KEY_TEMP_FOLDER, gRes);
    strcat(gRes, "/gtl.sqlite");
    unlink(gRes);
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main()
{
    int r;

    // With this seed and recipe and default args,
    // we should expect 185 PAT at least on win32
    srand(69);

    // Reconnection mode
    char lRM[256] = "";
    gtl_get((char*) GTL_KEY_RECONNECTION_MODE, lRM);
    printf("Reconnection mode %s\n", lRM);
    gtl_set((char*) GTL_KEY_RECONNECTION_MODE, (char*) "on");

    // Let's check GTL version
    char lLV[255] = "?";
    gtl_get((char*) GTL_KEY_LIB_VERSION, lLV);
    printf("GTL version: %s\n", lLV);

    // Mandatory: Setting the number of sites expected for the session
    char lNumOfSitesString[255] = "";
    sprintf(lNumOfSitesString, "%d", gNumOfSites);
    r = gtl_set((char*) GTL_KEY_MAX_NUMBER_OF_ACTIVE_SITES, lNumOfSitesString);
    if (r != 0)
    {
        printf("Failed to set num of sites: %d\n", r);
        exit(EXIT_FAILURE);
    }
    // Mandatory: set site numbers
    char lSiteNumbersString[2048] = "";
    char lSiteNumberString[4];
    for (int i = 0; i < gNumOfSites && i < 255; ++i)
    {
        // Max number of sites is anyway today 255
        sprintf(lSiteNumberString, "%d ", i);
        strcat(lSiteNumbersString, lSiteNumberString);
    }
    r = gtl_set((char*) GTL_KEY_SITES_NUMBERS, lSiteNumbersString);
    if (r != 0)
    {
        printf("Failed to set sites numbers: %d", r);
        exit(EXIT_FAILURE);
    }

    // Setting the max mem for GTL messages :
    // Currently a message is 1032 octet, so
    // 30000 bytes will allow 30 messages max in the stack
    gtl_set((char*) GTL_KEY_MAX_MESSAGES_STACK_SIZE, (char*) "90064");

    gtl_command((char*) "-gtl_debugon");
    gtl_command((char*) "-gtl_testlist");

    gtl_get((char*) GTL_KEY_FIELDS_TO_MATCH, gRes);
    if (strcmp(gRes,
               "product_id,lot_id,sublot_id,tester_name,tester_type") != 0)
    {
        printf("gtl get GTL_KEY_FIELDS_TO_MATCH failed: %s\n", gRes);
        return EXIT_FAILURE;
    }

    printf("Test 1 (default parameters)\n");
    unlinkSqlite();
    gtlInit();
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 2 (all parameters)\n");
    gtl_set((char*) GTL_KEY_FIELDS_TO_MATCH, (char*)
            "product_id,lot_id,sublot_id,tester_name,tester_type,"
            "station_number,retest_code,retest_hbins,job_name,"
            "job_revision,operator_name,test_code,facility_id,"
            "test_temperature,user_text,family_id,spec_name,datafile_name");
    gtlInit();
    gtlOpen();
    run(1, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 3 (syntax error)\n");
    gtl_set((char*) GTL_KEY_FIELDS_TO_MATCH, (char*)
            "product_id,lotid,sublot_id,tester_name,tester_type");
    gtlInit();
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    gtl_set((char*) GTL_KEY_FIELDS_TO_MATCH, (char*)
            "product_id,lot_id,sublot_id,tester_name,tester_type,"
            "station_number,retest_code,retest_hbins,job_name,"
            "job_revision,operator_name,test_code,facility_id,"
            "test_temperature,user_text,family_id,spec_name,datafile_name");

    printf("Test 4 (Product)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_PRODUCT_ID, (char*) "ProductB");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 5 (Lot)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_LOT_ID, (char*) "Lot3");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 6 (Sublot)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_SUBLOT_ID, (char*) "SublotY");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 7 (TesterName)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_TESTER_NAME, (char*) "my tester 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 8 (TesterType)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_TESTER_TYPE, (char*) "GStester2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 9 (Station)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_STATION_NUMBER, (char*) "7070");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 10 (ProgramName)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_JOB_NAME, (char*) "unit test 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 11 (ProgramRevision)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_JOB_REVISION, (char*) "4.1");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 12 (Operator)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_OPERATOR_NAME, (char*) "Seb");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 13 (TestingCode)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_TEST_CODE, (char*) "WAFER");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 14 (Facility)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_FACILITY_ID, (char*) "My superb facility 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 15 (Temperature)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_TEST_TEMPERATURE, (char*) "5");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 16 (UserText)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_USER_TXT, (char*) "My not too long user text 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 17 (Family)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_FAMILY_ID, (char*) "My big family 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 18 (SpecName)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_SPEC_NAME, (char*) "My superb specs 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 19 (FileName)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_DATA_FILENAME, (char*) "none 2");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 20 (Reload limits default value)\n");
    gtl_get((char*) GTL_KEY_RELOAD_LIMITS, gRes);
    if (strcmp(gRes, "on") != 0)
    {
        printf("gtl default value for reload limits failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Test 21 (Reload limits off)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_RELOAD_LIMITS, (char*) "off");
    gtlOpen();
    run(1, GTL_SITESTATE_BASELINE);
    run(gNumOfRuns, GTL_SITESTATE_DPAT);
    gtlClose();

    printf("Test 22 (Reload limits on)\n");
    gtlInit();
    gtl_set((char*) GTL_KEY_RELOAD_LIMITS, (char*) "on");
    gtlOpen();
    run(1, GTL_SITESTATE_DPAT);
    gtlClose();

    return EXIT_SUCCESS;
}
