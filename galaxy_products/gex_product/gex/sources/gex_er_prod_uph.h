#ifndef GEX_ER_PROD_UPH_H
#define GEX_ER_PROD_UPH_H

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
// Name			:	GexERProdUPH
//
// Description	:	Class which creates Production UPH enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERProdUPH : public GexAbstractEnterpriseReport
{
public:

    GexERProdUPH(FILE* hReportFile,
                 GexDatabaseEntry* pDatabaseEntry,
                 GexDbPlugin_Filter& cFilters,
                 GS::Gex::ReportTemplateSection* pSection,
                 GexDbPlugin_XYChartList& clXYChartList);
	~GexERProdUPH();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeReport();
	void						writeHtmlData(QString strSplitValue);

	GexDbPlugin_XYChartList&	m_clXYChartList;
};

#endif // GEX_ER_PROD_UPH_H
