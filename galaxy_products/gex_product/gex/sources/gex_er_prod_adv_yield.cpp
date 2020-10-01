///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_er_prod_adv_yield.h"
#include "browser.h"
#include "colors_generator.h"
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

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern	void				WriteDebugMessageFile(const QString & strMessage);
extern	QString				formatHtmlImageFilename(const QString& strImageFileName);

///////////////////////////////////////////////////////////////////////////////////
// Defines
///////////////////////////////////////////////////////////////////////////////////
#define COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR 150

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERProdAdvancedYield
//
// Description	:	Class which creates Production Advanced Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexERProdAdvancedYield::GexERProdAdvancedYield(
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
GexERProdAdvancedYield::~GexERProdAdvancedYield()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexERProdAdvancedYield::computeData()
//
// Description	:	Compute data for the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexERProdAdvancedYield::computeData()
{
	// Compute Yield data for specified filters
	if(!m_pDatabaseEntry->m_pExternalDatabase->ER_Prod_GetParts(m_cFilters, m_clER_PartsData))
	{
		QString strErrorMsg;
		m_pDatabaseEntry->m_pExternalDatabase->GetLastError(strErrorMsg);
		strErrorMsg.replace("\n", "<br>");
		fprintf(m_hReportFile,"<b>*WARNING*</b>Failed creating Yield chart:<br><br>");
		fprintf(m_hReportFile,"Database=%s<br>", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
		fprintf(m_hReportFile,"Error=%s<br>", strErrorMsg.toLatin1().constData());
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdAdvancedYield::writeRawData()
//
// Description	:	Create the raw data file
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdAdvancedYield::writeRawData()
{
	GexDbPlugin_ER_Parts_Graph *		pGraph;
	GexDbPlugin_ER_Parts_Layer *		pLayer;
	GexDbPlugin_ER_Parts_SerieData *	pSerieData;
	GexDbPlugin_ER_Parts_SerieDef *		pSerieDef;
	unsigned int						uiDataPoint, uiSerie;
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
		stream << "Report type,Advanced Yield" << endl;
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

			stream << m_clER_PartsData.m_strField_Aggregate << ",Parts";
//			pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//			while(pSerieDef)
            foreach(pSerieDef, m_clER_PartsData.m_plistSerieDefs)
			{
				stream << "," << pSerieDef->m_strSerieName;
//				pSerieDef = m_clER_PartsData.m_plistSerieDefs.next();
			}
			stream << endl;

			// Data
//			pGraph = m_clER_PartsData.first();
//			while(pGraph)
            foreach(pGraph, m_clER_PartsData)
			{
//				pLayer = pGraph->first();
//				while(pLayer)
                foreach(pLayer, *pGraph)
				{
					for(uiDataPoint = 0; uiDataPoint < pLayer->m_uiDataPoints; uiDataPoint++)
					{
						for(it = pGraph->m_strlGraphSplitValues.begin(); it != pGraph->m_strlGraphSplitValues.end(); it++)
							stream << (*it) << ",";

						for(it = pLayer->m_strlLayerSplitValues.begin(); it != pLayer->m_strlLayerSplitValues.end(); it++)
							stream << (*it) << ",";
						stream << pLayer->m_strlAggregateLabels[uiDataPoint] << ",";

						stream << pLayer->m_uilNbParts[uiDataPoint];

						for(uiSerie=0; uiSerie<pLayer->m_uiSeries; uiSerie++)
						{
							pSerieData = pLayer->at(uiSerie);
							stream << "," << (*pSerieData)[uiDataPoint].m_uiMatchingParts;
						}
						stream << endl;
					}
//					pLayer = pGraph->next();
				}
//				pGraph = m_clER_PartsData.next();
			}
		}
		queryFile.close();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdAdvancedYield::writeHtmlHeader()
//
// Description	:	Write the html header
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdAdvancedYield::writeHtmlHeader()
{
	QStringList::ConstIterator		it;
	QString							strString;
	GexDbPlugin_ER_Parts_SerieDef *	pSerieDef;
	int								nSerieNb = 0;

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
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">DataType</td>\n");
	fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", m_pSection->pEnterpriseReport->m_strDatabaseType.toLatin1().constData());
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
	fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#CCFFFF\">X-Axis</td>\n");
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

//	pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, m_clER_PartsData.m_plistSerieDefs)
	{
		if(nSerieNb > 0)
		{
			fprintf(m_hReportFile, "<tr>\n");
			fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\"> </td>\n");
			fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\"> </td>\n");
			fprintf(m_hReportFile, "</tr>\n");
		}
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Serie name</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Binnings</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strBinnings.toLatin1().constData());
		fprintf(m_hReportFile, "</tr>\n");
		nSerieNb++;
//		pSerieDef = m_clER_PartsData.m_plistSerieDefs.next();
	}

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
// Name			:	void GexERProdAdvancedYield::writeReport()
//
// Description	:	Write the graphs and tables
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdAdvancedYield::writeReport()
{
	// Get nb of axis
	if(m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() != "disabled (hide axis)")
		m_clER_PartsData.m_uiNbAxis++;
	if(m_pSection->pEnterpriseReport->m_strRightAxis.toLower() != "disabled (hide axis)")
		m_clER_PartsData.m_uiNbAxis++;

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
		// Add a page break
		gexReport->WritePageBreak();

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
			strImage_Base		= m_strChartBaseFileName+ "_c";
			strImage_Base		+= QString::number(nIndex++);
			strImage			= strImage_Base + ".png";
			strImageFullName	= m_strChartDirectory + strImage;

			// Create image
			makeChart(pGraph, strImageFullName, m_pSection->pEnterpriseReport);

			// Write HTML code to insert created image
			fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\"></img><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

			// If creating a Powerpoint presentation, save Section title.
			if (pGraph->m_strlGraphSplitValues.isEmpty())
				gexReport->SetPowerPointSlideName("Advanced yield chart");
			else
				gexReport->SetPowerPointSlideName("Advanced yield chart : " + pGraph->m_strlGraphSplitValues.join("_"));
		}
		else
		{
			fprintf(m_hReportFile,"Chart disabled\n");
		}

		fprintf(m_hReportFile,"<br><br>\n");

		// Write HTML data
		writeHtmlData(pGraph);

		fprintf(m_hReportFile,"<br><br>\n");

		// Check for yield exceptions, and insert drill-down graphs if any
		writeYieldExceptionGraphs(pGraph, m_pSection->pEnterpriseReport, strImage_Base);

		fprintf(m_hReportFile,"<br><br><br>\n");

//		pGraph = m_clER_PartsData.next();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdAdvancedYield::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
//
// Description	:	Write the raw data table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdAdvancedYield::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
{
	QString								strReference;
	QStringList::iterator				it;
	GexDbPlugin_ER_Parts_Layer *		pLayer;
	GexDbPlugin_ER_Parts_SerieDef *		pSerieDef;
	GexDbPlugin_ER_Parts_SerieData *	pSerieData;

	// Add a page break
	gexReport->WritePageBreak();

	// If creating a Powerpoint presentation, save Section title.
	if(pGraph->m_strlGraphSplitValues.isEmpty())
		gexReport->SetPowerPointSlideName("Advanced yield raw data");
	else
		gexReport->SetPowerPointSlideName("Advanced yield raw data : " + pGraph->m_strlGraphSplitValues.join("_"));

	// Links to export data
	//if(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
	QString of=ReportOptions.GetOption("output", "format").toString();
	if ( (of=="INTERACTIVE") || (of=="HTML") )
	{
		fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile,"<tr>\n");

		if(pGraph->m_strlGraphSplitValues.isEmpty())
		{
			strReference.sprintf("%s%s--mission=prod--report=yield_advanced--action=saveexcelfile", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=prod--report=yield_advanced--action=saveexcelclipboard", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}
		else
		{
			strReference.sprintf("%s%s--mission=prod--report=yield_advanced--action=saveexcelfile--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=prod--report=yield_advanced--action=saveexcelclipboard--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
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

	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", m_clER_PartsData.m_strField_Aggregate.toLatin1().constData());
	strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Parts</font></td>\n");

	if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - splitlots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - splitlots"))
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Splitlots</font></td>\n");
	if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - wafers") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - wafers"))
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Wafers</font></td>\n");
	if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - lots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - lots"))
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Lots</font></td>\n");
	if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick wafers") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick wafers"))
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Maverick wafers</font></td>\n");
	if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick lots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick lots"))
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Maverick lots</font></td>\n");

//	pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, m_clER_PartsData.m_plistSerieDefs)
	{
        if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
		{
			strLine += "<td bgcolor=\"#CCFFFF\"><font size=\"3\">";
			strLine += pSerieDef->m_strSerieName;
			strLine += "(%%)</font></td>\n";
		}
        if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
		{
			strLine += "<td bgcolor=\"#CCFFFF\"><font size=\"3\">";
			strLine += pSerieDef->m_strSerieName;
			strLine += "(#)</font></td>\n";
		}
//		pSerieDef = m_clER_PartsData.m_plistSerieDefs.next();
	}
	strLine += "</tr>\n";

	// Set the html table header
	erTableHtml.setHeader(strLine);

	// Data
	unsigned int	uiDataIndex, uiNbTotalParts , uiNbSerieParts;
	double			lfData;
	bool			bException;
	QString			strFontSpec;
//	pLayer = pGraph->first();
//	while(pLayer)
    foreach(pLayer, *pGraph)
	{
		for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
		{
			uiNbTotalParts = pLayer->m_uilNbParts[uiDataIndex];

			// First check if there is a yield exception
			bException = false;
//			pSerieData = pLayer->first();
//			pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//			while(pSerieData)
            int indexSerieDef = 0;
            foreach(pSerieData, *pLayer)
            {
                pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];
                if (!pSerieData || !pSerieDef)
                    break;

				uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
				lfData = ((double)uiNbSerieParts/(double)uiNbTotalParts)*100.0;
				if(pSerieDef->m_uiYieldExceptionLimit_Type == 1)
				{
					if(pSerieDef->m_bYieldExceptionLimit_Strict && (lfData < pSerieDef->m_fYieldExceptionLimit))
						bException = true;
					else if(lfData <= pSerieDef->m_fYieldExceptionLimit)
						bException = true;
				}
				else if(pSerieDef->m_uiYieldExceptionLimit_Type == 2)
				{
					if(pSerieDef->m_bYieldExceptionLimit_Strict && (lfData > pSerieDef->m_fYieldExceptionLimit))
						bException = true;
					else if(lfData >= pSerieDef->m_fYieldExceptionLimit)
						bException = true;
				}

//				pSerieData = pLayer->next();
                indexSerieDef++;
			}

			if(bException)
				strFontSpec = "<font size=\"3\" color=\"#FF3333\">";
			else
				strFontSpec = "<font size=\"3\">";

			strLine = "<tr>\n";

			for(it = pGraph->m_strlGraphSplitValues.begin(); it != pGraph->m_strlGraphSplitValues.end(); it++)
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());

			for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());

			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (pLayer->m_strlAggregateLabels[uiDataIndex]).toLatin1().constData());
			strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbTotalParts);

			if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - splitlots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - splitlots"))
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), pLayer->m_uilNbSplitlots[uiDataIndex]);

			if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - wafers") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - wafers"))
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), pLayer->m_uilNbWafers[uiDataIndex]);

			if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - lots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - lots"))
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), pLayer->m_uilNbLots[uiDataIndex]);

			if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick wafers") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick wafers"))
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), pLayer->m_uilNbMaverickWafers[uiDataIndex]);

			if((m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick lots") || (m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick lots"))
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), pLayer->m_uilNbMaverickLots[uiDataIndex]);

//			pSerieData = pLayer->first();
//			pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
            indexSerieDef = 0;
//			while(pSerieData)
            foreach(pSerieData, *pLayer)
			{
                pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];
                if (!pSerieData || !pSerieData)
                    break;
				uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
				lfData = ((double)uiNbSerieParts/(double)uiNbTotalParts)*100.0;
                if(pSerieDef->m_strTableData.toLower().indexOf("yield") != -1)
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%5.2f</font></td>\n", strFontSpec.toLatin1().constData(), lfData);
                if(pSerieDef->m_strTableData.toLower().indexOf("volume") != -1)
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbSerieParts);

//				pSerieData = pLayer->next();
                indexSerieDef++;
			}
			strLine += "</tr>\n";

			// Add the line to the html table
			erTableHtml.addLine(strLine);
		}

//		pLayer = pGraph->next();
	}

	erTableHtml.close();
}

/******************************************************************************!
 * \fn makeChart
 * \brief Create the chart image
 ******************************************************************************/
void GexERProdAdvancedYield::makeChart(
    GexDbPlugin_ER_Parts_Graph* pGraph,
    const QString& strChartFullName,
    const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
	double							*plfData_Yield;
	double							*plfData_Volume, lfMaxVolume=0.0;
	char							**pcLabels;
	unsigned int					uiLayerIndex, uiSerieIndex, uiNbBarGraphs, uiNbLineGraphs;
	unsigned int					uiIndex, uiNbMergedLabels, uiMergedIndex;
	QString							strVolumeAxisLabel_Left, strVolumeAxisLabel_Right;
	QStringList::iterator			it;
    GexDbPlugin_ER_Parts_Layer		*pLayer = NULL;
    GexDbPlugin_ER_Parts_SerieData	*pSerieData = NULL;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef = NULL;

	// Create a XYChart object of size 800 x 500 pixels.
	XYChart *pChart = new XYChart(800,500);

	// Compute plot area
	QRect	clPlotRect;
	int		nStartY;
    if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().startsWith("3d"))
		nStartY = 50+(int)ceil((double)(pGraph->count())/5.0)*30;
	else
		nStartY = 30+(int)ceil((double)(pGraph->count())/5.0)*30;

#if 0 // Not enough space on the right for Y-axis title if only right-axis selected
	if(m_clER_PartsData.m_uiNbAxis == 2)
		clPlotRect.setRect(70, nStartY, 640, 410-nStartY);
	else
		clPlotRect.setRect(70, nStartY, 710, 410-nStartY);
#endif
	clPlotRect.setRect(70, nStartY, 640, 410-nStartY);

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

	// Compute X-Axis label merge
	QStringList strlLabelMerge;
//	pLayer = pGraph->first();
//	while(pLayer)
    foreach(pLayer, *pGraph)
	{
		for(it=pLayer->m_strlAggregateLabels.begin(); it!=pLayer->m_strlAggregateLabels.end(); it++)
		{
            if(strlLabelMerge.indexOf(*it) == -1)
				strlLabelMerge.append(*it);
		}
//		pLayer = pGraph->next();
	}
	uiNbMergedLabels = strlLabelMerge.count();

	// Sort X-Axis label merge
	strlLabelMerge.sort();

	// Create list of short labels
	QStringList strlLabelMerge_Short;
	QString		strLabel;
	for(uiIndex=0; uiIndex<uiNbMergedLabels; uiIndex++)
	{
		strLabel = strlLabelMerge[uiIndex];
		if(strLabel.length() > 18)
		{
			strLabel = strLabel.left(15);
			strLabel += "...";
		}
		strlLabelMerge_Short.append(strLabel);
	}

	// Allocate mem space for merged label and data pointers
	plfData_Yield	= new double[uiNbMergedLabels];
	plfData_Volume	= new double[uiNbMergedLabels];
	pcLabels		= new char*[uiNbMergedLabels];

	// Init merged label pointer
	for(uiIndex=0; uiIndex<uiNbMergedLabels; uiIndex++)
    {
        pcLabels[uiIndex] = new char[strlLabelMerge_Short[uiIndex].size() + 1];
        strcpy(pcLabels[uiIndex], strlLabelMerge_Short[uiIndex].toLatin1().constData());
    }

	// Set the labels on the x axis.
	pChart->xAxis()->setLabelStyle(0, 7.5, 0, 30);
	pChart->xAxis()->setLabels(StringArray(pcLabels, uiNbMergedLabels));

	// Add a title to the y axis, and set label format
	bool bLeftVolume=false, bRightVolume=false, bYield=false;

	if(pEnterpriseReport->m_strLeftAxis.toLower() == "serie - yield")
	{
		bYield=true;
		pChart->yAxis()->setTitle("Yield (%)");
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - volume")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Volume";
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - splitlots")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Splitlots";
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - wafers")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Wafers";
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - lots")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Lots";
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick wafers")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Maverick wafers";
	}
	if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick lots")
	{
		bLeftVolume=true;
		strVolumeAxisLabel_Left = "Maverick lots";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "serie - yield")
	{
		bYield=true;
		pChart->yAxis2()->setTitle("Yield (%)");
	}

	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - volume")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Volume";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - splitlots")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Splitlots";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - wafers")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Wafers";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - lots")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Lots";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick wafers")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Maverick wafers";
	}
	if(pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick lots")
	{
		bRightVolume=true;
		strVolumeAxisLabel_Right = "Maverick lots";
	}

	// Customize axis label format
	pChart->yAxis()->setAutoScale(0.1, 0.1, 0.8);
	pChart->yAxis2()->setAutoScale(0.1, 0.1, 0.8);

	// Add a title to the x axis
	pChart->xAxis()->setTitle(pEnterpriseReport->m_strAggregateField.toLatin1().constData());

	// Add data layers
	LineLayer	*pLineLayer_Left=NULL, *pLineLayer_Right=NULL;
	LineLayer	*pSplineLayer_Left=NULL, *pSplineLayer_Right=NULL;
	LineLayer	*pSteplineLayer_Left=NULL, *pSteplineLayer_Right=NULL;
	BarLayer	*pBarLayer_Left=NULL, *pBarLayer_Right=NULL;
	Layer		*pDataLayer;
	DataSet		*pDataSet;
	TextBox		*pTextBox=NULL;

	// Draw the ticks between label positions (instead of at label positions)
	pChart->xAxis()->setTickOffset(0.5);

	// Create all layers, even if not used:
	float fOverlappingRatio = (float)(pEnterpriseReport->m_YieldWizard_Global_nOverlappingRatio)/100.0F;
	// LEFT LINE layer
	pLineLayer_Left = pChart->addLineLayer();
	pLineLayer_Left->setGapColor(Chart::SameAsMainColor);						// Make sure data gaps are filled with a line using same color as the data
	// RIGHT LINE layer
	pLineLayer_Right = pChart->addLineLayer();
	pLineLayer_Right->setGapColor(Chart::SameAsMainColor);						// Make sure data gaps are filled with a line using same color as the data
	pLineLayer_Right->setUseYAxis2();
	// LEFT SPLINE layer
	pSplineLayer_Left = pChart->addSplineLayer();
	pSplineLayer_Left->setGapColor(Chart::SameAsMainColor);						// Make sure data gaps are filled with a line using same color as the data
	// RIGHT SPLINE layer
	pSplineLayer_Right = pChart->addSplineLayer();
	pSplineLayer_Right->setGapColor(Chart::SameAsMainColor);					// Make sure data gaps are filled with a line using same color as the data
	pSplineLayer_Right->setUseYAxis2();
	// LEFT STEPLINE layer
	pSteplineLayer_Left = pChart->addStepLineLayer();
	pSteplineLayer_Left->setGapColor(Chart::SameAsMainColor);					// Make sure data gaps are filled with a line using same color as the data
	// RIGHT STEPLINE layer
	pSteplineLayer_Right = pChart->addStepLineLayer();
	pSteplineLayer_Right->setGapColor(Chart::SameAsMainColor);					// Make sure data gaps are filled with a line using same color as the data
	pSteplineLayer_Right->setUseYAxis2();
	// LEFT BAR layer
	pBarLayer_Left = pChart->addBarLayer(Chart::Side);
	pBarLayer_Left->setBarGap(0.2, Chart::TouchBar);							// Configure the bars within a group to touch each others (no gap)
    if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().indexOf("overlapping") != -1)
		pBarLayer_Left->setOverlapRatio(fOverlappingRatio, false);				// Set overlap between bars
	if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().startsWith("3d"))								// Set layer to 3D depth
		pBarLayer_Left->set3D(10);
	if(pEnterpriseReport->m_YieldWizard_Global_bGradientBarColor)
		pBarLayer_Left->setBorderColor(0, Chart::softLighting(Chart::Right));	// Use gradient colors in bars
    if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().indexOf("cylindrical") != -1)						// Set bar shape
		pBarLayer_Left->setBarShape(Chart::CircleShape);
	// RIGHT BAR layer
	pBarLayer_Right = pChart->addBarLayer(Chart::Side);
	pBarLayer_Right->setBarGap(0.2, Chart::TouchBar);							// Configure the bars within a group to touch each others (no gap)
    if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().indexOf("overlapping") != -1)
		pBarLayer_Right->setOverlapRatio(fOverlappingRatio, false);				// Set overlap between bars
	if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().startsWith("3d"))								// Set layer to 3D depth
		pBarLayer_Right->set3D(10);
	if(pEnterpriseReport->m_YieldWizard_Global_bGradientBarColor)
		pBarLayer_Right->setBorderColor(0, Chart::softLighting(Chart::Right));	// Use gradient colors in bars
    if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().indexOf("cylindrical") != -1)						// Set bar shape
		pBarLayer_Right->setBarShape(Chart::CircleShape);
	pBarLayer_Right->setUseYAxis2();

	// Compute merged dataset for each split chart, and add it to the layer
	int				nDataColor, nR, nG, nB;
	QString			strName, strColor;
	bool			bBarGraph=false;
	QColor			cColor;
	bool			bOK, bAddDataset;

	// Set dimension for color generator
	// More readable if we use the highest dimension as the colour dimension, and the smaller one as the shade dimension
	// Compare layer splits and datasets (series + volume)
    int uiNbDatasets=0;
	if(bYield)
		uiNbDatasets = m_clER_PartsData.m_uiSeries;
	if(bLeftVolume || bRightVolume)
		uiNbDatasets++;
    if(pEnterpriseReport->m_YieldWizard_strColorMgt.toLower() == "auto")
	{
		if(pGraph->count() > uiNbDatasets)
			ColorsGenerator::setDim(pGraph->count(), uiNbDatasets);
		else
			ColorsGenerator::setDim(uiNbDatasets, pGraph->count());
	}

	uiNbBarGraphs = uiNbLineGraphs = uiLayerIndex = 0;
    foreach(pLayer, *pGraph)
	{
        if (!pLayer)
            break;
		///////////////////////////////////////////////////////
		// ADD VOLUME DATA ON LEFT AXIS??
		///////////////////////////////////////////////////////

		// Init volume array
		lfMaxVolume = 0.0;
		for(uiIndex=0; uiIndex<uiNbMergedLabels; uiIndex++)
			plfData_Volume[uiIndex] = Chart::NoValue;

		// Set color
        if(pEnterpriseReport->m_YieldWizard_strColorMgt.toLower() == "auto")
		{
			if(pGraph->count() > uiNbDatasets)
				ColorsGenerator::getColor(uiLayerIndex, 0, cColor);
			else
				ColorsGenerator::getColor(0, uiLayerIndex, cColor);
		}
		else
		{
			cColor = pEnterpriseReport->m_YieldWizard_Volume_cColor;
			for(uiIndex=0; uiIndex<uiLayerIndex; uiIndex++)
			{
				if(pEnterpriseReport->m_YieldWizard_Global_strLayerPolicy == "Dark to Light")
					cColor = cColor.lighter(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
				else
					cColor = cColor.darker(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
			}
		}
		strColor.sprintf("%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
		nDataColor = strColor.toInt(&bOK, 16);

		pDataSet = NULL;
		pDataLayer = NULL;
		pTextBox = NULL;
		bAddDataset = false;
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - volume")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Volume)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbParts[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbParts[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbParts[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - splitlots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Splitlots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbSplitlots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbSplitlots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbSplitlots[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - wafers")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Wafers)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbWafers[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbWafers[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbWafers[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - lots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Lots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbLots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbLots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbLots[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick wafers")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (MaverickWafers)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbMaverickWafers[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbMaverickWafers[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbMaverickWafers[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strLeftAxis.toLower() == "total - maverick lots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (MaverickLots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbMaverickLots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbMaverickLots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbMaverickLots[uiIndex]);
			}
			bAddDataset = true;
		}

		// Volume data added??
		if(bAddDataset)
		{
			bBarGraph = false;

			// Set layer
			if(pEnterpriseReport->m_YieldWizard_Volume_strChartingMode.toLower() == "bars")
			{
				// Use transparency?
				if(pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar)
					nDataColor = nDataColor | 0x80000000;
				pDataLayer = pBarLayer_Left;
				bBarGraph = true;
			}
			else
			{
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineProperty.toLower() != "solidline")
					nDataColor = pChart->dashLineColor(nDataColor);
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineStyle.toLower().startsWith("spline"))
					pDataLayer = pSplineLayer_Left;
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineStyle.toLower().startsWith("step"))
					pDataLayer = pSteplineLayer_Left;
				else
					pDataLayer = pLineLayer_Left;
			}

			// Add dataset
			pDataSet = pDataLayer->addDataSet(DoubleArray(plfData_Volume, uiNbMergedLabels), nDataColor, strName.toLatin1().constData());

			// Set Y-Axis label, data labels, and scales
			pChart->yAxis()->setAutoScale(0.1, 0.1, 0.8);
			if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() != "no label")
			{
				pTextBox = pDataLayer->setDataLabelStyle(0, 8, Chart::TextColor, 0);
				// Move labels ?
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "bottom")
					pTextBox->setAlignment(Chart::Top);
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "left")
					pTextBox->setAlignment(Chart::Right);
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "right")
					pTextBox->setAlignment(Chart::Left);
			}
			if(lfMaxVolume > 1000000000000.0)
			{
				pChart->yAxis()->setLabelFormat("{={value}/1000000000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000000000|2}");
				strVolumeAxisLabel_Left += " (T)";
			}
			else if(lfMaxVolume > 1000000000.0)
			{
				pChart->yAxis()->setLabelFormat("{={value}/1000000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000000|2}");
				strVolumeAxisLabel_Left += " (G)";
			}
			else if(lfMaxVolume > 1000000.0)
			{
				pChart->yAxis()->setLabelFormat("{={value}/1000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000|2}");
				strVolumeAxisLabel_Left += " (M)";
			}
			else if(lfMaxVolume > 1000.0)
			{
				pChart->yAxis()->setLabelFormat("{={value}/1000}");
				pDataLayer->setDataLabelFormat("{={value}/1000|2}");
				strVolumeAxisLabel_Left += " (K)";
			}
			else
			{
				pChart->yAxis()->setLabelFormat("{={value}}");
				pDataLayer->setDataLabelFormat("{value}");
			}
            pChart->yAxis()->setTitle(strVolumeAxisLabel_Left.toLatin1().constData());

			// Use spots?
			if(pDataSet && !bBarGraph)
			{
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "square")
					pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "circle")
					pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "diamonds")
					pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "triangle")
					pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "cross")
					pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
			}
		}

		///////////////////////////////////////////////////////
		// ADD VOLUME DATA ON RIGHT AXIS??
		///////////////////////////////////////////////////////

		// Init volume array
		lfMaxVolume = 0.0;
		for(uiIndex=0; uiIndex<uiNbMergedLabels; uiIndex++)
			plfData_Volume[uiIndex] = Chart::NoValue;

		// Set color
        if(pEnterpriseReport->m_YieldWizard_strColorMgt.toLower() == "auto")
		{
			if(pGraph->count() > uiNbDatasets)
				ColorsGenerator::getColor(uiLayerIndex, 0, cColor);
			else
				ColorsGenerator::getColor(0, uiLayerIndex, cColor);
		}
		else
		{
			cColor = pEnterpriseReport->m_YieldWizard_Volume_cColor;
			for(uiIndex=0; uiIndex<uiLayerIndex; uiIndex++)
			{
				if(pEnterpriseReport->m_YieldWizard_Global_strLayerPolicy == "Dark to Light")
					cColor = cColor.lighter(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
				else
					cColor = cColor.darker(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
			}
		}
		strColor.sprintf("%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
		nDataColor = strColor.toInt(&bOK, 16);
		QString strMessage = "Use Volume color: " + strColor;
		strMessage += " (" + QString::number(nDataColor);
		strMessage += ")";
		WriteDebugMessageFile(strMessage);

		pDataSet = NULL;
		pDataLayer = NULL;
		pTextBox = NULL;
		bAddDataset = false;
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - volume")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Volume)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbParts[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbParts[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbParts[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - splitlots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Splitlots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbSplitlots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbSplitlots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbSplitlots[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - wafers")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Wafers)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbWafers[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbWafers[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbWafers[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - lots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (Lots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbLots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbLots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbLots[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick wafers")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (MaverickWafers)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbMaverickWafers[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbMaverickWafers[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbMaverickWafers[uiIndex]);
			}
			bAddDataset = true;
		}
		if(pEnterpriseReport->m_strRightAxis.toLower() == "total - maverick lots")
		{
			strName = pLayer->m_strlLayerSplitValues.join(",") + " (MaverickLots)";
			for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
			{
                uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
				plfData_Volume[uiMergedIndex] = (double)(pLayer->m_uilNbMaverickLots[uiIndex]);
				if(lfMaxVolume < (double)(pLayer->m_uilNbMaverickLots[uiIndex]))
					lfMaxVolume = (double)(pLayer->m_uilNbMaverickLots[uiIndex]);
			}
			bAddDataset = true;
		}

		// Volume data added??
		if(bAddDataset)
		{
			bBarGraph = false;

			// Set layer
			if(pEnterpriseReport->m_YieldWizard_Volume_strChartingMode.toLower() == "bars")
			{
				// Use transparency?
				if(pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar)
					nDataColor = nDataColor | 0x80000000;
				pDataLayer = pBarLayer_Right;
				bBarGraph = true;
			}
			else
			{
				// Dashed line?
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineProperty.toLower() != "solidline")
					nDataColor = pChart->dashLineColor(nDataColor);
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineStyle.toLower().startsWith("spline"))
					pDataLayer = pSplineLayer_Right;
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineStyle.toLower().startsWith("step"))
					pDataLayer = pSteplineLayer_Right;
				else
					pDataLayer = pLineLayer_Right;
			}

			// Add dataset
			pDataSet = pDataLayer->addDataSet(DoubleArray(plfData_Volume, uiNbMergedLabels), nDataColor, strName.toLatin1().constData());

			// Set Y-Axis label, data labels, and scales
			pChart->yAxis2()->setAutoScale(0.1, 0.1, 0.8);
			if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() != "no label")
			{
				pTextBox = pDataLayer->setDataLabelStyle(0, 8, Chart::TextColor, 0);
				// Move labels ?
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "bottom")
					pTextBox->setAlignment(Chart::Top);
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "left")
					pTextBox->setAlignment(Chart::Right);
				if(pEnterpriseReport->m_YieldWizard_Volume_strDataLabels.toLower() == "right")
					pTextBox->setAlignment(Chart::Left);
			}

			// Display labels with same scale as the Y-Axis
			if(lfMaxVolume > 1000000000000.0)
			{
				pChart->yAxis2()->setLabelFormat("{={value}/1000000000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000000000|2}");
				strVolumeAxisLabel_Right += " (T)";
			}
			else if(lfMaxVolume > 1000000000.0)
			{
				pChart->yAxis2()->setLabelFormat("{={value}/1000000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000000|2}");
				strVolumeAxisLabel_Right += " (G)";
			}
			else if(lfMaxVolume > 1000000.0)
			{
				pChart->yAxis2()->setLabelFormat("{={value}/1000000}");
				pDataLayer->setDataLabelFormat("{={value}/1000000|2}");
				strVolumeAxisLabel_Right += " (M)";
			}
			else if(lfMaxVolume > 1000.0)
			{
				pChart->yAxis2()->setLabelFormat("{={value}/1000}");
				pDataLayer->setDataLabelFormat("{={value}/1000|2}");
				strVolumeAxisLabel_Right += " (K)";
			}
			else
			{
				pChart->yAxis2()->setLabelFormat("{={value}}");
				pDataLayer->setDataLabelFormat("{value}");
			}
            pChart->yAxis2()->setTitle(strVolumeAxisLabel_Right.toLatin1().constData());

			// Use spots?
			if(!bBarGraph)
			{
				if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "square")
					pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "circle")
					pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "diamonds")
					pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "triangle")
					pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
				else if(pEnterpriseReport->m_YieldWizard_Volume_strLineSpots.toLower() == "cross")
					pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
			}
		}

		///////////////////////////////////////////////////////
		// ADD SERIE DATA ??
		///////////////////////////////////////////////////////

		uiSerieIndex = 0;
        int indexSerieDef(0);
//		while(pSerieData)
        foreach(pSerieData, *pLayer)
		{
            pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];

            if (!pSerieData || !pSerieDef)
                break;


			if(pSerieDef->m_bPlotSerie)
			{
				// Dataset name
				strName = pLayer->m_strlLayerSplitValues.join(",") + " (";
				strName += pSerieDef->m_strSerieName + ")";

				// Compute yield
				for(uiIndex=0; uiIndex<uiNbMergedLabels; uiIndex++)
					plfData_Yield[uiIndex] = Chart::NoValue;
				for(uiIndex=0; uiIndex<pLayer->m_uiDataPoints; uiIndex++)
				{
                    uiMergedIndex = strlLabelMerge.indexOf(pLayer->m_strlAggregateLabels[uiIndex]);
					plfData_Yield[uiMergedIndex] = ((double)((*pSerieData)[uiIndex].m_uiMatchingParts)/(double)(pLayer->m_uilNbParts[uiIndex]))*100.0;
				}

				// Get color
                if(pEnterpriseReport->m_YieldWizard_strColorMgt.toLower() == "auto")
				{
					if(pGraph->count() > uiNbDatasets)
						ColorsGenerator::getColor(uiLayerIndex, uiSerieIndex+1, cColor);
					else
						ColorsGenerator::getColor(uiSerieIndex+1, uiLayerIndex, cColor);
				}
				else
				{
					nB = pSerieDef->m_nColor & 0x0ff;
					nG = (pSerieDef->m_nColor & 0x0ff00) >> 8;
					nR = (pSerieDef->m_nColor & 0x0ff0000) >> 16;
					cColor.setRgb(nR, nG, nB);
					for(uiIndex=0; uiIndex<uiLayerIndex; uiIndex++)
					{
						if(pEnterpriseReport->m_YieldWizard_Global_strLayerPolicy == "Dark to Light")
							cColor = cColor.lighter(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
						else
							cColor = cColor.darker(COLOR_DARKTOLIGHT_LIGHTTODARK_FACTOR);
					}
				}
				nDataColor = cColor.rgb();
				strColor.sprintf("%02x%02x%02x",cColor.red(),cColor.green(),cColor.blue());
				nDataColor = strColor.toInt(&bOK, 16);

				// Yield on left axis?
				bBarGraph = false;
				if(pEnterpriseReport->m_strLeftAxis.toLower() == "serie - yield")
				{
					if(pSerieDef->m_strChartingMode.toLower() == "bars")
					{
						// Use transparency?
						if(pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar)
							nDataColor = nDataColor | 0x80000000;
						pDataLayer = pBarLayer_Left;
						bBarGraph = true;
					}
					else
					{
						// Dashed line?
						if(pSerieDef->m_strLineProperty.toLower() != "solidline")
							nDataColor = pChart->dashLineColor(nDataColor);
						if(pSerieDef->m_strLineStyle.toLower().startsWith("spline"))
							pDataLayer = pSplineLayer_Left;
						else if(pSerieDef->m_strLineStyle.toLower().startsWith("step"))
							pDataLayer = pSteplineLayer_Left;
						else
							pDataLayer = pLineLayer_Left;
					}

					// Add data
					pDataSet = pDataLayer->addDataSet(DoubleArray(plfData_Yield, uiNbMergedLabels), nDataColor, strName.toLatin1().constData());

					// Enable data labels on data
					if(pSerieDef->m_strDataLabels.toLower() != "no label")
					{
						pTextBox = pDataLayer->setDataLabelStyle(0, 8, Chart::TextColor, 0);
						pDataLayer->setDataLabelFormat("{value|2}");

						// Move labels ?
						if(pSerieDef->m_strDataLabels.toLower() == "bottom")
							pTextBox->setAlignment(Chart::Top);
						if(pSerieDef->m_strDataLabels.toLower() == "left")
							pTextBox->setAlignment(Chart::Right);
						if(pSerieDef->m_strDataLabels.toLower() == "right")
							pTextBox->setAlignment(Chart::Left);
					}

					// Use spots?
					if(!bBarGraph)
					{
						if(pSerieDef->m_strLineSpots.toLower() == "square")
							pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "circle")
							pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "diamonds")
							pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "triangle")
							pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "cross")
							pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
					}
				}

				// Yield on right axis?
				bBarGraph = false;
				if(pEnterpriseReport->m_strRightAxis.toLower() == "serie - yield")
				{
					if(pSerieDef->m_strChartingMode.toLower() == "bars")
					{
						// Use transparency?
						if(pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar)
							nDataColor = nDataColor | 0x80000000;
						pDataLayer = pBarLayer_Right;
						bBarGraph = true;
					}
					else
					{
						// Dashed line?
						if(pSerieDef->m_strLineProperty.toLower() != "solidline")
							nDataColor = pChart->dashLineColor(nDataColor);
						if(pSerieDef->m_strLineStyle.toLower().startsWith("spline"))
							pDataLayer = pSplineLayer_Right;
						else if(pSerieDef->m_strLineStyle.toLower().startsWith("step"))
							pDataLayer = pSteplineLayer_Right;
						else
							pDataLayer = pLineLayer_Right;
					}

					// Add data
					pDataLayer->setUseYAxis2();
					pDataSet = pDataLayer->addDataSet(DoubleArray(plfData_Yield, uiNbMergedLabels), nDataColor, strName.toLatin1().constData());

					// Enable data labels on data
					if(pSerieDef->m_strDataLabels.toLower() != "no label")
					{
						pTextBox = pDataLayer->setDataLabelStyle(0, 8, Chart::TextColor, 0);
						pDataLayer->setDataLabelFormat("{value|2}");

						// Move labels ?
						if(pSerieDef->m_strDataLabels.toLower() == "bottom")
							pTextBox->setAlignment(Chart::Top);
						if(pSerieDef->m_strDataLabels.toLower() == "left")
							pTextBox->setAlignment(Chart::Right);
						if(pSerieDef->m_strDataLabels.toLower() == "right")
							pTextBox->setAlignment(Chart::Left);
					}

					// Use spots?
					if(!bBarGraph)
					{
						if(pSerieDef->m_strLineSpots.toLower() == "square")
							pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "circle")
							pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "diamonds")
							pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "triangle")
							pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
						else if(pSerieDef->m_strLineSpots.toLower() == "cross")
							pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
					}
				}
			}

//			pSerieData = pLayer->next();
            indexSerieDef++;
			uiSerieIndex++;

			if(bBarGraph)
				uiNbBarGraphs++;
			else
				uiNbLineGraphs++;
		}

//		pLayer = pGraph->next();
		uiLayerIndex++;
	}

	// output the chart
	pChart->makeChart(strChartFullName.toLatin1().constData());

	//free up resources
	delete [] plfData_Yield;
	delete [] plfData_Volume;
    for (unsigned int lLabelIndx = 0; lLabelIndx < uiNbMergedLabels; ++lLabelIndx)
        if (pcLabels[lLabelIndx])
            delete pcLabels[lLabelIndx];
	delete [] pcLabels;
	delete pChart;
    pChart = 0;
}

/******************************************************************************!
 * \fn writeYieldExceptionGraphs
 * \brief Write the Yields exception tables and charts
 ******************************************************************************/
void GexERProdAdvancedYield::writeYieldExceptionGraphs(
    GexDbPlugin_ER_Parts_Graph* pGraph,
    const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport,
    const QString& strImage_Base)
{
	QString							strImage, strImageFullName, strExceptionKey;
	QStringList::iterator			it1, it2;
	GexDbPlugin_ER_Parts_Layer		*pLayer;
	GexDbPlugin_ER_Parts_SerieData	*pSerieData;
    GexDbPlugin_ER_Parts_SerieDef	*pSerieDef = 0;
	GexDbPlugin_BinList				clBinList;
    QMap<QString, int>				mapExceptions;
    QMap<QString, int>::iterator	itExceptions;
    GexDbPlugin_BinInfo				*pBinInfo;

    // Data
	unsigned int	uiDataIndex, uiNbTotalParts, uiNbTotalParts_Pass, uiNbTotalParts_Fail, uiNbSerieParts, uiLayerIndex=0, uiNbExceptions=0;
	double			lfData;
	bool			bException;

	// Add a page break
    gexReport->WritePageBreak();

	// If creating a Powerpoint presentation, save Section title.
	if(pGraph->m_strlGraphSplitValues.isEmpty())
		gexReport->SetPowerPointSlideName("Advanced yield exceptions");
	else
		gexReport->SetPowerPointSlideName("Advanced yield exceptions : " + pGraph->m_strlGraphSplitValues.join("_"));

//	pLayer = pGraph->first();
//	while(pLayer)
    foreach(pLayer, *pGraph)
	{
		for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
		{
			uiNbTotalParts = pLayer->m_uilNbParts[uiDataIndex];
			uiNbTotalParts_Pass = pLayer->m_uilNbParts_Good[uiDataIndex];
			uiNbTotalParts_Fail = uiNbTotalParts-uiNbTotalParts_Pass;

//			pSerieData = pLayer->first();
//			pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
            int indexSerieDef = 0;
//			while(pSerieData)
            foreach(pSerieData, *pLayer)
			{
                if (!pSerieData || !pSerieDef)
                    break;

                pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];
				bException = false;
				uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
				lfData = ((double)uiNbSerieParts/(double)uiNbTotalParts)*100.0;

				if(pSerieDef->m_uiYieldExceptionLimit_Type == 1)
				{
					if(pSerieDef->m_bYieldExceptionLimit_Strict && (lfData < pSerieDef->m_fYieldExceptionLimit))
						bException = true;
					else if(lfData <= pSerieDef->m_fYieldExceptionLimit)
						bException = true;
				}
				else if(pSerieDef->m_uiYieldExceptionLimit_Type == 2)
				{
					if(pSerieDef->m_bYieldExceptionLimit_Strict && (lfData > pSerieDef->m_fYieldExceptionLimit))
						bException = true;
					else if(lfData >= pSerieDef->m_fYieldExceptionLimit)
						bException = true;
				}
				strExceptionKey = pLayer->m_strlLayerSplitValues.join("#") + "#" + pLayer->m_strlAggregateLabels[uiDataIndex];
                itExceptions = mapExceptions.find(strExceptionKey);
                if(itExceptions != mapExceptions.end())
					bException = false;

				if(bException)
				{
					if(uiNbExceptions != 0)
						// Add a page break
						gexReport->WritePageBreak();

					fprintf(m_hReportFile, "<table border=\"1\" width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

					// Clear BinList to free ressources
					clBinList.clear();

					// Compute Yield data for specified filters
					if(m_pDatabaseEntry->m_pExternalDatabase->ER_Prod_GetBinnings(m_cFilters, m_clER_PartsData, pGraph, pLayer, pLayer->m_strlAggregateLabels[uiDataIndex], clBinList))
					{
						// Build Image name
						strImage = strImage_Base;
						strImage += "_l" + QString::number(uiLayerIndex);
						strImage += "_d" + QString::number(uiDataIndex);
						strImage += ".png";
						strImageFullName = m_strChartDirectory + strImage;

						// Create image
						makechart_Advanced_BinningPareto(pLayer, strImageFullName, pEnterpriseReport, clBinList, uiDataIndex);

						// NEW EXCEPTION: add row in Exception table
						fprintf(m_hReportFile, "<tr><td>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

						// 1st row = Dataset details + Binning table
						fprintf(m_hReportFile, "<tr><td>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"10\"> </td>\n");
						fprintf(m_hReportFile, "<td>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Dataset:</b></font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"> </font></td>\n");
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						it1=m_clER_PartsData.m_strlFields_GraphSplit.begin();
						it2=pGraph->m_strlGraphSplitValues.begin();
						while(it1 != m_clER_PartsData.m_strlFields_GraphSplit.end() && it2 != pGraph->m_strlGraphSplitValues.end())
						{
							fprintf(m_hReportFile, "<tr>\n");
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it1).toLatin1().constData());
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it2).toLatin1().constData());
							fprintf(m_hReportFile, "</tr>\n");
							it1++;
							it2++;
						}
						it1=m_clER_PartsData.m_strlFields_LayerSplit.begin();
						it2=pLayer->m_strlLayerSplitValues.begin();
						while(it1!=m_clER_PartsData.m_strlFields_LayerSplit.end() && it2!=pLayer->m_strlLayerSplitValues.end())
						{
							fprintf(m_hReportFile, "<tr>\n");
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it1).toLatin1().constData());
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it2).toLatin1().constData());
							fprintf(m_hReportFile, "</tr>\n");
							it1++;
							it2++;
						}
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", m_clER_PartsData.m_strField_Aggregate.toLatin1().constData());
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", pLayer->m_strlAggregateLabels[uiDataIndex].toLatin1().constData());
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">Total parts</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%d</font></td>\n", uiNbTotalParts);
						fprintf(m_hReportFile, "</tr>\n");
						lfData = ((double)uiNbTotalParts_Pass/(double)uiNbTotalParts)*100.0;
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">Total PASS parts</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%d (%5.2f%%)</font></td>\n", uiNbTotalParts_Pass, lfData);
						fprintf(m_hReportFile, "</tr>\n");
						lfData = ((double)uiNbTotalParts_Fail/(double)uiNbTotalParts)*100.0;
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">Total FAIL parts</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%d (%5.2f%%)</font></td>\n", uiNbTotalParts_Fail, lfData);
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "</td>\n");

						fprintf(m_hReportFile, "<td>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass"))
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Bin counts (PASS parts):</b></font></td>\n");
						else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail"))
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Bin counts (FAIL parts):</b></font></td>\n");
						else
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Bin counts (ALL parts):</b></font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"> </font></td>\n");
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" border=\"1\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass"))
						{
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin #</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Name</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Count</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%% (on PASS parts)</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%% (on ALL parts)</font></td>\n");
						}
						else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail"))
						{
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin #</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Name</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Count</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%% (on FAIL parts)</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%% (on ALL parts)</font></td>\n");
						}
						else
						{
							fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin #</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Name</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Count</font></td>\n");
							fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%% (on ALL parts)</font></td>\n");
						}
						fprintf(m_hReportFile, "</tr>\n");
//						for(pBinInfo=clBinList.first(); pBinInfo; pBinInfo=clBinList.next())
                        foreach(pBinInfo, clBinList)
						{
							if((pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass")) && (pBinInfo->m_cBinCat.toLower() == 'p'))
							{
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<tr>\n");
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinNo);
								if(pBinInfo->m_strBinName.isEmpty())
									fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">-</font></td>\n");
								else
									fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%s</font></td>\n", pBinInfo->m_strBinName.toLatin1().constData());
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinCount);
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts_Pass)*100.0;
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
								fprintf(m_hReportFile, "</tr>\n");
							}
							else if((pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail")) && (pBinInfo->m_cBinCat.toLower() == 'f'))
							{
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<tr>\n");
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinNo);
								if(pBinInfo->m_strBinName.isEmpty())
									fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">-</font></td>\n");
								else
									fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%s</font></td>\n", pBinInfo->m_strBinName.toLatin1().constData());
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinCount);
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts_Fail)*100.0;
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<td width=\"20%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
								fprintf(m_hReportFile, "</tr>\n");
							}
							else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("all"))
							{
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<tr>\n");
								fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinNo);
								if(pBinInfo->m_strBinName.isEmpty())
									fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">-</font></td>\n");
								else
									fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%s</font></td>\n", pBinInfo->m_strBinName.toLatin1().constData());
								fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinCount);
								lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
								fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
								fprintf(m_hReportFile, "</tr>\n");
							}
						}
						fprintf(m_hReportFile, "</table><br>\n");
						fprintf(m_hReportFile, "</td>\n");
						fprintf(m_hReportFile, "<td width=\"10\"> </td>\n");
						fprintf(m_hReportFile, "</tr>\n");

						// Close first row
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "</td></tr>\n");

						// 2nd row = Binning graph
						fprintf(m_hReportFile, "<tr><td>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"10\"> </td>\n");
						fprintf(m_hReportFile, "<td>\n");
						fprintf(m_hReportFile, "<align=\"center\"><img border=\"0\" src=\"../images/%s\"></img>\n", formatHtmlImageFilename(strImage).toLatin1().constData());
						fprintf(m_hReportFile, "</td>\n");
						fprintf(m_hReportFile, "<td width=\"10\"> </td>\n");
						fprintf(m_hReportFile, "</tr>\n");

						// Close 2nd row
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "</td></tr>\n");

						// CLOSE Exception row
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "<br><br>\n");
						fprintf(m_hReportFile, "</td></tr>\n");

						// Keep track of exceptions for which we already displayed the binning pareto
						mapExceptions[strExceptionKey] = 1;
						uiNbExceptions++;
#if 0
						// New table
						fprintf(m_hReportFile,"<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");

						strImage = strImage_Base;
						strImage += "_l" + QString::number(uiLayerIndex);
						strImage += "_d" + QString::number(uiDataIndex);
						strImage += ".png";
						strImageFullName = m_strChartDirectory + strImage;

						// ROW1 = Dataset details + Bin count table
						fprintf(m_hReportFile,"<tr>\n");

						// 1. Dataset details: Graph splits values, Layer split values, X-Axis
						fprintf(m_hReportFile, "<td width=\"33%%\"><align=\"left\"><valign=\"top\">\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Dataset:</b></font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"> </font></td>\n");
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						it1=m_clER_PartsData.m_strlFields_GraphSplit.begin();
						it2=pGraph->m_strlGraphSplitValues.begin();
						while(it1!=m_clER_PartsData.m_strlFields_GraphSplit.end() && it2!=pGraph->m_strlGraphSplitValues.end())
						{
							fprintf(m_hReportFile, "<tr>\n");
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it1).toLatin1().constData());
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it2).toLatin1().constData());
							fprintf(m_hReportFile, "</tr>\n");
							it1++;
							it2++;
						}
						it1=m_clER_PartsData.m_strlFields_LayerSplit.begin();
						it2=pLayer->m_strlLayerSplitValues.begin();
						while(it1!=m_clER_PartsData.m_strlFields_LayerSplit.end() && it2!=pLayer->m_strlLayerSplitValues.end())
						{
							fprintf(m_hReportFile, "<tr>\n");
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it1).toLatin1().constData());
							fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", (*it2).toLatin1().constData());
							fprintf(m_hReportFile, "</tr>\n");
							it1++;
							it2++;
						}
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", m_clER_PartsData.m_strField_Aggregate.toLatin1().constData());
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%s</font></td>\n", pLayer->m_strlAggregateLabels[uiDataIndex].toLatin1().constData());
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">Total parts</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\">%d</font></td>\n", uiNbTotalParts);
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "</td>\n");

						// 2. Bin counts
						fprintf(m_hReportFile, "<td width=\"67%%\"><align=\"left\"><valign=\"top\">\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"><b>Bin counts:</b></font></td>\n");
						fprintf(m_hReportFile, "<td width=\"50%%\"><font size=\"3\"> </font></td>\n");
						fprintf(m_hReportFile, "</tr>\n");
						fprintf(m_hReportFile, "</table>\n");
						fprintf(m_hReportFile, "<table width=\"100%%\" border=\"1\" cellspacing=\"0\" cellpadding=\"0\">\n");
						fprintf(m_hReportFile, "<tr>\n");
						fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin #</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Name</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">Bin Count</font></td>\n");
						fprintf(m_hReportFile, "<td width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"3\">%%</font></td>\n");
						fprintf(m_hReportFile, "</tr>\n");
						for(pBinInfo=clBinList.first(); pBinInfo; pBinInfo=clBinList.next())
						{
							lfData = ((double)pBinInfo->m_nBinCount/(double)uiNbTotalParts)*100.0;
							fprintf(m_hReportFile, "<tr>\n");
							fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinNo);
							if(pBinInfo->m_strBinName.isEmpty())
								fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">-</font></td>\n");
							else
								fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%s</font></td>\n", pBinInfo->m_strBinName.toLatin1().constData());
							fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%d</font></td>\n", pBinInfo->m_nBinCount);
							fprintf(m_hReportFile, "<td width=\"25%%\"><font size=\"3\">%5.2f</font></td>\n", lfData);
							fprintf(m_hReportFile, "</tr>\n");
						}
						fprintf(m_hReportFile, "</table><br>\n");
						fprintf(m_hReportFile, "</td>\n");

						fprintf(m_hReportFile, "</tr>\n");

						// Create image
						makechart_Advanced_BinningPareto(pLayer, strImageFullName, pEnterpriseReport, clBinList, uiDataIndex);

						// Write HTML code to insert created image
						fprintf(m_hReportFile,"<tr><align=\"left\"><img border=\"0\" src=\"../images/%s\"></img></tr>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

						// Keep track of exceptions for which we already displayed the binning pareto
						mapExceptions[strExceptionKey] = 1;
						uiNbExceptions++;

						// Close table
						fprintf(m_hReportFile,"</table>\n");
#endif
					}
					// Close table
					fprintf(m_hReportFile,"</table>\n");
				}

//				pSerieData = pLayer->next();
                indexSerieDef++;
			}
		}

		uiLayerIndex++;
//		pLayer = pGraph->next();
	}

	// If no exceptions, say so, else close the table
	if(uiNbExceptions == 0)
	{
		fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile, "<tr>\n");
		fprintf(m_hReportFile,"<td width=\"100%%\"><font size=\"3\"><b>No yield exceptions<b><td>\n");
		fprintf(m_hReportFile, "</tr>\n");
		fprintf(m_hReportFile,"</table>\n");
	}
//	else
//		fprintf(m_hReportFile,"</table>\n");
}

/******************************************************************************!
 * \fn makechart_Advanced_BinningPareto
 * \brief Create the bin pareto chart for yield exceptions
 ******************************************************************************/
void GexERProdAdvancedYield::makechart_Advanced_BinningPareto(
    GexDbPlugin_ER_Parts_Layer* pLayer,
    const QString& strChartFullName,
    const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport,
    GexDbPlugin_BinList& clBinList,
    unsigned int uiDataIndex)
{
	double							*plfData;
    char*						*pcLabels;
	unsigned int					uiLabelCount;
	GexDbPlugin_BinInfo				*pBinInfo;
	QString							strBinLabel, strTitle;
	QStringList						strlLabels;
	int								nOtherCount;
	unsigned int					uiMaxCategories = pEnterpriseReport->m_YieldWizard_BinPareto_nMaxCategories;
	unsigned int					uiTotalParts;

	// Set total parts to use, depending on binning type
	if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass"))
		uiTotalParts = pLayer->m_uilNbParts_Good[uiDataIndex];
	else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail"))
		uiTotalParts = pLayer->m_uilNbParts[uiDataIndex] - pLayer->m_uilNbParts_Good[uiDataIndex];
	else
		uiTotalParts = pLayer->m_uilNbParts[uiDataIndex];

	// Set title
	if(pEnterpriseReport->m_YieldWizard_BinPareto_strTitle.toLower() == "default")
	{
		strTitle = "Top " + QString::number(uiMaxCategories);

		if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass"))
			strTitle += " PASS";
		else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail"))
			strTitle += " FAIL";

		strTitle += " Bin Summary (";

		if(m_clER_PartsData.m_bSoftBin)
			strTitle += "SBIN)";
		else
			strTitle += "HBIN)";
	}
	else
		strTitle = pEnterpriseReport->m_YieldWizard_BinPareto_strTitle;

	// Sort BinList by descending BinCount
	clBinList.Sort(GexDbPlugin_BinList::eSortOnBinCount, false);

	// Allocate mem space for label and data pointers
	plfData = new double[100];
    pcLabels = new char*[100];
    // Set all pointers to NULL so we can later delete all non NULL pointers
    for(unsigned int lIndex=0; lIndex<100; ++lIndex)
        pcLabels[lIndex] = NULL;

	// Set labels: limit to N categories, Nth category is 'Other'
	uiLabelCount = 0;
	nOtherCount = 0;
//	for(pBinInfo=clBinList.first(); pBinInfo; pBinInfo=clBinList.next())
    foreach(pBinInfo, clBinList)
	{
		if((pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("all")) ||
			((pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass")) && (pBinInfo->m_cBinCat.toLower() == 'p')) ||
			((pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail")) && (pBinInfo->m_cBinCat.toLower() == 'f')))
		{
			if(uiLabelCount == uiMaxCategories)
			{
				strBinLabel = "Others";
				strlLabels.append(strBinLabel);
                pcLabels[uiLabelCount] = new char[strlLabels[uiLabelCount].size() + 1];
                strcpy(pcLabels[uiLabelCount], strlLabels[uiLabelCount].toLatin1().constData());
			}
			if(uiLabelCount >= uiMaxCategories)
				nOtherCount += pBinInfo->m_nBinCount;
			else
			{
				if(!pBinInfo->m_strBinName.isEmpty())
					strBinLabel = pBinInfo->m_strBinName;
				else
					strBinLabel = "B" + QString::number(pBinInfo->m_nBinNo);
				strlLabels.append(strBinLabel);
                pcLabels[uiLabelCount] = new char[strlLabels[uiLabelCount].size() + 1];
                strcpy(pcLabels[uiLabelCount], strlLabels[uiLabelCount].toLatin1().constData());
				plfData[uiLabelCount] = ((double)(pBinInfo->m_nBinCount)/(double)(uiTotalParts))*100.0;
			}
			uiLabelCount++;
		}
	}
	if(uiLabelCount > uiMaxCategories)
	{
		uiLabelCount = uiMaxCategories+1;
		plfData[uiMaxCategories] = ((double)(nOtherCount)/(double)(uiTotalParts))*100.0;
	}

	// Check chart type
	if(pEnterpriseReport->m_YieldWizard_BinPareto_strChartingMode.toLower() == "pie")
	{
		// Create a PieChart object of size 800 x 300 pixels
		PieChart *pChart = new PieChart(800, 300, Chart::BackgroundColor, 0, 0);

		// Set the center of the pie at (400, 150) and the radius to 80 pixels
		pChart->setPieSize(400, 150, 80);

		// Add a title box using 15 pts Times Bold Italic font and metallic pink
		// background color
		pChart->addTitle(strTitle.toLatin1().constData(), "timesbi.ttf", 12)->setBackground(Chart::metalColor(0xC0C0C0));

		// Draw the pie in 3D
		pChart->set3D(20);

		// Use the side label layout method
		pChart->setLabelLayout(Chart::SideLayout);
		// Set the start angle to 10 degrees may improve layout when there are many
		// small sectors at the end of the data array (that is, data sorted in descending
		// order). It is because this makes the small sectors position near the
		// vertical axis, so that the text labels have the most chances to be split between
		// the left and the right side of the pie.
		pChart->setStartAngle(10);

#if 0	// DOES NOT WORK
		// Swap first and second auto-colors in the palette (index 8 and 9), as first sector will be the one with the
		// highest count, generally Bin 1, and first auto-color is red, whereas second auto-color is green
		int nColor8 = pChart->getColor(8);
		int nColor9 = pChart->getColor(9);
		pChart->setColor(8, nColor9);
		pChart->setColor(9, nColor8);
#endif

		// Set the label box background color the same as the sector color, with glass
		// effect, and with 5 pixels rounded corners
		TextBox *t = pChart->setLabelStyle(0, 7, 0);
		t->setBackground(Chart::SameAsMainColor, Chart::Transparent, Chart::glassEffect());
		t->setRoundedCorners(5);

		// Set the border color of the sector the same color as the fill color. Set the
		// line color of the join line to black (0x0)
		pChart->setLineColor(Chart::SameAsMainColor, 0x000000);

		// Set the pie data and the pie labels
		pChart->setData(DoubleArray(plfData, uiLabelCount), StringArray(pcLabels, uiLabelCount));

		// output the chart
		pChart->makeChart(strChartFullName.toLatin1().constData());

		//free up resources
		delete pChart;
		pChart=0;
	}
	else
	{
		// Create a XYChart object of size 800 x 300 pixels.
		XYChart *pChart = new XYChart(800, 300, Chart::BackgroundColor, 0, 0);

		// Add a title
		pChart->addTitle(strTitle.toLatin1().constData(), "timesbi.ttf", 12)->setBackground(Chart::metalColor(0xC0C0C0));

		// Compute plot area
		QRect	clPlotRect;
		clPlotRect.setRect(100, 30, 580, 160);

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

		// Set the labels on the x axis.
		pChart->xAxis()->setLabelStyle(0, 7.5, 0, 30);
		pChart->xAxis()->setLabels(StringArray(pcLabels, uiLabelCount));

		// Add a title to the y axis
		if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("pass"))
			pChart->yAxis()->setTitle("% of PASS parts");
		else if(pEnterpriseReport->m_YieldWizard_BinPareto_strBinnings.toLower().startsWith("fail"))
			pChart->yAxis()->setTitle("% of FAIL parts");
		else
			pChart->yAxis()->setTitle("% of ALL Parts");
		pChart->yAxis()->setAutoScale(0.1, 0.1, 0);

		// Add a title to the x axis
		if(m_clER_PartsData.m_bSoftBin)
			pChart->xAxis()->setTitle("Soft Bins");
		else
			pChart->xAxis()->setTitle("Hard Bins");

		// Add data layer
		// Compute merged dataset for each split chart, and add it to the layer
		LineLayer	*pLineLayer=NULL;
		BarLayer	*pBarLayer=NULL;
		Layer		*pDataLayer;
		DataSet		*pDataSet;
		int			nDataColor = pEnterpriseReport->m_YieldWizard_BinPareto_cColor.rgb() & 0xffffff;
		bool		bBarGraph=false;

		if(pEnterpriseReport->m_YieldWizard_BinPareto_strChartingMode.toLower() == "bar")
		{
			bBarGraph = true;

			// Add bar layer
			pBarLayer = pChart->addBarLayer(Chart::Side);

			// Configure the bars within a group to touch each others (no gap)
			pBarLayer->setBarGap(0.2);

			// Draw the ticks between label positions (instead of at label positions)
			pChart->xAxis()->setTickOffset(0.5);

			// Set layer to 3D depth
			if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().startsWith("3d"))
				pBarLayer->set3D(10);

			// Use gradient colors in bars?
			if(pEnterpriseReport->m_YieldWizard_Global_bGradientBarColor)
				pBarLayer->setBorderColor(0, Chart::softLighting(Chart::Right));

			// Set bar shape
            if(pEnterpriseReport->m_YieldWizard_Global_strBarStyle.toLower().indexOf("cylindrical") != -1)
				pBarLayer->setBarShape(Chart::CircleShape);

			// Set data layer pointer
			pDataLayer = pBarLayer;
		}
		else
		{
			// Add line layer
			if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineStyle.toLower().startsWith("spline line"))
				pLineLayer = pChart->addSplineLayer();
			else if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineStyle.toLower().startsWith("step line"))
				pLineLayer = pChart->addStepLineLayer();
			else
				pLineLayer = pChart->addLineLayer();

			// Set data layer pointer
			pDataLayer = pLineLayer;
		}

		// Enable data labels on data
		if(pEnterpriseReport->m_YieldWizard_BinPareto_strDataLabels.toLower() != "no label")
		{
			pDataLayer->setDataLabelStyle(0, 8, Chart::TextColor, 0);
			pDataLayer->setDataLabelFormat("{value|2}");
		}

		if(bBarGraph)
		{
			// Use transparency?
			if(pEnterpriseReport->m_YieldWizard_Global_bSemiTransparentBar)
				nDataColor = nDataColor | 0x80000000;
			pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, uiLabelCount), nDataColor, "");
		}
		else
		{
			// Dashed line?
			if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineProperty.toLower() == "solidline")
				pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, uiLabelCount), nDataColor, "");
			else
				pDataSet = pDataLayer->addDataSet(DoubleArray(plfData, uiLabelCount), pChart->dashLineColor(nDataColor), "");

			// Use spots?
			if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots.toLower() == "square")
				pDataSet->setDataSymbol(Chart::SquareSymbol, 7);
			else if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots.toLower() == "circle")
				pDataSet->setDataSymbol(Chart::CircleSymbol, 7);
			else if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots.toLower() == "diamonds")
				pDataSet->setDataSymbol(Chart::DiamondSymbol, 7);
			else if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots.toLower() == "triangle")
				pDataSet->setDataSymbol(Chart::TriangleSymbol, 7);
			else if(pEnterpriseReport->m_YieldWizard_BinPareto_strLineSpots.toLower() == "cross")
				pDataSet->setDataSymbol(Chart::CrossSymbol, 7);
		}

		// output the chart
		pChart->makeChart(strChartFullName.toLatin1().constData());

		//free up resources
		delete pChart;
		pChart=0;
	}

	delete [] plfData;
    for (int lLabelIndx = 0; lLabelIndx < 100; ++lLabelIndx)
        if (pcLabels[lLabelIndx])
        delete pcLabels[lLabelIndx];
	delete [] pcLabels;
}
