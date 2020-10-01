#ifndef GEX_ABSTRACT_ER_H
#define GEX_ABSTRACT_ER_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "report_build.h"

class GexDatabaseEntry;

namespace GS
{
    namespace Gex
    {
        class ReportTemplateSection;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractEnterpriseReport
//
// Description	:	Base class for enteprise report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAbstractEnterpriseReport
{
public:
    GexAbstractEnterpriseReport(FILE *hReportFile,
                                GexDatabaseEntry *pDatabaseEntry,
                                GexDbPlugin_Filter & cFilters,
                                GS::Gex::ReportTemplateSection* pSection);
	virtual ~GexAbstractEnterpriseReport();

	void								create();

protected:

	virtual bool						computeData()		= 0;

	virtual void						writeRawData()		= 0;
	virtual void						writeHtmlHeader();
	virtual void						writeReport()		= 0;

	void								makeXYChart(GexDbPlugin_XYChartList& clXYChartList, const QString& strChartFullName, const QString& strYAxis, int nMarkerPrecision, bool bBarGraph = true);
	void								makeXYChart(const GexDbPlugin_XYChart_Data * pXYChartData, const QString& strChartFullName, bool bBarGraph = true);


	FILE *								m_hReportFile;
	GexDatabaseEntry *					m_pDatabaseEntry;
	GexDbPlugin_Filter&					m_cFilters;
    GS::Gex::ReportTemplateSection*     m_pSection;

	QString								m_strChartBaseFileName;
	QString								m_strChartDirectory;
	QString								m_strRawDataFullFileName;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERTableHtml
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
class GexERTableHtml
{
public :

	GexERTableHtml(FILE * hReportFile, int nMaxLine = 35);
	~GexERTableHtml();

	void	open(const QString& strOption);
	void	close();
	void	setHeader(const QString& strHeader);
	void	addLine(const QString& strLine);


private:

	int			m_nMaxLine;
	int			m_nLineCount;
	QString		m_strHeader;
	FILE *		m_hReportFile;
};

#endif // GEX_ABSTRACT_ER_H
