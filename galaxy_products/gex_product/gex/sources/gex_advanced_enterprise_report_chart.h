#ifndef GEX_ADVANCED_ENTERPRISE_REPORT_CHART_H
#define GEX_ADVANCED_ENTERPRISE_REPORT_CHART_H

#include "gex_advanced_enterprise_report.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartSerie
//
// Description	:	Class which represents a serie in a chart
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportChartSerie
{
public:

    GexAdvancedEnterpriseReportChartSerie();
    ~GexAdvancedEnterpriseReportChartSerie();

    const QString&		name() const									{ return m_strName; }
    const QString&		value() const									{ return m_strValue; }
    const QString&		printWhen() const								{ return m_strPrintWhen; }
    const QString&		printDataWhen() const							{ return m_strJSPrintDataWhen; }

    void				setName(const QString& strName)					{ m_strName = strName; }
    void				setValue(const QString& strValue)				{ m_strValue = strValue; }
    void				setPrintWhen(const QString& strPrintWhen)		{ m_strPrintWhen = strPrintWhen; }
    void				setPrintDataWhen(const QString& strJSExpression){ m_strJSPrintDataWhen = strJSExpression; }

private:

    QString				m_strName;
    QString				m_strValue;
    QString				m_strPrintWhen;
    QString             m_strJSPrintDataWhen;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartYAxis
//
// Description	:	Class which represents the YAxis on a chart
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportChartYAxis
{
public:

    enum Type
    {
        TypeLine = 0,
        TypeBars,
        TypeStackedBars,
        TypeBars3D
    };

    enum Position
    {
        Left = 0,
        Right
    };

    enum ZOrder
    {
        NoOrder = -1,
        Front = 0,
        Back
    };

    GexAdvancedEnterpriseReportChartYAxis();
    ~GexAdvancedEnterpriseReportChartYAxis();

    const QList<GexAdvancedEnterpriseReportChartSerie *>&	series() const									{ return m_lstSerie; }
    const QString&											name() const									{ return m_strName; }
    double													max() const										{ return m_dMax; }
    double													min() const										{ return m_dMin; }
    ZOrder													zOrder() const									{ return m_eZOrder; }
    Position												pos() const										{ return m_ePos; }
    Type													type() const									{ return m_eType; }

    void													setName(const QString& strName)					{ m_strName = strName; }
    void													setMax(double dMax)								{ m_dMax = dMax; }
    void													setMin(double dMin)								{ m_dMin = dMin; }
    void													setPos(Position ePos)							{ m_ePos = ePos; }
    void													setType(Type eType)								{ m_eType = eType; }
    void													setZOrder(ZOrder eZOrder)						{ m_eZOrder = eZOrder; }

    void													addSerie(GexAdvancedEnterpriseReportChartSerie * pSerie);

private:

    QString													m_strName;
    double													m_dMax;
    double													m_dMin;
    ZOrder													m_eZOrder;
    Position												m_ePos;
    Type													m_eType;
    QList<GexAdvancedEnterpriseReportChartSerie *>			m_lstSerie;

};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChartXAxis
//
// Description	:	Class which represents the X-Axis on a chart
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportChartXAxis
{
public:

    enum Position
    {
        Bottom = 0,
        Top
    };

    GexAdvancedEnterpriseReportChartXAxis();
    ~GexAdvancedEnterpriseReportChartXAxis();

    int														limit() const									{ return m_nLimit; }
    int                                                     maxLabelSize() const                            { return mMaxLabelSize; }
    const QString&											name() const									{ return m_strName; }
    const QString&											expressionLabel() const							{ return m_strExpressionLabel; }
    Position												pos() const										{ return m_ePos; }

    void                                                    setMaxLabelSize(int lSize)                      { mMaxLabelSize = lSize; }
    void													setLimit(int nLimit)							{ m_nLimit = nLimit; }
    void													setName(const QString& strName)					{ m_strName = strName; }
    void													setExpressionLabel(const QString& strExpression){ m_strExpressionLabel = strExpression; }
    void													setPos(Position ePos)							{ m_ePos = ePos; }

private:

    int														m_nLimit;
    QString													m_strName;
    QString													m_strExpressionLabel;
    Position												m_ePos;
    int                                                     mMaxLabelSize;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	GexAdvancedEnterpriseReportChart
//
// Description	:	Class which represents a chart in a report
//
///////////////////////////////////////////////////////////////////////////////////
class GexAdvancedEnterpriseReportChart : public GexAdvancedEnterpriseReportSection
{
public:

    GexAdvancedEnterpriseReportChart();
    ~GexAdvancedEnterpriseReportChart();

    enum LegendMode
    {
        LegendOff	= 0,
        LegendOn	= 1,
        LegendAuto	= 2
    };

    void													addYAxis(GexAdvancedEnterpriseReportChartYAxis * pYAxis);
    void													setXAxis(GexAdvancedEnterpriseReportChartXAxis * pXAxis);
    void													setLegendMode(const QString& strLegendMode);

    void													exportToHtml(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const;
//	void													exportToCsv(QTextStream& txtStream, const GexDbPluginERDatasetGroup& datasetGroup) const {};

protected:

    void													fillSerieColor(QVector<QColor>& vecColor, const GexDbPluginERDatasetGroup& datasetGroup, bool bDrab) const;

private:

    LegendMode												m_eLegendMode;
    QString													m_strXAxisLabel;
    QMap<int, GexAdvancedEnterpriseReportChartYAxis *>		m_mapYAxis;
    GexAdvancedEnterpriseReportChartXAxis *					m_pXAxis;
};

#endif // GEX_ADVANCED_ENTERPRISE_REPORT_CHART_H
