///////////////////////////////////////////////////////////
// Constants for Yield123 module
///////////////////////////////////////////////////////////

#ifndef YIELD123_CONSTANTS_H
#define YIELD123_CONSTANTS_H

// Testing stage IDs
#define YIELD123_TESTINGSTAGE_ID_UNKNOWN	0	// Undefined testing stage
#define YIELD123_TESTINGSTAGE_ID_ETEST		1	// Electrical test
#define YIELD123_TESTINGSTAGE_ID_WTEST		2	// Wafer test
#define YIELD123_TESTINGSTAGE_ID_FTEST		3	// Final test

// Testing stage Names
#define YIELD123_TESTINGSTAGE_NAME_UNKNOWN	"Undefined"	// Undefined testing stage
#define YIELD123_TESTINGSTAGE_NAME_ETEST	"ETEST"		// Electrical test
#define YIELD123_TESTINGSTAGE_NAME_WTEST	"WTEST"		// Wafer test
#define YIELD123_TESTINGSTAGE_NAME_FTEST	"FTEST"		// Final test

// Debug directory and page names
#define YIELD123_DEBUGDIRECTORY				"y123_debug"
#define YIELD123_DEBUGPAGE_RESULT			"y123_debug_result.htm"	

// Some enum types for filtering
enum ResultFilter
{
	eResult_Any,			// Use any valid result for a given parameter
	eResult_Outlier,		// Use only outlier results for a given parameter
	eResult_LowOutlier,		// Use only low outlier results (< Mean - n*Sigma) for a given parameter
	eResult_HighOutlier,	// Use only high outlier results (> Mean + n*Sigma) for a given parameter
	eResult_NotOutlier,		// Use only non-outlier results for a given parameter
	eResult_Pass,			// Use only Pass results for a given parameter
	eResult_Fail,			// Use only Fail results for a given parameter
	eResult_LowFail,		// Use only low Fail results (<LL) for a given parameter
	eResult_HighFail		// Use only high Fail results (>HL) for a given parameter
};

enum RunFilter
{
	eRun_Any,					// Use any run
	eRun_Pass,					// Use only good runs
	eRun_Fail,					// Use only failing runs
	eRun_PassWithOutliers,		// Use only PASS runs with outliers
	eRun_FailWithOutliers,		// Use only FAIL runs with outliers
	eRun_WithOutliers,			// Use only runs with outliers
	eRun_LastRetestWithOutliers	// Use only lates retest runs with outliers
};

enum ParameterFilter
{
	eParameter_Any,							// Use any parameter
	eParameter_WithResults,					// Use only parameters having fails
	eParameter_WithFails,					// Use only parameters having fails
	eParameter_WithOutliers,				// Use only parameters having outliers
	eParameter_WithOutliersOnGoodRuns,		// Use only parameters having outliers on good runs
	eParameter_WithOutliersOnLatestRetest	// Use only parameters having outliers on good parts
};




#endif // ifdef YIELD123_CONSTANTS_H

