/////////////////////////////////////////////////////////////////////////////
// Class To sort Cpk, Failures, Binnings
/////////////////////////////////////////////////////////////////////////////
#ifndef GEX_REPORT_CLASSES_SORTING_H
#define GEX_REPORT_CLASSES_SORTING_H

#include "report_build.h"
#include "gex_file_in_group.h"

class CBinning;


/////////////////////////////////////////////////////////////////////////////
// Class used for sorting Binning (Pareto) : Sort BinCount from highest to lowest
/////////////////////////////////////////////////////////////////////////////
class	qtTestListBinning: public QList<CBinning*>
{
public:
};


/////////////////////////////////////////////////////////////////////////////
// Class used for sorting Test statistics table
/////////////////////////////////////////////////////////////////////////////
class	qtTestListStatistics: public QList <CTest*>
{
public:

    typedef enum
    {
        SortOnTestNumber	= 0,
        SortOnTestName		= 1,
        SortOnTestFlowID	= 2,
        SortOnFailCount		= 3,
        SortOnMean			= 4,
        SortOnMeanShift		= 5,
        SortOnSigma			= 6,
        SortOnCP			= 7,
        SortOnCPK			= 8,
        SortOnRandR			= 9
    }SortField;

	qtTestListStatistics();

    inline SortField getSortField() const {return  m_eSortOnField;}



private:

    SortField m_eSortOnField;
};



#endif
