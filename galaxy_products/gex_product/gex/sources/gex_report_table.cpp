#include "gex_report_table.h"

GexReportTableText::GexReportTableText() : m_eFontStyle(noStyle), m_nSize(0)
{
}

GexReportTableText::GexReportTableText(const GexReportTableText &txtData)
{
    *this = txtData;
}

GexReportTableText::~GexReportTableText()
{

}

GexReportTableText& GexReportTableText::operator =(const GexReportTableText& txtData)
{
    if (this != &txtData)
    {
        m_eFontStyle	= txtData.m_eFontStyle;
        m_nSize			= txtData.m_nSize;
        m_strColor		= txtData.m_strColor;
        m_strFace		= txtData.m_strFace;
        m_strText		= txtData.m_strText;
    }

    return *this;
}

void GexReportTableText::toHtml(QTextStream &txtStream) const
{
    if (m_strText.isEmpty() == false)
    {
        QString strText = m_strText;

        if (m_eFontStyle & BoldStyle)
        {
            strText.insert(0, "<b>");
            strText.append("</b>");
        }

        if (m_eFontStyle & ItalicStyle)
        {
            strText.insert(0, "<i>");
            strText.append("</i>");
        }

        if (m_eFontStyle & UnderLineStyle)
        {
            strText.insert(0, "<u>");
            strText.append("</u>");
        }

        txtStream << "<font ";

        if (m_nSize != 0)
            txtStream << "size=\"" << m_nSize << "\" ";

        if (m_strColor.isEmpty() == false)
            txtStream << "color=\"" << m_strColor << "\" ";

        if (m_strFace.isEmpty() == false)
            txtStream << "face=\"" << m_strFace << "\" ";

        txtStream << ">";
        txtStream << strText;
        txtStream << "</font>";
    }
}

void GexReportTableText::toCsv(QTextStream &txtStream) const
{
    txtStream << m_strText;
}

void GexReportTableText::toSpreadSheet(QTextStream &txtStream) const
{
    txtStream << m_strText;
}

GexReportTableCell::GexReportTableCell()
{

}

GexReportTableCell::GexReportTableCell(const GexReportTableCell &cellData)
{
    *this = cellData;
}

GexReportTableCell::~GexReportTableCell()
{

}

GexReportTableCell& GexReportTableCell::operator =(const GexReportTableCell& cellData)
{
    if (this != &cellData)
    {
        m_dataValue	= cellData.m_dataValue;
        m_strAlign	= cellData.m_strAlign;
        m_strColor	= cellData.m_strColor;
        m_strWidth	= cellData.m_strWidth;
    }

    return *this;
}

void GexReportTableCell::toHtml(QTextStream &txtStream) const
{
    // Open cell
    txtStream << "<td ";

    if (m_strColor.isEmpty() == false)
        txtStream << "bgcolor=\"" << m_strColor << "\" ";

    if (m_strWidth.isEmpty() == false)
        txtStream << "width=\"" << m_strWidth << "\" ";

    if (m_strAlign.isEmpty() == false)
        txtStream << "align=\"" << m_strAlign << "\" ";

    txtStream << ">" << endl;

    // Add data to the html output
    for (int nData = 0; nData < m_dataValue.count(); ++nData)
        m_dataValue.at(nData).toHtml(txtStream);

    // Close cell
    txtStream << "</td>" << endl;
}

void GexReportTableCell::toCsv(QTextStream &txtStream) const
{
    // Add data to the html output
    for (int nData = 0; nData < m_dataValue.count(); ++nData)
        m_dataValue.at(nData).toCsv(txtStream);

    txtStream << ",";
}

void GexReportTableCell::toSpreadSheet(QTextStream &txtStream) const
{
    // Add data to the html output
    for (int nData = 0; nData < m_dataValue.count(); ++nData)
        m_dataValue.at(nData).toSpreadSheet(txtStream);

    txtStream << "\t";
}

GexReportTableLine::GexReportTableLine()
{

}

GexReportTableLine::GexReportTableLine(const GexReportTableLine &lineData)
{
    *this = lineData;
}

GexReportTableLine::~GexReportTableLine()
{

}

GexReportTableLine& GexReportTableLine::operator =(const GexReportTableLine& lineData)
{
    if (this != &lineData)
    {
        m_lstCell	= lineData.m_lstCell;
        m_strColor	= lineData.m_strColor;
        m_strHeight	= lineData.m_strHeight;
    }

    return *this;
}

void GexReportTableLine::toHtml(QTextStream &txtStream) const
{
    // Open line
    txtStream << "<tr";

    if (m_strColor.isEmpty() == false)
        txtStream << "bgcolor=\"" << m_strColor << "\" ";

    if (m_strHeight.isEmpty() == false)
        txtStream << "height=\"" << m_strHeight << "\" ";

    txtStream << ">" << endl;

    // Add cells to the html output
    for (int nCell = 0; nCell < m_lstCell.count(); ++nCell)
        m_lstCell.at(nCell).toHtml(txtStream);

    // Close line
    txtStream << "</tr>" << endl;
}

void GexReportTableLine::toCsv(QTextStream &txtStream) const
{
    // Add cells to the csv output
    for (int nCell = 0; nCell < m_lstCell.count(); ++nCell)
        m_lstCell.at(nCell).toCsv(txtStream);

    txtStream << endl;
}

void GexReportTableLine::toSpreadSheet(QTextStream &txtStream) const
{
    // Add cells to the csv output
    for (int nCell = 0; nCell < m_lstCell.count(); ++nCell)
        m_lstCell.at(nCell).toSpreadSheet(txtStream);

    txtStream << endl;
}


GexReportTable::GexReportTable() : m_nBorder(0), m_nCellSpacing(0), m_nCellPadding(0)
{

}

GexReportTable::~GexReportTable()
{

}

void GexReportTable::toOutput(QTextStream &txtStream, OutputFormat eOutput) const
{
    switch(eOutput)
    {
        default					:
        case HtmlOutput			:	toHtml(txtStream);
                                    break;

        case CsvOutput			:	toCsv(txtStream);
                                    break;

        case SpreadSheetOutput	:	toSpreadSheet(txtStream);
                                    break;
    }
}

void GexReportTable::toHtml(QTextStream &txtStream) const
{
    // Open html table.
    txtStream << "<table ";

    if (m_nBorder != -1)
        txtStream << "border=\"" << m_nBorder << "\" ";

    if (m_nCellSpacing != -1)
        txtStream << "cellspacing=\"" << m_nCellSpacing << "\" ";

    if (m_nCellPadding != -1)
        txtStream << "cellpadding=\"" << m_nCellPadding << "\" ";

    if (m_strWidth.isEmpty() == false)
        txtStream << "width=\"" << m_strWidth << "\" ";

    if (m_strAlign.isEmpty() == false)
        txtStream << "align=\"" << m_strAlign << "\" ";

    txtStream << ">" << endl;

    // Add lines to the html output
    for (int nLine = 0; nLine < m_lstLine.count(); ++nLine)
        m_lstLine.at(nLine).toHtml(txtStream);

    // Close html table
    txtStream << "</table>" << endl;
}

void GexReportTable::toCsv(QTextStream &txtStream) const
{
    // Add lines to the csv output
    for (int nLine = 0; nLine < m_lstLine.count(); ++nLine)
        m_lstLine.at(nLine).toCsv(txtStream);
}

void GexReportTable::toSpreadSheet(QTextStream &txtStream) const
{
    // Add lines to the spreadsheet output
    for (int nLine = 0; nLine < m_lstLine.count(); ++nLine)
        m_lstLine.at(nLine).toSpreadSheet(txtStream);
}
