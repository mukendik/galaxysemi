#ifndef GEX_ER_GENEALOGY_YIELD_VS_YIELD_H
#define GEX_ER_GENEALOGY_YIELD_VS_YIELD_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_abstract_er.h"

namespace GS
{
    namespace Gex
    {
        class ReportTemplateSection;
        class CustomReportEnterpriseReportSection;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERGenealogyYieldvsYield
//
// Description	:	Class which creates Genealogy Yield vs Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERGenealogyYieldvsYield : public GexAbstractEnterpriseReport
{
public:

    GexERGenealogyYieldvsYield(
        FILE* hReportFile,
        GexDatabaseEntry* pDatabaseEntry,
        GexDbPlugin_Filter& cFilters,
        GS::Gex::ReportTemplateSection* pSection,
        GexDbPlugin_ER_Parts& clER_PartsData);
	~GexERGenealogyYieldvsYield();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeHtmlHeader();
	void						writeReport();
	void						writeHtmlData(GexDbPlugin_ER_Parts_Graph *	pGraph);

    void makeChart(
        GexDbPlugin_ER_Parts_Graph* pGraph,
        const QString& strChartFullName,
        const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport);

	GexDbPlugin_ER_Parts&		m_clER_PartsData;
};

#endif // GEX_ER_GENEALOGY_YIELD_VS_YIELD_H
