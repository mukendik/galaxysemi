///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_er_wyr_standard.h"
#include "gex_report.h"
#include "browser.h"
#include "gex_database_entry.h"
#include "report_template.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern	CGexReport*		gexReport;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERWyrStandard
//
// Description	:	Class which creates Weekly Yield Report Standard enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexERWyrStandard::GexERWyrStandard(
    FILE* hReportFile,
    GexDatabaseEntry* pDatabaseEntry,
    GexDbPlugin_Filter& cFilters,
    GS::Gex::ReportTemplateSection* pSection,
    GexDbPlugin_WyrData& clWyrData)
  : GexAbstractEnterpriseReport(hReportFile,
                                pDatabaseEntry,
                                cFilters,
                                pSection),
    m_clWyrData(clWyrData)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexERWyrStandard::~GexERWyrStandard()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexERWyrStandard::computeData()
//
// Description	:	Compute data for the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexERWyrStandard::computeData()
{
	// Compute WYR data for specified filters
	if(!m_pDatabaseEntry->m_pExternalDatabase->GetDataForWyr_Standard(m_cFilters, m_clWyrData))
	{
		QString strErrorMsg;
		m_pDatabaseEntry->m_pExternalDatabase->GetLastError(strErrorMsg);
		strErrorMsg.replace("\n", "<br>");
		fprintf(m_hReportFile,"<b>*WARNING*</b>Failed creating WYR standard report:<br><br>");
		fprintf(m_hReportFile,"Database=%s<br>", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
		fprintf(m_hReportFile,"Error=%s<br>", strErrorMsg.toLatin1().constData());
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERWyrStandard::writeRawData()
//
// Description	:	Create the raw data file
//
///////////////////////////////////////////////////////////////////////////////////
void GexERWyrStandard::writeRawData()
{
	// No raw data file for the WYR report
	m_strRawDataFullFileName.clear();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERWyrStandard::writeReport()
//
// Description	:	Write the graphs and tables
//
///////////////////////////////////////////////////////////////////////////////////
void GexERWyrStandard::writeReport()
{
	// Go through all datasets and create one section per dataset (Site, Year, Week)
//	GexDbPlugin_WyrDataset *pWyrDataset = m_clWyrData.first();
//	while(pWyrDataset)
    foreach(GexDbPlugin_WyrDataset *pWyrDataset, m_clWyrData)
	{
		// Add a page break
		gexReport->WritePageBreak();

		writeHtmlDataset(pWyrDataset);
//		pWyrDataset = m_clWyrData.next();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERWyrStandard::writeHtmlDataset(GexDbPlugin_WyrDataset * pWyrDataset)
//
// Description	:	Write the dataset into table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERWyrStandard::writeHtmlDataset(GexDbPlugin_WyrDataset * pWyrDataset)
{
	QStringList					strlColumns;
	QStringList::ConstIterator	itColumn, itRow;
	QString						strReference;

	// Write dataset details
	QString strDatasetDetails = "Site: ";
	strDatasetDetails += pWyrDataset->m_strSiteName;
	strDatasetDetails += ", Year: ";
	strDatasetDetails += QString::number(pWyrDataset->m_uiYear);
	strDatasetDetails += ", Week: ";
	strDatasetDetails += QString::number(pWyrDataset->m_uiWeekNb);
	fprintf(m_hReportFile, "<br><br>\n");
	fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td colspan=2 width=\"100%%\" bgcolor=\"#94BDDE\"><font size=\"5\"><b><i><a name=\"DatasetDetails\">%s</a></i></b></font></td>\n", strDatasetDetails.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	// Links to export data
	fprintf(m_hReportFile,"<tr>\n");
	strReference.sprintf("%s%s--mission=wyr--report=standard--action=saveexcelfile--site=%s--year=%d--week=%d", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pWyrDataset->m_strSiteName.toLatin1().constData(), pWyrDataset->m_uiYear, pWyrDataset->m_uiWeekNb);
    fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
	strReference.sprintf("%s%s--mission=wyr--report=standard--action=saveexcelclipboard--site=%s--year=%d--week=%d", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pWyrDataset->m_strSiteName.toLatin1().constData(), pWyrDataset->m_uiYear, pWyrDataset->m_uiWeekNb);
    fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
	fprintf(m_hReportFile,"</tr>\n");
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "<br><br><br>\n");

	// Data table
	fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"1\" cellpadding=\"1\">\n");
	// First row is the title row
    strlColumns = pWyrDataset->m_strTitleRow.split(",");
	fprintf(m_hReportFile, "<tr>\n");
	for(itColumn = strlColumns.begin(); itColumn != strlColumns.end(); itColumn++)
	{
		if((*itColumn).isEmpty())
			fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">-</font></td>\n");
		else
			fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", (*itColumn).toLatin1().constData());
	}
	fprintf(m_hReportFile, "</tr>\n");
	// Write data rows
	for(itRow = pWyrDataset->m_strlDataRows.begin(); itRow != pWyrDataset->m_strlDataRows.end(); itRow++)
	{
        strlColumns = (*itRow).split(",");
		fprintf(m_hReportFile, "<tr>\n");
		for(itColumn = strlColumns.begin(); itColumn != strlColumns.end(); itColumn++)
		{
			if((*itColumn).isEmpty())
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\"><font size=\"3\">-</font></td>\n");
			else
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\"><font size=\"3\">%s</font></td>\n", (*itColumn).toLatin1().constData());
		}
		fprintf(m_hReportFile, "</tr>\n");
	}
	fprintf(m_hReportFile, "</table><br>\n");
}
