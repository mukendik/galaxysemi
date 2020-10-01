/******************************************************************************!
 * \file memsize.cpp
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <QMap>
#include <QVariant>
#include <gqtl_sysutils.h>

/******************************************************************************!
 * \fn getMemInfo
 ******************************************************************************/
void getMemInfo(int& total, int& avail, int& rsize)
{
    QMap<QString, QVariant> lMemInfo =
        CGexSystemUtils::GetMemoryInfo(false, true);

    total = lMemInfo["TotalPhysMem"].toString().replace("Mo", "").toInt();
    avail = lMemInfo["AvailPhys"].toString().replace("Mo", "").toInt();
    rsize = lMemInfo["MemUsedByProcess"].toString().replace("Mo", "").toInt();
}

/******************************************************************************!
 * \fn TestMemAllocation
 ******************************************************************************/
bool TestMemAllocation(int memsize, double allowedDiffPct)
{
    const uint MEM_TO_ALLOCATE = memsize;  // Mo
    const ulong MALLOC_SIZE = MEM_TO_ALLOCATE * 1048576l;
    ulong i;
    int total_before, avail_before, used_by_process_before;
    int total_after, avail_after, used_by_process_after;
    double lPctDiffUsedByProcess;

    printf("\nChecking with %dMo...", memsize);

    // Check mem used
    getMemInfo(total_before, avail_before, used_by_process_before);

    // Check if enough memory
    if ((uint) avail_before < MEM_TO_ALLOCATE)
    {
        printf("Not enough mem: available(%dMo) < mem to allocate(%uMo)\n",
               avail_before, MEM_TO_ALLOCATE);
        return true;
    }

    // Allocate memory
    char* m = (char*) ::malloc(MALLOC_SIZE);
    if (m == NULL)
    {
        printf(" FAIL: malloc return NULL\n");
        return false;
    }
    for (i = 0; i < MALLOC_SIZE; ++i)
    {
        m[i] = i % 256;
    }
    // Check mem used
    getMemInfo(total_after, avail_after, used_by_process_after);
    printf("Process now uses %dMo\n", used_by_process_after);

    // Free mem
    ::free(m);

    int lTmp = qAbs(qAbs(used_by_process_after
                         - used_by_process_before)
                    - (int) MEM_TO_ALLOCATE);
    if (lTmp == 0)
    {
        lPctDiffUsedByProcess = 0;
    }
    else
    {
        lPctDiffUsedByProcess =
            (double) 100.0 * (lTmp / (double) MEM_TO_ALLOCATE);
    }

    // if diff greater than allowed percentage --> failed
    if(lPctDiffUsedByProcess > allowedDiffPct)
    {
        printf(" FAIL:\n");
        printf("\tTotal mem: before:%d --> after:%d\n",
               total_before,
               total_after);
        printf("\tAvailable mem: before:%d --> after:%d\n",
               avail_before,
               avail_after);
        printf("\tUsed mem by process: before:%d --> after:%d\n",
               used_by_process_before,
               used_by_process_after);
        printf("\tDiff of %f %% from expected used memory, "
               "greater than %f\n",
               lPctDiffUsedByProcess,
               allowedDiffPct);
        return false;
    }

    printf(" OK\n");
    return true;
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main()
{
    // Allowed diff %
    // The OS Available mem is hardly predictable
    // Example : linux32 : allocating 128 Mo :
    // available before 216 after 133 : it should be 216 - 128 = 88 Mo
    // Let s increase the percent
    double lAllowedDiffPct = 40;
    printf("Test of memory : %.0f %% of error between avaiable and"
           " estimated will be tolerated\n", lAllowedDiffPct);

    // on windows 64bits: bellow 8 MB the test is not reliable
    // on linux 64bits: bellow 2 MB the test is not reliable
    bool lTest0 = TestMemAllocation(8, lAllowedDiffPct);
    bool lTest1 = TestMemAllocation(32, lAllowedDiffPct);
    bool lTest2 = TestMemAllocation(64, lAllowedDiffPct);
    bool lTest3 = TestMemAllocation(128, lAllowedDiffPct);
    bool lTest4 = TestMemAllocation(512, lAllowedDiffPct);
    bool lTest5 = TestMemAllocation(1024, lAllowedDiffPct);

    // under 32bit, it is usually impossible to alloc more than 2G
    // Let's do it only under 64b
    bool lTest6 = true;
    bool lTest7 = true;
    if ((sizeof(void*) << 3) > 32)
    {
        lTest6 = TestMemAllocation(2048, lAllowedDiffPct);
        lTest7 = TestMemAllocation(4096, lAllowedDiffPct);
    }
    bool lResult =
        lTest0 && lTest1 && lTest2 && lTest3 &&
        lTest4 && lTest5 && lTest6 && lTest7;

    if (! lResult)
    {
        printf(" FAIL\n");
        return EXIT_FAILURE;
    }

    printf(" OK\n");
    return EXIT_SUCCESS;
}
