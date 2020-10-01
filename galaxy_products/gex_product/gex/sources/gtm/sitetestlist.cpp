#include <gqtl_log.h>
#include "sitetestlist.h"
#include "ctest.h"

namespace GS
{
namespace Gex
{

unsigned SiteTestList::sNumOfInstances=0;

SiteTestList::SiteTestList()
{
    sNumOfInstances++;
    Init();
    Clear();
}

SiteTestList::~SiteTestList()
{
    sNumOfInstances--;
    Clear();
}

void SiteTestList::Clear()
{
    // Destroy Test list
    CTest *lTestCell = mTestList;
    CTest *lNextTest=0;
    while(lTestCell != NULL)
    {
        lNextTest = lTestCell->GetNextTest();
        delete lTestCell;
        lTestCell = lNextTest;
    };
    mTestList=NULL;
    mPrevCell=NULL;
    mCellFound=NULL;
    bMergeDuplicateTestName=true;
    bMergeDuplicateTestNumber=true;
}

void SiteTestList::ResetResults()
{
    // Loop through all tests
    CTest *lTestCell = mTestList;
    while(lTestCell != NULL)
    {
        // Invalidate test results
        for(long lIndex=0; lIndex<lTestCell->m_testResult.count(); ++lIndex)
            lTestCell->m_testResult.invalidateResultAt(lIndex);
        // Reset some counters
        lTestCell->lfSamplesTotal = 0;          // Sum of X
        lTestCell->lfSamplesTotalSquare = 0;	// Sum of X*X
        lTestCell->ldSamplesExecs = 0;          // Total samples collected
        lTestCell = lTestCell->GetNextTest();
    };
}

void SiteTestList::Init()
{
    mTestKey = TESTNUMBER;
    // Pointers
    mTestList=NULL;
    mPrevCell=NULL;
    mCellFound=NULL;
}

CTest* SiteTestList::FindTestCell(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                                  bool ResetList, const QString & TestName)
{
    if (mTestKey == TESTNAME_AND_TESTNUMBER)
    {
        bMergeDuplicateTestNumber=false;
        return FindTestCellNumber(TestNumber, lRtnIndex, CreateIfNew, ResetList, TestName);
    }

    if(mTestKey == TESTNAME)
        return FindTestCellName(TestNumber, lRtnIndex, CreateIfNew, ResetList, TestName);
    else
        return FindTestCellNumber(TestNumber, lRtnIndex, CreateIfNew, ResetList, TestName);
}

CTest *SiteTestList::CreateTestCell(unsigned int lTestNumber, int lRtnIndex, const QString &lTestName)
{
    CTest * lNewTest = new CTest;

    lNewTest->lTestNumber   = lTestNumber;
    lNewTest->lPinmapIndex  = lRtnIndex;
    lNewTest->strTestName   = lTestName;
    //lNewTest->ptNextTest    = NULL; it is set to null in the constructer

    if(mPrevCell == NULL)
    {
        // This cell becomes head of list
        // This list is only a local list. Not used in the report all pages.
        lNewTest->SetNextTest(mTestList);
        mTestList = lNewTest;

    }
    else
    {
        // Insert cell in list
        lNewTest->SetNextTest(mPrevCell->GetNextTest());
        mPrevCell->SetNextTest(lNewTest);
    }

    return lNewTest;	// Success
}

CTest* SiteTestList::FindTestCellName(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                                      bool ResetList, const QString & TestName)
{
    CTest*  lTestCell   = 0;
    int     lCompare    = 0;

    // Clear pointer that keeps track of previous test.
    mPrevCell = NULL;

    if(TestName.isEmpty())
        return NULL;

    if(mTestList == NULL)
    {
        if(CreateIfNew)
        {
            // First test : list is currently empty.
            mTestList = new CTest;
            mTestList->lTestNumber = TestNumber;
            mTestList->strTestName = TestName;
//            mTestList->ptNextTest  = NULL;
            mCellFound = mTestList;
            mPrevCell = NULL;
            return mTestList;	// Success
        }

        return NULL;
    }

    // Check if rewind list prior to search in it.
    if (ResetList || mCellFound == NULL ||
        mCellFound->strTestName.compare(TestName, Qt::CaseInsensitive) > 0 ||
        (mCellFound->strTestName.compare(TestName, Qt::CaseInsensitive) == 0 &&
         mCellFound->lPinmapIndex > lRtnIndex))
    {
        // Start from first test in list
        mCellFound = mTestList;
        mPrevCell = NULL;
    }

    // Scan list from current cell to find cell (if exists)
    lTestCell = mCellFound;
    while(lTestCell != NULL)
    {
        // We've passed the cell
        lCompare = lTestCell->strTestName.compare(TestName, Qt::CaseInsensitive);
        if(lCompare > 0)
        {
            if(CreateIfNew)
            {
                mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
                return mCellFound;
            }
            else
                return NULL;
        }

        if(lCompare == 0)
        {
            if (mCellFound->lPinmapIndex > lRtnIndex)
            {
                if(CreateIfNew)
                {
                    mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
                    return mCellFound;
                }
                else
                    return NULL;
            }

            if (mCellFound->lPinmapIndex == lRtnIndex)
            {
                mCellFound = lTestCell;
                return mCellFound; // Test found, pointer to it returned.
            }
        }

        mPrevCell = lTestCell;
        lTestCell = lTestCell->GetNextTest();
    };

    if(CreateIfNew)
    {
        mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
        return mCellFound;
    }
    else
        return NULL;
}

CTest* SiteTestList::FindTestCellNumber(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                                        bool ResetList, const QString &TestName)
{
    CTest* lTestCell = 0;

    // Clear pointer that keeps track of previous test.
    mPrevCell = NULL;

    if(mTestList == NULL)
    {
        if(CreateIfNew == true)
        {
            // First test : list is currently empty.
            mTestList = new CTest;
            mTestList->lTestNumber  = TestNumber;
            mTestList->lPinmapIndex = lRtnIndex;
            mTestList->strTestName  = TestName;
//            mTestList->ptNextTest   = NULL;
            mCellFound              = mTestList;
            mPrevCell               = NULL;

            return mTestList;	// Success
        }
        else
            return NULL;
    }

    // Check if rewind list prior to search in it.
    if (ResetList || mCellFound == NULL || mCellFound->lTestNumber > TestNumber ||
        (mCellFound->lTestNumber == TestNumber &&
         (mCellFound->lPinmapIndex > lRtnIndex ||
         (mCellFound->lPinmapIndex == lRtnIndex && mCellFound->strTestName.compare(TestName)) > 0)))
    {
        // Start from first test in list
        mCellFound = mTestList;
        mPrevCell = NULL;
    }

    // Scan list from current cell to find cell (if exists)
    lTestCell = mCellFound;
    while(lTestCell != NULL)
    {
        // We've passed the cell
        if(lTestCell->lTestNumber > TestNumber)
        {
            if(CreateIfNew)
            {
                mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
                return mCellFound;
            }
            else
                return NULL;
        }

        // Matching test#
        if(lTestCell->lTestNumber == TestNumber)
        {
            if (lTestCell->lPinmapIndex > lRtnIndex)
            {
                if(CreateIfNew)
                {
                    mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
                    return mCellFound;
                }
                else
                    return NULL;
            }

            if (lTestCell->lPinmapIndex == lRtnIndex)
            {
                // If searching for a given Name
                if(TestName.isEmpty() || bMergeDuplicateTestNumber == true ||
                   lTestCell->strTestName == TestName)
                {
                    mCellFound = lTestCell;
                    return mCellFound; // Test found, pointer to it returned.
                }
                else if (lTestCell->strTestName.compare(TestName) > 0)
                {
                    if(CreateIfNew)
                    {
                        mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
                        return mCellFound;
                    }
                    else
                        return NULL;
                }
            }
        }

        mPrevCell = lTestCell;
        lTestCell = lTestCell->GetNextTest();
    };

    // Cell not in list, if we have to create it, let's do it
    if(CreateIfNew)
    {
        mCellFound = CreateTestCell(TestNumber, lRtnIndex, TestName);
        return mCellFound;
    }
    else
        return NULL;
}

} // Gex
} // GS

