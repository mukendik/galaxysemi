#ifndef GEX_REPORT_H
#define GEX_REPORT_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>

#include <gqtl_utils.h>

#include "report_classes_sorting.h"
#include "gex_file_in_group.h"
#include "gex_undo_command.h"
#include "report_log_list.h"
#include "pat_nnr_summary.h"
#include "stdf_data_file.h"

class CGexChartOverlays;
class CReportOptions;
class GexDatabaseQuery;
class CPartInfo;
class CWafBinMismatchPareto;
class PATOptionReticle;
class CPatInfo;

namespace GS
{
    namespace Gex
    {
        class ReportUnit;
#ifdef GCORE15334
        class PATProcessing;
#endif
        class ReportTemplate;
        class ReportTemplateSection;
        class CustomReportTestAggregateSection;
        class CustomReportFileAuditSection;
    }
}


/////////////////////////////////////////////////////////////////////////////
// Failure Signatures in MPR tests.
/////////////////////////////////////////////////////////////////////////////
class CMprFailureVector
{
public:
    QString strVectorName;	// List of MPR tests failing
    long	lFailCount;		// Vector failing rate
};


struct HtmlTableItem
{
    QString mData;
    QString mAttribute;
    QString mAnchor;
};
typedef QList<HtmlTableItem> HtmlTableRow;


///////////////////////////////////////////////////////////
// This class is the top level class. Encapsulates all others.
///////////////////////////////////////////////////////////
class	CGexReport : public QObject
{
    Q_OBJECT

    long	lTestsInProgram;			// Total tests in program
    // report database : db storing all reports results (tests stats, bins, pearsons, shift aerts, ...)
    QString mReportDBName;
    // Id to identify this report : there are currently 1 or 2 GexReport in a gex session
    int mId;
    // spy
    static int sNumberOfIntances;

public slots:
    // Means that the script has been executed without errors and has not been aborted
    bool isCompleted() const;
    // Set the report status as complete.
    void setCompleted(bool completed);
    // reset all CGexReport attributes;
    bool reset();

    // Exec the query on the report DB
    // return "ok" or "error..."
    QString ExecQuery(const QString &query, QVector< QVector<QVariant> > &results);

    // Get number of groups
    int GetNumberOfGroup() { return pGroupsList.size(); }
    // return the section/option value if any
    QVariant GetOption(QString section, QString option);
    // Return report name without extension
    QString reportName();
    // Return report name with extension
    QString reportFileName();
    // Return flat HTML absolute path (only for doc,pdf,ppt)
    QString reportFlatHtmlAbsPath();
    // Return pages folder absolute path
    QString reportPagesFolderAbsPath();
    // Return images folder absolute path
    QString reportImagesFolderAbsPath();
    // Return report file absolute path
    QString reportAbsFilePath();
    // Return report file relative path (relative to the root folder)
    QString reportRelativeFilePath();
    // Return report folder absolute path
    QString reportFolderAbsPath();
    // Return report subfolders
    QString reportSubFolders();
    // Return report root folder absolute path
    QString reportRootFolderAbsPath();
    // Return report type, used to build default report name
    QString reportType();
    // Return report generation mode
    QString reportGenerationMode();
    // Return report output format
    QString	outputFormat();
    // Evaluate if needed report name, folder and subfolder
    QString	evaluateReportJsAttributes();
    // Return Maxmulti limits items
    int GetMaxMultiLimitItems() { return mMaxMLItems; }
    // Build report name + create necessary folders & copy files!
    void	overloadOutputFormat(QString &strReportType);
    // Add file into group
    int	addFile(int group, QString fileName, int processSite, int processBins, const char *rangeList,
                QString mapTests, QString waferToExtract="",
                double temperature=-1000, QString datasetName="", QString samplegroup="");
    // When alls groups relate to same dataset but filtered by site#, tell which group holds a given site
    int     getGroupForSite(int iSite);
    // When alls groups relate to same dataset but filtered by site#, tell which group holds a given die coordinates
    int		getSiteForDie(int iDieX,int iDieY);
    QString getDiePartId(int groupId, int coordX, int coordY);
    // Returns testing site used to test a given die location
    // Note: if iGroup < 0: scan all groups until site found
    int		getDieTestingSite(int iGroup,int iFile,int iDieX,int iDieY);
    // Return report format:based on file extension
    static QString		reportFormat(const QString &strReportFile);
    // Return String to write on report images.
    QString			GetImageCopyrightString(void);
    // Return Part process type : All parts, Good parts only, Failing parts only (all but Bin1), ...
    QString			GetPartFilterReportType(int processBins);
    // Returns the WaferMap data type (hard bin, soft bin, etc...)
    QString			GetWaferMapReportType(void);
    // Description : Give the Zone Nb in the Wafer
    int				getZoneIndex(int nXLoc, int nYLoc, int m_nXMax, int m_nYMax);

    // return true a test condition exist with this name
    bool    IsExistingTestConditions(const QString& testCondition);
    //returns the number of test conditions defined
    int     GetTestConditionsCount() const;
    // returns the Test Conditions name for the given index
    QString GetTestConditions(int index);
    // adds a new test conditions name to the report object
    void    AddTestConditions(const QString &strTestConditionName);
    // reset the Test conditions list
    void    ClearTestConditions();
    // Create wafermap image: simpler interface compatible with JS API
    // Returns "error..." or "ok"
    QString	CreateWaferMapImage(int eWafermapMode,
                                int GroupIndex,
                                int FileIndex,
                                const QString &WafermapImagePath);

public:

    enum virtualRetestParts
    {
        allParts = 0,
        goodParts,
        failedParts
    };

    enum wafermapMode
    {
        individualWafermap = 0,
        stackedWafermap,
        compareBinMismatch,
        compareBinToBinCorrelation,
        lowYieldPattern
    };

    // Define the repair option to use when auto repairing a stdf file
    enum autoRepairOption
    {
        repairUndefined = -1,
        repairTempFile = 0,			// Use a temporary file to write the repaired file
        repairReplaceFile,			// Replace the original file with the repaired one
        repairCreateFile			// Create a file prefixed by "gex_repaired"
    };

    //! \brief Constructor: top class to build report.
    CGexReport();
    virtual ~CGexReport();

    Q_INVOKABLE void setReportName(const QString& lReportName);
    // case 4044 {m_strLegacyReportAbsFilePath = strName;}
    Q_INVOKABLE void setLegacyReportName(const QString &strName);
    Q_INVOKABLE void setLegacyReportRootFolder(const QString &strRootFolder) {m_strLegacyReportRootFolder = strRootFolder;}
    //! \brief Build legacy monitoring report name
    QString	buildLegacyMonitoringReportName(QString &strReportName, const QString &strExtension);
    void	setJsReportFileName(const QString &strReportFileName)		{m_strJsReportName = strReportFileName;}
    void	setJsReportSubFolders(const QString &strReportSubFolders)	{m_strJsReportSubFolders = strReportSubFolders;}
    void	setJsReportRootFolderAbsPath(const QString &strReportFolder) {m_strJsReportRootFolderAbsPath = strReportFolder;}
    Q_INVOKABLE void setReportGenerationMode(const QString &strGenerationMode);

    // write to Reportfile if not null. Does not handle output format (HTML, CSV,...)
    Q_INVOKABLE bool WriteToReportFile(QString s);

    // Tool functions
    //! \brief Add global object to QScript engine
    bool	setQScriptEngineProperties();
    //! \brief Return evaluated file name if the JS expression is valid file name and error msg is empty,
    // else "" and error msg contains msg
    QString	jsToFileName(const QString &strJSExpression, QString& strErrorMsg);
    //! \brief Return evaluated sub folder if the JS expression is valid folder path and error msg is empty,
    // else "" and error msg contains msg
    QString	jsToSubFolders(const QString &strJSExpression, QString& strErrorMsg);
    //! \brief Return evaluated root folder path if the JS expression is valid folder path and error msg is empty,
    // else "" and error msg contains msg
    QString	jsToRootFolderPath(const QString &strJSExpression, QString& strErrorMsg);

    static QString outputLocation(CReportOptions	*pReportOptions);

    //! \brief Do prep-work before creating report (eg: load config files, etc...)
    Q_INVOKABLE void BuildReportPrepWork(void);
    //! \brief Launches report generation.
    QString	BuildReport(CReportOptions *);
    //! \brief Rebuild report on same data (no scan of data), simply rebuild report pages.
    Q_INVOKABLE void RebuildReport(void);
    //! \brief create report name + folders & copy images
    bool	legacyBuildReportFolderName(CReportOptions *pReportOptions, QString strReportType="");
    //! \brief Load default images into report's folder.
    bool	legacyLoadImagesReportFolder(CReportOptions *pReportOptions,QString strFirstFileName,
                                         bool bCreateReportFolder,bool bCreateReportFile,bool bAllowPopup);
    //! \brief Load default images into report's folder.
    bool	loadImagesReportFolder(CReportOptions *pReportOptions,bool bCreateReportFolder,
                                   bool bCreateReportFile, bool bAllowPopup);
    //! \brief Define report root folder, sub-folders, report file name
    bool	buildReportArborescence(CReportOptions *pReportOptions,QString strReportType="");
    //! \brief Converts HTML flat file to final format (Word, PowerPoint, PDF,...)
    //! \return "ok" or "error:..."
    QString	ReportPostProcessingConverter(void);

    bool	CreateDatabaseReportFolder(CReportOptions *pReportOptions,const QString &strReportName,QString &strReportType);
    //! \brief Return sum of valid execs over group list
    int static validSamplesExecsOverGroups(unsigned int testNumber, int pinmapIndex, QString testName,
                                           QList<CGexGroupOfFiles*> groupsList);
    // Create a new group
    int     addGroup(QString strGroupName, GexDatabaseQuery *pQuery=NULL);

    // If report created using a congfiguration file (eg: PAt-Man, then this is where to store it!)
    QString	strCustomConfigFile;
    // Exit error code when script completed.
    int	iScriptExitCode;
    int         CheckForTesterPlatform(char *szExecType,char *szExecVer,char *szTesterType,int iStdfAtrTesterBrand);
    const char *	CreateResultStringCpCrCpk(double lfValue);
    const char *	CreateResultStringPercent(double lfValue);
    const char *	CreateResultString(long lValue);

    // Writes a page break in document (flat HTML file)
    // this is a special html code not seeable in usual browser
    // if hFile null, will try to use hReportFile if not null
    // bOpenHtml = will rewrite or not the start of the html : <html><body>...
    // WARNING : if PPT/ODP, flush the html file and write 1 image, and then clear the file
    void	WritePageBreak(FILE *hFile=NULL, bool bOpenHtml =true);

    void			setReportFile(FILE *hHtmlReportFile);
    // Prepares the report section to be written (.CSV & .HTML)
    // Force handle used when creating reports...
    FILE*	getReportFile(void);
    void    setAdvancedReportFile(FILE *hHtmlAdvancedReportFile);
    FILE*   getAdvancedReportFile(void);
    // Prepares the report section to be written (.CSV & .HTML)
    void	WriteHeaderHTML(FILE *hHtmlReportFile,const char *szColor, const char *szBackground="#FFFFFF",
                            QString strHeadLines="",bool bReportFile=true,bool bWriteTOC=true);
    void	WriteTableOfContents_UserFlow(FILE *hHtmlReportFile,bool bWriteTOC=true);
    /*!
     * \fn WriteTableOfContents_Template
     */
    void WriteTableOfContents_Template(FILE* hHtmlReportFile,
                                       GS::Gex::ReportTemplate& reportTemplate);
    char*   HMSString(int lElapsedTime,int lMillisec,char *szString);
    void	BuildTestTypeString(CGexFileInGroup *pFile, CTest *ptTestCell, char *szString,BOOL bShortType);
    /*!
     * \fn BuildTestNumberString
     */
    void BuildTestNumberString(CTest* ptTestCell);
    void	BuildTestNameString(CGexFileInGroup *pFile, CTest *ptTestCell, char *szString);
    //! \brief Builds a string that include the Test name and PinmapIndex#:name if any (Multi-result param)
    void	BuildTestNameString(CGexFileInGroup *pFile, CTest *ptTestCell, QString& strString);
    void	BuildPinNameString(CGexFileInGroup *pFile,CTest *ptTestCell,QString & strPinName);
    //! \brief Build Image string name based on Test# and test name.
    QString	BuildImageUniqueName(QString strPrefix,CTest *ptTestCell=NULL,int iGroup=-1,
                                 QString strPattren="", int iPart=-1);
    void	ComputeDataStatistics(bool computeShape=false, bool bFilter=false);
    //! \brief Write Custom markers defined thru the scripting interface
    void	PlotScriptingCustomMarkersEx(CTest *ptTestCell,CGexFileInGroup *pFile,QColor &cLayerRGB,
                                         int iLayer,double fCustomScaleFactor,bool bVerticalMarker,
                                         double *plfChartBottom/*=NULL*/,double *plfChartTop/*=NULL*/);
    void	WriteAdvHistoChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestCell,
                                   int iChartSize);
    void	WriteAdvHistoFunctionalChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                             CTest *ptTestCell,int iChartSize);
    void	WriteAdvTrendChartPageAggregateEx(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                       CTest *ptTestReferenceCell,int iChartSize,bool bPluginCall,int iAggregateInfo);
    void	WriteAdvTrendChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,CTest *ptTestCell,
                                   int iChartSize,bool bPluginCall=false);
    //! \brief Create the advanced histogram image, and save it to disk
    void	CreateAdvHistoChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestReferenceCell, int iChartSize, bool bStandardHisto, int iChartType,QString strImage);
    void	CreateAdvHistoFunctionaChartImageEx(const QMap<uint,int> &oListHistoFuncTest,
                                                CGexChartOverlays *pChartsInfo,
                                                CTest *ptTestReferenceCell, int iChartSize, bool bStandardHisto,
                                                int iChartType,QString strImage,QString strPattern, int iPart);
    void	CreateAdvTrendChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestCell,	int iChartSize, QString strImage, bool bPluginCall=false);
    void	CreateAdvProbabilityPlotChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestReferenceCell, int iChartSize, bool bStandardHisto, int iChartType,QString strImage);
    void	CreateAdvBoxPlotChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestReferenceCell, int iChartSize, bool bStandardHisto, int iChartType,QString strImage);
    void	CreateAdvScatterChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestCellX, CTest *ptTestCellY, int iChartSize, QString strImage);
    void    CreateAdvCorrelationBoxWhiskerChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestCellX, CTest *ptTestCellY, int iChartSize, QString strImage);
    void	CreateAdvCharac1ChartImageEx(CGexChartOverlays *pChartsInfo,
                CTest *ptTestCell,	int iChartSize, QString strImage);
    //! \brief Writes Scatter statistics header in HTML page.
    void	WriteScatterHtmlTestInfo(CGexChartOverlays *pChartsInfo,CGexGroupOfFiles *pGroupX,
                                     CGexFileInGroup *pFileX,CTest *ptTestCellX,CGexGroupOfFiles *pGroupY,
                                     CGexFileInGroup *pFileY,CTest *ptTestCellY);
    void	WriteAdvScatterChartPage(CGexChartOverlays *pChartsInfo,CGexGroupOfFiles *pGroupX,
                                     CGexFileInGroup *pFileX,CTest *ptTestCellX,CGexGroupOfFiles *pGroupY,
                                     CGexFileInGroup *pFileY,CTest *ptTestCellY,int iChartNumber,int iChartSize);
    void	WriteAdvProbabilityPlotChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                             CTest *ptTestCell,int iChartSize);
    void	WriteAdvBoxplotExChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                   CTest *ptTestCell,int iChartSize);
    void	WriteAdvMultiChartPage(CGexChartOverlays *pChartsInfo,CGexFileInGroup *pFile,
                                   CTest *ptTestCell,int iChartSize);
    void	InteractiveBoxPlot(CGexChartOverlays *pChartsInfo);
    void	InteractiveProbabilityPlot(CGexChartOverlays *pChartsInfo);
    void	computeProbabilityPlotData(CGexChartOverlays *pChartsInfo);
    int		RemoveTableDataPoint(CTest *ptTestCell,int iIndex,bool bComputeStats=false);
    int		RemoveDataPoints(CGexChartOverlays *pChartsInfo, CGexGroupOfFiles *pGroup, CTest *ptTestCell,
                             double fLowLimit,double fHighLimit, int iRemoveMode,bool bForceStatsUpdate=false,
                             bool bRemoveAssociatedParts = false,
                             QList<TestRemoveResultCommand::TestRemovedResult  *> *poRunIdxRemoved =0);
    int     removeDataPointsScatter(const QList<QRectF> &rAreaToRemove,
                                    bool bForceStatsUpdate, bool bRemoveAssociatedParts);
    void	ListAdvTrendData(CGexChartOverlays *pChartsInfo, QTableWidget * pTableWidget,
                             CGexFileInGroup *pFile,CTest *ptTestCell);
    void	ListAdvScatterData(CGexChartOverlays *pChartsInfo, QTableWidget * pTableWidget,
                               CGexFileInGroup *pFile,CTest *ptTestCellX,CTest *ptTestCellY);
    void	UpdateSamplesRawData(CGexChartOverlays *pChartsInfo,QTableWidget * pTableWidget,
                                 CGexFileInGroup *pFile,CTest *ptTestReferenceCell);

    // Writes test table info (test name, limits, mean/sigma, shifts, etc..in HTML
    // page (or QString if specified in last argument)
    void	WriteHistoHtmlTestInfo(CGexChartOverlays *pChartsInfo, const char * szChartType,
                                   CGexFileInGroup *pFile, CTest *ptTestCell,
                                   CGexFileInGroup *pFileY, CTest *ptTestCellY, BOOL bAdvancedReport,
                                   const QString& strImageName = QString(), int iSize=0, int bIsAdvancedFunc=false);

    // Writes test table info (test name, limits, mean/sigma, shifts, etc..in HTML
    // page (or QString if specified in last argument)
    void	WriteHistoHtmlTestInfo(CGexChartOverlays *pChartsInfo, const char * szChartType,
                                   CGexFileInGroup *pFile, CTest *ptTestCell,
                                   CGexFileInGroup *pFileY, CTest *ptTestCellY, BOOL bAdvancedReport,
                                   const QStringList& lstImageName = QStringList(),
                                   int iSize=0, int bIsAdvancedFunc=false);

    // Writes HTML Multi-layer Legend table & colors assigned per parameter
    void	WriteHtmlPageLayerLegends(CGexChartOverlays *pChartsInfo);
    QString BuildParameterNameFromURL(QString strName);
    QString	BuildParameterNameToURL(QString strName);
//    bool	DeleteTestEntry(CGexGroupOfFiles *pGroup=NULL,CTest *ptTestCell=NULL);
    QString	ValueAndPercentageString(double lfValue, double lfLimitSpace,double &lfPercent);
    void	getR_R_AlarmColor(CTest *ptTestCell, double lfPercent, QString& pDataColor, QColor &cColor);
    void	VirtualRetest(bool bStopOnFail,bool bUpdateReport, virtualRetestParts eComputeOn);

    void	BuildAllTestLimitsStrings(void);
    //! \brief Generates .csv / HTMLfile
    int		CreateTestReportPages(bool *pbNeedsPostProcessing);
    int		CreateTestReportPages_UserFlow();

    // Custom markers
    void	ClearCustomMarkers(QString lMarkerName="");
    bool    HasCustomMarkers(const QString& lMarkerName);

    // Report Center
    int		CreateTestReportPages_ReportCenter();

    // My Reports (Templates)
    int		CreateTestReportPages_Template();
    /*!
     * \fn WriteSection_Template
     */
    int WriteSection_Template(GS::Gex::ReportTemplateSection* pSection,
                              FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Aggregate
     */
    int WriteSection_Template_Aggregate(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Aggregate_Statistics
     */
    void WriteSection_Template_Aggregate_Statistics(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_Histogram
     */
    void WriteSection_Template_Aggregate_Histogram(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_Trend
     */
    void WriteSection_Template_Aggregate_Trend(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_Scatter
     */
    void WriteSection_Template_Aggregate_Scatter(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_Probabilityplot
     */
    void WriteSection_Template_Aggregate_Probabilityplot(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_BoxplotEx
     */
    void WriteSection_Template_Aggregate_BoxplotEx(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_MultiCharts
     */
    void WriteSection_Template_Aggregate_MultiCharts(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Aggregate_Boxplot
     */
    void WriteSection_Template_Aggregate_Boxplot(
        GS::Gex::CustomReportTestAggregateSection* pAggregate,
        qtTestListStatistics& qtStatisticsList);
    /*!
     * \fn WriteSection_Template_Wafmap
     */
    int WriteSection_Template_Wafmap(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Binning
     */
    int WriteSection_Template_Binning(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Pareto
     */
    int WriteSection_Template_Pareto(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Pearson
     */
    int WriteSection_Template_Pearson(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_TesterCorrelationGB
     */
    int WriteSection_Template_TesterCorrelationGB(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Production
     */
    int WriteSection_Template_Production(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_GlobalInfo
     */
    int WriteSection_Template_GlobalInfo(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_FileAudit
     */
    int WriteSection_Template_FileAudit(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_ER
     */
    int WriteSection_Template_ER(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);
    /*!
     * \fn WriteSection_Template_Datalog
     */
    int WriteSection_Template_Datalog(
        GS::Gex::ReportTemplateSection* pSection,
        FILE* hReportFile);

    void    ComputeRAndRTestList();
    void    ComputeStatisticStat( qtTestListStatistics* pqtStatisticsList_Stats);
    /******************************************************************************!
     * \fn ComputePagesHyperlinks
     * \brief Compute Hyperlinks for various report pages & links
     ******************************************************************************/
    void	ComputePagesHyperlinks(qtTestListStatistics *pqtStatisticsList_Stats,
                                   qtTestListStatistics *pqtStatisticsList_Histo,
                                   qtTestListStatistics *qtStatisticsList_Wafermap);
    void	CheckForNewHtmlPage(CGexChartOverlays *pChartsInfo, int iSectionType=0,
                                int iTestPage=0, const char * szTitle = NULL);
    void	WriteHistoLine(CGexFileInGroup *pFile,CTest *ptTestCell,bool bPluginCall=false);
    void	WriteHtmlSectionTitle(FILE *hReportFile,QString strBookmarkName,QString strSectionTitle);
    FILE*   CreateReportFile(QString strFileName);
    void    CloseReportFile(FILE *hFile=NULL);
    FILE*	reportFileHandle() { return hReportFile; }

    // Opens HTML Advanced file.
    int		OpenFile_Advanced(const QString& strFile = QString());
    FILE *	advancedReportHandle()						{ return hAdvancedReport; }

    // Write the HTML code to open a standard table
    void	WriteHtmlOpenTable(int iWidth, int iCellSpacing);

    // Adds field to report unless it's an empty field
    void	WriteInfoLine(const char *szLabel, const char *szData);
    // Adds field to report 'hFile' unless it's an empty field
    void	WriteInfoLine(const char *szLabel, int iData);
    void	WriteInfoLine(FILE *hFile,const char *szLabel, const char *szData);
    void	WriteInfoLine(FILE *hFile,const char *szLabel,int iData);

    // Write a text : in csv, will simply write the text, in html, will use the given attributes and tag to write text
    // tag could be : "div" or "a" or "span" or "h1"...
    // attributes are : href="..." name="..." style="..." ...
    bool    WriteText(QString txt, QString tag="div", QString attributes="");
    // Begin a table. dont forget to EndTable(). attribute could be :  align=center style="..." ...
    bool    BeginTable(QString attributes="");
    // write a line of table supporting either csv or html output
    // attributes will be the <td> attributes : example: style="..." name="..." width="..." height="..."
    bool    WriteTableRow(const QList<QString> &l, const QString &attributes="",
                           const QList<QString> &anchors=QList<QString>() );
    // attributes will be the <td> attributes : example: style="..." name="..." width="..." height="..."
    bool    WriteTableRow(const QList<QString> &l, const QList<QString> &attributes,
                           const QList<QString> &anchors=QList<QString>() );
    // attributes will be the <td> attributes : example: style="..." name="..." width="..." height="..."
    bool    WriteTableRow(const HtmlTableRow &aTableLine);
    // End table
    bool    EndTable();
    // ad a return to line : <br> for html, '\n' for csv,...
    bool    EndLine();

    void	WriteDatalogTestResult(CTest * ptTestCell, CGexFileInGroup * pFile, int nSample,
                                   CPartInfo * pPartInfo, int& nTestsInDatalog);
    void	WriteDatalogBinResult(int nBin, CPartInfo * pPartInfo, int& nTestsInDatalog);
    void	WriteDatalogPartHeader(CPartInfo * pPartInfo, CGexFileInGroup * pFile, int& lTestsInDatalog);

    // List of groups to process
protected:
    QList<CGexGroupOfFiles*> pGroupsList;
public:
    Q_INVOKABLE inline QList<CGexGroupOfFiles*> &getGroupsList()
    {
        return pGroupsList;
    }

    CWaferMap WaferMapAllData;				// Wafermap data resulting from merging all groups (used by PAT-Man).
    void	StackedGroupsWaferMap(void);	// Create a stacked wafermap in Group#1 from all groups

    int		iCurrentHtmlPage;			// Used while generating HTML pages.
    //! \brief Holds the 'small'font size to use in HTML pages: 3 = normal, 2= medium, 1= small
    int		iHthmSmallFontSize;
    //! \brief Holds the 'standard' font size to use in HTML pages: 3 = normal, 2= medium, 1= small
    int		iHthmNormalFontSize;
    //! \brief Holds the 'small'font size to use in HTML pages: 12 = normal pixel size, 10 = small
    int		iHthmSmallFontSizePixels;
    int		iHthmNormalFontSizePixels;	// Holds the 'standard' font size to use in HTML pages
    //! \brief Total number of lines per flat page (may vary if page is for Word, or PPT,...)
    int		iLinesPerFlatPage;
    //! \brief Total number of lines per flat page (may vary if page is for Word, or PPT,...)
    int		iLinesPercentagePerFlatPage;
    //! \brief Holds current report page# created; incremented at each new page
    long	lReportPageNumber;

    // List of Tests with custom limits, markers, etc... (eg: for guard banding analysis., PAT-Man,...)
    QList <CTestCustomize *>	m_pTestCustomFieldsList;
    //! \brief Charting object (based on ChartDirector commercial product)
    CChartDirector *m_pChartDirector;

    // PowerPoint generation functions
    void	SetPowerPointSlideName(QString strSlideName, bool bAppend=false);

    // Create wafermap image
    void	CreateWaferMapImage(wafermapMode eWafermapMode,
                                CGexGroupOfFiles * pGroup,
                                CGexFileInGroup * pFile,
                                BOOL bWriteReport,
                                const char * szWafermapImagePath,
                                const char * szWafermapImage,
                                bool bSaveImage = false,
                                bool bAllowWaferArrayReload = true,
                                bool *bBreakPage = NULL,
                                int waferMapType = -1,
                                bool noReport = false,
                                CTest* testCell = NULL,
                                QStringList deactivatedBinsList = QStringList());

    // Reload/Fill wafermap structure
    void	FillWaferMap(CGexGroupOfFiles *,CGexFileInGroup *,CTest *ptTestCell,
                         int iWaferType, bool bSetOrgBin=false,
                         double dLowLimitEx = GEX_C_DOUBLE_NAN , double dHighLimitEx = GEX_C_DOUBLE_NAN);
    //! \brief Fill (group,file) wafermap with given parameter
    void	FillWaferMap(int iGroup,int iFile,CTest *ptTestCell,int iWaferType,
                         bool bSetOrgBin=false,double dLowLimitEx = GEX_C_DOUBLE_NAN ,
                         double dHighLimitEx = GEX_C_DOUBLE_NAN);
    //! \brief 3D-mining: Select all dies where parameter value is within given range
    int		SelectWaferMapDies(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest *ptTestCell,
                               double lfLow,double lfHigh);

    // Get testing site used for: testing a given die
    int		getDieTestingSite(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,int iDieX,int iDieY);

    // Return handle to layer list
    CGexChartOverlays*	m_pChartsInfo;	// Holds list of charts to draw + how they must be drawn (zoom ratio, viewport).

    // Create/Write Post-Processing outlier report strings into STDF file
    void	BuildPP_OutlierRemovalReport(QString strReportType,bool bStdfDestination=true);
#ifdef GCORE15334
    void	WritePP_OutlierRemovalReport(GS::StdLib::Stdf *pStdf,
                        GS::Gex::PATProcessing &cFields,QString strString="");
#endif

    void	WritePP_OutlierReportLine(QString strLine,bool bStdfDestination=true);

    // Report pages creation
    int		CreatePages_Stats(qtTestListStatistics *pqtStatisticsList_Stats,int &iCellIndex,
                              bool bMeanShiftAlarmOnly=false,bool bSigmaShiftAlarmOnly=false,int iMaxLines=-1);

    int		ConvertHtmlToPDF(const QString& pdfFileDestination  =QString(),
                             const QString &htmlFile            =QString(),
                             const QString& imageOption         =QString(),
                             const QString &imageOptionFile     =QString());					// PDF generation function

    CBinningColor cDieColor;			// Colors used in wafermap and binnings.
    QString	strPatTraceabilityReport;	// Holds PAT post-processing traceability report (CSV or HTML string)

    // Enterprise Reports: WYR data retrieved from GEXDB
    // (data needs to be kept so that the user can click a link to save a table from the HTML report)
    GexDbPlugin_ER_Parts	clER_PartsData;
    GexDbPlugin_XYChartList	clXYChartList;
    GexDbPlugin_WyrData		clWyrData;
    void ProcessActionLink_EnterpriseReports(const QString & strGexHtmlToolbarAction);
    void Enterprise_Wyr_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction);
    void Enterprise_Wyr_ExportToExcelFile(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Uph_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Uph_ExportToExcelFile(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Yield_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Yield_ExportToExcelFile(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Yield_Advanced_ExportToExcelClipboard(const QString & strGexHtmlToolbarAction);
    void Enterprise_Prod_Yield_Advanced_ExportToExcelFile(const QString & strGexHtmlToolbarAction);

    // Advanced Enterprise Reports
    void ProcessActionLink_AdvEnterpriseReports(const QString & strGexHtmlToolbarAction);

    void clearInteractiveTestFilter();
    void setInteractiveTestFilter(const QString& strInteractiveTestFilter);
    bool hasInteractiveTestFilter() const;
    bool isInteractiveTestFiltered(CTest * pTest);

    void SetProcessingFile(bool lProcessingFile);

    autoRepairOption	autoRepair() const						{ return m_eAutoRepairOption; }
    void				setAutoRepair(autoRepairOption eOption)	{ m_eAutoRepairOption = eOption; }

    const QTime&		processTime() const						{ return m_tProcessTime; }

    static QColor		GetChartingColor(int iGroupIndex);

    // Test name mapping method
    unsigned int		findMappedTestName(long lTestType, unsigned int nTestNumber, char * szTestName);

    // Statistics engine.
    CGexStats		m_cStats;

    // Returns test name formated for output device (eg: truncated string, etc...)
    QString			buildDisplayName(QString strString, bool bUseOutputFormat = true);

    QList<CTest*> &getAdvancedTestsListToReport()
    {
        return mAdvancedTestsListToReport;
    }
    GS::Gex::ReportUnit *getReportUnit (const QString &strReport)
    {
        if(mReportUnits.contains(strReport))
            return mReportUnits[strReport];
        else
            return 0;
    }

private:

    class TestNameMappingPrivate
    {
        public:
            TestNameMappingPrivate()	{ }
            ~TestNameMappingPrivate()	{ }
            unsigned int						findTestNumber(unsigned int nTestNumber, char * szTestName);
        private:
            QMap<unsigned int, unsigned int>	m_mapTestNumber;
            QMap <QString, unsigned int>		m_mapTestName;
    };

    QMap<QString, GS::Gex::ReportUnit*> mReportUnits;

    QMap<int, TestNameMappingPrivate> m_testNameMap; ///< MAPs used if must ignore test# and only rely on test name.
    QList<CTest*> mAdvancedTestsListToReport; ///< List of tests for Adv report, Will be filled in PrepareSection_...

    autoRepairOption	m_eAutoRepairOption;

    GexAdvancedEnterpriseReport *	m_pAdvancedEnterpriseReport;

    bool            mCompleted;                             ///< Used to incidate if the GexReport is completed and can be used

    // Counters, Flags
    int				iMission;                               ///< GEX_ONEFILE_WIZARD or GEX_CMPFILE_WIZARD, etc...
    bool            mProcessingFile;                        ///< Set to true when the report is based on file, false when it is based on Dataset (DB)

    int             mMaxMLItems;                            ///< Holds max multi limit item for a test
    double			lfTotalFileSize;                        ///< Total files size to process
    QTime			m_tProcessTime;                         ///< Measures Time when STDF start being processed: used to compute time for building report !
    QStringList		m_lstInteractiveTestFilter;

    QString			m_strOutputFormat;						///< Holds report output format
    QString			m_strLegacyReportRootFolder;			///< Legacy report root folder
    QString			m_strLegacyReportAbsFilePath;			///< Legacy Report name
    QString			m_strEvaluatedReportRelativeFilePath;	///< Report file path relative to root folder
    QString			m_strEvaluatedReportName;				///< Report name
    QString			m_strEvaluatedReportSubFolders;			///< Report sub-folders
    QString			m_strEvaluatedReportRootFolderAbsPath;	///< Report root folder
    QString			m_strEvaluatedReportType;				///< Report type

    QString			m_strJsReportName;						///< Report name
    QString			m_strJsReportSubFolders;				///< Report sub-folders
    QString			m_strJsReportRootFolderAbsPath;			///< Report root folder
    QString			m_strReportGenerationMode;				///< Generation mode: legacy or normalized

    QScriptEngine*	m_pScriptEngine;                        ///< Scripting engine

    void			PrepareSecondPass(CGexGroupOfFiles *pGroup, CReportOptions *ptReportOptions);
    void			FinishPasses(CGexGroupOfFiles *pGroup, CReportOptions *ptReportOptions);
    QString			buildDisplayName(CTest *ptTestCell);

    void CreateNewHtmlStatisticPage();

    // Check if a file exists and return a new file name if necessary
    // return true if the file exist else return false
    bool            checkIfReportExist(const QString &filePath, QString &newFilename);
    // the user defined report name used to generate pdf, ppt, doc report file.
    QString m_strUserDefRepotFileName;
    // check if the user already entred a user def report name.
    bool m_bUserAlreadyAsked;

    // Compute the Pearsons correlation for this pair of test
    // count will give the number of results the computation has been done on
    // could return GEX_C_DOUBLE_NAN or 0 if error or no or not enough pair of XY found
    double          ComputePearsonValue(CTest *ptCellX, CTest *ptCellY, int &count);

    // Compute the max delta between normalized test results on X and normalized test results on Y
    // return -1 on error, 0 if no samples at all, else the number of samples found
    int             ComputeMaxShiftBetweenXandYtest(double &d, CTest* ptCellX, CTest* ptCellY);

    // Compute value space
    void			computeValuesSpaceWafermap(CTest * ptTestCell, double dLowValueEx, double dHighValueEx,
                                               int iWaferType,
                                               double& dLowValue, double& dHighValue, double& dSpaceValue);

    // Tells if must use Summary files instead of regular files
    // Applies to ExaminatorDB only....
    bool			CheckIfUseSummary();

    // Compute all statisitcs shifts (mean, sigma, etc...): only if multi-groups.
    int				ComputeStatsShift(void);
    TestShift       ComputeStatsShift(CTest* referenceTest, CTest* testToCompare, TestShift &testShift);
    double			ComputeAggregateValue(CTest *ptTestCell,long lFromDataSample,long lToDataSample,int iAggregateInfo);
    void			ComputeOutlierSigmaLimits(CTest *ptTestCell, double fRatio);
    //! \brief Write test list index page...only on HTML standard page
    int				WriteTestListIndexPage(int iPageIndexType,BOOL bValidSection,
                                           qtTestListStatistics *pqtStatisticsList=NULL);
    void			WriteMismatchTestsPage(void);
    int				EndDataCollection();
    void			WriteMirProcessingTime(void);
    int				WriteGlobalPageExaminator(void);
    void			WriteGlobalPageExaminatorGroupDetailed(CGexGroupOfFiles *pGroup, CGexFileInGroup *pFile);
    void			WriteGlobalPageExaminatorGroupSummarized(CGexGroupOfFiles *pGroup, CGexFileInGroup *pFile);
    int				WriteGlobalPageExaminatorDB(void);
    void			WriteStatisticsLabelLine(bool bInteractiveURL=false);
    void			WriteWaferMapTableLine(const char *szLabel, const char *szData, bool bAlarm = false);
    void			BuildTestPinString(char *szFrom,int iFromTestNumber,int iFromPinNumber,char *szTo,
                                       int iToTestNumber,int iToPinNumber);
    void			SetCurveStyles(long lChartID,int iGroup, int iCurvePenSize, CGexSingleChart *pChart,
                                   QColor &cLayerColor,int iReportType=-1);
    void			SetLayerStyles(int iGroup, CGexSingleChart *pChart, int& nFillColor, int& nLineColor,
                                   Chart::SymbolType& layerSymbol, int iReportType = -1);

    // Writes Test statistics of ONE test (may be multiple lines if comparing files)
    void			WriteStatsLinesEx(CGexGroupOfFiles* pGroup, CGexFileInGroup* pFile,
                                      uint uGroupID, CTest * ptTestReferenceCell, CTest *ptTestCell,
                                      int& iLine,
                                      const QString &strStatisticsFieldsOptions,
                                      bool bMeanShiftAlarmOnly=false, bool bSigmaShiftAlarmOnly=false
                                       );
    //! \brief Writes Test statistics of ONE test (may be multiple lines if comparing files)
    void			WriteStatsLines(CTest *ptTestCell, int iLine,
                                    const QString &strStatisticsFieldsOptions,
                                    bool bMeanShiftAlarmOnly=false, bool bSigmaShiftAlarmOnly=false,
                                    const QString &strOutputFormat = "" );
    char *			CheckColumnPassFail(CTest *ptTestCell,double fValueL,double fValueH);
    void			WriteNavigationButtons(const char *szPageName, bool bTop = false);
    void			plotLotMarkersEx(CTest *ptTestCell, CReportOptions * pOptions, bool bAggregateData=false);
    void			WriteAdvTrendDifferenceChartPageEx(CGexChartOverlays *pChartsInfo,int iChartSize);
    void			WriteAdvTrendBinningChartPageEx(CGexChartOverlays *pChartsInfo,int iChartSize);
    void			WriteBoxPlotLabelLine(bool bChartOverLimits,int &iTotalRows);
    void			ComputeLimitsSpace(CGexFileInGroup *pFile,CTest *ptTestCell,double &lfLimitSpace);
    void			ComputeTestGage(CTest *ptTestCell);
    void			WriteAdvBoxPlotLines(CGexFileInGroup *pFile,CTest *ptTestCell,int &iBoxPlotLine);
    //! \brief Gage  R&R statistical information
    void			PrintValueAndPercentage(int iGroup,FILE *hReportFile,CTest *ptTestCell, const char *pDataColor,
                                            double lfValue, double lfLimitSpace,bool bCheckAlarm);

    int             GetResultPosition(CGexGroupOfFiles *aGroup, int aDiePos, int aTrialId, int aPartsCount);
    void			ComputeTestXBarR(CTest* ptTestCell);

    // Write Pearson report using given options
    // returns "ok" or "error ..."
    QString			WriteAdvPersonReport(CReportOptions* ro);

    // returns "ok" or "error ..."
    // uses Tests in m_AdvancedTestsListToReport !
    QString         WriteAdvHistoFunctional(CReportOptions* ro);
    QString			WriteAdvHisto(CReportOptions* ro);
    // returns "ok" or "error ..."
    QString			WriteAdvTrend(CReportOptions* ro);
    // returns "ok" or "error ..."
    QString			WriteAdvProbabilityPlot(CReportOptions* ro);
    // returns "ok" or "error ..."
    QString			WriteAdvMultiChart(CReportOptions* ro);
    // returns "ok" or "error ..."
    QString			WriteAdvBoxPlotEx(CReportOptions* ro);
    // returns "ok" or "error ..."
    QString			WriteAdvScatter(CReportOptions* ro);

    // Compute intragroup correlation pearsons tests in the group
    // stock CPearsonTest in CTestListPearson
    // returns "ok" or "error..."
    QString			ComputePearsonsInGroup(CGexGroupOfFiles* g, CTestListPearson &list, const float	dPearsonCutOff);

    // Compute intergroup correlation pearsons G1 test by G2 test
    // stock CPearsonTest in CTestListPearson
    // warns could contain 1 or more warning that could be for example reported in the report
    // returns "ok" or "error..."
    QString			ComputePearsonsBetweenGroups(
      CGexGroupOfFiles* g1, CGexGroupOfFiles* g2, CTestListPearson &list, const float	dPearsonCutOff,
      QStringList &warns);

    void			BuildPearsonHtmlTable(CGexGroupOfFiles* pGroup1, CGexGroupOfFiles* pGroup2);
    void			WriteAdvPatTraceability();
    void			WriteAdvOutlierRemovalReport();
    void			SplitPatReportWebPage(const QString &strPage);
    void			WriteAdvProductionYield();
    void			BuildDPAT_PartsHtmlTable(bool bStdfDestination);
    void            BuildMVPATRulesChartsPage(const QStringList &lRules,
                                              bool lSplitPatReportPages,
                                              bool lStdfDestination);
    void            BuildMVPATRulesDetailsPage(const QString &lPageName,
                                               bool lSplitPatReportPages,
                                               bool lStdfDestination);
    void            BuildMVPATRulesDetailsTableHeader(bool lStdfDestination);
    void            BuildMVPATRulesDetailsTable(bool lStdfDestination);
    void            BuildMVPATRulesSummaryPage(const QString &lPageName,
                                               bool lSplitPatReportPages,
                                               bool lStdfDestination);
    void            BuildMVPATRulesSummaryTableHeader(bool lStdfDestination);
    QStringList     BuildMVPATRulesSummaryTable(bool lStdfDestination);
    void			BuildMVPATFailingPartsPage(const QString &lPageName,
                                               bool lSplitPatReportPages,
                                               bool lStdfDestination);
    void			BuildMVPATFailingPartsTableHeader(bool bStdfDestination);
    void			BuildMVPATFailingPartsTable(bool bStdfDestination);
    void			BuildDPAT_SummaryHtmlTable(bool bStdfDestination);
    void            BuildNNRSummaryPage(const QString &lPageName,
                                        bool lSplitPatReportPages,
                                        bool lStdfDestination);
    void			BuildNNRSummaryTableHeader(bool bStdfDestination);
    void			BuildNNRSummaryTable(const QList<GS::Gex::PATNNRTestSummary>& lNNRTestList, bool bStdfDestination);
    void			BuildNNRDetailedTableHeader(bool lStdfDestination);
    void			BuildNNRDetailedTable(const QList<GS::Gex::PATNNRTestSummary>& lNNRTestList, bool lStdfDestination);
    void			BuildIDDQ_Delta_SummaryHtmlTable(bool bStdfDestination);
    void			BuildReticleHtmlTable(bool bStdfDestination);
    void			WriteAdvOptimizerDiagsPage(void);
    void			WriteGroupNameLabel(CGexGroupOfFiles *pGroup);
    void			PrepareSection_CpParetoReport(void);
    void			PrepareSection_CpkParetoReport(void);
    void			PrepareSection_FailuresParetoReport(void);
    void			PrepareSection_FunctionalFailureSignaturesParetoReport(void);
    void			PrepareSection_ParametricFailureSignaturesParetoReport(void);
    void			PrepareSection_BinningParetoReport(QString strBinType);
    void			OpenCpCpkParetoTable(const char *szType);
    void			OpenFailuresParetoTable(void);
    void			OpenFunctionalFailureSignaturesParetoTable(void);
    void			OpenParametricFailureSignaturesParetoTable(void);
    void			OpenBinningParetoTable(const char *szBinningType);
    void			CreateCpkParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *,CTest *);
    void			CreateCpParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *,CTest *);
    void			CreateFailuresParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *,CTest *);
    void			CreateFailureSignaturesParetoReport(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,CTest *ptTestList);
    void			CreateBinningParetoReport(CGexGroupOfFiles *pGroup,CBinning *ptBinList,bool bSoftBin,long lTotalParts);
    CBinning *		findBinningCell(CBinning	*ptBinList,int iBinNumber);
    void			OpenBinningTable(const char *szBinType);
    //! \brief Writes one full section of bin result
    void			WriteBinLines(CGexGroupOfFiles *pGroup,CBinning* ptBinCell,long lTotalBins,
                                  const char *szTitle,bool bSoftBin=true);
    //! \brief Writes one line of Binning result
    void			WriteBinLine(CBinning *ptBinCell, bool bSoftBin, long lTotalBin, CGexGroupOfFiles *group = 0,
                                 bool bPassFailCumul=false, long lTotalPassOrFail=0);
    bool			WriteCompareWaferMap(BOOL bWriteReport);
    void			WriteDeltaYieldSection(CWafBinMismatchPareto *pWafBinMismatchPareto,
                      const QString & strSectionTitle,
                      const QString & strColumn1Label,
                      const QString & strColumn2Label,
                      unsigned int uiCommonDies);
    QColor *		CreateColorSpectrum(void);

    void			ComputeZonalRegions(CGexGroupOfFiles *pGroup,QZonalRegion cZonalRegions[12]);
    //! \brief Writes the ZONAL wafermap for a given group.
    bool			WriteZonalWaferMap(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport,
                                       int iGroupID,QString strImageName="");
    bool			WriteStackedWaferMap(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport,int iGroupID);
    bool			WriteStackedGroupsWaferMap(int iTotalWafers,BOOL bWriteReport);
    void			WriteBinningLegend(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,BOOL bWriteReport);
    void			WriteBinningLegend(CGexGroupOfFiles *pGroup,BOOL bWriteReport);
    void			WritePassFailLegend(int nPassCount, int nFailCount);
    void			WriteBinToBinColorCorrelationLegend();
    //! \brief Write Wafer header info.
    bool			WriteIndividualWafer(CGexGroupOfFiles *pGroup,CGexFileInGroup *pFile,
                                         BOOL bWriteReport,int iGroupID,int iFileID,
                                         QString strImageName="",bool bAllowWaferArrayReload=true);
    int				WriteWaferMapCharts(BOOL bWriteReport,QString strImageName="");
    int				GetWaferImageSize(bool bStackedWafer=false);
    int				GetWaferSizeRequested(int iChartSize);
    void			CloseReport(void);
    //! \brief Write the HTML code to display the DRILL ZoomIn/out toolbar in the report and other optionnal links.
    void			WriteHtmlToolBar(int iSizeX,bool bAllowInteractive,QString strArgument,
                                     QString strText2="",QString strImage2="",QString strLink2="", QString strText3="",
                                     QString strImage3="",QString strLink3="");
    //! \brief Write the HTML code to display a percentage bar (using a image, or table)
    void			WriteHtmlPercentageBar(FILE *hReportFile, int iCellSizePercentage,
                                           QString strDataColor, int iColorMapIndex, bool bSoftBin,
                                           QString strImage, int iImageSize, float fPercentage, QString strComment, const CMergedResults::T_SiteCounter &siteProportion, bool displaySiteRation);
    //! \brief For convenient, call the function above
    void			WriteHtmlPercentageBar(FILE *hReportFile, int iCellSizePercentage,
                                           QString strDataColor, int iColorMapIndex, bool bSoftBin,
                                           QString strImage, int iImageSize, float fPercentage, QString strComment = " ");


    const char *	GetChartingHtmlColor(int iGroupIndex);
    const char *	GetReportSortingMode(QString strSortingMode);
    const char *	CreateResultStringCpCrCpkShift(double lfValue);
    // Handle for writing to Output file(s) (.HTML / .CSV ASCII)
    FILE *			hReportFile;
    //! \brief Handle for writing Advanced Report (Datalog,...) into .HTML ONLY
    FILE *			hAdvancedReport;
    //! \brief List of expressions to evaluate and see if R&R% value is matching an alarm criteria
    QList<CGageWarning>	m_lstGageWarning;

    // HTML Histograms
    long	mTotalStatisticsPages;		// Total HTML Histogram pages to be generated.
    long	mTotalHistogramPages;		// Total HTML Histogram pages to be generated.
    long	mTotalWafermapPages;		// Total HTML wafermap pages to be generated.
    long	mTotalAdvancedPages;		// Total HTML Advanced pages to be generated
    long	mTotalHtmlPages;			// buffer holding number of HTML pages to create (is set with

    int		CreatePages_Global();
    int		CreatePages_Histo(qtTestListStatistics *pqtStatisticsList_Histo);
    int		CreatePages_Wafermap(qtTestListStatistics * pqtStatistics_Wafermap, QString strImageName="");
    int		CreatePages_Pareto(void);
    int		CreatePages_Binning(void);
    // Creates ALL pages for the Advanced report
    int		CreatePages_Advanced(void);
    /*!
     * \fn CreatePages_FileAudit
     */
    int CreatePages_FileAudit(
        GS::Gex::CustomReportFileAuditSection* pFileAudit);
    /*!
     * \fn CreatePages_ER
     */
    int CreatePages_ER(GS::Gex::ReportTemplateSection* pSection);
    int		CreatePages_Datalog(qtTestListStatistics * pqtStatisticsList);
    // Compute Chart image size to use (depends of # of images to create, report type...)
    int		GetChartImageSize(int iChartSize, int iTotalCharts);
    // Get the chart size int (defines via GEX_CHARTSIZE_...) according to the csl string ("auto","small",...) ChartSize
    int		GetChartImageSize(QString ChartSize, int iTotalCharts);

    //! \brief Writes Table of contents in flat HTML file (first page)...
    // unless plugin activated (in which case, plugin builds its own table of contents)
    void	WriteExaminatorTableOfcontents(FILE *);
    QString BuildTargetReportName(QString &strHtmlReportFolder,const QString strExtension);
    // Build Header page to include in MS-Office report or PDF file.
    void	BuildHeaderFooterText(QString &strHeader,QString &strFooter);
    int		ConvertHtmlToWord(void);				// MS-Word generation function

    // ODT generation function
    // return -1 on error
    int		ConvertHtmlToODT(void);

    // PowerPoint generation functions
    //! \brief Actions to first do when preparing PPT convertion (create few HTML + XML files)
    void WritePowerPointBegin(void);
    //! \brief Actions to finish building HTML+XML files
    void WritePowerPointFinish(void);
    void OpenTemporaryHtmlPowerPoint(bool bOpenHtml = true);
    //! \brief Update Slide name while building pages with 2 or more charts per page
    void RefreshPowerPointSlideName(QString strSection,int iChartInPage=1,int iMaxChartsPerPage=1,
                                    CTest *ptTestCellX=NULL,CTest *ptTestCellY=NULL);

    //! \brief Build HTML page in PPT sub-folder: includes PPT related code + PNG image (of one full HTML page)
    //!  \return A string starting with either "ok" or "error: rootcause"
    QString	WritePowerPointSlide(FILE *hFile = NULL,QString strSlideName="",bool bOpenHtml=true);

    //! \brief Convert HTML+XML files & folder to PPT
    //! \return 'ok' or 'error: root cause'
    QString	ConvertHtmlToPowerpoint(void);
    /*!
     * \fn UpdateUserTestNameAndTestNumber
     */
    void UpdateUserTestNameAndTestNumber();

    QString	strPowerpointSubFolder;			// Subfloder where ALL slides (HTML pages + PNG images) are created
    QString	strSlideTitle;					// Slide Title as building a section.
    long	lSlideIndex;					// Used to create slides (incremented after each slide is created)

    int	PrepareSection_Global(BOOL bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int	PrepareSection_Stats(BOOL bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int	PrepareSection_Histo(BOOL bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int	PrepareSection_Wafermap(BOOL bValidSection, int nFileIndex = 0);
    int	PrepareSection_Pareto(BOOL bValidSection);
    int	PrepareSection_SoftBinning(BOOL bValidSection);
    int	PrepareSection_HardBinning(BOOL bValidSection);
    int	PrepareSection_Datalog(bool bValidSection);
    // Prepare the Advanced sections
    int		PrepareSection_Advanced(BOOL bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int		PrepareSection_AdvHisto(bool bValidSection);
    int     PrepareSection_AdvHistoFunctional(BOOL bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int		PrepareSection_AdvTrend(BOOL bValidSection);
    int		PrepareSection_AdvScatter(BOOL bValidSection);
    int		PrepareSection_AdvProbabilityPlot(BOOL bValidSection);
    int		PrepareSection_AdvBoxPlot(BOOL bValidSection);
    int		PrepareSection_AdvBoxplotEx(BOOL bValidSection);
    int		PrepareSection_AdvMultiChart(BOOL bValidSection);
    int		PrepareSection_AdvDatalog(bool bValidSection);
    //! \brief Prepares the report section to be written (.CSV & .HTML)
    int		PrepareSection_AdvPearson(BOOL bValidSection);
    int		PrepareSection_AdvProductionYield(BOOL bValidSection);
    int		PrepareSection_AdvPatTraceability(BOOL bValidSection);
    int		PrepareSection_AdvOutlierRemoval(BOOL bValidSection);
    int		PrepareSection_AdvOptimizerDiags(BOOL bValidSection);
    int		OpenFile_Drill(void);
    int		PrepareSection_Drill(BOOL bValidSection);

    bool	IfHtmlSectionCreated(int iSection);
    int		CloseSection_Global();
    int		CloseSection_Stats(BOOL bValidSection, qtTestListStatistics	*pqtStatisticsList);
    int		CloseSection_Histo(qtTestListStatistics	*pqtStatisticsList);
    int		CloseSection_Wafermap();
    int		CloseSection_Pareto();
    int		CloseSection_SoftBinning();
    int		CloseSection_HardBinning();
    int		CloseSection_Datalog();
    int		CloseSection_Advanced();
    int		CloseSection_AdvHisto();
    int     CloseSection_AdvFunctionalHisto();
    int		CloseSection_AdvTrend();
    int		CloseSection_AdvScatter();
    int		CloseSection_AdvProbabilityPlot();
    int		CloseSection_AdvBoxPlot();
    int		CloseSection_AdvBoxplotEx();
    int		CloseSection_AdvMultiChart();
    int		CloseSection_AdvDatalog();
    int		CloseSection_AdvProductionYield();
    int		CloseSection_AdvPearson();
    int		CloseSection_AdvPatTraceability();
    int		CloseSection_AdvOutlierRemoval();
    int		CloseSection_AdvOptimizerDiags();
    int		CloseSection_Drill(void);

signals:

    void	interactiveFilterChanged(bool bState);

protected:

    int     mTestMergeRule;

    bool    UpdateOptions(CReportOptions * lOptions);

private:
    QStringList m_oConditionList;

protected:
    GS::QtLib::NumberFormat mNumberFormat;
    CReportOptions          *m_pReportOptions;		// Pointer to GEX options
    GS::Gex::ReportLogList  mReportLogList;

public:
    GS::QtLib::NumberFormat *getNumberFormat();
    CReportOptions *getReportOptions();
    void setReportOptions(CReportOptions *poReportOptions);
    //ReportMessageList Getter
    GS::Gex::ReportLogList &GetReportLogList();

    void WriteLegend(FILE *hReportFile, bool pareto);

    const char *CreateTestDrilTL(CTest *lTestCell);
    const char *CreateTestDrilFlow(CTest *lTestCell);
    const char *CreateTestDriftHigh(CTest *lTestCell);
    void CreateTestFailPercentage(CTest *lTestCell, QString &outputString);
    StdfDataFile*     FindStdfDataFile (const QString& name);
    bool getProcessingFile() const;

private:
    // -- to distinguish a created pixmap
    int     mIndicePixmap;
    QString CreatePixmap(int total, const CMergedResults::T_SiteCounter &siteProportion);
    QColor  GetColorSite(int indexColor);

    void    CreateStepDefectivityCheckSection(const PATOptionReticle &lReticle,
                                                const CPatInfo& lPatInfo,
                                                int lIdxRule,
                                                const QString& strOutputFormat, bool bStdfDestination);
    void    CreateRepeatingPatternSection(const PATOptionReticle &lReticle,
                                                const CPatInfo& lPatInfo,
                                                int lIdxRule,
                                                const QString& strOutputFormat, bool bStdfDestination);
    void    CreateCornerRuleSection(const PATOptionReticle &lReticle,
                                                const CPatInfo& lPatInfo,
                                                int lIdxRule,
                                                const QString& strOutputFormat, bool bStdfDestination);
    bool    CreateReticleHeatmapImage(const QJsonArray &data, const CPatInfo& lPatInfo, const QString& filename,
                                      double threshold = GEX_C_DOUBLE_NAN);
    bool    CreateReticleHeatmapCSV(const QJsonArray &data, const CPatInfo &lPatInfo, bool bStdfDestination);

    /*!
     * \brief fill the list of colors used per site for the pareto
     */
    void    buildSiteColorPareto();

    /*!
     * \brief get a color from the mColorsPerSitePareto
     * \param site number used to get a color
     * \return a color
     */
    QColor  getSiteColorPareto(int site);

    /*!
     * \brief contain a hard coded list of color
     */
    QList<QColor>   mColorsPerSitePareto;

    QMap<QString, StdfDataFile*>    mStdfDataFiles;
    QList<StdfDataFile*>            mOrderedStdfDataFiles;

};


#endif // GEX_REPORT_H
