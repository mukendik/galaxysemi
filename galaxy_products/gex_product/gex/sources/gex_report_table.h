#ifndef GEX_REPORT_TABLE_H
#define GEX_REPORT_TABLE_H

#include <QList>
#include <QString>
#include <QTextStream>

//class GexReportTableData
//{
//public:
//
//	GexReportTableData();
//	virtual ~GexReportTableData();
//};

class GexReportTableText
{
public:

	enum FontStyle
	{
		noStyle			= 0x0000,
		BoldStyle		= 0x0001,
		ItalicStyle		= 0x0002,
		UnderLineStyle	= 0x0004
	};

	GexReportTableText();
	GexReportTableText(const GexReportTableText& txtData);
	virtual ~GexReportTableText();

	FontStyle		fontStyle() const									{ return m_eFontStyle; }
	int				size() const										{ return m_nSize; }
	const QString&	color() const										{ return m_strColor; }
	const QString&	text() const										{ return m_strText; }
	const QString&	face() const										{ return m_strFace; }

	void			setFontStyle(FontStyle eFontStyle)					{ m_eFontStyle	= eFontStyle; }
	void			setSize(int nSize)									{ m_nSize		= nSize; }
	void			setColor(const QString& strColor)					{ m_strColor	= strColor; }
	void			setText(const QString& strText)						{ m_strText		= strText; }
	void			setFace(const QString& strFace)						{ m_strFace		= strFace; }

	GexReportTableText&	operator=(const GexReportTableText& txtData);

	void			toHtml(QTextStream& txtStream) const;
	void			toCsv(QTextStream& txtStream) const;
	void			toSpreadSheet(QTextStream& txtStream) const;

private:

	FontStyle		m_eFontStyle;
	int				m_nSize;
	QString			m_strFace;
	QString			m_strColor;
	QString			m_strText;
};

class GexReportTableCell
{
public:

	GexReportTableCell();
	GexReportTableCell(const GexReportTableCell& cellData);
	~GexReportTableCell();

	const QString&	color() const										{ return m_strColor; }
	const QString&	align() const										{ return m_strAlign; }
	const QString&	width() const										{ return m_strWidth; }

	void			setColor(const QString& strColor)					{ m_strColor	= strColor; }
	void			setAlign(const QString& strAlign)					{ m_strAlign	= strAlign; }
	void			setWidth(const QString& strWidth)					{ m_strWidth	= strWidth; }

	GexReportTableCell&	operator=(const GexReportTableCell& cellData);

	void			addData(const GexReportTableText& text)				{ m_dataValue.append(text); }

	void			toHtml(QTextStream& txtStream) const;
	void			toCsv(QTextStream& txtStream) const;
	void			toSpreadSheet(QTextStream& txtStream) const;

private:

	QString						m_strColor;
	QString						m_strAlign;
	QString						m_strWidth;
	QList<GexReportTableText>	m_dataValue;
};

class GexReportTableLine
{
public:

	GexReportTableLine();
	GexReportTableLine(const GexReportTableLine& lineData);
	~GexReportTableLine();

	const QString&	color() const										{ return m_strColor; }
	const QString&	height() const										{ return m_strHeight; }

	void			setColor(const QString& strColor)					{ m_strColor	= strColor; }
	void			setHeight(const QString& strHeight)					{ m_strHeight	= strHeight; }

	void			addCell(const GexReportTableCell& cell)				{ m_lstCell.append(cell); }

	GexReportTableLine&	operator=(const GexReportTableLine& lineData);

	void			toHtml(QTextStream& txtStream) const;
	void			toCsv(QTextStream& txtStream) const;
	void			toSpreadSheet(QTextStream& txtStream) const;

private:

	QList<GexReportTableCell>	m_lstCell;
	QString						m_strHeight;
	QString						m_strColor;
};

class GexReportTable
{
public:

	enum OutputFormat
	{
		HtmlOutput = 0,
		CsvOutput = 1,
		SpreadSheetOutput = 2
	};

	GexReportTable();
	~GexReportTable();

	int				border() const										{ return m_nBorder; }
	int				cellSpacing() const									{ return m_nCellSpacing; }
	int				cellPadding() const									{ return m_nCellPadding; }
	const QString&	align() const										{ return m_strAlign; }
	const QString&	width() const										{ return m_strWidth; }
	int				lineCount() const									{ return m_lstLine.count(); }

	void			setBorder(int nBorder)								{ m_nBorder			= nBorder; }
	void			setCellSpacing(int nCellSpacing)					{ m_nCellSpacing	= nCellSpacing; }
	void			setCellPadding(int nCellPadding)					{ m_nCellPadding	= nCellPadding; }
	void			setAlign(const QString& strAlign)					{ m_strAlign	= strAlign; }
	void			setWidth(const QString& strWidth)					{ m_strWidth	= strWidth; }

	void			addLine(const GexReportTableLine& line)				{ m_lstLine.append(line); }
	void			clear()												{ m_lstLine.clear(); }

	void			toOutput(QTextStream& txtStream, OutputFormat eOutput) const;
	void			toHtml(QTextStream& txtStream) const;
	void			toCsv(QTextStream& txtStream) const;
	void			toSpreadSheet(QTextStream& txtStream) const;

private:

	QList<GexReportTableLine>	m_lstLine;
	int							m_nBorder;
	int							m_nCellSpacing;
	int							m_nCellPadding;
	QString						m_strAlign;
	QString						m_strWidth;
};

#endif // GEX_REPORT_TABLE_H
