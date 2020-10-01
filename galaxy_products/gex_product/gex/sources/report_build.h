/******************************************************************************!
 * \file report_build.h
 ******************************************************************************/
#if ! defined(REPORT_CLASSES_H__INCLUDED_)
#define REPORT_CLASSES_H__INCLUDED_
#include <time.h>

#include <QTableWidget>
#include <QScriptEngine>
#include <QHash>
#include <QList>
#include <chartdir.h>

#include "chtmlpng.h"
#include "scripting_io.h"
#include "classes.h"
#include "db_transactions.h"
#include "db_external_database.h"
#include "db_gexdatabasequery.h"
#include "interactive_charts.h"  // Layer Style class
#include "report_classes_sorting.h"
#include "ctest.h"
#include "cstats.h"
#include "gex_constants.h"
#include "gexdatasetconfig.h"
#include "test_list_pearson.h"
#include "wafermap.h"

class CReportOptions;
class GP_SiteDescription;
class GP_SiteDescriptionMap;
class GP_DataResult;
class GP_ParameterDef;
class GP_ParameterStats;
class CGexTestChartingOptions;  // ctest_chart_options.h
class CChartDirector;
class qtTestListStatistics;  // Sorting object for tests
class GexAdvancedEnterpriseReport;

// Different sections of report generated
#define SECTION_MIR 10
#define SECTION_STATS 20
#define SECTION_HISTO 30
#define SECTION_WAFMAP 40
#define SECTION_SOFTBIN 50
#define SECTION_HARDBIN 60
#define SECTION_PARETO 70
#define SECTION_ADVANCED 80
// For HTML page generation only
#define SECTION_ADV_HISTO 90
#define SECTION_ADV_TREND 91
#define SECTION_ADV_SCATTER 92
#define SECTION_ADV_BOXPLOT 93
#define SECTION_ADV_DATALOG 94
#define SECTION_ADV_BOXPLOT_EX 95
#define SECTION_ADV_PROBABILITY_PLOT 96
#define SECTION_ADV_MULTICHART 97
#define SECTION_ADV_HISTOFUNCTIONAL 98

/******************************************************************************!
 * \class CGageWarning
 * \brief This class holds an expression and it's color if expression is
 *        verified (eg: If Cpk > 1 and R_R > 45 Then Color = Yellow)
 ******************************************************************************/
class CGageWarning
{
public:

    CGageWarning();

    const QString& expression() const { return m_strExpression; }
    const QColor& color() const { return m_clrColor; }
    int fileLine() const { return m_nFileLine; }
    bool syntaxError() const { return m_bSyntaxError; }

    void setExpression(const QString& strExpression) {
        m_strExpression = strExpression;
    }
    void setColor(const QColor& clrColor) { m_clrColor = clrColor; }
    void setFileLine(int nFileLine) { m_nFileLine = nFileLine; }
    void setSyntaxError(bool bError) { m_bSyntaxError = bError; }

private:
    /*!
     * \var m_strExpression
     * \brief Expression
     */
    QString m_strExpression;
    /*!
     * \var m_clrColor
     * \brief Color assigned if expression is true
     */
    QColor m_clrColor;
    /*!
     * \var m_nFileLine
     * \brief Line# of this expression in the source file
     */
    int m_nFileLine;
    /*!
     * \var m_bSyntaxError
     * \brief 'true' if invalid expression, incorrect syntax detected
     */
    bool m_bSyntaxError;
};

#define GEX_MAX_STRING 257  // Max. String length in STDF file

/******************************************************************************!
 * \class CTestCustomize
 * \brief Test with custom Limits or marker definition
 *        (thru scripting interface) for: GuardBanding simulation, etc.
 ******************************************************************************/
class CTestCustomize
{
public:
    CTestCustomize();
    ~CTestCustomize();

    /*!
     * \var lTestNumber
     */
    unsigned int lTestNumber;
    /*!
     * \var strTestName
     */
    QString strTestName;
    /*!
     * \var lPinmapIndex
     * \brief =GEX_PTEST (-1) for standard Parametric test
     */
    int lPinmapIndex;
    /*!
     * \var lfLowLimit
     */
    double lfLowLimit;
    /*!
     * \var lfHighLimit
     */
    double lfHighLimit;
    /*!
     * \var bLimitWhatIfFlag
     * \brief bit0=1 (no low limit forced), bit1=1 (no high limit forced)
     */
    BYTE bLimitWhatIfFlag;
    /*!
     * \var ptMarker
     * \brief Parameter marker
     */
    TestMarker* ptMarker;
    /*!
     * \var ptChartOptions
     * \brief Charting options: X & Y viewport size
     */
    CGexTestChartingOptions* ptChartOptions;
};

/******************************************************************************!
 * \class CBinningColor
 * \brief This class used when building wafer map: hold top10 colors
 ******************************************************************************/
class CBinningColor
{
public:

    CBinningColor();

    QString GetBinColorImageName(const QColor& cColor);
    QString GetBinNumberImageName(int iBinNumber, bool bSoftBin = true);
    QString GetBinColorHtmlCode(int iBinningValue, bool bSoftBin = true);
    QColor GetWafmapDieColor(int iBinningValue, bool bSoftBin = true);

    QColor assignBinColor(int nBinNumber, bool bPassBin);

    const QMap<int, QColor>& mapDefaultBinColors() const {
        return m_mapBinColors;
    }

private:

    QList<QColor> m_lstPassColor;
    QList<QColor> m_lstFailColor;

    QMap<int, QColor> m_mapBinColors;

    /*!
     * \var m_colorBinOne
     * \brief Color reserved for bin 1
     */
    static QColor m_colorBinOne;
    /*!
     * \var m_colorDefault
     * \brief Default color to assign to the extra bins
     */
    static QColor m_colorDefault;
};

/******************************************************************************!
 * \class CPinmap
 * \brief Pinmap cell
 ******************************************************************************/
class CPinmap
{
public:
    CPinmap();
    int iPinmapIndex;
    QString strChannelName;
    QString strPhysicName;
    QString strLogicName;
    BYTE bHead;
    BYTE m_site;
    CPinmap* ptNextPinmap;

private:
    // FIXME: please develop copy constructor and operator= to remove this macro
    Q_DISABLE_COPY(CPinmap)
};

// Flag to tell testing site failed a test
#define GEX_RUN_FAIL_TEST 1
// Flag to tell testing site failed a 'What-if' test (failed its new limits)
#define GEX_RUN_FAIL_WHATIFTEST 2

typedef QMap<int, int> CTestNumberMap;
typedef QMap<QString, QString> CTestNameMap;

/******************************************************************************!
 * \class QZonalRegion
 * \brief Class used for computing the 12 zones of Zonal Wafermaps
 ******************************************************************************/
class QZonalRegion
{
public:
    QZonalRegion();  // Constructor
    int iTotalParts;  // total parts in zone
    int iTotalMatch;  // Total parts matching bin we focus on
};

#endif
