#ifndef GEX_ER_WYR_STANDARD_H
#define GEX_ER_WYR_STANDARD_H

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
// Name			:	GexERWyrStandard
//
// Description	:	Class which creates Weekly Yield Report Standard enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexERWyrStandard : public GexAbstractEnterpriseReport
{
public:

    GexERWyrStandard(
        FILE* hReportFile,
        GexDatabaseEntry* pDatabaseEntry,
        GexDbPlugin_Filter& cFilters,
        GS::Gex::ReportTemplateSection* pSection,
        GexDbPlugin_WyrData& clWyrData);
	~GexERWyrStandard();

protected:

	bool						computeData();

	void						writeRawData();
	void						writeReport();
	void						writeHtmlDataset(GexDbPlugin_WyrDataset * pWyrDataset);

	GexDbPlugin_WyrData&		m_clWyrData;
};

#endif // GEX_ER_WYR_STANDARD_H
