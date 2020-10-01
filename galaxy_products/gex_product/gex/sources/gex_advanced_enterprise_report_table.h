#ifndef GEX_ADVANCED_ENTERPRISE_REPORT_TABLE_H
#define GEX_ADVANCED_ENTERPRISE_REPORT_TABLE_H

#include "gex_advanced_enterprise_report.h"
#include "gex_report_table.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableAlarm
//
// Description	:	Class which dsecribes an alarm on table
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTableAlarm
{
public:

	GexAdvancedEnterpriseReportTableAlarm();
	GexAdvancedEnterpriseReportTableAlarm(const QString& strJSCondition, const QString& strBackColor);
	GexAdvancedEnterpriseReportTableAlarm(const GexAdvancedEnterpriseReportTableAlarm& other);
	~GexAdvancedEnterpriseReportTableAlarm()	{};

	const QString&		condition() const							{ return m_strJSCondition; }
	const QString&		bkColor()	const							{ return m_strBackColor; }

	void				setCondition(const QString& strJSCondition)	{ m_strJSCondition	= strJSCondition; }
	void				setBkColor(const QString& strBackColor)		{ m_strBackColor	= strBackColor; }

private:

	QString				m_strJSCondition;							// javascript expression
	QString				m_strBackColor;								// background color
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportCellDesc
//
// Description	:	Class which dsecribes a cell in a line
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportCellDesc
{
public:

	GexAdvancedEnterpriseReportCellDesc(const QString& strValue, const QString& strAlign);
	~GexAdvancedEnterpriseReportCellDesc()	{};

	const QString&		alarmName() const							{ return m_strAlarmName; }
	const QString&		value() const								{ return m_strValue; }
	const QString&		alignment() const							{ return m_strAlignment; }
	bool				repeadtedValue() const						{ return m_bRepeatedValue; }

	void				setRepeatedValue(bool bRepeated)			{ m_bRepeatedValue	= bRepeated; }
	void				setAlarmName(const QString& strAlarmName)	{ m_strAlarmName	= strAlarmName; }

	QString				previousValue(const QString& strValue) const;

private:

	QString												m_strValue;
	QString												m_strAlignment;
	QString												m_strAlarmName;
	mutable QString										m_strPreviousValue;
	bool												m_bRepeatedValue;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableLineDesc
//
// Description	:	Class which dsecribes a line in a table
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTableLineDesc
{
public:

	GexAdvancedEnterpriseReportTableLineDesc();
	~GexAdvancedEnterpriseReportTableLineDesc();

	const QList<GexAdvancedEnterpriseReportCellDesc>&	cells() const					{ return m_lstCell; }

	void												addCell(const QString& strValue, const QString& strAlign = "center", const QString& strRepeatedValue = "true");
	void												addCell(const GexAdvancedEnterpriseReportCellDesc& cellDescriptor);

private:

	QList<GexAdvancedEnterpriseReportCellDesc>			m_lstCell;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableRowDesc
//
// Description	:	Class which describes a row in a table
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTableRowDesc
{
public:

	GexAdvancedEnterpriseReportTableRowDesc();
	~GexAdvancedEnterpriseReportTableRowDesc();

	int														sequence() const												{ return m_nSequence; }
	bool													printFirstOnly() const											{ return m_bPrintFirstOnly; }
	const QString&											printWhen() const												{ return m_strPrintWhen; }
	const QList<GexAdvancedEnterpriseReportTableLineDesc>&	lines() const													{ return m_lstLine; }

	void													setSequence(int nSequence)										{ m_nSequence = nSequence; }
	void													setPrintWhen(const QString& strPrintWhen)						{ m_strPrintWhen = strPrintWhen; }
	void													setPrintFirstOnly(bool bPrintFirstOnly)							{ m_bPrintFirstOnly = bPrintFirstOnly; }

	void													addLine(const GexAdvancedEnterpriseReportTableLineDesc& line)	{ m_lstLine.append(line); }

private:

	int														m_nSequence;
	QString													m_strPrintWhen;
	bool													m_bPrintFirstOnly;
	QList<GexAdvancedEnterpriseReportTableLineDesc>			m_lstLine;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTable
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTable : public GexAdvancedEnterpriseReportSection
{
public:

	GexAdvancedEnterpriseReportTable();
	~GexAdvancedEnterpriseReportTable();

	int													border() const																{ return m_nBorder; }
	int													colsCount() const															{ return m_nColsCount; }

	void												setBorder(int nBorder)														{ m_nBorder = nBorder; }
	void												setColsCount(int nCount)													{ m_nColsCount = nCount; }
	void												setSequenceCount(int nCount)												{ m_nSequenceCount = nCount; }

	void												setHeader(const GexAdvancedEnterpriseReportTableRowDesc& rowHeader)			{ m_rowHeader = rowHeader; }
	void												setFooter(const GexAdvancedEnterpriseReportTableRowDesc& rowFooter)			{ m_rowFooter = rowFooter; }
	void												addRow(const GexAdvancedEnterpriseReportTableRowDesc& row)					{ m_lstRow.append(row); }
	void												addAlarm(const QString& strName, const QString& strJSCondition, const QString& strBackColor);

	void												exportToHtml(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;
	void												exportToCsv(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;
	void												exportToSpreadsheet(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;

protected:

	GexAdvancedEnterpriseReportTableRowDesc				m_rowHeader;
	GexAdvancedEnterpriseReportTableRowDesc				m_rowFooter;
	QList<GexAdvancedEnterpriseReportTableRowDesc>		m_lstRow;
	int													m_nMaxLine;
	int													m_nSequenceCount;

	GexAdvancedEnterpriseReportTableAlarm				findAlarm(const QString& strName) const;

	virtual void										exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const = 0;
	virtual void										writeHeader(GexReportTable& reportTable) const;
	virtual void										writeFooter(GexReportTable& reportTable, const GexDbPluginERDatasetGroup& datasetGroup) const;

private:

	int													m_nBorder;
	int													m_nColsCount;
	QMap<QString,GexAdvancedEnterpriseReportTableAlarm>	m_mapAlarms;
};


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableVertical
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTableVertical : public GexAdvancedEnterpriseReportTable
{
public:

	GexAdvancedEnterpriseReportTableVertical();
	~GexAdvancedEnterpriseReportTableVertical();

	void												exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportTableCustom
//
// Description	:	Class which represents a table in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportTableCustom : public GexAdvancedEnterpriseReportTable
{
public:

	GexAdvancedEnterpriseReportTableCustom();
	~GexAdvancedEnterpriseReportTableCustom();

	const QList<QString>&								definedAggregate() const								{ return m_lstDefinedAggregate; }

	void												addDefinedAggregate(const QString& strAggregate)		{ m_lstDefinedAggregate.append(strAggregate); }

	void												exportToOutput(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup, GexReportTable::OutputFormat eOutput) const;

protected:

	void												writeFooter(GexReportTable& reportTable, const GexDbPluginERDatasetGroup& datasetGroup) const;

private:

	QList<QString>										m_lstDefinedAggregate;
};

#endif // GEX_ADVANCED_ENTERPRISE_REPORT_TABLE_H
