///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_er_genealogy_yield_vs_param.h"
#include "browser.h"
#include "report_options.h"
#include "gex_report.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "report_template.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern	CGexReport*		gexReport;
extern	CReportOptions		ReportOptions;

extern QString				formatHtmlImageFilename(const QString& strImageFileName);

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
GexERGenealogyYieldvsParameter::GexERGenealogyYieldvsParameter(
    FILE* hReportFile,
    GexDatabaseEntry* pDatabaseEntry,
    GexDbPlugin_Filter& cFilters,
    GS::Gex::ReportTemplateSection* pSection,
    GexDbPlugin_ER_Parts& clER_PartsData)
  : GexAbstractEnterpriseReport(hReportFile,
                                pDatabaseEntry,
                                cFilters,
                                pSection),
    m_clER_PartsData(clER_PartsData)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexERGenealogyYieldvsParameter::~GexERGenealogyYieldvsParameter()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexERGenealogyYieldvsParameter::computeData()
//
// Description	:	Compute data for the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexERGenealogyYieldvsParameter::computeData()
{
	// Compute Genealogy data for specified filters
	if(!m_pDatabaseEntry->m_pExternalDatabase->ER_Genealogy_YieldVsParameter_GetParts(m_cFilters, m_clER_PartsData))
	{
		QString strErrorMsg;
		m_pDatabaseEntry->m_pExternalDatabase->GetLastError(strErrorMsg);
		strErrorMsg.replace("\n", "<br>");
		fprintf(m_hReportFile,"<b>*WARNING*</b>Failed creating Genealogy chart:<br><br>");
		fprintf(m_hReportFile,"Database=%s<br>", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
		fprintf(m_hReportFile,"Error=%s<br>", strErrorMsg.toLatin1().constData());

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERGenealogyYieldvsParameter::writeRawData()
//
// Description	:	Create the raw data file
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsParameter::writeRawData()
{
	GexDbPlugin_ER_Parts_Graph *		pGraph;
	GexDbPlugin_ER_Parts_Layer *		pLayer;
	GexDbPlugin_ER_Parts_SerieData *	pSerieData;
	GexDbPlugin_ER_Parts_SerieDef *		pSerieDef;
	unsigned int						uiDataPoint;
	QStringList::iterator				it;

	QFile queryFile(m_strRawDataFullFileName);

	if(queryFile.open(QIODevice::WriteOnly))
	{
		QTextStream stream(&queryFile);

		// Header
        stream << "# Semiconductor Yield Analysis is easy with Quantix!" << endl;
		stream << "# Check latest news: www.mentor.com" << endl;
		stream << "#" << endl;
		stream << "# Examinator SQL report raw data" << endl;
		stream << "#" << endl;
        stream << "# Report created with: " << GS::Gex::Engine::GetInstance().Get("AppFullName").toString()
               << " - www.mentor.com" << endl;
		stream << endl;
		stream << "# Global information" << endl;
		stream << "Date," << QDate::currentDate().toString("dd MMM yyyy") << endl;
		stream << "Time," << QTime::currentTime().toString("hh:mm:ss") << endl;
		stream << "Report type,Genealogy (Yield vs. Parameter)" << endl;
		stream << "Report title," << m_pSection->pEnterpriseReport->m_strSectionTitle << endl;
		stream << endl;
		stream << "# SQL Query" << endl;
		stream << "\"" << m_clER_PartsData.m_strYieldQuery << "\"" << endl;
		stream << endl;

		// Write raw data?
		if(m_pSection->pEnterpriseReport->m_bDumpRawData)
		{
			stream << "# Raw Data" << endl;
			// First line = column names
			for(it = m_clER_PartsData.m_strlFields_GraphSplit.begin(); it != m_clER_PartsData.m_strlFields_GraphSplit.end(); it++)
				stream << (*it) << ",";

			for(it = m_clER_PartsData.m_strlFields_LayerSplit.begin(); it != m_clER_PartsData.m_strlFields_LayerSplit.end(); it++)
				stream << (*it) << ",";

			stream << m_clER_PartsData.m_strField_Aggregate;
			// First serie is the parameter
			pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(0);
			stream << "," << pSerieDef->m_strSerieName;
			stream << " (Execs)";
			stream << "," << pSerieDef->m_strSerieName;
			stream << " (Sum)";
			// Second serie is the yield
			pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(0);
			stream << "," << pSerieDef->m_strSerieName;
			stream << " (Parts)";
			stream << "," << pSerieDef->m_strSerieName;
			stream << " (Matching)";
			stream << endl;

			// Data
            foreach(pGraph, m_clER_PartsData)
			{

                foreach(pLayer, *pGraph)
				{
					for(uiDataPoint=0; uiDataPoint<pLayer->m_uiDataPoints; uiDataPoint++)
					{
						for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
							stream << (*it) << ",";
						for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
							stream << (*it) << ",";
						stream << pLayer->m_strlAggregateLabels[uiDataPoint];
						// First serie is the parameter
						pSerieData = pLayer->at(0);
						stream << "," << (*pSerieData)[uiDataPoint].m_uiParameterExecs;
						stream << "," << (*pSerieData)[uiDataPoint].m_lfParameterSum;
						// Second serie is the yield
						pSerieData = pLayer->at(1);
						stream << "," << (*pSerieData)[uiDataPoint].m_uiTotalParts;
						stream << "," << (*pSerieData)[uiDataPoint].m_uiMatchingParts;
						stream << endl;
					}
				}
			}
		}
		queryFile.close();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERGenealogyYieldvsParameter::writeHtmlHeader()
//
// Description	:	Write the html header
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsParameter::writeHtmlHeader()
{
	QStringList::ConstIterator		it;
	QString							strString;
	GexDbPlugin_ER_Parts_SerieDef *	pSerieDef;

	fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"1\" cellpadding=\"1\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Dataset</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Database</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">TimePeriod</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strTimePeriod.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	if(m_cFilters.bUseTimePeriod)
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">From</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_cFilters.calendarFrom.toString("dd MMM yyyy").toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">To</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_cFilters.calendarTo.toString("dd MMM yyyy").toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
	}

    for(it = m_pSection->pEnterpriseReport->m_strFiltersList.begin(); it != m_pSection->pEnterpriseReport->m_strFiltersList.end(); it++)
    {
        strString = (*it).section('=', 1, 1);
        if(!strString.isNull())
        {
            fprintf(m_hReportFile, "<tr>\n");
            fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">%s</td>\n", (*it).section('=', 0, 0).toLatin1().constData());
            fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", (*it).section('=', 1, 1).toLatin1().constData());
            fprintf(m_hReportFile, "</tr>\n");
        }
    }
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Product</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">%s</td>\n", m_pSection->pEnterpriseReport->m_Genealogy_strProduct.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Aggregation granularity</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\">%s</td>\n", m_clER_PartsData.m_strField_Aggregate.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	strString = "";

	for(it = m_clER_PartsData.m_strlFields_GraphSplit.begin(); it != m_clER_PartsData.m_strlFields_GraphSplit.end(); it++)
	{
		if(!strString.isEmpty())
			strString += ",";
		strString += (*it);
	}
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Fields for graph split</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	if(strString.isEmpty())
		fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\">none</td>\n");
	else
		fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\">%s</td>\n", strString.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	strString = "";

	for(it = m_clER_PartsData.m_strlFields_LayerSplit.begin(); it != m_clER_PartsData.m_strlFields_LayerSplit.end(); it++)
	{
		if(!strString.isEmpty())
			strString += ",";
		strString += (*it);
	}
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Fields for layer split</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
	fprintf(m_hReportFile, "<tr>\n");
	if(strString.isEmpty())
		fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\">none</td>\n");
	else
		fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\">%s</td>\n", strString.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Series definition</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
	fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

	// First serie is parameter
	pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(0);
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Serie name</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Data type</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strTestingStage.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Parameter</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s (%s)</td>\n", pSerieDef->m_strParameterName.toLatin1().constData(), pSerieDef->m_strParameter.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");

	// Second serie is yield
	pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(1);
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\"> </td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\"> </td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Serie name</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Data type</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strTestingStage.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Binnings</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strBinnings.toLatin1().constData());
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "</table>\n");
	fprintf(m_hReportFile, "</td>\n");
	fprintf(m_hReportFile, "</tr>\n");
	fprintf(m_hReportFile, "<tr>\n");

	if(m_clER_PartsData.m_bWorkOnBinnings)
	{
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Bin type</td>\n");
		if(m_clER_PartsData.m_bSoftBin)
			fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">Soft Bin</td>\n");
		else
			fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">Hard Bin</td>\n");
		fprintf(m_hReportFile, "</tr>\n");
	}

	if(!m_strRawDataFullFileName.isEmpty())
	{
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">Raw data</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">\n");
		fprintf(m_hReportFile, "<table border=\"0\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"100%%\" bgcolor=\"#F8F8F8\"><a href=\"file:///%s\">%s</a></td>\n", m_strRawDataFullFileName.toLatin1().constData(), m_strRawDataFullFileName.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
		fprintf(m_hReportFile, "</table>\n");
		fprintf(m_hReportFile, "</td>\n");
		fprintf(m_hReportFile, "</tr>\n");
	}
	fprintf(m_hReportFile, "</table><br>\n");
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERGenealogyYieldvsParameter::writeReport()
//
// Description	:	Write the graphs and tables
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsParameter::writeReport()
{
	// Get nb of axis
	if(m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() != "disabled (hide axis)")
		m_clER_PartsData.m_uiNbAxis++;
	if(m_pSection->pEnterpriseReport->m_strRightAxis.toLower() != "disabled (hide axis)")
		m_clER_PartsData.m_uiNbAxis++;
	m_clER_PartsData.m_uiNbAxis=1;

	// Create graphs
	GexDbPlugin_ER_Parts_Graph *	pGraph;
	QString							strImage_Base, strImage, strImageFullName;
	QString							strSplitValues;
	QStringList::iterator			it1, it2;

//	pGraph = m_clER_PartsData.first();
	int nIndex = 0;
//	while(pGraph)
    foreach(pGraph, m_clER_PartsData)
	{

		// Graph title
		strSplitValues = "";
		if(m_clER_PartsData.m_strlFields_GraphSplit.count() == 0)
			fprintf(m_hReportFile,"<font size=\"6\" face=\"Arial\"><b><u>Split#%s</u></b></font><br>\n", QString::number(nIndex).toLatin1().constData());
		else
		{
			it1 = m_clER_PartsData.m_strlFields_GraphSplit.begin();
			it2 = pGraph->m_strlGraphSplitValues.begin();

			while(it1 != m_clER_PartsData.m_strlFields_GraphSplit.end() && it2 != pGraph->m_strlGraphSplitValues.end())
			{
				if(!strSplitValues.isEmpty())
					strSplitValues += ",";
				strSplitValues += *it1 + "=";
				strSplitValues += *it2;
				it1++;
				it2++;
			}
			strSplitValues = "(" + strSplitValues;
			strSplitValues += ")";
			fprintf(m_hReportFile,"<u><font size=\"6\" face=\"Arial\"><b>Split#%s</b>  <i>%s</i></font></u><br>\n", QString::number(nIndex).toLatin1().constData(), strSplitValues.toLatin1().constData());
		}
		fprintf(m_hReportFile,"<br><br>\n");

		// Display graph?
		if(m_clER_PartsData.m_uiNbAxis > 0)
		{

			// Image name
			strImage_Base = m_strChartBaseFileName + "_c";
			strImage_Base += QString::number(nIndex++);
			strImage = strImage_Base + ".png";
			strImageFullName = m_strChartDirectory + strImage;

			// Create image
			makeChart(pGraph, strImageFullName, m_pSection->pEnterpriseReport);

			// Write HTML code to insert created image
			fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\"></img>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

			// If creating a Powerpoint presentation, save Section title.
			if (pGraph->m_strlGraphSplitValues.isEmpty())
				gexReport->SetPowerPointSlideName("Genealogy yield vs parameter chart");
			else
				gexReport->SetPowerPointSlideName("Genealogy yield vs parameter chart : " + pGraph->m_strlGraphSplitValues.join("_"));
		}
		else
		{
			fprintf(m_hReportFile,"Chart disabled (not enough data)<br>\n");
		}

		fprintf(m_hReportFile,"<br><br>\n");

		// Write HTML data
        writeHtmlData(pGraph);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERGenealogyYieldvsParameter::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
//
// Description	:	Write the raw data table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsParameter::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
{
	QString								strReference;
	QStringList::iterator				it;
	GexDbPlugin_ER_Parts_Layer *		pLayer;
	GexDbPlugin_ER_Parts_SerieDef *		pSerieDef;
	GexDbPlugin_ER_Parts_SerieData *	pSerieData;

	// Add a page break
	gexReport->WritePageBreak();

	// If creating a Powerpoint presentation, save Section title.
	if (pGraph->m_strlGraphSplitValues.isEmpty())
		gexReport->SetPowerPointSlideName("Genealogy yield vs parameter raw data");
	else
		gexReport->SetPowerPointSlideName("Genealogy yield vs parameter raw data : " + pGraph->m_strlGraphSplitValues.join("_"));

	// Links to export data
	//if (ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
	QString of=ReportOptions.GetOption("output", "format").toString();
	if ( (of=="INTERACTIVE") || (of=="HTML") )
	{
		fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile,"<tr>\n");

		if(pGraph->m_strlGraphSplitValues.isEmpty())
		{
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_parameter--action=saveexcelfile", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_parameter--action=saveexcelclipboard", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}
		else
		{
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_parameter--action=saveexcelfile--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_parameter--action=saveexcelclipboard--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}

		fprintf(m_hReportFile,"</tr>\n");
		fprintf(m_hReportFile, "</table>\n");
		fprintf(m_hReportFile, "<br><br><br>\n");
	}

	// Data table
	GexERTableHtml erTableHtml(m_hReportFile);
	QString strLine;

	erTableHtml.open("<table border=\"1\" width=\"800\" cellspacing=\"1\" cellpadding=\"1\">\n");

	// First line = column names
	strLine = "<tr>\n";

	for(it = m_clER_PartsData.m_strlFields_GraphSplit.begin(); it != m_clER_PartsData.m_strlFields_GraphSplit.end(); it++)
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", (*it).toLatin1().constData());
	for(it = m_clER_PartsData.m_strlFields_LayerSplit.begin(); it != m_clER_PartsData.m_strlFields_LayerSplit.end(); it++)
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", (*it).toLatin1().constData());
	if(m_clER_PartsData.m_strField_Aggregate.toLower() == "wafer")
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">TrackingLotID/WaferID</font></td>\n");
	else
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">TrackingLotID</font></td>\n");

	// First serie = Parameter
	pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(0);
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (execs)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (sum)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (mean)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());

	// Second serie = Yield
	pSerieDef = m_clER_PartsData.m_plistSerieDefs.at(1);
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (parts)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (matching)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (%%)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
	strLine + "</tr>\n";

	// Set the html table header
	erTableHtml.addLine(strLine);

	// Data
	unsigned int	uiDataIndex, uiNbSerieParts, uiNbSerieMatching;
	unsigned int	uiSerieExecs;
	double			lfSerieSum, lfData;
	bool			bException;
	QString			strFontSpec;
    foreach(pLayer, *pGraph)
	{
		for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
		{
			// First check if there is a yield exception
			bException = false;
			// Second serie = Yield
			pSerieDef	= m_clER_PartsData.m_plistSerieDefs.at(1);
			pSerieData	= pLayer->at(1);
			uiNbSerieParts		= (*pSerieData)[uiDataIndex].m_uiTotalParts;
			uiNbSerieMatching	= (*pSerieData)[uiDataIndex].m_uiMatchingParts;
			lfData				= ((double)uiNbSerieMatching/(double)uiNbSerieParts)*100.0;

			if(bException)
				strFontSpec = "<font size=\"3\" color=\"#FF3333\">";
			else
				strFontSpec = "<font size=\"3\">";

			strLine = "<tr>\n";

			for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());
			for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (pLayer->m_strlAggregateLabels[uiDataIndex]).toLatin1().constData());

			// First serie = Parameter
			pSerieDef		= m_clER_PartsData.m_plistSerieDefs.at(0);
			pSerieData		= pLayer->at(0);
			uiSerieExecs	= (*pSerieData)[uiDataIndex].m_uiParameterExecs;
			lfSerieSum		= (*pSerieData)[uiDataIndex].m_lfParameterSum;
			lfData			= (lfSerieSum/(double)uiSerieExecs);

			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiSerieExecs);
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%g</font></td>\n", strFontSpec.toLatin1().constData(), lfSerieSum);
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%5.2f</font></td>\n", strFontSpec.toLatin1().constData(), lfData);

			// Second serie = Yield
			pSerieDef			= m_clER_PartsData.m_plistSerieDefs.at(1);
			pSerieData			= pLayer->at(1);
			uiNbSerieParts		= (*pSerieData)[uiDataIndex].m_uiTotalParts;
			uiNbSerieMatching	= (*pSerieData)[uiDataIndex].m_uiMatchingParts;
			lfData				= ((double)uiNbSerieMatching/(double)uiNbSerieParts)*100.0;

			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbSerieParts);
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbSerieMatching);
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%5.2f</font></td>\n", strFontSpec.toLatin1().constData(), lfData);
			strLine += "</tr>\n";

			// add line to the html table
			erTableHtml.addLine(strLine);
        }
	}

	erTableHtml.close();
}

/******************************************************************************!
 * \fn makeChart
 * \brief Create the chart image
 ******************************************************************************/
void GexERGenealogyYieldvsParameter::makeChart(
    GexDbPlugin_ER_Parts_Graph* pGraph,
    const QString& strChartFullName,
    const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
	double *							plfData_Yield_X = NULL;
	double *							plfData_Yield_Y = NULL;
//	unsigned int						uiLayerIndex;
	unsigned int						uiIndex;
	GexDbPlugin_ER_Parts_Layer *		pLayer;
	GexDbPlugin_ER_Parts_SerieData *	pSerieData_X, *pSerieData_Y;
	GexDbPlugin_ER_Parts_SerieDef *		pSerieDef_X, *pSerieDef_Y;
	QString								strString;

	// Create a XYChart object of size 800 x 500 pixels.
	XYChart *pChart = new XYChart(800,500);

	// Compute plot area
	QRect	clPlotRect;
	int		nStartY = 30+(int)ceil((double)(pGraph->count())/5.0)*30;

	if(m_clER_PartsData.m_uiNbAxis == 2)
		clPlotRect.setRect(70, nStartY, 640, 410-nStartY);
	else
		clPlotRect.setRect(70, nStartY, 710, 410-nStartY);

	// Add a legend box
	LegendBox *pLegendBox = pChart->addLegend(70, 20, false, "arialbi.ttf", 8);
	pLegendBox->setBackground(Chart::Transparent);

	// Compute background colors
	int nBgColor = Chart::Transparent, nAltBgColor = -1;
	if(pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle.toLower() == "white")
		nBgColor = 0xffffff;
	else if(pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle.toLower() == "white & grey")
	{
		nBgColor = 0xf8f8f8;
		nAltBgColor = 0xffffff;
	}
	else if(pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle.toLower() == "custom color...")
		nBgColor = pEnterpriseReport->m_YieldWizard_Global_cBackgroundColor.rgb() & 0xffffff;
	else if(pEnterpriseReport->m_YieldWizard_Global_strBackgroundStyle.toLower() == "gradient color...")
		nBgColor = pChart->linearGradientColor(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), pEnterpriseReport->m_YieldWizard_Global_cBackgroundColor.rgb() & 0xffffff, 0xffffff);

	// Set plot area
	pChart->setPlotArea(clPlotRect.left(), clPlotRect.top(), clPlotRect.width() , clPlotRect.height(), nBgColor, nAltBgColor);

	// Add a title to the X axis
	pSerieDef_X = m_clER_PartsData.m_plistSerieDefs.at(0);
	strString	= pSerieDef_X->m_strTestingStage;
	strString	+= "- (Parameter: ";
	strString	+= pSerieDef_X->m_strParameterName;
	strString	+= " (" + pSerieDef_X->m_strParameter;
	strString	+= "))";

	pChart->xAxis()->setTitle(strString.toLatin1().constData());
	pChart->xAxis()->setAutoScale(0.1, 0.1, 0);

	// Add a title to the Y axis
	pSerieDef_Y = m_clER_PartsData.m_plistSerieDefs.at(1);
	strString	= pSerieDef_Y->m_strTestingStage;
	strString	+= "- (Binnings: ";
	strString	+= pSerieDef_Y->m_strBinnings;
	strString	+= ")";

	pChart->yAxis()->setTitle(strString.toLatin1().constData());
	pChart->yAxis()->setAutoScale(0.1, 0.1, 0);

	// Create scatter graphs for each layer
	int				nDataColor	= pEnterpriseReport->m_cDefaultColor.rgb() & 0xffffff;
	int				nSymbol		= Chart::SquareSymbol;
	QString			strName;

    foreach(pLayer, *pGraph)
	{
		// Allocate mem space for data pointers
		if(plfData_Yield_X != NULL)
			delete [] plfData_Yield_X;
		if(plfData_Yield_Y != NULL)
			delete [] plfData_Yield_Y;
		plfData_Yield_X = new double[pLayer->m_uiDataPoints];
		plfData_Yield_Y = new double[pLayer->m_uiDataPoints];

		///////////////////////////////////////////////////////
		// ADD X/Y Scatter
		///////////////////////////////////////////////////////

		pSerieDef_X		= m_clER_PartsData.m_plistSerieDefs.at(0);
		pSerieData_X	= pLayer->at(0);
		pSerieDef_Y		= m_clER_PartsData.m_plistSerieDefs.at(1);
		pSerieData_Y	= pLayer->at(1);

		// Dataset name
		strName = pLayer->m_strlLayerSplitValues.join(",");

		// Compute yield
		for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
		{
			plfData_Yield_X[uiIndex] = ((double)((*pSerieData_X)[uiIndex].m_lfParameterSum)/(double)((*pSerieData_X)[uiIndex].m_uiParameterExecs));
			plfData_Yield_Y[uiIndex] = ((double)((*pSerieData_Y)[uiIndex].m_uiMatchingParts)/(double)((*pSerieData_Y)[uiIndex].m_uiTotalParts))*100.0;
		}

		// Use spots?
		if(pEnterpriseReport->m_strLineSpots.toLower() == "square")
			nSymbol = Chart::SquareSymbol;
		else if(pEnterpriseReport->m_strLineSpots.toLower() == "circle")
			nSymbol = Chart::CircleSymbol;
		else if(pEnterpriseReport->m_strLineSpots.toLower() == "diamonds")
			nSymbol = Chart::DiamondSymbol;
		else if(pEnterpriseReport->m_strLineSpots.toLower() == "triangle")
			nSymbol = Chart::TriangleSymbol;
		else if(pEnterpriseReport->m_strLineSpots.toLower() == "cross")
			nSymbol = Chart::CrossSymbol;

		// Add scatter
		pChart->addScatterLayer(DoubleArray(plfData_Yield_X, pLayer->m_uiDataPoints), DoubleArray(plfData_Yield_Y, pLayer->m_uiDataPoints),
								strName.toLatin1().constData(), nSymbol, 7, nDataColor);

	}

	// output the chart
	pChart->makeChart(strChartFullName.toLatin1().constData());

	//free up resources
	if(plfData_Yield_X != NULL)
		delete [] plfData_Yield_X;
	if(plfData_Yield_Y != NULL)
		delete [] plfData_Yield_Y;

	delete pChart;
	pChart=0;
}
