#ifndef GEX_ER_PROD_ADV_YIELD_H
#define GEX_ER_PROD_ADV_YIELD_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_abstract_er.h"

namespace GS
{
    namespace Gex
    {
        class CustomReportEnterpriseReportSection;
        class ReportTemplateSection;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERProdAdvancedYield
//
// Description	:	Class which creates Production Advanced Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERProdAdvancedYield : public GexAbstractEnterpriseReport
{
public:

    /*!
     * \fn GexERProdAdvancedYield
     * \brief Constructor
     */
    GexERProdAdvancedYield(FILE* hReportFile,
                           GexDatabaseEntry* pDatabaseEntry,
                           GexDbPlugin_Filter& cFilters,
                           GS::Gex::ReportTemplateSection* pSection,
                           GexDbPlugin_ER_Parts& clER_PartsData);
	~GexERProdAdvancedYield();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeHtmlHeader();
	void						writeReport();
	void						writeHtmlData(GexDbPlugin_ER_Parts_Graph *	pGraph);
    void writeYieldExceptionGraphs(
        GexDbPlugin_ER_Parts_Graph* pGraph,
        const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport,
        const QString& strImage_Base);

    void makeChart(
        GexDbPlugin_ER_Parts_Graph* pGraph,
        const QString& strChartFullName,
        const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport);
    void makechart_Advanced_BinningPareto(
        GexDbPlugin_ER_Parts_Layer* pLayer,
        const QString& strChartFullName,
        const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport,
        GexDbPlugin_BinList& clBinList,
        unsigned int uiDataIndex);

	GexDbPlugin_ER_Parts&		m_clER_PartsData;
};

#endif // GEX_ER_PROD_ADV_YIELD_H
