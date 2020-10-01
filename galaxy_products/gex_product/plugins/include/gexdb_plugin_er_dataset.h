#ifndef GEXDB_PLUGIN_ER_RESULTS_H
#define GEXDB_PLUGIN_ER_RESULTS_H

#include <QStringList>
#include <QHash>
#include <QList>
#include <QDate>
#include <QMap>
#include <QSet>
#include <QVariant>

class GexDbPluginERDatasetField
{
public:

	GexDbPluginERDatasetField(const QString& strName);
	GexDbPluginERDatasetField(const GexDbPluginERDatasetField& erField);
	~GexDbPluginERDatasetField();

	const QString&				name() const						{ return m_strName; }
	const QVariant&				value() const						{ return m_varValue; }

	void						setValue(const QVariant& varValue)	{ m_varValue = varValue; }

	GexDbPluginERDatasetField&	operator=(const GexDbPluginERDatasetField& erField);
	bool						operator==(const GexDbPluginERDatasetField& erField) const;

	bool						match(const QString& strOperator, const QString& strValue) const;

private:

	QString						m_strName;
	QVariant					m_varValue;
};

class GexDbPluginERDatasetRow
{
public:

	enum Type
	{
		RowEmpty = 0,
		RowValid
	};

	enum BinGroupBy
	{
		BinGroupByNone		= 0x00,
		BinGroupByGroup		= 0x01,
		BinGroupBySerie		= 0x02,
		BinGroupByAggregate	= 0x04
	};

	GexDbPluginERDatasetRow(Type eType = RowValid);
	virtual ~GexDbPluginERDatasetRow();

	const QSet<GexDbPluginERDatasetField>& fields() const													{ return m_setField; }
	const GexDbPluginERDatasetField		field(const QString& strFieldName) const;
	Type								type() const														{ return m_eType; }
	bool								hasBinGroupBy() const												{ return m_eBinGroupBy !=  BinGroupByNone; }
	BinGroupBy							binGroupBy() const													{ return m_eBinGroupBy; }
	unsigned int						startTime() const													{ return m_uiStartTime; }
	unsigned int						volume() const														{ return m_uiVolume; }
	unsigned int						binVolume() const													{ return m_uiBinVolume; }
	unsigned int						goodVolume() const													{ return m_uiGoodVolume; }
	unsigned int						grossVolume() const													{ return m_uiGrossVolume; }
	unsigned int						waferCount() const													{ return m_uiWaferCount; }
	double								yield() const														{ return (m_uiVolume > 0) ? ((double)m_uiGoodVolume / (double) m_uiVolume) * 100.0 : 0; }
	double								binYield() const													{ return (m_uiVolume > 0) ? ((double)m_uiBinVolume / (double) m_uiVolume) * 100.0 : 0; }
	double								grossYield() const													{ return (m_uiGrossVolume > 0) ? ((double)m_uiGoodVolume / (double) m_uiGrossVolume) * 100.0 : 0; }
	double								grossBinYield() const												{ return (m_uiGrossVolume > 0) ? ((double)m_uiBinVolume / (double) m_uiGrossVolume) * 100.0 : 0; }
	QString								group() const; //														{ return m_strGroup; }
	QString								serie() const; //														{ return m_strSerie; }
	QString								aggregate() const; //													{ return m_strAggregate; }


	void								setField(const GexDbPluginERDatasetField& field);
	void								setField(const QString& strName, const QVariant& varValue);
	void								setBinGroupBy(BinGroupBy eBinGroupBy)								{ m_eBinGroupBy		= eBinGroupBy; }
	void								setStartTime(unsigned int uiStartTime)								{ m_uiStartTime		= uiStartTime; }
	void								setVolume(unsigned int uiVolume)									{ m_uiVolume		= uiVolume; }
	void								setGoodVolume(unsigned int uiGoodVolume)							{ m_uiGoodVolume	= uiGoodVolume; }
	void								setBinVolume(unsigned int uiBinVolume)								{ m_uiBinVolume		= uiBinVolume; }
	void								setGrossVolume(unsigned int uiGrossVolume)							{ m_uiGrossVolume	= uiGrossVolume; }
	void								setWaferCount(unsigned int uiWaferCount)							{ m_uiWaferCount	= uiWaferCount; }
	void								setGroup(const QString& strGroup); //									{ m_strGroup		= strGroup; }
	void								setSerie(const QString& strSerie); //									{ m_strSerie		= strSerie; }
	void								setAggregate(const QString& strAggregate); //							{ m_strAggregate	= strAggregate; }

	virtual void						clean();

	bool								operator<(const GexDbPluginERDatasetRow& datasetRow) const;

private:

	QSet<GexDbPluginERDatasetField>		m_setField;
	Type								m_eType;
	BinGroupBy							m_eBinGroupBy;
	unsigned int						m_uiStartTime;
	unsigned int						m_uiVolume;
	unsigned int						m_uiBinVolume;
	unsigned int						m_uiGoodVolume;
	unsigned int						m_uiGrossVolume;
	unsigned int						m_uiWaferCount;
//	QString								m_strGroup;
//	QString								m_strSerie;
//	QString								m_strAggregate;
};

class GexDbPluginERDatasetSerie : public GexDbPluginERDatasetRow
{
public:

	GexDbPluginERDatasetSerie();
	virtual ~GexDbPluginERDatasetSerie();

	int												indexOfAggregate(const QString& strAggregate) const;
	const GexDbPluginERDatasetRow&					aggregateAt(int nIndex) const;
	const QList<GexDbPluginERDatasetRow>&			aggregates() const									{ return m_lstAggregate; }

	virtual bool									addAggregate(const GexDbPluginERDatasetRow& resultAggregate);
	void											clean();

	bool											operator==(const GexDbPluginERDatasetSerie& resultSerie) const;
	bool											operator!=(const GexDbPluginERDatasetSerie& resultSerie) const;

protected:

	QList<GexDbPluginERDatasetRow>					m_lstAggregate;
	QHash<QString, int>								m_hashAggregatePosition;
};

class GexDbPluginERDatasetGroup : public GexDbPluginERDatasetSerie
{
public:

	GexDbPluginERDatasetGroup();
	~GexDbPluginERDatasetGroup();

	int										indexOfSerie(const QString& strSerie) const;
	const GexDbPluginERDatasetSerie&		serieAt(int nIndex) const;
	int										groupId() const				{ return m_nGroupID; }
	const QList<GexDbPluginERDatasetSerie>&	series() const				{ return m_lstSerie; }
	const QList<GexDbPluginERDatasetRow>&	rows() const				{ return m_lstRow; }
	const QList<QString>&					aggregateValues() const		{ return m_lstAggregateValue; }

	void									setGroupId(int nGroupId)	{ m_nGroupID = nGroupId; }

	bool									operator==(const GexDbPluginERDatasetGroup& resultGroup) const;
	bool									operator!=(const GexDbPluginERDatasetGroup& resultGroup) const;

	void									addRow(const GexDbPluginERDatasetRow& rowResult);
	void									addRow(const QString& strEmptyAggregate);
	void									clean();
	void									sort(const QStringList& lstKeyOrder = QStringList());
	void									executeTopNFailBin(int nTopN);
	const GexDbPluginERDatasetSerie&		worstSerie() const;
	const GexDbPluginERDatasetSerie&		bestSerie() const;

protected:

	bool									addAggregate(const GexDbPluginERDatasetRow& resultAggregate);
	void									pushAggregateValue(const QString& strAggregateValue);
	void									groupRowBy(const QStringList& strlstField);

private:

	friend class GexDbPluginERPostProcessing;

	int										m_nGroupID;
	QList<GexDbPluginERDatasetRow>			m_lstRow;
	QList<GexDbPluginERDatasetSerie>		m_lstSerie;
	QList<QString>							m_lstAggregateValue;
	QList<QString>							m_lstSerieValue;
	QHash<QString, int>						m_hashAggregateSerie;
};

class GexDbPluginERPostProcessingCriteria
{
public:

	QString		m_strField;
	QString		m_strOperator;
	QString		m_strValue;
};

class GexDbPluginERPostProcessing
{
public:

	GexDbPluginERPostProcessing();
	GexDbPluginERPostProcessing(const GexDbPluginERPostProcessing& other);

	bool			execute(GexDbPluginERDatasetGroup& datasetGroup) const;

	void			setTopNCount(uint uiTopNCount)					{ m_uiTopNCount = uiTopNCount; }
	void			setOtherName(const QString& strOtherName)		{ m_strOtherName = strOtherName; }
	void			addField(const QString& strField, const QString& strOrder);
	void			addCriteria(const GexDbPluginERPostProcessingCriteria& criteria);

	GexDbPluginERPostProcessing& operator=(const GexDbPluginERPostProcessing& other);

protected:

	uint										m_uiTopNCount;
	QString										m_strOtherName;
	QStringList									m_strlstFields;
	QList<GexDbPluginERPostProcessingCriteria>	m_lstCriteria;
};

class GexDbPluginERDatasetSettings
{
public:

	enum BinType
	{
		NoBin  = -1,
		HardBin,
		SoftBin
	};

	enum FilterOperator
	{
		OpEqual = 0,
		OpNotEqual,
		OpGreaterThan,
		OpLesserThan,
		OpGreaterThanOrEqual,
		OpLesserThanOrEqual
	};

	class GexDbPluginERDatasetPrivateFilter
	{
	public:

        GexDbPluginERDatasetPrivateFilter(const QString& strField, const QString& strValue,
                                          GexDbPluginERDatasetSettings::FilterOperator eOperator = GexDbPluginERDatasetSettings::OpEqual,
                                          QString strAdditional_data="");
		~GexDbPluginERDatasetPrivateFilter();

		const QString&		field() const			{ return m_strField; }
		const QString&		value() const			{ return m_strValue; }
        const QString&      AdditionalData() const    { return m_strAdditional_data; }
		FilterOperator		filterOperator() const	{ return m_eOperator; }
		QString				toString() const;

		bool				operator==(const GexDbPluginERDatasetPrivateFilter& privateFilter) const;

	private:

		QString			m_strField;
		QString			m_strValue;
		FilterOperator	m_eOperator;
        QString         m_strAdditional_data;
	};

	GexDbPluginERDatasetSettings();
	GexDbPluginERDatasetSettings(const GexDbPluginERDatasetSettings& datasetSettings);
	~GexDbPluginERDatasetSettings();

	const QString&										report() const								{ return m_strReport; }
	const QString&										name() const								{ return m_strName; }
	const QString&										database() const							{ return m_strDatabase; }
	const QString&										testingStage() const						{ return m_strTestingStage; }
	int													timePeriod() const							{ return m_nTimePeriod; }
	const QString&										timeStep() const							{ return m_strTimeStep; }
	long												timeNFactor() const							{ return m_lTimeNFactor; }
	BinType												binType() const								{ return m_eBinType; }
	const QDate&										dateFrom() const							{ return m_dtCalendarFrom; }
	const QDate&										dateTo() const								{ return m_dtCalendarTo; }
	const QStringList&									groups() const								{ return m_lstGroups; }
	const QStringList&									series() const								{ return m_lstSeries; }
	const QStringList&									aggregates() const							{ return m_lstAggregates; }
	const QStringList&									orders() const								{ return m_lstOrders; }
	const QList<GexDbPluginERDatasetPrivateFilter>&		filters() const								{ return m_lstFilters; }
	const QList<GexDbPluginERPostProcessing>&			postProcesses() const						{ return m_lstPostProcessing; }
	bool												overall() const								{ return m_bOverall; }
	bool												exportRawData() const						{ return m_bExportRawData; }
	bool												fullTimeLine() const						{ return m_bFullTimeLine; }
	uint												timestampFrom() const;
	uint												timestampTo() const;

	void												setFullTimeLine(bool bFullTimeLine)									{ m_bFullTimeLine = bFullTimeLine; }
	void												setReport(const QString& strReport)									{ m_strReport = strReport; }
	void												setName(const QString& strName)										{ m_strName = strName; }
	void												setDataBase(const QString& strDatabase)								{ m_strDatabase = strDatabase; }
	void												setTestingStage(const QString& strTestingStage)						{ m_strTestingStage = strTestingStage; }
	void												setTimePeriod(int nPeriod)											{ m_nTimePeriod = nPeriod; }
	void												setBinType(BinType eType)											{ m_eBinType = eType; }
	void												setDates(const QDate& dtFrom, const QDate& dtTo)					{ m_dtCalendarFrom = dtFrom; m_dtCalendarTo = dtTo; }
	void												setDates(const QString& strTimeSteps, long lTimeFactor);
	void												setOverall(bool bOverall)											{ m_bOverall = bOverall; }
	void												setExportRawData(bool bExport)										{ m_bExportRawData = bExport; }
	void												addGroup(const QString& strGroup)									{ m_lstGroups.append(strGroup); }
	void												addSerie(const QString& strSerie)									{ m_lstSeries.append(strSerie); }
	void												addAggregate(const QString& strAggregate)							{ m_lstAggregates.append(strAggregate); }
	void												addOrder(const QString& strOrder)									{ m_lstOrders.append(strOrder); }
	void												addPostProcessing(const GexDbPluginERPostProcessing& domElement)	{ m_lstPostProcessing.append(domElement); }
    void												addFilter(const QString& strField, const QString& strValue,
                                                                  GexDbPluginERDatasetSettings::FilterOperator eOperator = GexDbPluginERDatasetSettings::OpEqual,
                                                                  QString strAdditionalData = "");
	void												computeDateTimeConstraints(uint& uiTimestampFrom, uint& uiTimestampTo);

	bool												isSerieField(const QString& strField) const;
    bool												isGroupField(const QString& strField) const;
	bool												isFilteredField(const QString& strField, QString& strToString) const;
    bool                                                isOrderedField(const QString& strField, QString& orderDirection) const;
    bool                                                useIntermediateConsolidatedData() const;

    void                                                checkForIntermediateConsolidationSplit();
	bool												isEqual(const GexDbPluginERDatasetSettings& datasetSettings) const;

private:

	QString												m_strReport;
	QString												m_strName;
	QString												m_strDatabase;
	QString												m_strTestingStage;
	int													m_nTimePeriod;
	QString												m_strTimeStep;
	long												m_lTimeNFactor;
	BinType												m_eBinType;
	QDate												m_dtCalendarFrom;
	QDate												m_dtCalendarTo;
	QStringList											m_lstGroups;
	QStringList											m_lstSeries;
	QStringList											m_lstAggregates;
	QStringList											m_lstOrders;
	QList<GexDbPluginERDatasetPrivateFilter>			m_lstFilters;
	bool												m_bOverall;
	bool												m_bExportRawData;
	bool												m_bFullTimeLine;
	QList<GexDbPluginERPostProcessing>					m_lstPostProcessing;
};

class GexDbPluginERDataset
{
public:

	GexDbPluginERDataset();
	~GexDbPluginERDataset();

	const QList<GexDbPluginERDatasetGroup>&	groups() const										{ return m_lstGroup; }

	void									applyPostProcessing(const GexDbPluginERDatasetSettings& settings);
	void									addOrderBy(const QString& strOrderBy)				{ m_lstOrderBy.append(strOrderBy); }
	void									addRow(const GexDbPluginERDatasetRow& resultRow);
	void									addRow(const QString& strEmptyAggregate);

	void									sortGroups();

	int										currentGroup() const								{ return m_nCurrentGroup; }
	void									setCurrentGroup(const QString& strGroup);

private:

	QList<GexDbPluginERDatasetGroup>		m_lstGroup;
	QStringList								m_lstOrderBy;
	int										m_nCurrentGroup;
};

#endif // GEXDB_PLUGIN_ER_RESULTS_H
