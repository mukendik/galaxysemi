///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_er_genealogy_yield_vs_yield.h"
#include "browser.h"
#include "report_options.h"
#include "gex_report.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "report_template.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern	CGexReport *		gexReport;
extern	CReportOptions		ReportOptions;

extern QString				formatHtmlImageFilename(const QString& strImageFileName);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERGenealogyYieldvsYield
//
// Description	:	Class which creates Genealogy Yield vs Yield enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexERGenealogyYieldvsYield::GexERGenealogyYieldvsYield(
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
GexERGenealogyYieldvsYield::~GexERGenealogyYieldvsYield()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexERGenealogyYieldvsYield::computeData()
//
// Description	:	Compute data for the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexERGenealogyYieldvsYield::computeData()
{
	// Compute Genealogy data for specified filters
	if(!m_pDatabaseEntry->m_pExternalDatabase->ER_Genealogy_YieldVsYield_GetParts(m_cFilters, m_clER_PartsData))
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
// Name			:	void GexERGenealogyYieldvsYield::writeRawData()
//
// Description	:	Create the raw data file
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsYield::writeRawData()
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
		stream << "Report type,Genealogy (Yield vs. Yield)" << endl;
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

//			pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//			while(pSerieDef)
            foreach(pSerieDef, m_clER_PartsData.m_plistSerieDefs)
			{
				stream << "," << pSerieDef->m_strSerieName;
				stream << " (Parts)";
				stream << "," << pSerieDef->m_strSerieName;
				stream << " (Matching)";

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
					for(uiDataPoint=0; uiDataPoint<pLayer->m_uiDataPoints; uiDataPoint++)
					{
						for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
							stream << (*it) << ",";
						for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
							stream << (*it) << ",";
						stream << pLayer->m_strlAggregateLabels[uiDataPoint];
						for(uiSerie=0; uiSerie<pLayer->m_uiSeries; uiSerie++)
						{
							pSerieData = pLayer->at(uiSerie);
							stream << "," << (*pSerieData)[uiDataPoint].m_uiTotalParts;
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
// Name			:	void GexERGenealogyYieldvsYield::writeHtmlHeader()
//
// Description	:	Write the html header
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsYield::writeHtmlHeader()
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
		fprintf(m_hReportFile, "<td width=\"20%%\" bgcolor=\"#F8F8F8\">Data type</td>\n");
		fprintf(m_hReportFile, "<td width=\"80%%\" bgcolor=\"#F8F8F8\">: %s</td>\n", pSerieDef->m_strTestingStage.toLatin1().constData());
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
// Name			:	void GexERGenealogyYieldvsParameter::writeReport()
//
// Description	:	Write the graphs and tables
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsYield::writeReport()
{
	// Create graphs
	GexDbPlugin_ER_Parts_Graph *	pGraph;
	QString							strImage_Base, strImage, strImageFullName;
	QString							strSplitValues;
	QStringList::iterator			it1, it2;

	m_clER_PartsData.m_uiNbAxis=1;

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
			fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\"></img><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

			// If creating a Powerpoint presentation, save Section title.
			if (pGraph->m_strlGraphSplitValues.isEmpty())
				gexReport->SetPowerPointSlideName("Genealogy yield vs yield chart");
			else
				gexReport->SetPowerPointSlideName("Genealogy yield vs yield chart : " + pGraph->m_strlGraphSplitValues.join("_"));
		}
		else
		{
			fprintf(m_hReportFile,"Chart disabled<br>\n");
		}

		fprintf(m_hReportFile,"<br><br>\n");

		// Write HTML data
		writeHtmlData(pGraph);

		fprintf(m_hReportFile,"<br><br><br>\n");

//		pGraph = m_clER_PartsData.next();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERGenealogyYieldvsYield::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
//
// Description	:	Write the raw data table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERGenealogyYieldvsYield::writeHtmlData(GexDbPlugin_ER_Parts_Graph *pGraph)
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
		gexReport->SetPowerPointSlideName("Genealogy yield vs yield raw data");
	else
		gexReport->SetPowerPointSlideName("Genealogy yield vs yield raw data : " + pGraph->m_strlGraphSplitValues.join("_"));

	// Links to export data
	//if(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
	QString of=ReportOptions.GetOption("output", "format").toString();
	if ( (of=="INTERACTIVE")||(of=="HTML") )
	{
		fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile,"<tr>\n");

		if(pGraph->m_strlGraphSplitValues.isEmpty())
		{
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_yield--action=saveexcelfile", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_yield--action=saveexcelclipboard", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}
		else
		{
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_yield--action=saveexcelfile--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());
			strReference.sprintf("%s%s--mission=genealogy--report=yield_vs_yield--action=saveexcelclipboard--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, pGraph->m_strlGraphSplitValues.join("_").toLatin1().constData());
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
	fprintf(m_hReportFile, "<tr>\n");
	for(it = m_clER_PartsData.m_strlFields_GraphSplit.begin(); it != m_clER_PartsData.m_strlFields_GraphSplit.end(); it++)
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", (*it).toLatin1().constData());

	for(it = m_clER_PartsData.m_strlFields_LayerSplit.begin(); it != m_clER_PartsData.m_strlFields_LayerSplit.end(); it++)
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", (*it).toLatin1().constData());
	if(m_clER_PartsData.m_strField_Aggregate.toLower() == "wafer")
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">TrackingLotID/WaferID</font></td>\n");
	else
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">TrackingLotID</font></td>\n");

//    pSerieDef = m_clER_PartsData.m_plistSerieDefs.first();
//	while(pSerieDef)
    foreach(pSerieDef, m_clER_PartsData.m_plistSerieDefs)
	{
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (parts)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (matching)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
		fprintf(m_hReportFile, "<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s (%%)</font></td>\n", pSerieDef->m_strSerieName.toLatin1().constData());
//		pSerieDef = m_clER_PartsData.m_plistSerieDefs.next();
	}
	fprintf(m_hReportFile, "</tr>\n");

	// Data
	unsigned int	uiDataIndex, uiNbSerieParts, uiNbSerieMatching;
	double			lfData;
	bool			bException;
	QString			strFontSpec;
//	pLayer = pGraph->first();
//	while(pLayer)
    foreach(pLayer, *pGraph)
    {
		for(uiDataIndex=0; uiDataIndex<pLayer->m_uiDataPoints; uiDataIndex++)
		{
			// First check if there is a yield exception
			bException	= false;
//			pSerieData	= pLayer->first();
//			pSerieDef	= m_clER_PartsData.m_plistSerieDefs.first();
            int indexSerieDef = 0;
//			while(pSerieData)
            foreach(pSerieData, *pLayer)
			{
                pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];
                if (!pSerieData || !pSerieDef)
                    break;
				uiNbSerieParts = (*pSerieData)[uiDataIndex].m_uiTotalParts;
				uiNbSerieMatching = (*pSerieData)[uiDataIndex].m_uiMatchingParts;
				lfData = ((double)uiNbSerieMatching/(double)uiNbSerieParts)*100.0;

//				pSerieData	= pLayer->next();
                indexSerieDef++;
			}

			if(bException)
				strFontSpec = "<font size=\"3\" color=\"#FF3333\">";
			else
				strFontSpec = "<font size=\"3\">";

			fprintf(m_hReportFile, "<tr>\n");
			for(it=pGraph->m_strlGraphSplitValues.begin(); it!=pGraph->m_strlGraphSplitValues.end(); it++)
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());
			for(it=pLayer->m_strlLayerSplitValues.begin(); it!=pLayer->m_strlLayerSplitValues.end(); it++)
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (*it).toLatin1().constData());
			fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%s</font></td>\n", strFontSpec.toLatin1().constData(), (pLayer->m_strlAggregateLabels[uiDataIndex]).toLatin1().constData());

//			pSerieData	= pLayer->first();
//			pSerieDef	= m_clER_PartsData.m_plistSerieDefs.first();
//			while(pSerieData)
            indexSerieDef = 0;
            foreach(pSerieData, *pLayer)
			{
                pSerieDef = m_clER_PartsData.m_plistSerieDefs[indexSerieDef];
                if (!pSerieData || !pSerieDef)
                    break;
				uiNbSerieParts		= (*pSerieData)[uiDataIndex].m_uiTotalParts;
				uiNbSerieMatching	= (*pSerieData)[uiDataIndex].m_uiMatchingParts;
				lfData				= ((double)uiNbSerieMatching/(double)uiNbSerieParts)*100.0;

				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbSerieParts);
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%d</font></td>\n", strFontSpec.toLatin1().constData(), uiNbSerieMatching);
				fprintf(m_hReportFile, "<td bgcolor=\"#F8F8F8\">%s%5.2f</font></td>\n", strFontSpec.toLatin1().constData(), lfData);

//				pSerieData = pLayer->next();
                indexSerieDef++;
			}
			fprintf(m_hReportFile, "</tr>\n");
		}

//		pLayer = pGraph->next();
	}

	fprintf(m_hReportFile, "</table><br>\n");
}

/******************************************************************************!
 * \fn makeChart
 * \brief Create the chart image
 ******************************************************************************/
void GexERGenealogyYieldvsYield::makeChart(
    GexDbPlugin_ER_Parts_Graph* pGraph,
    const QString& strChartFullName,
    const GS::Gex::CustomReportEnterpriseReportSection* pEnterpriseReport)
{
	double *							plfData_Yield_X = NULL;
	double *							plfData_Yield_Y = NULL;
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
	strString = pSerieDef_X->m_strTestingStage;
	strString += "- (Binnings: ";
	strString += pSerieDef_X->m_strBinnings;
	strString += ")";
	pChart->xAxis()->setTitle(strString.toLatin1().constData());
	pChart->xAxis()->setAutoScale(0.1, 0.1, 0);

	// Add a title to the Y axis
	pSerieDef_Y = m_clER_PartsData.m_plistSerieDefs.at(1);
	strString = pSerieDef_Y->m_strTestingStage;
	strString += "- (Binnings: ";
	strString += pSerieDef_Y->m_strBinnings;
	strString += ")";
	pChart->yAxis()->setTitle(strString.toLatin1().constData());
	pChart->yAxis()->setAutoScale(0.1, 0.1, 0);

	// Create scatter graphs for each layer
	int				nDataColor = pEnterpriseReport->m_cDefaultColor.rgb() & 0xffffff;
	int				nSymbol = Chart::SquareSymbol;
    QString			strName;

//	pLayer = pGraph->first();
//	while(pLayer)
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
			plfData_Yield_X[uiIndex] = ((double)((*pSerieData_X)[uiIndex].m_uiMatchingParts)/(double)((*pSerieData_X)[uiIndex].m_uiTotalParts))*100.0;
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

//		pLayer = pGraph->next();
//		uiLayerIndex++;
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
