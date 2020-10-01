#include "export_csv_test.h"

CFileDataInfo::CFileDataInfo()
{
    clear();
}

void CFileDataInfo::clear()
{
    // Clear some fields
    m_strLotID              = "";					// Dataset Lot #
    m_strWaferID            = "";				// Dataset Wafer #
    m_strWaferOrientation   = "Unknown";
    m_tEndTime              = 0;
    m_tSetupTime            = 0;
    m_tStartTime            = 0;
    m_chrPosX               = ' ';
    m_chrPosY               = ' ';
    m_fWaferSize            = 0;
    m_fDieWidth             = 0;
    m_fDieHeight            = 0;
    m_chrWaferFlat          = ' ';
    m_sWaferCenterX         = -32768;
    m_sWaferCenterY         = -32768;
    m_cWaferUnits           = 0;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CFTRPatternResult
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////

CFTRPatternResult::CFTRPatternResult() : m_bValid(false), m_bResult(false)
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CFTRPatternResult::setResult(bool bSuccess)
//
// Description	:	Sets the FTR pattern result
//
///////////////////////////////////////////////////////////////////////////////////
void CFTRPatternResult::setResult(bool bSuccess)
{
    m_bValid	= true;
    m_bResult	= bSuccess;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CShortTest
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CShortTest::CShortTest()
{
    ptNextTest              = NULL;
    lfResult.clear();                           // Clear all test results for all sites.
    lTestNumber             = 0;				// TestNumber
    m_nTestFlowID           = -1;
    res_scal                = 0;				// No scaling
    llm_scal                = 0;
    hlm_scal                = 0;
    bTestType               = ' ';              // TestType: Parametric,Functional,...
    lfLowLimit              = 0;                // Test Low Limit
    lfHighLimit             = 0;                // Test High Limit
    bLimitFlag              = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL | CTEST_LIMITFLG_NOLSL | CTEST_LIMITFLG_NOHSL;	// No limits defined
    strTestName             = "";               // Test Name.
    *szTestLabel            = 0;                // TestNumber.Pinmap# ...string used in HTML, CSV reports
    *szTestUnits            = 0;                // Test Units.
    *szLowL                 = 0;				// Test low limit.
    *szHighL                = 0;				// Test low limit.
}
CShortTest::~CShortTest()
{
    while (mResultArrayIndexes.isEmpty() == false)
      delete[] mResultArrayIndexes.takeFirst();
}
