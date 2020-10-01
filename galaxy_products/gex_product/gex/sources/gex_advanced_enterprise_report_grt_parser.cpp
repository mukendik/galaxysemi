///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_advanced_enterprise_report_grt_parser.h"
#include "gexdb_plugin_base.h"
#include "gex_version.h"
#include "gex_shared.h"
#include <gqtl_log.h>

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QFile>

///////////////////////////////////////////////////////////
// External variables
///////////////////////////////////////////////////////////
extern const char * gexTimePeriodChoices[];

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAbstractAdvancedEnterpriseReportGrtParser
//
// Description	:	Abstract cClass to parse Advanced Enterprise Report params
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAbstractAdvancedEnterpriseReportGrtParser::GexAbstractAdvancedEnterpriseReportGrtParser()
{

}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAbstractAdvancedEnterpriseReportGrtParser::~GexAbstractAdvancedEnterpriseReportGrtParser()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAbstractAdvancedEnterpriseReportGrtParser::load(GexAdvancedEnterpriseReport& advEnterpriseReport, const QDomDocument& domDocument, QString& strErrorMsg)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool GexAbstractAdvancedEnterpriseReportGrtParser::load(GexAdvancedEnterpriseReport& advEnterpriseReport, const QDomDocument& domDocument, QString& strErrorMsg)
{
    bool bSuccess = false;

    // Clean up error message
    strErrorMsg.clear();

    if (domDocument.documentElement().tagName() == "galaxy_template")
    {
        float fVersion = domDocument.documentElement().attribute("version", "0").toFloat();

        // Read grt version v1
        if (fVersion >= GEX_MIN_GRT_VERSION_V1 && fVersion <= GEX_MAX_GRT_VERSION_V1)
        {
            GexAdvancedEnterpriseReportGrtParserV1 grtParser;

            grtParser.read(domDocument.documentElement(), advEnterpriseReport);
            bSuccess = true;
        }
        else if (fVersion >= GEX_MIN_GRT_VERSION_V2 && fVersion <= GEX_MAX_GRT_VERSION_V2)
        {
            GexAdvancedEnterpriseReportGrtParserV2 grtParser;

            grtParser.read(domDocument.documentElement(), advEnterpriseReport);
            bSuccess = true;
        }
        else
            strErrorMsg = QString("GRT version not supported (%1 not between %2 and %3).").arg(fVersion).arg(GEX_MIN_GRT_VERSION).arg(GEX_MAX_GRT_VERSION);
    }
    else
        strErrorMsg = QString("This is not a GRT format");

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool GexAbstractAdvancedEnterpriseReportGrtParser::create(GexAdvancedEnterpriseReport& advEnterpriseReport, const QString& strFileName, QString& strErrorMsg)
//
// Description	:
//
//////////////////////////////////////////////////////////////////////po/////////////
bool GexAbstractAdvancedEnterpriseReportGrtParser::load(GexAdvancedEnterpriseReport& advEnterpriseReport, const QString& strFileName, QString& strErrorMsg)
{
    QDomDocument									xmlDoc;
    QFile											xmlFile(strFileName); // Read the text from a file
    int												nErrorLine		= -1;
    int												nErrorColumn	= -1;
    bool											bSuccess		= false;

    // Clean up error message
    strErrorMsg.clear();

    if (xmlFile.open(QIODevice::ReadOnly))
    {
        if (xmlDoc.setContent(&xmlFile, &strErrorMsg, &nErrorLine, &nErrorColumn))
            bSuccess = GexAbstractAdvancedEnterpriseReportGrtParser::load(advEnterpriseReport, xmlDoc, strErrorMsg);
        else
            strErrorMsg = QString("%1 is not XML compliant. Error line %2 - Column %3").arg(strFileName).arg(nErrorLine).arg(nErrorColumn);
    }
    else
        strErrorMsg = QString("Unable to open GRT file : %1").arg(strFileName);

    xmlFile.close();

    return bSuccess;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readChartSection(const QDomElement &domElement, GexAdvancedEnterpriseReportGroup &advEnterpriseReportGroup)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readChartSection(const QDomElement &domElement, GexAdvancedEnterpriseReportGroup &advEnterpriseReportGroup)
{
    if (domElement.attribute("visible", "true").toLower() == "true")
    {
        GexAdvancedEnterpriseReportChart * pChart = new GexAdvancedEnterpriseReportChart();

        // Read axis scetion if any
        readAxisSection(domElement, pChart);

        QDomElement legendElement = domElement.firstChildElement("legend");

        if (legendElement.isNull() == false)
            pChart->setLegendMode(legendElement.attribute("visible", "true").toLower());

        advEnterpriseReportGroup.addSection(pChart);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readAxisSection(const QDomElement &domElement, GexAdvancedEnterpriseReportChart &advEnterpriseReportChart)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readAxisSection(const QDomElement &domElement, GexAdvancedEnterpriseReportChart * pAdvEnterpriseReportChart)
{
    QString			strAttribute;

    // Read Y Axis
    QDomNodeList	yAxisNodes	= domElement.elementsByTagName("yaxis");

    // Should be always 2 yaxis or less
    if (yAxisNodes.count() == 0 || yAxisNodes.count() > 2)
        GSLOG(4, " y-axis count invalid...");

    // User can define only one filter type
    for (int nItem = 0; nItem < yAxisNodes.count(); nItem++)
    {
        QDomElement	axisElement	= yAxisNodes.item(nItem).toElement();

        if (axisElement.attribute("visible", "true").toLower() == "true")
        {
            GexAdvancedEnterpriseReportChartYAxis * pYAxis = new GexAdvancedEnterpriseReportChartYAxis();

            pYAxis->setName(axisElement.attribute("name"));

            if (axisElement.hasAttribute("max"))
                pYAxis->setMax(axisElement.attribute("max").toDouble());

            if (axisElement.hasAttribute("min"))
                pYAxis->setMin(axisElement.attribute("min").toDouble());

            if (axisElement.attribute("pos").toLower() == "right")
                pYAxis->setPos(GexAdvancedEnterpriseReportChartYAxis::Right);

            strAttribute = axisElement.attribute("zorder");

            if (strAttribute.toLower() == "front")
                pYAxis->setZOrder(GexAdvancedEnterpriseReportChartYAxis::Front);
            else if (strAttribute.toLower() == "back")
                pYAxis->setZOrder(GexAdvancedEnterpriseReportChartYAxis::Back);

            strAttribute = axisElement.attribute("type");

            if (strAttribute.toLower() == "line")
                pYAxis->setType(GexAdvancedEnterpriseReportChartYAxis::TypeLine);
            else if (strAttribute.toLower() == "bars")
                pYAxis->setType(GexAdvancedEnterpriseReportChartYAxis::TypeBars);
            else if (strAttribute.toLower() == "stackedbars")
                pYAxis->setType(GexAdvancedEnterpriseReportChartYAxis::TypeStackedBars);

            // Read series
            readSerieSection(axisElement, pYAxis);

            // Add Y Axis to the chart
            pAdvEnterpriseReportChart->addYAxis(pYAxis);
        }
    }

    // Read X Axis
    QDomElement xAxisElement = domElement.firstChildElement("xaxis");

    if (xAxisElement.isNull() == false)
    {
        GexAdvancedEnterpriseReportChartXAxis * pXAxis = new GexAdvancedEnterpriseReportChartXAxis();

        pXAxis->setName(xAxisElement.attribute("name"));

        if (xAxisElement.attribute("pos").toLower() == "top")
            pXAxis->setPos(GexAdvancedEnterpriseReportChartXAxis::Top);

        pXAxis->setExpressionLabel(xAxisElement.attribute("labelExpression"));
        pXAxis->setLimit(xAxisElement.attribute("limit", "-1").toInt());

        int lMaxSize = xAxisElement.attribute("maxLabelSize", "18").toInt();

        if (lMaxSize > 3)
            pXAxis->setMaxLabelSize(lMaxSize);

        // Add X Axis to the chart
        pAdvEnterpriseReportChart->setXAxis(pXAxis);
    }
    else
    {
        GSLOG(4, "X-Axis is not defined...");
    }

}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readSerieSection(const QDomElement& domElement, GexAdvancedEnterpriseReportChartYAxis * advEnterpriseReportChartYAxis)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readSerieSection(const QDomElement& domElement, GexAdvancedEnterpriseReportChartYAxis * pAdvEnterpriseReportChartYAxis)
{
    // Dataset values
    QDomNodeList	serieNodes	= domElement.elementsByTagName("serie");

#ifdef QT_DEBUG
    // Should be at least on serie
    if (serieNodes.count() == 0)
        GSLOG(4, "readSerieSection : serie count invalid...");
#endif

    // User can define only one filter type
    for (int nItem = 0; nItem < serieNodes.count(); nItem++)
    {
        QDomElement	serieElement	= serieNodes.item(nItem).toElement();

        if (serieElement.hasAttribute("value"))
        {
            GexAdvancedEnterpriseReportChartSerie * pSerie = new GexAdvancedEnterpriseReportChartSerie();

            pSerie->setValue(serieElement.attribute("value"));
            pSerie->setPrintWhen(serieElement.attribute("printwhen"));
            pSerie->setPrintDataWhen(serieElement.attribute("printdatawhen"));
            pSerie->setName(serieElement.attribute("name"));

            pAdvEnterpriseReportChartYAxis->addSerie(pSerie);
        }
#ifdef QT_DEBUG
        else
        {
            GSLOG(4,"readSerieSection : No serie value defined");
        }
#endif
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readTableSection(const QDomElement &domElement, GexAdvancedEnterpriseReportGroup &advEnterpriseReportGroup)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTableSection(const QDomElement & domElement, GexAdvancedEnterpriseReportGroup &advEnterpriseReportGroup)
{
    if (domElement.attribute("visible", "true").toLower() == "true")
    {
        if (domElement.attribute("type").toLower() == "vertical")
        {
            GexAdvancedEnterpriseReportTableVertical * pTable = new GexAdvancedEnterpriseReportTableVertical();

            // Read vertical table information
            readTableVerticalSection(domElement, pTable);

            // Read Table alarms
            readTableAlarmSection(domElement, pTable);

            advEnterpriseReportGroup.addSection(pTable);
        }
        else if (domElement.attribute("type").toLower() == "custom")
        {
            GexAdvancedEnterpriseReportTableCustom * pTable = new GexAdvancedEnterpriseReportTableCustom();

            // Read vertical table information
            readTableCustomSection(domElement, pTable);

            // Read Table alarms
            readTableAlarmSection(domElement, pTable);

            advEnterpriseReportGroup.addSection(pTable);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readTableAlarmSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTable * pAdvEnterpriseReportTable)
//
// Description	:	read table alarms
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTableAlarmSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTable * pAdvEnterpriseReportTable)
{
    GEX_ASSERT(pAdvEnterpriseReportTable);

    QDomElement		alarmsElement	= domElement.firstChildElement("alarms");
    QDomNodeList	conditionNodes	= alarmsElement.elementsByTagName("condition");

    for (int nCondition = 0; nCondition < conditionNodes.count(); ++nCondition)
    {
        QDomElement		conditionElement	= conditionNodes.at(nCondition).toElement();

        if (conditionElement.attribute("name").isEmpty() == false)
            pAdvEnterpriseReportTable->addAlarm(conditionElement.attribute("name"), conditionElement.text(), conditionElement.attribute("bkcolor", "#FF0000"));
        else
                        qDebug("AER Parser: Alarm name is missing");
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readTableVerticalSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableVertical * pAdvEnterpriseReportTable)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTableVerticalSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableVertical * pAdvEnterpriseReportTable)
{
    // Header definition
    QDomElement		headerElement		= domElement.firstChildElement("header");

    if (headerElement.hasChildNodes())
    {
        QDomNodeList	headerLineNode		= headerElement.elementsByTagName("line");
        QDomNodeList	headerSeparatorNode	= headerElement.elementsByTagName("separator");

        GexAdvancedEnterpriseReportTableRowDesc rowHeader;

        for (int nLine = 0; nLine < headerLineNode.count(); ++nLine)
        {
            QDomElement		lineElement = headerLineNode.at(nLine).toElement();
            QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

            GexAdvancedEnterpriseReportTableLineDesc lineHeader;

            for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
            {
                QDomElement cellElement = cellNodes.at(nCell).toElement();

                lineHeader.addCell(cellElement.attribute("value"), cellElement.attribute("align"), cellElement.attribute("repeatedvalue", "true"));
            }

            rowHeader.addLine(lineHeader);

            pAdvEnterpriseReportTable->setColsCount(qMax(lineHeader.cells().count(), pAdvEnterpriseReportTable->colsCount()));
        }

        pAdvEnterpriseReportTable->setHeader(rowHeader);
    }

    // Row definition
    QDomNodeList	rowNodes		= domElement.elementsByTagName("row");

    for (int nRow = 0; nRow < rowNodes.count(); ++nRow)
    {
        QDomElement		rowElement		= rowNodes.at(nRow).toElement();
        QDomNodeList	rowLineNodes	= rowElement.elementsByTagName("line");

        if (rowLineNodes.count() > 0)
        {
            GexAdvancedEnterpriseReportTableRowDesc row;

            if (rowElement.attribute("printfirstonly", "false").toLower() == "true")
                row.setPrintFirstOnly(true);

            row.setPrintWhen(rowElement.attribute("printwhen"));

            for (int nLine = 0; nLine < rowLineNodes.count(); ++nLine)
            {
                QDomElement		lineElement = rowLineNodes.at(nLine).toElement();
                QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

                GexAdvancedEnterpriseReportTableLineDesc lineSerie;

                for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
                {
                    QDomElement cellElement = cellNodes.at(nCell).toElement();

                    GexAdvancedEnterpriseReportCellDesc cellDescriptor(cellElement.attribute("value"), cellElement.attribute("align"));

                    if (cellElement.attribute("repeatedvalue", "true") == "false")
                        cellDescriptor.setRepeatedValue(false);

                    cellDescriptor.setAlarmName(cellElement.attribute("alarm"));

                    lineSerie.addCell(cellDescriptor);
                }

                row.addLine(lineSerie);

                pAdvEnterpriseReportTable->setColsCount(qMax(lineSerie.cells().count(), pAdvEnterpriseReportTable->colsCount()));
            }

            pAdvEnterpriseReportTable->addRow(row);
        }
    }

    // Footer definition
//	QDomElement		footerElement		= domElement.firstChildElement("footer");
//
//	if (footerElement.hasChildNodes())
//	{
//		QDomNodeList	footerLineNode		= footerElement.elementsByTagName("line");
//		QDomNodeList	footerSeparatorNode	= footerElement.elementsByTagName("separator");
//
//		GexAdvancedEnterpriseReportTableRow rowFooter(GexAdvancedEnterpriseReportTableRow::Footer);
//
//		for (int nLine = 0; nLine < footerLineNode.count(); ++nLine)
//		{
//			QDomElement		lineElement = footerLineNode.at(nLine).toElement();
//			QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");
//
//			GexAdvancedEnterpriseReportTableLine lineFooter;
//
//			for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
//			{
//				QDomElement cellElement = cellNodes.at(nCell).toElement();
//
//				lineFooter.addCell(cellElement.attribute("value"), cellElement.attribute("align"), cellElement.attribute("repeatedvalue", "true"));
//			}
//
//			rowFooter.addLine(lineFooter);
//
//			pAdvEnterpriseReportTable->setColsCount(qMax(lineFooter.cells().count(), pAdvEnterpriseReportTable->colsCount()));
//		}
//
//		for (int nSeparator = 0; nSeparator < footerSeparatorNode.count(); ++nSeparator)
//		{
//			QDomElement											separatorElement = footerSeparatorNode.at(nSeparator).toElement();
//			GexAdvancedEnterpriseReportTableRow::LineSeparator	lineSeparator;
//
//			lineSeparator.setCols(separatorElement.attribute("cols").split(","));
//			lineSeparator.setPrintWhenExpressionChanges(separatorElement.attribute("printwhenexpressionchanges"));
//
//			rowFooter.addSeparator(lineSeparator);
//		}
//
//		pAdvEnterpriseReportTable->setFooter(rowFooter);
//	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readTableRowSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTableRowSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable)
{
    // Serie definition
    QDomNodeList	rowNodes		= domElement.elementsByTagName("row");

    for (int nRow = 0; nRow < rowNodes.count(); ++nRow)
    {
        QDomElement		rowElement		= rowNodes.at(nRow).toElement();
        QDomNodeList	rowLineNodes	= rowElement.elementsByTagName("line");

        if (rowLineNodes.count() > 0)
        {
            GexAdvancedEnterpriseReportTableRowDesc row;

            if (rowElement.attribute("printfirstonly", "false").toLower() == "true")
                row.setPrintFirstOnly(true);

            row.setPrintWhen(rowElement.attribute("printwhen"));
            row.setSequence(rowElement.attribute("sequence", "1").toInt());

            for (int nLine = 0; nLine < rowLineNodes.count(); ++nLine)
            {
                QDomElement		lineElement = rowLineNodes.at(nLine).toElement();
                QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

                GexAdvancedEnterpriseReportTableLineDesc lineSerie;

                for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
                {
                    QDomElement		cellElement		= cellNodes.at(nCell).toElement();

                    GexAdvancedEnterpriseReportCellDesc cellDescriptor(cellElement.attribute("value"), cellElement.attribute("align"));

                    if (cellElement.attribute("repeatedvalue", "true") == "false")
                        cellDescriptor.setRepeatedValue(false);

                    cellDescriptor.setAlarmName(cellElement.attribute("alarm"));

                    lineSerie.addCell(cellDescriptor);
                }

                row.addLine(lineSerie);

                pAdvEnterpriseReportTable->setColsCount(qMax(lineSerie.cells().count(), pAdvEnterpriseReportTable->colsCount()));
            }

            pAdvEnterpriseReportTable->addRow(row);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void GexAbstractAdvancedEnterpriseReportGrtParser::readTableCustomSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTableCustomSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable)
{
    // Header definition
    QDomElement		colsElement			= domElement.firstChildElement("cols");
    QDomNodeList	colsNode			= colsElement.elementsByTagName("col");

    if (colsNode.count() > 0)
    {
        for (int nCol = 0; nCol < colsNode.count(); ++nCol)
        {
            QDomElement		colElement = colsNode.at(nCol).toElement();

            pAdvEnterpriseReportTable->addDefinedAggregate(colElement.attribute("aggregate", ""));
        }
    }

    // Header definition
    QDomElement		headerElement		= domElement.firstChildElement("header");
    QDomNodeList	headerLineNode		= headerElement.elementsByTagName("line");

    if (headerLineNode.count() > 0)
    {
        GexAdvancedEnterpriseReportTableRowDesc rowHeader;

        for (int nLine = 0; nLine < headerLineNode.count(); ++nLine)
        {
            QDomElement		lineElement = headerLineNode.at(nLine).toElement();
            QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

            GexAdvancedEnterpriseReportTableLineDesc lineHeader;

            for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
            {
                QDomElement cellElement = cellNodes.at(nCell).toElement();

                lineHeader.addCell(cellElement.attribute("value"), cellElement.attribute("align", "center"), cellElement.attribute("repeatedvalue", "true"));
            }

            rowHeader.addLine(lineHeader);

            pAdvEnterpriseReportTable->setColsCount(qMax(lineHeader.cells().count(), pAdvEnterpriseReportTable->colsCount()));
        }

        pAdvEnterpriseReportTable->setHeader(rowHeader);
    }

    // Read Body definition
    QDomElement	bodyElement	= domElement.firstChildElement("body");

    if (bodyElement.isNull() == false)
    {
        pAdvEnterpriseReportTable->setSequenceCount(bodyElement.attribute("sequencecount", "1").toInt());
        readTableRowSection(bodyElement, pAdvEnterpriseReportTable);
    }
    else
        readTableRowSection(domElement, pAdvEnterpriseReportTable);

    // Serie definition
    QDomNodeList	rowNodes		= domElement.elementsByTagName("row");

    for (int nRow = 0; nRow < rowNodes.count(); ++nRow)
    {
        QDomElement		rowElement		= rowNodes.at(nRow).toElement();
        QDomNodeList	rowLineNodes	= rowElement.elementsByTagName("line");

        if (rowLineNodes.count() > 0)
        {
            GexAdvancedEnterpriseReportTableRowDesc row;

            if (rowElement.attribute("printfirstonly", "false").toLower() == "true")
                row.setPrintFirstOnly(true);

            row.setPrintWhen(rowElement.attribute("printwhen"));

            for (int nLine = 0; nLine < rowLineNodes.count(); ++nLine)
            {
                QDomElement		lineElement = rowLineNodes.at(nLine).toElement();
                QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

                GexAdvancedEnterpriseReportTableLineDesc lineSerie;

                for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
                {
                    QDomElement cellElement = cellNodes.at(nCell).toElement();

                    lineSerie.addCell(cellElement.attribute("value"), cellElement.attribute("align"), cellElement.attribute("repeatedvalue", "true"));
                }

                row.addLine(lineSerie);

                pAdvEnterpriseReportTable->setColsCount(qMax(lineSerie.cells().count(), pAdvEnterpriseReportTable->colsCount()));
            }

            pAdvEnterpriseReportTable->addRow(row);
        }
    }

    // Header definition
    QDomElement		footerElement		= domElement.firstChildElement("footer");
    QDomNodeList	footerLineNode		= footerElement.elementsByTagName("line");

    if (footerLineNode.count() > 0)
    {
        GexAdvancedEnterpriseReportTableRowDesc rowFooter;

        for (int nLine = 0; nLine < footerLineNode.count(); ++nLine)
        {
            QDomElement		lineElement = footerLineNode.at(nLine).toElement();
            QDomNodeList	cellNodes	= lineElement.elementsByTagName("cell");

            GexAdvancedEnterpriseReportTableLineDesc lineFooter;

            for (int nCell = 0; nCell < cellNodes.count(); ++nCell)
            {
                QDomElement cellElement = cellNodes.at(nCell).toElement();

                lineFooter.addCell(cellElement.attribute("value"), cellElement.attribute("align", "center"), cellElement.attribute("repeatedvalue", "true"));
            }

            rowFooter.addLine(lineFooter);

            pAdvEnterpriseReportTable->setColsCount(qMax(lineFooter.cells().count(), pAdvEnterpriseReportTable->colsCount()));
        }

        pAdvEnterpriseReportTable->setFooter(rowFooter);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readTitleSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readTitleSection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	titleNodes	= domElement.elementsByTagName("title");

    // User can define only one filter type
    for (int nItem = 0; nItem < titleNodes.count(); nItem++)
    {
        QDomElement		titleElement	= titleNodes.item(nItem).toElement();

        advEnterpriseReport.addTitle(titleElement.text());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readSettings(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readSettings(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	settingsNodes	= domElement.elementsByTagName("settings");
    QString			strLabel;
    QString			strValue;

    // User can define only one filter type
    for (int nItem = 0; nItem < settingsNodes.count(); nItem++)
    {
        QDomElement		settingsElement	= settingsNodes.item(nItem).toElement();
        QDomNodeList	customNodes		= settingsElement.elementsByTagName("custom");
        QDomElement		customElement;

        for (int nCustom = 0; nCustom < customNodes.count(); nCustom++)
        {
            customElement = customNodes.item(nCustom).toElement();

            strLabel = customElement.firstChildElement("label").text();
            strValue = customElement.firstChildElement("value").text();

            if (!strLabel.isEmpty() && !strValue.isEmpty())
                advEnterpriseReport.addCustomSettings(strLabel, strValue);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readSummarySection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAbstractAdvancedEnterpriseReportGrtParser::readSummarySection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	summaryNodes	= domElement.elementsByTagName("summary");

    // User can define only one filter type
    for (int nItem = 0; nItem < summaryNodes.count(); nItem++)
    {
        QDomElement		summaryElement	= summaryNodes.item(nItem).toElement();

        advEnterpriseReport.addSummary(summaryElement.text());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportGrtParserV1
//
// Description	:	Class which reads Advanced Enterprise Report params
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGrtParserV1::GexAdvancedEnterpriseReportGrtParserV1()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGrtParserV1::~GexAdvancedEnterpriseReportGrtParserV1()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool read(GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV1::read(const QDomElement& domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Find the home page dom element in the xml tree.
    QDomElement xmlHomePageElement = domElement.firstChildElement("section_home_page");

    if (xmlHomePageElement.hasAttribute("HomeText"))
        advEnterpriseReport.setFrontPageText(xmlHomePageElement.attribute("HomeText"));

    if (xmlHomePageElement.hasAttribute("HomeLogo"))
        advEnterpriseReport.setFrontPageImage(xmlHomePageElement.attribute("HomeLogo"));

    // Find the report dom element in the xml tree.
    QDomElement xmlReportElement = domElement.firstChildElement("report");

    // Set the report name
    advEnterpriseReport.setName(xmlReportElement.attribute("name"));

    readDatasetSection(xmlReportElement, advEnterpriseReport);
    readGroupSection(xmlReportElement, advEnterpriseReport);

    readTitleSection(xmlReportElement, advEnterpriseReport);
    readSummarySection(xmlReportElement, advEnterpriseReport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readDatasetSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV1::readDatasetSection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	datasetNodes	= domElement.elementsByTagName("dataset");

    // User can define only one filter type
    for (int nItem = 0; nItem < datasetNodes.count(); nItem++)
    {
        GexDbPluginERDatasetSettings	datasetSettings;
        QDomElement						datasetElement		= datasetNodes.item(nItem).toElement();
        QString							strAttribute;
        QDate							dtFrom;
        QDate							dtTo;

        if (datasetElement.hasAttribute("name"))
        {
            datasetSettings.setReport(advEnterpriseReport.name());
            datasetSettings.setName(datasetElement.attribute("name"));
            datasetSettings.setDataBase(datasetElement.attribute("DatabaseName"));

            // Testing stage
            datasetSettings.setTestingStage(datasetElement.attribute("DataType"));

            // Time period
            strAttribute = datasetElement.attribute("TimePeriod");

            int nTimePeriod = 0;
            while(gexTimePeriodChoices[nTimePeriod] != 0)
            {
                if(strAttribute.startsWith(gexTimePeriodChoices[nTimePeriod]))
                {
                    datasetSettings.setTimePeriod(nTimePeriod);
                    break;	// found matching string
                }
                else
                    ++nTimePeriod;
            };

            strAttribute = datasetElement.attribute("CalendarFrom");
            dtFrom = QDate::fromString(strAttribute, Qt::ISODate);

            strAttribute = datasetElement.attribute("CalendarTo");
            dtTo = QDate::fromString(strAttribute, Qt::ISODate);

            datasetSettings.setDates(dtFrom, dtTo);

            // Bin Type
            strAttribute = datasetElement.attribute("BinType");

            if (strAttribute.toLower() == "none")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::NoBin);
            else if (strAttribute.toLower() == "soft")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::SoftBin);
            else if (strAttribute.toLower() == "hard")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::HardBin);

            // Full Time Line
            datasetSettings.setFullTimeLine(datasetElement.attribute("FullAggregateRange", "false").toLower() == "true");

            // Roll Up
            strAttribute = datasetElement.attribute("export");

            if (strAttribute.toLower() == "true")
                datasetSettings.setExportRawData(true);

            // Overall
            strAttribute = datasetElement.attribute("WithOverall");
            if (strAttribute.toLower() == "true")
                datasetSettings.setOverall(true);

            // Read filters if any
            readDatasetFiltersSection(datasetElement, datasetSettings);

            m_mapDatasetModel.insert(datasetSettings.name(), datasetSettings);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readDatasetFiltersSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV1::readDatasetFiltersSection(const QDomElement &domElement, GexDbPluginERDatasetSettings &datasetSettings)
{
    // Filter values
    QDomElement		filtersElement	= domElement.firstChildElement("filters");
    QDomNodeList	filterNodes		= filtersElement.elementsByTagName("filter");

    // User can define only one filter type
    for (int nItem = 0; nItem < filterNodes.count(); nItem++)
    {
        QDomElement		datasetFilter	= filterNodes.item(nItem).toElement();
        QString			strOperator;

        if (datasetFilter.hasAttribute("field") && datasetFilter.hasAttribute("op") && datasetFilter.hasAttribute("value"))
        {
            GexDbPluginERDatasetSettings::FilterOperator eOperator = GexDbPluginERDatasetSettings::OpEqual;

            strOperator = datasetFilter.attribute("op");

            if (strOperator.toLower() == "!=")
                eOperator = GexDbPluginERDatasetSettings::OpNotEqual;

            datasetSettings.addFilter(datasetFilter.attribute("field"), datasetFilter.attribute("value"), eOperator);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readGroupSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV1::readGroupSection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	groupNodes	= domElement.elementsByTagName("group");

    // User can define only one filter type
    for (int nItem = 0; nItem < groupNodes.count(); nItem++)
    {
        GexDbPluginERDatasetSettings	datasetSettings;
        QDomElement						groupElement		= groupNodes.item(nItem).toElement();
        QString							strAttribute;

        if (groupElement.hasAttribute("dataset"))
        {
            strAttribute = groupElement.attribute("dataset");

            if (m_mapDatasetModel.contains(strAttribute))
            {
                datasetSettings = m_mapDatasetModel.value(strAttribute);

                if (groupElement.hasAttribute("Split"))
                {
                    QStringList lstGroup = groupElement.attribute("Split").split(",");

                    for (int nGroup = 0; nGroup < lstGroup.count(); nGroup++)
                        datasetSettings.addGroup(lstGroup.at(nGroup));
                }

                if (groupElement.hasAttribute("Aggregate"))
                {
                    QStringList lstAggregate = groupElement.attribute("Aggregate").split(",");

                    for (int nAggregate = 0; nAggregate < lstAggregate.count(); nAggregate++)
                    {
                        if (lstAggregate.at(nAggregate).isEmpty() == false)
                            datasetSettings.addAggregate(lstAggregate.at(nAggregate));
                    }
                }

                if (groupElement.hasAttribute("Serie"))
                {
                    QStringList lstSerie = groupElement.attribute("Serie").split(",");

                    for (int nSerie = 0; nSerie < lstSerie.count(); nSerie++)
                    {
                        if (lstSerie.at(nSerie).isEmpty() == false)
                            datasetSettings.addSerie(lstSerie.at(nSerie));
                    }
                }

                if (groupElement.hasAttribute("Order"))
                {
                    QStringList lstOrder = groupElement.attribute("Order").split(";");

                    for (int nOrder = 0; nOrder < lstOrder.count(); nOrder++)
                    {
                        if (lstOrder.at(nOrder).isEmpty() == false)
                            datasetSettings.addOrder(lstOrder.at(nOrder));
                    }
                }

                // Insert dataset into the report
                GexAdvancedEnterpriseReportGroup * pGroup	= new GexAdvancedEnterpriseReportGroup(advEnterpriseReport.addDataset(datasetSettings));

                pGroup->setName(groupElement.attribute("name", ""));

                // Read all sections
                QDomNodeList	sectionNodeList = groupElement.childNodes();
                QDomElement		sectionElement;

                for (int nSection = 0; nSection < sectionNodeList.count(); ++nSection)
                {
                    sectionElement = sectionNodeList.at(nSection).toElement();

                    if (sectionElement.tagName() == "chart")
                        // read chart section if any
                        readChartSection(sectionElement, *pGroup);
                    else if (sectionElement.tagName() == "table")
                        // read Table section if any
                        readTableSection(sectionElement, *pGroup);
                }

                // Add group section to the report
                advEnterpriseReport.addGroup(pGroup);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportGrtParserV2
//
// Description	:	Class which reads Advanced Enterprise Report params
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGrtParserV2::GexAdvancedEnterpriseReportGrtParserV2()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexAdvancedEnterpriseReportGrtParserV2::~GexAdvancedEnterpriseReportGrtParserV2()
{

}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool read(GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV2::read(const QDomElement& domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Find the report dom element in the xml tree.
    QDomElement xmlReportElement = domElement.firstChildElement("report");

    // Set the report name
    advEnterpriseReport.setName(xmlReportElement.attribute("name"));

    readDatasetSection(xmlReportElement, advEnterpriseReport);

    readGroupSection(xmlReportElement, advEnterpriseReport);

    readTitleSection(xmlReportElement, advEnterpriseReport);
    readSettings(xmlReportElement, advEnterpriseReport);
    readSummarySection(xmlReportElement, advEnterpriseReport);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readDatasetSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV2::readDatasetSection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomElement		xmlDatasetsElement	= domElement.firstChildElement("datasets");
    QDomNodeList	datasetNodes		= xmlDatasetsElement.elementsByTagName("dataset");

    // User can define only one filter type
    for (int nItem = 0; nItem < datasetNodes.count(); nItem++)
    {
        GexDbPluginERDatasetSettings	datasetSettings;
        QDomElement						datasetElement		= datasetNodes.item(nItem).toElement();
        QString							strAttribute;
        QDate							dtFrom;
        QDate							dtTo;

        if (datasetElement.hasAttribute("ID"))
        {
            datasetSettings.setReport(advEnterpriseReport.name());
            datasetSettings.setName(datasetElement.attribute("ID"));
            datasetSettings.setDataBase(datasetElement.attribute("DatabaseName"));

            // Testing stage
            datasetSettings.setTestingStage(datasetElement.attribute("Stage"));

            // Time period
            strAttribute = datasetElement.attribute("TimePeriod");

            int nTimePeriod = 0;
            while(gexTimePeriodChoices[nTimePeriod] != 0)
            {
                if(strAttribute.startsWith(gexTimePeriodChoices[nTimePeriod]))
                {
                    datasetSettings.setTimePeriod(nTimePeriod);
                    break;	// found matching string
                }
                else
                    ++nTimePeriod;
            };

            if (nTimePeriod == GEX_QUERY_TIMEPERIOD_LAST_N_X)
            {
                // Time Steps
                if (datasetElement.hasAttribute("TimeStep") == false)
                    GSLOG(3, "Attribute TimeSteps missing in the definition of the dataset.");

                // TimeNFactor
                if (datasetElement.hasAttribute("TimeNFactor") == false)
                    GSLOG(3, "Attribute TimeNFactor missing in the definition of the dataset.");

                datasetSettings.setDates(datasetElement.attribute("TimeStep", "Years"),
                                            datasetElement.attribute("TimeNFactor", "1").toLong());
            }
            else
            {
                strAttribute = datasetElement.attribute("CalendarFrom");
                dtFrom = QDate::fromString(strAttribute, Qt::ISODate);

                strAttribute = datasetElement.attribute("CalendarTo");
                dtTo = QDate::fromString(strAttribute, Qt::ISODate);

                datasetSettings.setDates(dtFrom, dtTo);
            }

            // Bin Type
            strAttribute = datasetElement.attribute("BinType");

            if (strAttribute.toLower() == "none")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::NoBin);
            else if (strAttribute.toLower() == "soft" || strAttribute.toLower() == "s")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::SoftBin);
            else if (strAttribute.toLower() == "hard" || strAttribute.toLower() == "h")
                datasetSettings.setBinType(GexDbPluginERDatasetSettings::HardBin);

            // Export
            strAttribute = datasetElement.attribute("export");

            if (strAttribute.toLower() == "true")
                datasetSettings.setExportRawData(true);

            // Overall
            strAttribute = datasetElement.attribute("WithOverall");
            if (strAttribute.toLower() == "true")
                datasetSettings.setOverall(true);

            // Read filters if any
            readDatasetRolesSection(datasetElement, datasetSettings);

            // Read postprocessing action if any
            readDatasetPostProcessingSection(datasetElement, datasetSettings);

            m_mapDatasetModel.insert(datasetSettings.name(), datasetSettings);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readDatasetPostProcessingSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV2::readDatasetPostProcessingSection(const QDomElement &domElement, GexDbPluginERDatasetSettings &datasetSettings)
{
    QDomElement	domElementPostProcessing	= domElement.firstChildElement("postprocessing");
    QDomElement	domElementAlgorithm			= domElementPostProcessing.firstChildElement();

    while (domElementAlgorithm.isNull() == false)
    {
        if (domElementAlgorithm.tagName() == "topnserie")
        {
            GexDbPluginERPostProcessing			postProcessing;
            GexDbPluginERPostProcessingCriteria	postProcessingCriteria;

            postProcessing.setTopNCount(domElementAlgorithm.attribute("count", "10").toUInt());
            postProcessing.setOtherName(domElementAlgorithm.attribute("othername", "Others"));

            QDomElement domElementField = domElementAlgorithm.firstChildElement("field");

            while (domElementField.isNull() == false)
            {
                if (domElementField.hasAttribute("name"))
                    postProcessing.addField(domElementField.attribute("name"), domElementField.attribute("order", ""));

                domElementField = domElementField.nextSiblingElement("field");
            }

            QDomElement	domElementCriteria	= domElementAlgorithm.firstChildElement("criteria");
            domElementField	= domElementCriteria.firstChildElement("field");

            while(domElementField.isNull() == false)
            {
                postProcessingCriteria.m_strField		= domElementField.attribute("name");
                postProcessingCriteria.m_strOperator	= domElementField.attribute("operator");
                postProcessingCriteria.m_strValue		= domElementField.attribute("value");

                postProcessing.addCriteria(postProcessingCriteria);

                domElementField = domElementField.nextSiblingElement("field");
            }

            datasetSettings.addPostProcessing(postProcessing);
        }

        domElementAlgorithm = domElementAlgorithm.nextSiblingElement();
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readDatasetRolesSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV2::readDatasetRolesSection(const QDomElement &domElement, GexDbPluginERDatasetSettings &datasetSettings)
{
    QDomNodeList		roleNodes		= domElement.childNodes();
    QDomElement			roleElement;
    QDomNodeList		fieldNodes;
    QMap<int, QString>	mapFields;
    QString				strRoleID;

    for (int nRole = 0; nRole < roleNodes.count(); nRole++)
    {
        // Clean up map fields
        mapFields.clear();

        roleElement	= roleNodes.at(nRole).toElement();
        fieldNodes	= roleElement.elementsByTagName("field");

        if (roleElement.attribute("FullRange", "false").toLower() == "true")
            datasetSettings.setFullTimeLine(true);

        // User can define only one filter type
        for (int nItem = 0; nItem < fieldNodes.count(); nItem++)
        {
            QDomElement		datasetRole	= fieldNodes.item(nItem).toElement();
            int				nIndex;
            QString			strOrder;
            QString			strLabel;
            QString			strOperator;
            QString			strValue;
            QString         strAdditional_data;

            if (datasetRole.hasAttribute("label"))
            {
                strLabel = datasetRole.attribute("label");
                strOrder = datasetRole.attribute("order", "");
                nIndex = datasetRole.attribute("index", "-1").toInt();

                if (strOrder.isEmpty() == false)
                    strLabel = strLabel + '|' + strOrder;

                if (nIndex >= 0)
                    mapFields.insert(nIndex, strLabel);

                // Does the field is also used as a filter
                if (datasetRole.hasAttribute("operator") && datasetRole.hasAttribute("value"))
                {
                    GexDbPluginERDatasetSettings::FilterOperator eOperator = GexDbPluginERDatasetSettings::OpEqual;

                    strOperator = datasetRole.attribute("operator");
                    strValue	= datasetRole.attribute("value");
                    if(datasetRole.hasAttribute("additional_data"))
                        strAdditional_data = datasetRole.attribute("additional_data");

                    if (strOperator.toLower() == "!=")
                        eOperator = GexDbPluginERDatasetSettings::OpNotEqual;

                    if (strValue.isEmpty() == false && strValue != "*")
                        datasetSettings.addFilter(strLabel, strValue, eOperator, strAdditional_data);
                }
            }
            else
                                qDebug("Role %s contains fields without label", (roleElement.nodeName()).toLatin1().constData() );
        }

        strRoleID = datasetSettings.name() + ":" + roleElement.nodeName();
        m_mapRole.insert(strRoleID, mapFields);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void readGroupSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
//
// Description	:	read config from txt file
//
///////////////////////////////////////////////////////////////////////////////////
void GexAdvancedEnterpriseReportGrtParserV2::readGroupSection(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport)
{
    // Dataset values
    QDomNodeList	groupNodes	= domElement.elementsByTagName("group");

    // User can define only one filter type
    for (int nItem = 0; nItem < groupNodes.count(); nItem++)
    {
        GexDbPluginERDatasetSettings	datasetSettings;
        QDomElement						groupElement		= groupNodes.item(nItem).toElement();
        QString							strAttribute;

        if (groupElement.hasAttribute("dataset"))
        {
            strAttribute = groupElement.attribute("dataset");

            if (m_mapDatasetModel.contains(strAttribute))
            {
                datasetSettings = m_mapDatasetModel.value(strAttribute);

                if (groupElement.hasAttribute("Split"))
                {
                    QMap<int, QString>				mapFields;
                    QMap<int, QString>::iterator	itField;
                    strAttribute = groupElement.attribute("Split");

                    QStringList lstField = strAttribute.split(",");
                    QString		strField;

                    foreach (strField, lstField)
                    {
                        if (m_mapRole.contains(strField))
                        {
                            mapFields	= m_mapRole.value(strField);
                            itField		= mapFields.begin();

                            while(itField != mapFields.end())
                            {
                                datasetSettings.addGroup(itField.value());
                                ++itField;
                            }
                        }
                        else if (strField.contains(":") == false)
                            datasetSettings.addGroup(strField);
                    }
                }

                if (groupElement.hasAttribute("Aggregate"))
                {
                    QMap<int, QString>				mapFields;
                    QMap<int, QString>::iterator	itField;
                    strAttribute = groupElement.attribute("Aggregate");

                    QStringList lstField = strAttribute.split(",");
                    QString		strField;

                    foreach (strField, lstField)
                    {
                        if (m_mapRole.contains(strField))
                        {
                            mapFields	= m_mapRole.value(strField);
                            itField		= mapFields.begin();

                            while(itField != mapFields.end())
                            {
                                datasetSettings.addAggregate(itField.value());
                                ++itField;
                            }
                        }
                        else if (strField.contains(":") == false)
                            datasetSettings.addAggregate(strField);
                    }
                }

                if (groupElement.hasAttribute("Serie"))
                {
                    QMap<int, QString>				mapFields;
                    QMap<int, QString>::iterator	itField;
                    strAttribute = groupElement.attribute("Serie");

                    QStringList lstField = strAttribute.split(",");
                    QString		strField;

                    foreach (strField, lstField)
                    {
                        if (m_mapRole.contains(strField))
                        {
                            mapFields	= m_mapRole.value(strField);
                            itField		= mapFields.begin();

                            while(itField != mapFields.end())
                            {
                                datasetSettings.addSerie(itField.value());
                                ++itField;
                            }
                        }
                        else if (strField.contains(":") == false)
                            datasetSettings.addSerie(strField);
                    }
                }

                if (groupElement.hasAttribute("Order"))
                {
                    QMap<int, QString>				mapFields;
                    QMap<int, QString>::iterator	itField;
                    strAttribute = groupElement.attribute("Order");

                    QStringList lstField = strAttribute.split(",");
                    QString		strField;

                    foreach (strField, lstField)
                    {
                        if (m_mapRole.contains(strField))
                        {
                            mapFields	= m_mapRole.value(strField);
                            itField		= mapFields.begin();

                            while(itField != mapFields.end())
                            {
                                datasetSettings.addOrder(itField.value());
                                ++itField;
                            }
                        }
                        else if (strField.contains(":") == false)
                            datasetSettings.addOrder(strField);
                    }
                }

                // For Intermediate consolidation, Check if we have a split on the 'consolidation name' field.
                // If no, add it as the first split field in the group.
                datasetSettings.checkForIntermediateConsolidationSplit();

                // Insert dataset into the report
                GexAdvancedEnterpriseReportGroup * pGroup	= new GexAdvancedEnterpriseReportGroup(advEnterpriseReport.addDataset(datasetSettings));

                pGroup->setName(groupElement.attribute("name", ""));
                pGroup->setBreakPage(groupElement.attribute("breakpage", "1").toInt());
                pGroup->setPrintWhen(groupElement.attribute("printwhen", ""));

                // Read all sections
                QDomNodeList	sectionNodeList = groupElement.childNodes();
                QDomElement		sectionElement;

                for (int nSection = 0; nSection < sectionNodeList.count(); ++nSection)
                {
                    sectionElement = sectionNodeList.at(nSection).toElement();

                    if (sectionElement.tagName() == "chart")
                        // read chart section if any
                        readChartSection(sectionElement, *pGroup);
                    else if (sectionElement.tagName() == "table")
                        // read Table section if any
                        readTableSection(sectionElement, *pGroup);
                    else if (sectionElement.tagName() == "exception")
                    {
                        if (sectionElement.hasAttribute("name"))
                            pGroup->addException(sectionElement.attribute("name"), sectionElement.text());
                    }
                }

                // Add group section to the report
                advEnterpriseReport.addGroup(pGroup);
            }
        }
    }
}

