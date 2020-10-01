///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "gex_er_prod_uph.h"
#include "browser.h"
#include "report_options.h"
#include "gex_report.h"
#include "gex_database_entry.h"
#include "engine.h"
#include "report_template.h"
#include "xychart_data.h"

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern	CGexReport*		gexReport;
extern	CReportOptions		ReportOptions;

extern	QString				formatHtmlImageFilename(const QString& strImageFileName);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexERProdUPH
//
// Description	:	Class which creates Production UPH enteprise report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexERProdUPH::GexERProdUPH(
    FILE* hReportFile,
    GexDatabaseEntry* pDatabaseEntry,
    GexDbPlugin_Filter& cFilters,
    GS::Gex::ReportTemplateSection* pSection,
    GexDbPlugin_XYChartList& clXYChartList)
  : GexAbstractEnterpriseReport(hReportFile,
                                pDatabaseEntry,
                                cFilters,
                                pSection),
    m_clXYChartList(clXYChartList)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexERProdUPH::~GexERProdUPH()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexERProdUPH::computeData()
//
// Description	:	Compute data for the report
//
///////////////////////////////////////////////////////////////////////////////////
bool GexERProdUPH::computeData()
{
	// Compute UPH data for specified filters
	if (!m_pDatabaseEntry->m_pExternalDatabase->GetDataForProd_UPH(m_cFilters, m_clXYChartList))
	{
		QString strErrorMsg;
		m_pDatabaseEntry->m_pExternalDatabase->GetLastError(strErrorMsg);
		strErrorMsg.replace("\n", "<br>");
		fprintf(m_hReportFile,"<b>*WARNING*</b>Failed creating UPH chart:<br><br>");
		fprintf(m_hReportFile,"Database=%s<br>", m_pSection->pEnterpriseReport->m_strDatabaseLogicalName.toLatin1().constData());
		fprintf(m_hReportFile,"Error=%s<br>", strErrorMsg.toLatin1().constData());

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdUPH::writeRawData()
//
// Description	:	Create the raw data file
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdUPH::writeRawData()
{
	GexDbPlugin_XYChart_Data *	pXYChartData;
	int							nIndex;

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
		stream << "Report type,UPH" << endl;
		stream << "Report title," << m_pSection->pEnterpriseReport->m_strSectionTitle << endl;
		stream << endl;
		stream << "# SQL Query" << endl;
		stream << "\"" << m_clXYChartList.m_strQuery << "\"" << endl;
		stream << endl;

		// Write raw data?
		if(m_pSection->pEnterpriseReport->m_bDumpRawData)
		{
			stream << "# Raw Data" << endl;
			// First line = column names
			if(!m_pSection->pEnterpriseReport->m_strSplitField.isEmpty())
				stream << m_pSection->pEnterpriseReport->m_strSplitField << ",";

			stream << m_pSection->pEnterpriseReport->m_strAggregateField << ",Total Parts,Total Time (s),UPH" << endl;

			// Data
//			for(pXYChartData = m_clXYChartList.first(); pXYChartData; pXYChartData = m_clXYChartList.next())
            foreach(pXYChartData, m_clXYChartList)
			{
				for(nIndex = 0; nIndex < pXYChartData->m_nNbDataPoints; nIndex++)
				{
					if(m_pSection->pEnterpriseReport->m_strSplitField.isEmpty())
						stream << pXYChartData->m_pcXAxisLabels[nIndex];
					else
						stream << pXYChartData->m_strSplitValue << "," << pXYChartData->m_pcXAxisLabels[nIndex];

					stream << "," << QString::number((int)(pXYChartData->m_plfData_1[nIndex]));
					stream << "," << QString::number((int)(pXYChartData->m_plfData_2[nIndex]));
					stream << "," << QString::number((int)(pXYChartData->m_plfData[nIndex]));
					stream << endl;
				}
			}
		}
		queryFile.close();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdUPH::writeHtmlData(QString strSplitValue)
//
// Description	:	Write the raw data table
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdUPH::writeHtmlData(QString strSplitValue)
{
	GexDbPlugin_XYChart_Data	*pXYChartData;
	int							nIndex;
	QString						strReference;

	// Add a page break
	gexReport->WritePageBreak();

	// If creating a Powerpoint presentation, save Section title.
	if (strSplitValue.isEmpty())
		gexReport->SetPowerPointSlideName("UPH raw data");
	else
		gexReport->SetPowerPointSlideName("UPH raw data : " + strSplitValue);

	QString of=ReportOptions.GetOption("output", "format").toString();

	// Links to export data
	//if(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
	if (
			(of=="HTML") || (of=="INTERACTIVE")
		)
	{
		fprintf(m_hReportFile, "<table border=\"1\" width=\"800\" cellspacing=\"0\" cellpadding=\"0\">\n");
		fprintf(m_hReportFile,"<tr>\n");

		if(strSplitValue.isEmpty())
		{
			strReference.sprintf("%s%s--mission=prod--report=uph--action=saveexcelfile", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());

			strReference.sprintf("%s%s--mission=prod--report=uph--action=saveexcelclipboard", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE);
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}
		else
		{
			strReference.sprintf("%s%s--mission=prod--report=uph--action=saveexcelfile--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, strSplitValue.toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"40%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/save_icon.png\"><b>Save table to spreadsheet CSV file</b></a></font></td>\n", strReference.toLatin1().constData());

			strReference.sprintf("%s%s--mission=prod--report=uph--action=saveexcelclipboard--split=%s", GEX_BROWSER_ACTIONBOOKMARK, GEX_BROWSER_ACT_ENTERPRISE, strSplitValue.toLatin1().constData());
            fprintf(m_hReportFile,"<td width=\"60%%\"><align=\"left\"><font size=\"4\"><a href=\"%s\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\"><b>Save table to clipboard in spreadsheet format, ready to be paste</b></a></font></td>\n", strReference.toLatin1().constData());
		}

		fprintf(m_hReportFile,"</tr>\n");
		fprintf(m_hReportFile, "</table>\n");
		fprintf(m_hReportFile, "<br><br><br>\n");
	}

	GexERTableHtml erTableHtml(m_hReportFile);
	QString strLine;

	erTableHtml.open("<table border=\"1\" width=\"800\" cellspacing=\"1\" cellpadding=\"1\">\n");

	if(strSplitValue.isEmpty())
	{
		// First line = column names
		strLine = "<tr>\n";

		if(!m_clXYChartList.m_strSplitField.isEmpty())
			strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", m_clXYChartList.m_strSplitField.toLatin1().constData());

		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", m_clXYChartList.m_strCumulField.toLatin1().constData());
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Parts</font></td>\n");
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Time (s)</font></td>\n");
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">UPH</font></td>\n");
		strLine += "</tr>\n";

		// set the html table header
		erTableHtml.setHeader(strLine);

		// Data
//		for(pXYChartData = m_clXYChartList.first(); pXYChartData; pXYChartData = m_clXYChartList.next())
        foreach(pXYChartData, m_clXYChartList)
		{
			for(nIndex = 0; nIndex < pXYChartData->m_nNbDataPoints; nIndex++)
			{
				strLine = "<tr>\n";

				if(!m_clXYChartList.m_strSplitField.isEmpty())
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%s</font></td>\n", pXYChartData->m_strSplitValue.toLatin1().constData());

				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%s</font></td>\n", pXYChartData->m_pcXAxisLabels[nIndex]);
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData_1[nIndex]));
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData_2[nIndex]));
				strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData[nIndex]));
				strLine += QString().sprintf("</tr>\n");

				// add new line
				erTableHtml.addLine(strLine);
			}
		}
	}
	else
	{
		// First line = column names
		strLine =  "<tr>\n";
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">%s</font></td>\n", m_clXYChartList.m_strCumulField.toLatin1().constData());
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Parts</font></td>\n");
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">Total Time (s)</font></td>\n");
		strLine += QString().sprintf("<td bgcolor=\"#CCFFFF\"><font size=\"3\">UPH</font></td>\n");
		strLine += "</tr>\n";

		erTableHtml.setHeader(strLine);

		// Data
//		for(pXYChartData = m_clXYChartList.first(); pXYChartData; pXYChartData = m_clXYChartList.next())
        foreach(pXYChartData, m_clXYChartList)
		{
			if(pXYChartData->m_strSplitValue == strSplitValue)
			{
				for(nIndex = 0; nIndex < pXYChartData->m_nNbDataPoints; nIndex++)
				{
					strLine = "<tr>\n";
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%s</font></td>\n", pXYChartData->m_pcXAxisLabels[nIndex]);
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData_1[nIndex]));
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData_2[nIndex]));
					strLine += QString().sprintf("<td bgcolor=\"#F8F8F8\"><font size=\"3\">%d</font></td>\n", (int)(pXYChartData->m_plfData[nIndex]));
					strLine + "</tr>\n";

					erTableHtml.addLine(strLine);
				}
				break;
			}
		}
	}

	// close  html table
	erTableHtml.close();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexERProdUPH::writeReport()
//
// Description	:	Write the graphs and tables
//
///////////////////////////////////////////////////////////////////////////////////
void GexERProdUPH::writeReport()
{
	GexDbPlugin_XYChart_Data *	pXYChartData;
	int							nIndex;

	// Create image(s)
	QString	strImage, strImageFullName;
	if((m_pSection->pEnterpriseReport->m_strLayerMode.toLower() == "multi-layers") && (m_clXYChartList.count() > 1))
	{
		// Add a page break
		gexReport->WritePageBreak();

		// One chart with all split layers
		// Image name
		strImage = m_strChartBaseFileName;
		strImage += ".png";
		strImageFullName = m_strChartDirectory + strImage;;

		// Create graph
		if(m_pSection->pEnterpriseReport->m_strChartingMode.toLower() == "bars")
			makeXYChart(m_clXYChartList, strImageFullName, "UPH (Units per hour)", 0, true);
		else
			makeXYChart(m_clXYChartList, strImageFullName, "UPH (Units per hour)", 0, false);

		// Write HTML code to insert all created images
		fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\"></img><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

		// If creating a Powerpoint presentation, save Section title.
		gexReport->SetPowerPointSlideName("UPH chart");

		// Write HTML data
		writeHtmlData("");
	}
	else
	{
		// As many charts as split values (or no split)
		nIndex = 0;
//		for(pXYChartData = m_clXYChartList.first(); pXYChartData; pXYChartData = m_clXYChartList.next())
        foreach(pXYChartData, m_clXYChartList)
		{
			// Add a page break
			gexReport->WritePageBreak();

			// Image name
			strImage = m_strChartBaseFileName + "_";
			strImage += QString::number(nIndex++);
			strImage += ".png";
			strImageFullName = m_strChartDirectory + strImage;;

			// Set data pointers
			if(m_pSection->pEnterpriseReport->m_strLeftAxis.toLower() == "total - uph")
			{
				pXYChartData->m_plfYAxisData	= pXYChartData->m_plfData;
				pXYChartData->m_strYAxisLegend	= "UPH (Units per hour)";

				if(m_pSection->pEnterpriseReport->m_strRightAxis.toLower() == "total - volume")
				{
					pXYChartData->m_plfYAxisData_2		= pXYChartData->m_plfData_1;
					pXYChartData->m_strYAxisLegend_2	= "Volume";
					pXYChartData->m_bDoubleYAxis		= true;
				}
			}
			else
			{
				pXYChartData->m_plfYAxisData	= pXYChartData->m_plfData_1;
				pXYChartData->m_strYAxisLegend	= "Volume";
			}

			// Create graph
			if(m_pSection->pEnterpriseReport->m_strChartingMode.toLower() == "bars")
				makeXYChart(pXYChartData, strImageFullName, true);
			else
				makeXYChart(pXYChartData, strImageFullName, false);

			// Write HTML code to insert all created images
			fprintf(m_hReportFile,"<img border=\"0\" src=\"../images/%s\"></img><br>\n", formatHtmlImageFilename(strImage).toLatin1().constData());

			// If creating a Powerpoint presentation, save Section title.
			gexReport->SetPowerPointSlideName("UPH chart");

			// Write HTML data
			writeHtmlData(pXYChartData->m_strSplitValue);
		}
	}
}
