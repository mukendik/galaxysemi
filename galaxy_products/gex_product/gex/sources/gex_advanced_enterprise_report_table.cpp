///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_advanced_enterprise_report_table.h"
#include "report_options.h"
#include "report_build.h"
#include "browser.h"
#include "gex_report.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////
extern CReportOptions	ReportOptions;
extern CGexReport*		gexReport;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTable
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTable::GexAdvancedEnterpriseReportTable() : GexAdvancedEnterpriseReportSection(), m_nSequenceCount(1), m_nBorder(0), m_nColsCount(0)
{
    QString pf=ReportOptions.GetOption("output", "paper_format").toString();
    QString of=ReportOptions.GetOption("output", "format").toString();
    if (pf=="portrait") //(ReportOptions.m_OutputOptionsMap["paper_format"]=="portrait") //if (ReportOptions.bPortraitFormat)
    {
        if(of=="DOC" || of=="ODT")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_WORD)
            m_nMaxLine = 32;
        else if	(of=="PDF")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_PDF)
            m_nMaxLine = 50;
        else if	(of=="PPT")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_PPT)
            m_nMaxLine = 40;
    }
    else
    {
        if (of=="DOC" || of=="ODT")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_WORD)
            m_nMaxLine = 26;
        else if	(of=="PDF")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_PDF)
            m_nMaxLine = 34;
        else if	(of=="PPT")	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_PPT)
            m_nMaxLine = 40;
    }
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTable::~GexAdvancedEnterpriseReportTable()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::addAlarm(const QString& strName, const QString& strJSCondition, const QString& strBackColor)
//
// Description	:	Add an alarm to the table
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTable::addAlarm(const QString& strName, const QString& strJSCondition, const QString& strBackColor)
{
    if (m_mapAlarms.contains(strName) == false)
    {
        m_mapAlarms.insert(strName, GexAdvancedEnterpriseReportTableAlarm(strJSCondition, strBackColor));
    }
    else
        GSLOG(SYSLOG_SEV_NOTICE, QString("AER Engine Parser: Alarms %1 already inserted... ").arg(strName).toLatin1().data() );
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableAlarm GexAdvancedEnterpriseReportTable::findAlarm(const QString& strName) const
//
// Description	:	return the alarm corresponding to the alarm name given
//
///////////////////////////////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableAlarm GexAdvancedEnterpriseReportTable::findAlarm(const QString& strName) const
{
    if (m_mapAlarms.contains(strName))
        return m_mapAlarms.value(strName);
    else
        return GexAdvancedEnterpriseReportTableAlarm();
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::exportToHtml(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
//
// Description	:	Export table to an html output stream
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTable::exportToHtml(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
{
    QString of=ReportOptions.GetOption("output", "format").toString();
    // Add a save button for Enterprise Report
    if(of=="INTERACTIVE" || (of=="HTML"))	//(ReportOptions.iOutputFormat & (GEX_OPTION_OUTPUT_INTERACTIVEONLY | GEX_OPTION_OUTPUT_HTML))
    {
        QString strSaveCsvFile		= QString("%1%2--action=savecsvfile--section=%3--dataset=%4").arg(GEX_BROWSER_ACTIONBOOKMARK).arg(GEX_BROWSER_ACT_ADV_ENTERPRISE).arg(sectionID()).arg(datasetGroup.groupId());
        QString strSaveClipboard	= QString("%1%2--action=saveexcelclipboard--section=%3--dataset=%4").arg(GEX_BROWSER_ACTIONBOOKMARK).arg(GEX_BROWSER_ACT_ADV_ENTERPRISE).arg(sectionID()).arg(datasetGroup.groupId());

        txtStream << "<table border=\"0\" cellspacing=\"30\" width=\"" << HTML_TABLE_WIDTH << "\" cellpadding=\"0\">" << endl;
        txtStream << "<tr>" << endl;
        txtStream << "<td width=\"50%%\" align=center>";
        txtStream << "<p><a href=\"" << strSaveCsvFile << "\"><img src=\"../images/save_icon.png\" border=\"0\" width=\"22\" height=\"19\"></a> :";
        txtStream << "<a href=\"" << strSaveCsvFile << "\"><b>Save table to Spreadsheet CSV file</b></a></p>";
        txtStream << "</td>" << endl;
        txtStream << "<td width=\"50%%\" align=center height=\"19\">" << endl;
        txtStream << "<a href=\"" << strSaveClipboard << "\"><img border=\"0\" src=\"../images/csv_spreadsheet_icon.png\" border=\"0\"></a>";
        txtStream << "<a href=\"" << strSaveClipboard << "\"><b>Save table to clipboard in Spreadsheet format, ready to be paste</b></a>" << endl ;
        txtStream << "</td>" << endl;
        txtStream << "</tr>" << endl;
        txtStream << "</table>" << endl;
    }

    exportToOutput(txtStream, datasetGroup, GexReportTable::HtmlOutput);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::exportToCsv(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
//
// Description	:	Export table to a csv output stream
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTable::exportToCsv(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
{
    exportToOutput(txtStream, datasetGroup, GexReportTable::CsvOutput);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::exportToSpreadsheet(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
//
// Description	:	Export table to a spreadsheet output stream
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTable::exportToSpreadsheet(QTextStream &txtStream, const GexDbPluginERDatasetGroup &datasetGroup) const
{
    exportToOutput(txtStream, datasetGroup, GexReportTable::SpreadSheetOutput);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportCellAlarmDesc
//
// Description	:	Class which represents a alarm condition in a cell
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableAlarm::GexAdvancedEnterpriseReportTableAlarm()
    : m_strJSCondition(""), m_strBackColor("")
{

}
GexAdvancedEnterpriseReportTableAlarm::GexAdvancedEnterpriseReportTableAlarm(const QString& strJSCondition, const QString& strBackColor)
    : m_strJSCondition(strJSCondition), m_strBackColor(strBackColor)
{

}

GexAdvancedEnterpriseReportTableAlarm::GexAdvancedEnterpriseReportTableAlarm(const GexAdvancedEnterpriseReportTableAlarm& other)
{
    m_strJSCondition	= other.m_strJSCondition;
    m_strBackColor		= other.m_strBackColor;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportCellDesc
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportCellDesc::GexAdvancedEnterpriseReportCellDesc(const QString &strValue, const QString& strAlign) : m_strValue(strValue), m_strAlignment(strAlign)
{
    if (m_strAlignment.isEmpty())
        m_strAlignment = "center";

    m_bRepeatedValue = true;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportCellDesc
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////
QString GexAdvancedEnterpriseReportCellDesc::previousValue(const QString &strValue) const
{
    QString strValueDisplayed = strValue;

    if (m_bRepeatedValue == false )
    {
        if (m_strPreviousValue == strValueDisplayed)
            strValueDisplayed.clear();
        else
            m_strPreviousValue = strValue;
    }

    return strValueDisplayed;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableLineDesc
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableLineDesc::GexAdvancedEnterpriseReportTableLineDesc()
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableLineDesc::~GexAdvancedEnterpriseReportTableLineDesc()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTableLine::addCell(const QString &strValue, const QString &strAlign, const QString & strRepeatedValue)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTableLineDesc::addCell(const QString &strValue, const QString &strAlign, const QString& strRepeatedValue)
{
    GexAdvancedEnterpriseReportCellDesc lineCell(strValue, strAlign);

    if (strRepeatedValue == "false")
        lineCell.setRepeatedValue(false);

    addCell(lineCell);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTableLineDesc::addCell(const GexAdvancedEnterpriseReportCellDesc& cellDescriptor)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTableLineDesc::addCell(const GexAdvancedEnterpriseReportCellDesc& cellDescriptor)
{
    m_lstCell.append(cellDescriptor);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableRowDesc
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableRowDesc::GexAdvancedEnterpriseReportTableRowDesc() : m_nSequence(0), m_bPrintFirstOnly(false)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableRowDesc::~GexAdvancedEnterpriseReportTableRowDesc()
{

}


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableVertical
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableVertical::GexAdvancedEnterpriseReportTableVertical() : GexAdvancedEnterpriseReportTable()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableVertical::~GexAdvancedEnterpriseReportTableVertical()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::writeHeader(GexReportTable& reportTable)
//
// Description	:	Write header
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTable::writeHeader(GexReportTable& reportTable) const
{
    reportTable.clear();

    for (int nHeaderLine = 0; nHeaderLine < m_rowHeader.lines().count(); ++nHeaderLine)
    {
        GexReportTableLine reportTableLine;

        for(int nColumn = 0; nColumn < m_rowHeader.lines().at(nHeaderLine).cells().count(); ++nColumn)
        {
            GexReportTableCell reportTableCell;
            GexReportTableText reportTableText;

            // set cell option
            reportTableCell.setAlign(m_rowHeader.lines().at(nHeaderLine).cells().at(nColumn).alignment());
            reportTableCell.setColor("#CCFFFF");

            // set text and font
            reportTableText.setSize(3);
            reportTableText.setText(m_aerScriptEngine.scriptEngine()->evaluate(m_rowHeader.lines().at(nHeaderLine).cells().at(nColumn).value()).toString());

            reportTableCell.addData(reportTableText);
            reportTableLine.addCell(reportTableCell);
        }

        reportTable.addLine(reportTableLine);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTable::writeFooter(GexReportTable& reportTable, const GexDbPluginERDatasetGroup& datasetGroup)
//
// Description	:	Write footer
//
///////////////////////////////////////////////////////////////////////////////////
void
GexAdvancedEnterpriseReportTable::
writeFooter(GexReportTable& reportTable,
            const GexDbPluginERDatasetGroup& /*datasetGroup*/) const
{
    for (int nFooterLine = 0; nFooterLine < m_rowFooter.lines().count(); ++nFooterLine)
    {
        GexReportTableLine reportTableLine;

        for(int nColumn = 0; nColumn < m_rowFooter.lines().at(nFooterLine).cells().count(); ++nColumn)
        {
            GexReportTableCell reportTableCell;
            GexReportTableText reportTableText;

            // set cell option
            reportTableCell.setAlign(m_rowFooter.lines().at(nFooterLine).cells().at(nColumn).alignment());
            reportTableCell.setColor("#CCFFFF");

            // set text and font
            reportTableText.setSize(3);
            reportTableText.setText(m_aerScriptEngine.scriptEngine()->evaluate(m_rowFooter.lines().at(nFooterLine).cells().at(nColumn).value()).toString());

            reportTableCell.addData(reportTableText);
            reportTableLine.addCell(reportTableCell);
        }

        // Add line to the table
        reportTable.addLine(reportTableLine);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTableVertical::exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const
//
// Description	:	Export data into an output stream
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTableVertical::exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const
{
    QString			strCellValue;

    // Fill group script variables
    m_aerScriptEngine.fillScriptGroupVariables(datasetGroup);

    QString of=ReportOptions.GetOption("output", "format").toString();

    // Open report Table
    GexReportTable reportTable;

    reportTable.setBorder(border());
    reportTable.setWidth(HTML_TABLE_WIDTH);
    reportTable.setCellPadding(1);
    reportTable.setCellSpacing(1);

    // write header
    writeHeader(reportTable);

    QList<GexDbPluginERDatasetRow>::const_iterator itRow = datasetGroup.rows().begin();

    while (itRow != datasetGroup.rows().end())
    {
        // Fill script variables for this group
        m_aerScriptEngine.fillScriptGroupVariables(datasetGroup, (*itRow).aggregate());

        // Fill script variables for this serie
        int nSerieIndex = datasetGroup.indexOfSerie((*itRow).serie());
        m_aerScriptEngine.fillScriptSerieVariables(datasetGroup.serieAt(nSerieIndex), (*itRow).aggregate());

        for (int nRow = 0; nRow < m_lstRow.count(); ++nRow)
        {
            if (m_lstRow.at(nRow).printWhen().isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(m_lstRow.at(nRow).printWhen()).toBoolean())
            {
                if ((reportTable.lineCount() + m_lstRow.count()) > m_nMaxLine
                    && ( (of=="DOC")||(of=="PDF")||(of=="PPT")||(of=="ODT") ) 	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                    && eOutput == GexReportTable::HtmlOutput)
                {
                    reportTable.toHtml(txtStream);

                    // Close section and add a page break
                    closeSection("Advanced table");
                    gexReport->WritePageBreak();

                    writeHeader(reportTable);
                }

                for (int nLine = 0; nLine < m_lstRow.at(nRow).lines().count(); ++nLine)
                {
                    GexReportTableLine reportTableLine;

                    for(int nCell = 0; nCell < m_lstRow.at(nRow).lines().at(nLine).cells().count(); ++nCell)
                    {
                        strCellValue = m_lstRow.at(nRow).lines().at(nLine).cells().at(nCell).previousValue(m_aerScriptEngine.scriptEngine()->evaluate(m_lstRow.at(nRow).lines().at(nLine).cells().at(nCell).value()).toString());

                        GexReportTableCell reportTableCell;
                        GexReportTableText reportTableText;

                        // set cell option
                        reportTableCell.setAlign(m_lstRow.at(nRow).lines().at(nLine).cells().at(nCell).alignment());
                        reportTableCell.setColor("#F8F8F8");

                        // set text and font
                        reportTableText.setSize(3);
                        reportTableText.setText(strCellValue);

                        reportTableCell.addData(reportTableText);
                        reportTableLine.addCell(reportTableCell);
                    }

                    reportTable.addLine(reportTableLine);
                }
            }
        }

        ++itRow;
    }

    // Write Footer
    writeFooter(reportTable, datasetGroup);

    // write into output file
    reportTable.toOutput(txtStream, eOutput);

    // Close Section
    closeSection("Advanced table");
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableCustom
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableCustom::GexAdvancedEnterpriseReportTableCustom() : GexAdvancedEnterpriseReportTable()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportTableCustom::~GexAdvancedEnterpriseReportTableCustom()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTableCustom::exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const
//
// Description	:	Export data into an output stream
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTableCustom::exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const
{
    QString		strCellValue;
    QString		strAggregate;
    QSet<int>	lstPrintedRow;

    // Fill group script variables
    m_aerScriptEngine.fillScriptGroupVariables(datasetGroup);

    // Open Html Table
    GexReportTable reportTable;

    QString of=ReportOptions.GetOption("output", "format").toString();

    reportTable.setBorder(border());
    reportTable.setWidth(HTML_TABLE_WIDTH);
    reportTable.setCellPadding(1);
    reportTable.setCellSpacing(1);

    // Write header
    writeHeader(reportTable);

    for (int nSequence = 1; nSequence <= m_nSequenceCount; nSequence++)
    {
        lstPrintedRow.clear();

        QList<GexDbPluginERDatasetSerie>::const_iterator		itSerie		= datasetGroup.series().begin();

        while (itSerie != datasetGroup.series().end())
        {
            m_aerScriptEngine.fillScriptSerieVariables(*itSerie);

            for (int nRow = 0; nRow < m_lstRow.count(); ++nRow)
            {
                if ((lstPrintedRow.contains(nRow) == false || m_lstRow.at(nRow).printFirstOnly() == false) && (nSequence == m_lstRow.at(nRow).sequence()) &&
                    (m_lstRow.at(nRow).printWhen().isEmpty() || m_aerScriptEngine.scriptEngine()->evaluate(m_lstRow.at(nRow).printWhen()).toBoolean()))
                {
                    if ((reportTable.lineCount() + m_lstRow.count()) > m_nMaxLine
                            && ( (of=="DOC")||(of=="PDF")||(of=="PPT")||of=="ODT" ) 	//(ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_FLAT_HTML)
                        && eOutput == GexReportTable::HtmlOutput)
                    {
                        reportTable.toHtml(txtStream);

                        // Close section and add a page break
                        closeSection("Advanced table");
                        gexReport->WritePageBreak();

                        writeHeader(reportTable);
                    }

                    // Mark the row as printed
                    lstPrintedRow.insert(nRow);

                    for (int nLine = 0; nLine < m_lstRow.at(nRow).lines().count(); ++nLine)
                    {
                        GexReportTableLine reportTableLine;

                        for(int nColumn = 0; nColumn < m_lstDefinedAggregate.count(); ++nColumn)
                        {
                            strAggregate = m_lstDefinedAggregate.at(nColumn);

                            m_aerScriptEngine.fillScriptGroupVariables(datasetGroup, strAggregate);
                            m_aerScriptEngine.fillScriptSerieVariables(*itSerie, strAggregate);

                            GexReportTableCell reportTableCell;
                            GexReportTableText reportTableText;

                            // set cell option
                            reportTableCell.setAlign(m_lstRow.at(nRow).lines().at(nLine).cells().at(nColumn).alignment());
                            reportTableCell.setColor("#F8F8F8");

                            // Set alarm color
                            if (m_lstRow.at(nRow).lines().at(nLine).cells().at(nColumn).alarmName().isEmpty() == false)
                            {
                                GexAdvancedEnterpriseReportTableAlarm alarmDescriptor = findAlarm(m_lstRow.at(nRow).lines().at(nLine).cells().at(nColumn).alarmName());

                                if (m_aerScriptEngine.scriptEngine()->canEvaluate(alarmDescriptor.condition()))
                                {
                                    if (m_aerScriptEngine.scriptEngine()->evaluate(alarmDescriptor.condition()).toBool() == true)
                                        reportTableCell.setColor(alarmDescriptor.bkColor());
                                }
                            }

                            // set text and font
                            strCellValue = m_lstRow.at(nRow).lines().at(nLine).cells().at(nColumn).previousValue(m_aerScriptEngine.scriptEngine()->evaluate(m_lstRow.at(nRow).lines().at(nLine).cells().at(nColumn).value()).toString());

                            reportTableText.setSize(3);
                            reportTableText.setText(strCellValue);

                            reportTableCell.addData(reportTableText);
                            reportTableLine.addCell(reportTableCell);
                        }

                        reportTable.addLine(reportTableLine);
                    }
                }
            }

            ++itSerie;
        }
    }

    // Write footer
    writeFooter(reportTable, datasetGroup);

    // Export to output file
    reportTable.toOutput(txtStream, eOutput);

    // Close Html Table
    closeSection("Advanced Table");
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAdvancedEnterpriseReportTableCustom::writeFooter(GexReportTable& reportTable, const GexDbPluginERDatasetGroup& datasetGroup)
//
// Description	:	Write footer
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportTableCustom::writeFooter(GexReportTable& reportTable, const GexDbPluginERDatasetGroup& datasetGroup) const
{
    QString strAggregate;

    for (int nFooterLine = 0; nFooterLine < m_rowFooter.lines().count(); ++nFooterLine)
    {
        GexReportTableLine reportTableLine;

        for(int nColumn = 0; nColumn < m_lstDefinedAggregate.count(); ++nColumn)
        {
            strAggregate = m_lstDefinedAggregate.at(nColumn);

            m_aerScriptEngine.fillScriptGroupVariables(datasetGroup, strAggregate);

            GexReportTableCell reportTableCell;
            GexReportTableText reportTableText;

            // set cell option
            reportTableCell.setAlign(m_rowFooter.lines().at(nFooterLine).cells().at(nColumn).alignment());
            reportTableCell.setColor("#F8F8F8");

            // set text and font
            reportTableText.setSize(3);
            reportTableText.setText(m_aerScriptEngine.scriptEngine()->evaluate(m_rowFooter.lines().at(nFooterLine).cells().at(nColumn).value()).toString());
            reportTableText.setFontStyle(GexReportTableText::BoldStyle);

            reportTableCell.addData(reportTableText);
            reportTableLine.addCell(reportTableCell);
        }

        // Add line to the table
        reportTable.addLine(reportTableLine);
    }
}
