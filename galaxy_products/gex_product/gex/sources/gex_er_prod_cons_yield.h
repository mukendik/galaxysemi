#ifndef GEX_ER_PROD_CONS_YIELD_H
#define GEX_ER_PROD_CONS_YIELD_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_abstract_er.h"

namespace GS
{
    namespace Gex
    {
        class ReportTemplateSection;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERProdConsolidatedYield
//
// Description	:	Class which creates Production Consolidated Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERProdConsolidatedYield : public GexAbstractEnterpriseReport
{
public:

    GexERProdConsolidatedYield(
        FILE* hReportFile,
        GexDatabaseEntry* pDatabaseEntry,
        GexDbPlugin_Filter& cFilters,
        GS::Gex::ReportTemplateSection* pSection,
        GexDbPlugin_XYChartList& clXYChartList);
	~GexERProdConsolidatedYield();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeReport();
	void						writeHtmlData(QString strSplitValue);

	GexDbPlugin_XYChartList&	m_clXYChartList;
};

#endif // GEX_ER_PROD_CONS_YIELD_H
