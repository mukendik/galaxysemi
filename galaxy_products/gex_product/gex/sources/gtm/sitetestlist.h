#ifndef SITETESTLIST_H
#define SITETESTLIST_H

#include <QObject>

// Move me into a global header !
#if defined unix || __MACH__
  #define STRING_COMPARE(a,b)  (strcasecmp(a,b))
#else
  #define STRING_COMPARE(a,b)  (stricmp(a,b))
#endif

class CTest;

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////
// Holds test list for a given site
///////////////////////////////////////////////////////////
class SiteTestList : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SiteTestList)
    static unsigned sNumOfInstances;

public:
    enum siteTestKey
    {
        TESTNUMBER,
        TESTNAME,
        TESTNAME_AND_TESTNUMBER
    };

    // Constructor/Destructor
    SiteTestList();
    virtual ~SiteTestList();

    // Init counters/variables
    void        Init();
    // Clear counters/variables
    void        Clear();
    // Reset test results (invalidate all results, reset some counters)
    void        ResetResults();

    // Get
    CTest*      TestList() {return mTestList;}

    // Set
    void SetTestKey(const siteTestKey TestKey) {mTestKey=TestKey;}

    // Find Test Cell in Test List
    CTest* FindTestCell(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew, bool ResetList,
                        const QString &TestName);

private:

    /*!
      @brief    Create a test and insert it at the current position in the test list

      @param    lTestNumber Test number
      @param    lRtnIndex   RTN index (-1 if PTR)
      @param    lTestName   Test name

      @return   Pointer on the created instance. Return NULL if it fails
      */
    CTest *     CreateTestCell(unsigned int lTestNumber, int lRtnIndex, const QString &lTestName);

    // TestList find functions
    // Find Test Cell (by name) structure in Test List. Does not test test number at all, just test name.
    CTest* FindTestCellName(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                            bool ResetList, const QString &TestName);

    // Find Test Cell structure (by test# and test name) in Test List.
    // If test list null, it will create a new CTest
    // if bMergeDuplicateTestNumber is true (default), wont compare test name
    CTest* FindTestCellNumber(unsigned int TestNumber, int lRtnIndex, bool CreateIfNew,
                              bool ResetList, const QString &TestName);

    siteTestKey mTestKey;           // Key for identifying unique test
    CTest*      mTestList;          // Pointer to the list of Test cells.
    CTest*      mPrevCell;          // After call to 'FindTestCell', points to test preceeding test# searched.
    CTest*      mCellFound;         // Internal variable, used in 'FindTestCell'
    bool        bMergeDuplicateTestName;
    bool        bMergeDuplicateTestNumber;
};

} // Gex

} // GS

#endif // SITETESTLIST_H
