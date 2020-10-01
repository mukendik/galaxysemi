#ifndef GEX_ER_GENEALOGY_YIELD_VS_PARAM_H
#define GEX_ER_GENEALOGY_YIELD_VS_PARAM_H

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
// Name			:	GexERGenealogyYieldvsParameter
//
// Description	:	Class which creates Genealogy Yield vs Parameter enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
class GexERGenealogyYieldvsParameter : public GexAbstractEnterpriseReport
{
public:
    /*!
     * \fn GexERGenealogyYieldvsParameter
     * \brief Constructor
     */
    GexERGenealogyYieldvsParameter(
        FILE* hReportFile,
        GexDatabaseEntry* pDatabaseEntry,
        GexDbPlugin_Filter& cFilters,
        GS::Gex::ReportTemplateSection* pSection,
        GexDbPlugin_ER_Parts& clER_PartsData);
	~GexERGenealogyYieldvsParameter();

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

#endif // GEX_ER_GENEALOGY_YIELD_VS_PARAM_H
