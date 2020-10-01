#ifndef GEX_ER_PROD_YIELD_H
#define GEX_ER_PROD_YIELD_H

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
// Name			:	GexERProdYield
//
// Description	:	Class which creates Production Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERProdYield : public GexAbstractEnterpriseReport
{
public:

    GexERProdYield(
        FILE* hReportFile,
        GexDatabaseEntry* pDatabaseEntry,
        GexDbPlugin_Filter& cFilters,
        GS::Gex::ReportTemplateSection* pSection,
        GexDbPlugin_XYChartList& clXYChartList);
	~GexERProdYield();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeReport();
	void						writeHtmlData(QString strSplitValue);

	GexDbPlugin_XYChartList&	m_clXYChartList;
};

#endif // GEX_ER_PROD_YIELD_H
