#ifndef GEX_ADVANCED_ENTERPRISE_REPORT_H
#define GEX_ADVANCED_ENTERPRISE_REPORT_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gexdb_plugin_er_dataset.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QTextStream>
#include <QMap>
#include <QScriptEngine>
#include <QtScriptTools>

///////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////

// of must be retrieved with such line : QString of=ReportOptions.GetOption("output","format").toString();
#define HTML_TABLE_WIDTH ((of=="PPT") ? "800" : "100%%")

class PrivateCustomSettings;
class GexDatabaseEntry;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportScriptEngine
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportScriptEngine
{
public:

	GexAdvancedEnterpriseReportScriptEngine();
	~GexAdvancedEnterpriseReportScriptEngine();

	QScriptEngine *			scriptEngine() const				{ return m_pScriptEngine; }

	void					fillScriptGroupVariables(const GexDbPluginERDatasetGroup& datasetGroup, const QString& strAggregate = QString()) const;
	void					fillScriptSerieVariables(const GexDbPluginERDatasetSerie& datasetSerie, const QString& strAggregate = QString()) const;
	void					fillScriptRowGroupVariables(const GexDbPluginERDatasetRow& datasetRow) const;
	void					fillScriptRowSerieVariables(const GexDbPluginERDatasetRow& datasetRow) const;

private:



	QScriptEngine *			m_pScriptEngine;

#ifdef QT_DEBUG
	QScriptEngineDebugger	m_scriptDebugger;
#endif

};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportDataset
//
// Description	:	Class which represents a dataset in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportDataset
{
public:

	GexAdvancedEnterpriseReportDataset(const GexDbPluginERDatasetSettings& datasetSettings, int nID);
	~GexAdvancedEnterpriseReportDataset();

	bool											dataStatus() const		{ return m_bDataStatus; }
	const QString&									errorMessage() const	{ return m_strErrorMsg; }
	const GexDbPluginERDatasetSettings&				settings() const		{ return m_settings; }
	const GexDbPluginERDataset&						results() const			{ return m_results; }
	QString											rawDataFilename() const;

	bool											requestData();
	void											sortResults();

protected:

	void											exportRawData(GexDatabaseEntry * pDatabaseEntry) const;

private:

	bool											m_bDataStatus;
	QString											m_strErrorMsg;
	GexDbPluginERDatasetSettings					m_settings;
	GexDbPluginERDataset							m_results;
	int												m_nID;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportSection
//
// Description	:	Class which represents a section in a group of a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportSection
{
public:

	GexAdvancedEnterpriseReportSection();
	virtual ~GexAdvancedEnterpriseReportSection();

	int											sectionID() const							{ return m_nSectionID; }

	virtual void								exportToHtml(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const = 0;
	virtual void								exportToCsv(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;
	virtual void								exportToSpreadsheet(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;

	void										setDatasetProperty(const GexAdvancedEnterpriseReportDataset * pDataset);

protected:

	GexAdvancedEnterpriseReportScriptEngine		m_aerScriptEngine;

	void										closeSection(const QString& strPPTName) const;

private:

	static int									m_nGlobalSectionID;
	int											m_nSectionID;
};


///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportGroup
//
// Description	:	Class which represents a group in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportGroup
{
public:

	GexAdvancedEnterpriseReportGroup(GexAdvancedEnterpriseReportDataset * pDataset);
	~GexAdvancedEnterpriseReportGroup();

	const QString&									name() const												{ return m_strName; }
	int												groupID() const												{ return m_nGroupID; }

	void											setName(const QString& strName)								{ m_strName = strName; }
	void											setGroupID(int nGroupID)									{ m_nGroupID = nGroupID; }
	void											setBreakPage(int nBreakPage)								{ m_nBreakPage = nBreakPage; }
	void											setPrintWhen(const QString& strJSPrintWhen)					{ m_strJSPrintWhen = strJSPrintWhen; }

	void											addException(const QString& strName, const QString& strJSExpression);
	void											addSection(GexAdvancedEnterpriseReportSection * pSection);
	void											addOrder(const QString& strOrder);

	const GexAdvancedEnterpriseReportDataset *		dataset() const												{ return m_pDataset; }
	const QStringList&								orders() const												{ return m_lstOrder; }

	void											exportToHtml(QTextStream& txtStream);
	bool											exportToCSV(QTextStream& txtStream, int nSectionID, int nDatasetID);
	bool											exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID);

protected:

	void											manageException(const GexDbPluginERDatasetGroup& datasetGroup);

private:

	int												m_nGroupID;
	int												m_nBreakPage;
	QString											m_strName;
	QString											m_strJSPrintWhen;
	QStringList										m_lstOrder;
	QMap<QString, QString>							m_mapException;
	QList<GexAdvancedEnterpriseReportSection *>		m_lstSection;
	GexAdvancedEnterpriseReportDataset *			m_pDataset;
	GexAdvancedEnterpriseReportScriptEngine			m_aerScriptEngine;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReport
//
// Description	:	Class which contains information about a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReport
{
public:

	GexAdvancedEnterpriseReport();
	~GexAdvancedEnterpriseReport();

	const QString&									name() const										{ return m_strName; }
	QString											htmlName() const;

	void											setName(const QString& strName)						{ m_strName = strName; }
	void											setFrontPageText(const QString& strFrontPageTest)	{ m_strFrontPageText = strFrontPageTest; }
	void											setFrontPageImage(const QString& strFrontPageImage)	{ m_strFrontPageImage = strFrontPageImage; }

	GexAdvancedEnterpriseReportDataset *			addDataset(const GexDbPluginERDatasetSettings& m_settings);
	void											addGroup(GexAdvancedEnterpriseReportGroup * pGroup);
	void											addTitle(const QString& strTitle)					{ m_lstTitle.append(strTitle); }
	void											addSummary(const QString& strSummary)				{ m_lstSummary.append(strSummary); }
	void											addCustomSettings(const QString& strLabel, const QString& strValue);


    bool load(const QString& strAERFile);
    bool generate(QTextStream& txtStream);
    bool exportToCsv(QTextStream& txtStream, int nSectionID, int nDatasetID);
    bool exportToSpreadSheet(QTextStream& txtStream, int nSectionID, int nDatasetID);

protected:

	void											writeHeader(QTextStream& txtStream);
	void											writeSettings(QTextStream& txtStream);
	void											writeMinimalSettings(QTextStream& txtStream);
	void											requestData();

private:

	enum dataStatus
	{
		dataNotRequested	= 0,
		dataRequested		= 1,
		dataError
	};

	dataStatus										m_eDataStatus;
	int												m_nRequestTime;
	QString											m_strErrorMsg;
	QString											m_strName;
	QString											m_strFrontPageText;
	QString											m_strFrontPageImage;
	QList<GexAdvancedEnterpriseReportGroup *>		m_lstGroup;
	QList<GexAdvancedEnterpriseReportDataset *>		m_lstDataset;
	QStringList										m_lstTitle;
	QStringList										m_lstSummary;
	QList<PrivateCustomSettings *>					m_lstCustomSettings;

};

#endif // GEX_ADVANCED_ENTERPRISE_REPORT_H
