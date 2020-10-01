#ifndef REPORT_OPTIONS_H
#define REPORT_OPTIONS_H

#include <QString>
#include <QList>
#include <QColor>
#include <QMap>

#include "gstdl_type.h" /// for typedef BYTE...
#include "report_options_type_definition.hpp"
#include "gex_options_handler.h"

class CGexTestRange;
class CGexSingleChart;
class CBinColor;

namespace GS
{
    namespace QtLib
    {
        class Range;
    }

    namespace Gex
    {
        class CharacLineChartTemplate;
        class CharacBoxWhiskerTemplate;
    }
}

/// TO REVIEW
//class CReportOptionsProcessPrivate;


///////////////////////////////////////////////////////////
// This class holds all report settings information
///////////////////////////////////////////////////////////
class CReportOptions : public QObject
{
    Q_OBJECT

    static int								s_nInstances;

    // map containing the desired reports as requested by gexReportType in csl
    // key is the report keyword : 'histogram', 'stats', 'wafer', 'adv_my_report', 'adv_shift', ...
    // value of the map is the list of parameters/options : 'disabled', 'all', ...
    QMap<QString, QVector<QString> > mReportsMap;

public:

    // add a new entry in mReportsMap
    // key is the gexReportType keyword : 'histogram', 'stats', 'wafer', 'adv_my_report', 'adv_shift', ...
    // params are the options given by csl functions, 'all',...
    bool addReportUnit(QString key, QVector<QString> params);

    // get the list of ReportUnit
    const QMap<QString, QVector<QString> >& GetReportUnits() { return  mReportsMap; }

    static int GetNumberOfInstances() { return s_nInstances; }

    CReportOptions();		// Constructor
    CReportOptions(const CReportOptions& other);
    ~CReportOptions();

    CReportOptions& operator=(const CReportOptions& other);

    void	Reset(bool bDefault);					// Resets or Clears options
    void	keepInteractive(CReportOptions *ptTo);	// Exports Options related to interactive pages (markers, style, etc...)

    // create the default RnR rules .txt file
    bool	CreateDefaultRnRRulesFile();

    GS::Gex::OptionsHandler&	GetOptionsHandler();
    const QString&              GetPATTraceabilityFile() const;

    QString				GetLocalDatabaseFolder();
    QString				GetServerDatabaseFolder(bool bUseLocalDatabaseFolderIfNotDefined=false);

    // Set the option to the given value. If the option does not exist, it will be added to the map
    bool				SetOption(const QString section, const QString option, const QString value);
    // Set the specific option of a flag option to true / false depending on bToSetOn argument value
    bool				SetSpecificFlagOption(const QString strSection, const QString strField, const QString strValue, const bool bToSetOn);

    void                SetPATTraceabilityFile(const QString& lPatTraceability);

    // Option map accessors				; PYC, 17/05/2011
    //	QMap<QString, QMap<QString, QString > >		GetOptionsMap();

    // Write all registered options to given csl file : 'gexOptions(....); gexOptions(....)...'
    bool WriteOptionSectionToFile(FILE *hFile);		// write all the csl option section
    bool WriteChartStyleToFile(FILE* hFile);			// Write chart style definition
    bool WriteBinColorsToFile(FILE* hFile);			// Write bin colors definition

    // Refresh OptionsCenter widget with current options
    bool RefreshOptionsCenter();

    // Check if report output is HTML based
    // Returns true if report output is HTML based (first generate a HTML report, then eventually convert it):
    // HTML, WORD, PDF, PPT, INTERACTIVEONLY, ODT
    bool isReportOutputHtmlBased();

    // Get the num of seconds before autoclose. Mainly used in OnCheckForScriptCompleted()
    int	GetTimeBeforeAutoClose(QString value="");

    //_______________________________________________________________________________________________________

    // Number of groups and files: computed at runtime
    int	iGroups;				// 1 if one group, 2 if 3 groups (compare), etc...
    int	iFiles;					// Total number of files to analyse

    // Part-Bin Filter
    GS::QtLib::Range*		pGexRangeList;			// List of Ranges to process (parts/bins,...)

    // Monitoring

    // Output section
    //	List of HTML sections NOT to create (used when multi-pass involved such as for 'guard-banding'
    int		iHtmlSectionsToSkip;
    QString	strReportDirectory;		// Home of report file to generate.
    QString	strReportTitle;			// Report title string.
    QString	strReportNormalizedTitle;	// Report title with non alphanumerical characters changed to '_'(sub-folder name)

    // Build report
    bool	bQueryDataset;			// true if report results from running Queries. false if results from Files analysis.

    // Speed Optimization
    bool		mComputeAdvancedTest;				// 'true' if don't compute advanced stats, false otherwsie.
    // Speed Optimization processing attributes
    bool		bSpeedCollectSamples;				// 'true' if iSpeedCollectSamples condition is true, false otherwsie.
    bool		bSpeedUseSummaryDB;					// 'true' if iSpeedUseSummaryDB condition is true, false otherwsie.

    // Test statistics section
    int		iStatsType;				// disabled, or type of report to generate
    CGexTestRange *pGexStatsRangeList;	// List of Test Ranges to process
    double	lfStatsLimit;			// Cp,Cpk limit.
    bool	bStatsTableSuggestLim;	// true if table shows suggested limits for 100% yield

    // Wafermap section
    bool	bWafmapMirrorX;						// Mirror X axis
    bool	bWafmapMirrorY;						// Mirror Y axis

    int		iWafermapType;						// disabled, or type of report to generate
    int		iWafermapTests;						// 0 = wafermap for  all tests, 1 = only specific list 2 = top n failtests
    int		iWafermapNumberOfTests;				//
    CGexTestRange *pGexWafermapRangeList;		// Wafermap report: holds list of parameters or binnings

    // Standard Histogram section
    bool	bForceAdvancedHisto;				// if set to 'true', forces the standard Histogram section to plot the advanced histogram instead (buiding full plot instead of using predefined bars). Typically use if need to show custom markers!
    int		iHistogramType;						// disabled or type of report to generate
    int		iHistogramTests;					// 0=chart all tests, 1=only specific list, 2=top N failtests
    int		iHistogramNumberOfTests;			// top N
    CGexTestRange *pGexHistoRangeList;			// List of Test Ranges to process

    // For all Advanced+datalog charts:
protected:
    int		iAdvancedReport;				// Type of Advanced report to generate
    int		iAdvancedReportSettings;		// options relative to the report (all tests, single test,...)
public:
    void setAdvancedReport(int iReport);
    int getAdvancedReport() const ;

    void setAdvancedReportSettings(int iReportSettings );
    int getAdvancedReportSettings() const ;

    bool	bAdvancedUsingSoftBin;			// Set to 'true' if need to operate advanced report over soft-bin. (false=hard_bin)
    QString	strAdvancedTestList;			// List of tests used for the advanced reports (string list as entered by the end-user under the GUI)
    CGexTestRange *pGexAdvancedRangeList;	// List of Test Ranges to process (conversion of the string list entered by end-user into a list of ranges)
    long	lAdvancedHtmlPages;				// Total number of Advanced HTML pages to create.
    long	lAdvancedHtmlLinesInPage;		// Dynamic changed: holds number of lines written in current HTML page (used for Datalog)

    // adv_histogram	marker
    bool	bHistoMarkerMin;		// Marker on chart: show Min
    bool	bHistoMarkerMax;		// Marker on chart: show Max

    // Advanced Trend
    bool	bTrendMarkerMin;		// Marker on chart: show Min
    bool	bTrendMarkerMax;		// Marker on chart: show Max

    // Advanced Correlation
    int		iScatterChartType;			// Type of chart: Lines, Spots, both
    bool	bScatterMarkerMin;			// Marker on chart: show Min
    bool	bScatterMarkerMax;			// Marker on chart: show Max

    // Advanced Probability plot
    int		iProbPlotChartType;			// Type of chart: Lines, Spots, both

    // adv_probabilityplot	marker
    bool	bProbPlotMarkerMin;			// Marker on chart: show Min
    bool	bProbPlotMarkerMax;			// Marker on chart: show Max

    // Advanced Box plot
    int		iBoxPlotExChartType;		// Type of chart: Lines, Spots, both

    // adv_boxplot_ex	marker
    bool	bBoxPlotExMarkerMin;		// Marker on chart: show Min
    bool	bBoxPlotExMarkerMax;		// Marker on chart: show Max

    // Production Yield reports
    QString	strProdReportTitle;		// Holds Chart title
    unsigned int	uiYieldAxis;	// Defines the viewport of the yield axis
    unsigned int	uiVolumeAxis;	// Defines the viewport of the volume axis
    unsigned int	uiVolumeHL;		// Custom High Limit for volume axis viewport
    unsigned int	uiYieldMarker;	// Red yield marker on the graph (0-100)

    // MyReport Template file
    QString	strTemplateFile;				// Path to template file to use for creating custom report (MyReport)

    // Report Center Template
    // Path to file to use for creating Report (from Reports Center)
    QString	strReportCenterTemplateFile;

    // For all Interactive Drill charts:
    int		iDrillReport;				// Type of Drilling done.

    // Background color for all charts
    QColor	cBkgColor;
    bool	bPlotLegend;				// Set to 'true' if overlay curve name on chart.
    int     mTextRotation;
    int     mTotalBars;
    int     mYScale;
    bool    mCustom;
    int     mTotalBarsCustom;
    bool    mPlotQQLine;

    // List of Layers style definitions (colors, line width, etc...)
    QList<CGexSingleChart*>		pLayersStyleList;

    // List of Soft & Hard bins, and color assigned to them.
    QList<CBinColor>	softBinColorList;
    QList<CBinColor>	hardBinColorList;
    QList<CBinColor>	siteColorList;
    bool				bUseCustomBinColors;	// set to true if user wants to use his own colors.*

    // Charac template
    GS::Gex::CharacLineChartTemplate *   mLineChartTemplate;
    GS::Gex::CharacBoxWhiskerTemplate *  mBoxWhiskerTemplate;

public slots:

    // request an option value. Return false if the option does not exist !
    QVariant			GetOption(const QString section,const QString option_name);
    QString             GetOptionType(const QString section,const QString option_name);

private:

    GS::Gex::OptionsHandler     mOptionsHandler;
    QString                     mPATTraceabilityFile;

    // internal method(s)
    void clearMemory();
};


#endif // REPORT_OPTIONS_H
