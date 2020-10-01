#include "max_shift_calculator.h"
#include "gex_report.h"

#include <list>
#include <map>

/**
 * Symbols declared in an unnamed namespace have internal linkage and are isolated in this translation unit
 */
namespace
{

/**
 * @brief The MaxShiftCalculatorDetails class contains all implementation details related to
 * maximum statistics shifts calculations
 */
class MaxShiftCalculatorDetails
{
public :
    /**
     * @brief The ShiftStats struct contains statistical values from which we want to compute maximum shifts
     */
    struct ShiftStats
    {
        double mMean, mSigma, mCP, mCPK;
    };

    typedef std::list< ShiftStats > ShiftStatsForAllGroupOfFiles;
    typedef std::map< CTest * const, ShiftStatsForAllGroupOfFiles > AllTestsShiftStats;

    /**
     * @brief The ByMeanDesc struct is a functor used in a sorting operation (pre c++11)
     */
    struct ByMeanDesc
    {
        bool operator()( const ShiftStats &lLeft, const ShiftStats &lRight ) const
        { return lLeft.mMean > lRight.mMean; }
    };

    /**
     * @brief The BySigmaDesc struct is a functor used in a sorting operation (pre c++11)
     */
    struct BySigmaDesc
    {
        bool operator()( const ShiftStats &lLeft, const ShiftStats &lRight ) const
        { return lLeft.mSigma > lRight.mSigma; }
    };

    /**
     * @brief The ByCPDesc struct is a functor used in a sorting operation (pre c++11)
     */
    struct ByCPDesc
    {
        bool operator()( const ShiftStats &lLeft, const ShiftStats &lRight ) const
        { return lLeft.mCP > lRight.mCP; }
    };

    /**
     * @brief The ByCPKDesc struct is a functor used in a sorting operation (pre c++11)
     */
    struct ByCPKDesc
    {
        bool operator()( const ShiftStats &lLeft, const ShiftStats &lRight ) const
        { return lLeft.mCPK > lRight.mCPK; }
    };

public :
    MaxShiftCalculatorDetails( CGexReport *aUnderlyingReport ) : mUnderlyingReport( aUnderlyingReport ) {}

    void Compute();

private :
    CGexReport *mUnderlyingReport;
    ShiftStatsForAllGroupOfFiles mTestShiftStatsCacheForAllGroupOfFiles;
    AllTestsShiftStats mAllTestsShiftStats;

private :
    void CollectAllTestsShiftStats();
    CTest * GetReferenceGroupFileTestList();
    CGexGroupOfFiles * GetReferenceGroupOfFiles();
    void AddOneTestShiftStats( CTest *aTestToAdd );
    void AddComparedTestShiftStats( int aGroupOfFileIndex, CTest *aReferenceTestList );
    CTest * GetTestToCompare( int aGroupOfFileIndex, CTest *aReferenceTestList );
    CTest * FindTestCellToCompare( const CGexGroupOfFiles *aComparedGroup, CTest *aReferenceTest );
    int GetPinmapIndexFromTest( CTest *aTest );
    void AddTestShiftStats( CTest *aComparedTest );
    void RecordAllShiftStatsInReferenceTests();
    void RecordShiftStatsInOneReferenceTest( AllTestsShiftStats::iterator aStatsIt );
};

}

static bool IsThereAtLeastTwoGroupsOfFiles( CGexReport *aUnderlyingReport )
{
    const int lGroupOfFileCount = aUnderlyingReport->getGroupsList().count();

    if( lGroupOfFileCount <= 1)
        return false;

    return true;
}

CGexGroupOfFiles * MaxShiftCalculatorDetails::GetReferenceGroupOfFiles()
{
    return mUnderlyingReport->getGroupsList().first();
}

CTest * MaxShiftCalculatorDetails::GetReferenceGroupFileTestList()
{
    const CGexGroupOfFiles * const lReferenceGroupOfFiles = GetReferenceGroupOfFiles();

    return lReferenceGroupOfFiles->cMergedData.ptMergedTestList;
}

int MaxShiftCalculatorDetails::GetPinmapIndexFromTest( CTest *aTest )
{
    if( aTest->lPinmapIndex >= 0 )
        return aTest->lPinmapIndex;

    return GEX_PTEST;
}

CTest * MaxShiftCalculatorDetails::FindTestCellToCompare( const CGexGroupOfFiles *aComparedGroup, CTest *aReferenceTest )
{
    CGexFileInGroup * const lFileToCompare = aComparedGroup->pFilesList.first();
    int lPinmapIndex = GetPinmapIndexFromTest( aReferenceTest ) ;

    CTest *lComparedTest;
    if(lFileToCompare->FindTestCell( aReferenceTest->lTestNumber,
                                     lPinmapIndex,
                                     &lComparedTest,
                                     false,
                                     false,
                                     aReferenceTest->strTestName.toLatin1().data() ) != 1 )
        return NULL;

    return lComparedTest;
}

CTest * MaxShiftCalculatorDetails::GetTestToCompare( int aGroupOfFileIndex, CTest *aReferenceTestList )
{
    const CGexGroupOfFiles *lComparedGroup = mUnderlyingReport->getGroupsList().at( aGroupOfFileIndex );

    // defensive stuff
    if ( ! lComparedGroup || lComparedGroup->pFilesList.isEmpty() )
        return NULL;

    return FindTestCellToCompare( lComparedGroup, aReferenceTestList );
}

void MaxShiftCalculatorDetails::AddTestShiftStats( CTest *aComparedTest )
{
    ShiftStats lShiftStats;
    const GS::Core::MultiLimitItem * const lMLItem = aComparedTest->GetCurrentLimitItem();

    lShiftStats.mMean = aComparedTest->lfMean;
    lShiftStats.mSigma = aComparedTest->lfSigma;
    lShiftStats.mCP = lMLItem->lfCp;
    lShiftStats.mCPK = lMLItem->lfCpk;

    mTestShiftStatsCacheForAllGroupOfFiles.push_back( lShiftStats );
}

void MaxShiftCalculatorDetails::AddComparedTestShiftStats( int aGroupOfFileIndex, CTest *aReferenceTestList )
{
    CTest *lComparedTest = GetTestToCompare( aGroupOfFileIndex, aReferenceTestList );

    if( lComparedTest == NULL )
        return;

    AddTestShiftStats( lComparedTest );
}

void MaxShiftCalculatorDetails::AddOneTestShiftStats( CTest *aTestToAdd )
{
    for( int lInnerGroupOfFileIndex = 0;
         lInnerGroupOfFileIndex < mUnderlyingReport->getGroupsList().count();
         ++lInnerGroupOfFileIndex )
        AddComparedTestShiftStats( lInnerGroupOfFileIndex, aTestToAdd );

    mAllTestsShiftStats[ aTestToAdd ] = mTestShiftStatsCacheForAllGroupOfFiles;

    mTestShiftStatsCacheForAllGroupOfFiles.clear();
}

void MaxShiftCalculatorDetails::CollectAllTestsShiftStats()
{
    for( CTest *lReferenceTestList = GetReferenceGroupFileTestList();
         lReferenceTestList != NULL;
         lReferenceTestList = lReferenceTestList->GetNextTest() )
        AddOneTestShiftStats( lReferenceTestList );
}

void MaxShiftCalculatorDetails::RecordShiftStatsInOneReferenceTest( AllTestsShiftStats::iterator aStatsIt )
{
    CTest * const lTestFromReferenceGroupOfFiles = aStatsIt->first;
    ShiftStatsForAllGroupOfFiles &lStatsForAllGroupOfFiles = aStatsIt->second;

    GS::Core::MLShift lMLShift = lTestFromReferenceGroupOfFiles->mTestShifts[ 0 ].GetMlShift( lTestFromReferenceGroupOfFiles->GetCurrentLimitItem() );

    lStatsForAllGroupOfFiles.sort( ByMeanDesc() );
    //lMLShift.SetMeanShiftValue( lStatsForAllGroupOfFiles.front().mMean - lStatsForAllGroupOfFiles.back().mMean );
    if( lStatsForAllGroupOfFiles.back().mMean != 0.0 )
        lMLShift.SetMeanShiftPct( ( lStatsForAllGroupOfFiles.front().mMean / lStatsForAllGroupOfFiles.back().mMean - 1 ) * 100 );

    lStatsForAllGroupOfFiles.sort( BySigmaDesc() );
    lMLShift.SetSigmaShiftValue( lStatsForAllGroupOfFiles.front().mSigma - lStatsForAllGroupOfFiles.back().mSigma );
    if( lStatsForAllGroupOfFiles.back().mSigma != 0.0 )
        lMLShift.SetSigmaShiftPct( ( lStatsForAllGroupOfFiles.front().mSigma / lStatsForAllGroupOfFiles.back().mSigma - 1 ) * 100 );

    lStatsForAllGroupOfFiles.sort( ByCPDesc() );
    lMLShift.SetCpShiftValue( lStatsForAllGroupOfFiles.front().mCP - lStatsForAllGroupOfFiles.back().mCP );
    if( lStatsForAllGroupOfFiles.back().mCP != 0.0 )
        lMLShift.SetCpShiftPct( ( lStatsForAllGroupOfFiles.front().mCP / lStatsForAllGroupOfFiles.back().mCP - 1 ) * 100 );

    lStatsForAllGroupOfFiles.sort( ByCPKDesc() );
    lMLShift.SetCpkShiftValue( lStatsForAllGroupOfFiles.front().mCPK - lStatsForAllGroupOfFiles.back().mCPK );
    if( lStatsForAllGroupOfFiles.back().mCPK != 0.0 )
        lMLShift.SetCpkShiftPct( ( lStatsForAllGroupOfFiles.front().mCPK / lStatsForAllGroupOfFiles.back().mCPK - 1 ) * 100 );

    lTestFromReferenceGroupOfFiles->mTestShifts.first().SetMlShift( lTestFromReferenceGroupOfFiles->GetCurrentLimitItem(), lMLShift );
}

void MaxShiftCalculatorDetails::RecordAllShiftStatsInReferenceTests()
{
    for( AllTestsShiftStats::iterator lStatsIt = mAllTestsShiftStats.begin(), lStatsItEnd = mAllTestsShiftStats.end();
         lStatsIt != lStatsItEnd;
         ++lStatsIt )
        RecordShiftStatsInOneReferenceTest( lStatsIt );
}

MaxShiftCalculator::MaxShiftCalculator(CGexReport *aReport) : mUnderlyingReport( aReport ) {}

void MaxShiftCalculator::ComputeMaxShiftInReferenceGroupOfFiles()
{
    if( ! ::IsThereAtLeastTwoGroupsOfFiles( mUnderlyingReport ) )
        return;

    MaxShiftCalculatorDetails lCalculator( mUnderlyingReport );

    lCalculator.Compute();
}

void MaxShiftCalculatorDetails::Compute()
{
    CollectAllTestsShiftStats();
    RecordAllShiftStatsInReferenceTests();
}
