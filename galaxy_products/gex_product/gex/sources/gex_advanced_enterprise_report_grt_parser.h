#ifndef GEX_ADVANCED_ENTERPRISE_REPORT_GRT_PARSER_H
#define GEX_ADVANCED_ENTERPRISE_REPORT_GRT_PARSER_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include "gex_advanced_enterprise_report_chart.h"
#include "gex_advanced_enterprise_report_table.h"

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QDomDocument>
#include <QMap>

class GexAbstractAdvancedEnterpriseReportGrtParser
{
public:

	GexAbstractAdvancedEnterpriseReportGrtParser();
  virtual ~GexAbstractAdvancedEnterpriseReportGrtParser();

	static bool		load(GexAdvancedEnterpriseReport& advEnterpriseReport, const QString& strFileName, QString& strErrorMsg);
	static bool		load(GexAdvancedEnterpriseReport& advEnterpriseReport, const QDomDocument& domDocument, QString& strErrorMsg);

protected:

	QMap<QString, GexDbPluginERDatasetSettings>	m_mapDatasetModel;

	virtual void	read(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport) = 0;

	void			readTitleSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
	void			readSettings(const QDomElement &domElement, GexAdvancedEnterpriseReport &advEnterpriseReport);
	void			readSummarySection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
	void			readChartSection(const QDomElement& domElement, GexAdvancedEnterpriseReportGroup& advEnterpriseReportGroup);
	void			readAxisSection(const QDomElement& domElement, GexAdvancedEnterpriseReportChart * pAdvEnterpriseReportChart);
	void			readSerieSection(const QDomElement& domElement, GexAdvancedEnterpriseReportChartYAxis * pAdvEnterpriseReportChartYAxis);
	void			readTableSection(const QDomElement& domElement, GexAdvancedEnterpriseReportGroup& advEnterpriseReportGroup);
	void			readTableVerticalSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableVertical *pAdvEnterpriseReportTable);
	void			readTableCustomSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable);
	void			readTableRowSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTableCustom * pAdvEnterpriseReportTable);
	void			readTableAlarmSection(const QDomElement& domElement, GexAdvancedEnterpriseReportTable * pAdvEnterpriseReportTable);
};

class GexAdvancedEnterpriseReportGrtParserV1 : public GexAbstractAdvancedEnterpriseReportGrtParser
{
public:

	GexAdvancedEnterpriseReportGrtParserV1();
	~GexAdvancedEnterpriseReportGrtParserV1();

	void			read(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);

private:

	void			readDatasetSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
	void			readDatasetFiltersSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
	void			readGroupSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
};

class GexAdvancedEnterpriseReportGrtParserV2 : public GexAbstractAdvancedEnterpriseReportGrtParser
{
public:

	GexAdvancedEnterpriseReportGrtParserV2();
	~GexAdvancedEnterpriseReportGrtParserV2();

	void			read(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);

protected:

	typedef QMap<int, QString> tdField;
	QMap<QString, tdField>	m_mapRole;

private:

	void			readDatasetSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
	void			readDatasetRolesSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
	void			readDatasetPostProcessingSection(const QDomElement& domElement, GexDbPluginERDatasetSettings& datasetSettings);
	void			readGroupSection(const QDomElement& domElement, GexAdvancedEnterpriseReport& advEnterpriseReport);
};

#endif // GEX_ADVANCED_ENTERPRISE_REPORT_GRT_PARSER_H
