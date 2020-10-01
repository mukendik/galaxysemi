/////////////////////////////////////////////////////////////////////////////
// Class To sort Cpk, Failures, Binnings
/////////////////////////////////////////////////////////////////////////////
#include "report_classes_sorting.h"
#include "report_options.h"
#include "cbinning.h"
#include "gex_file_in_group.h"
#include "gex_report.h"
#include <gqtl_log.h>

// in report_build.cpp
extern CGexReport* gexReport;			// Handle to report class


/////////////////////////////////////////////////////////////////////////////
// QList of test for statistics tables
/////////////////////////////////////////////////////////////////////////////
qtTestListStatistics::qtTestListStatistics() : QList<CTest*>()
{
    QString strOptionStorageDevice = (gexReport->getReportOptions()->GetOption("statistics","sorting")).toString();

	if (strOptionStorageDevice == "test_number")
		m_eSortOnField = SortOnTestNumber;
	else if (strOptionStorageDevice == "test_name")
		m_eSortOnField = SortOnTestName;
	else if (strOptionStorageDevice == "test_flow_id")
		m_eSortOnField = SortOnTestFlowID;
	else if (strOptionStorageDevice == "fail_count")
		m_eSortOnField = SortOnFailCount;
	else if (strOptionStorageDevice == "mean")
		m_eSortOnField = SortOnMean;
	else if (strOptionStorageDevice == "mean_shift")
		m_eSortOnField = SortOnMeanShift;
	else if (strOptionStorageDevice == "sigma")
		m_eSortOnField = SortOnSigma;
	else if (strOptionStorageDevice == "cp")
		m_eSortOnField = SortOnCP;
	else if (strOptionStorageDevice == "cpk")
		m_eSortOnField = SortOnCPK;
	else if (strOptionStorageDevice == "r&r")
		m_eSortOnField = SortOnRandR;
}

