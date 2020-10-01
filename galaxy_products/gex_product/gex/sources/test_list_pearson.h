#ifndef TEST_LIST_PEARSON_H
#define TEST_LIST_PEARSON_H

#include <QList>
#include "report_options.h"
#include "ctest.h"

extern CReportOptions	ReportOptions;		// Holds options (report_build.h)

class CTest;

class CPearsonTest
{
public:
	CTest *ptTestX;
	CTest *ptTestY;
	double	lfPearsonRSquaredScore;
    QMap< QString, QVariant > mData;
};


class	CTestListPearson: public QList<CPearsonTest>
{
	enum SortingField
	{
        SortOnPearsonDsc= 0,
        SortOnTestName	= 1,
        SortOnPearsonAsc= 2
	};

	SortingField	m_eSortField;
public:
	CTestListPearson(CReportOptions* ro);
	// Sort using the option specified in CReportOptions* from constructor
	bool Sort();

};



#endif // TEST_LIST_PEARSON_H
