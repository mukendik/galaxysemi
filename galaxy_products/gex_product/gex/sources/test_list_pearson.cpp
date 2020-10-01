#include <QVariant>
#include "test_list_pearson.h"
#include "report_options.h"
#include "ctest.h"

CTestListPearson::CTestListPearson(CReportOptions* ro)
{
	QString strPearsonSortField = ro->GetOption("adv_pearson", "sorting").toString();

	if (strPearsonSortField == "test_name")
		m_eSortField = SortOnTestName;
	else if (strPearsonSortField == "pearson")
        m_eSortField = SortOnPearsonDsc;
    else if (strPearsonSortField == "pearson_asc")
        m_eSortField = SortOnPearsonAsc;
    else if (strPearsonSortField == "pearson_dsc")
        m_eSortField = SortOnPearsonDsc;
}

bool TestNameLessThan(const CPearsonTest  &s1, const CPearsonTest  &s2)
{
	int c=QString::compare( s1.ptTestX->strTestName.toLower(), s2.ptTestX->strTestName.toLower());
	return (c>=0)?false:true;
}

bool PearsonLessThan(const CPearsonTest &s1, const CPearsonTest &s2)
{
	return s1.lfPearsonRSquaredScore>s2.lfPearsonRSquaredScore;
}

bool PearsonMoreThan(const CPearsonTest &s1, const CPearsonTest &s2)
{
    return s1.lfPearsonRSquaredScore<s2.lfPearsonRSquaredScore;
}

bool CTestListPearson::Sort()
{
    if (m_eSortField==SortOnPearsonAsc)
        qSort(this->begin(), this->end(), PearsonMoreThan);
    if (m_eSortField==SortOnPearsonDsc)
        qSort(this->begin(), this->end(), PearsonLessThan);
    else if (m_eSortField==SortOnTestName)
		qSort(this->begin(), this->end(), TestNameLessThan);
	else
		return false;
	return true;
}

