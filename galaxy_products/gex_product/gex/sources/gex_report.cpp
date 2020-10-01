#include <QSqlQuery>
#include <QSqlError>
#include <QSqlResult>
#include <QSqlRecord>
#include <gqtl_log.h>
#include <QUndoStack>

#include "gex_report_unit.h"
#include "report_build.h"
#include "gex_file_in_group.h"
#include "gex_group_of_files.h"
#include "gex_report.h"
#include "gexscatterchart.h"
#include "gex_advanced_enterprise_report.h"
#include "chart_director.h"
#include "gex_charac_boxwhisker_chart.h"
#include "gex_charac_histo_chart.h"
#include "browser_dialog.h"
#include "gexperformancecounter.h"
#include "engine.h"

int CGexReport::sNumberOfIntances=0;

extern GexMainwindow *pGexMainWindow;

CGexReport::CGexReport() : QObject(), m_pAdvancedEnterpriseReport(NULL),mIndicePixmap(0),mMaxMLItems(0)
{
    mId=sNumberOfIntances;
    sNumberOfIntances++;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("new GexReport : id=%1 Total %2 instances...").arg( mId).arg(sNumberOfIntances)
          .toLatin1().constData());

    setObjectName("gex_report_"+QString::number(mId));
    // Reset variables.
    mCompleted = false;
    lfTotalFileSize = 0;	// Total size of all files to process
    iScriptExitCode = GS::StdLib::Stdf::NoError;

    // Ensure all pointers are well initialized.
    m_tProcessTime.start();	// Current time => will be used to compute time spent processing file.

    hReportFile = NULL;			// Makes sure handle is NULL.
    hAdvancedReport = NULL;		// Makes sure handle is NULL.
    m_pReportOptions = NULL;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new CChartDirector ").toLatin1().constData());

    m_pChartDirector	= new CChartDirector();		// ChartDirector drawing object

    // Html font size
    iHthmSmallFontSize = 3;			// Holds the 'small'font size to use in HTML pages...may be overwritten at run time depending of output format (HTML, DOC,...) and platform (PC, unix,...)
    iHthmNormalFontSize = 3;		// Holds the 'standard' font size to use in HTML pages...may be overwritten at run time depending of output format (HTML, DOC,...) and platform (PC, unix,...)
    iHthmSmallFontSizePixels = 10;	// 12pixels: Holds the 'small'font size to use in HTML pages: 12 = normal pixel size, 10 = small
    iHthmNormalFontSizePixels = 12;	// 12pixels: Holds the 'standard' font size to use in HTML pages

    iLinesPerFlatPage		   = MAX_STATSLINES_PER_FLAT_PAGE;			// Total number of lines per flat page (may vary if page is for Word, or PPT,...)
    iLinesPercentagePerFlatPage = MAX_PERCENTAGE_LINES_PER_WORD_PAGE;	// Total number of lines per flat page (may vary if page is for Word, or PPT,...)

    // Holds layers when in interactive mode.
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new CGexChartOverlays ").toLatin1().constData());
    m_pChartsInfo = new CGexChartOverlays;

    // MAP structures used if must ignore test# and only rely on test name.
    m_testNameMap.clear();

    // Set the status to false when starting a new report
    m_eAutoRepairOption = repairUndefined;

#ifdef GEX_NO_JS_SOLARIS
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("m_pScriptEngine = NULL ").toLatin1().constData());
    m_pScriptEngine = NULL;
#else
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new QScriptEngine ").toLatin1().constData());
    m_pScriptEngine = new QScriptEngine();
#endif

    m_strJsReportName = "";
    m_strJsReportSubFolders = "";
    m_strJsReportRootFolderAbsPath = "";
    m_strEvaluatedReportName = "";
    m_strEvaluatedReportSubFolders = "";
    m_strEvaluatedReportRootFolderAbsPath = "";
    m_strEvaluatedReportType = "";
    m_strReportGenerationMode = "";
    m_strUserDefRepotFileName = "";
    m_strLegacyReportAbsFilePath = "";
    m_strLegacyReportRootFolder = "";
    m_strOutputFormat = "";
    m_strEvaluatedReportRelativeFilePath = "";
    m_bUserAlreadyAsked = false;
    mProcessingFile     = true;
    mTestMergeRule      = TEST_NO_MERGE;

    //
    mReportDBName = QString("gex_report_%1").arg(mId);
    QSqlDatabase lReportDB = QSqlDatabase::addDatabase("QSQLITE", mReportDBName);
    lReportDB.setDatabaseName(":memory:"); // mandatory for a 'in mem' DB. If the DB name is diff, a file with that name will be created near the executable.
    //mReportDB.setDatabaseName(QString("gex_report_%1").arg(mId) );
    if (!lReportDB.open())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot open report DB");
    }
    else
    {
        //query.exec("PRAGMA synchronous = OFF;"); // todo ?
    }
    if(pGexMainWindow && pGexMainWindow->getUndoStack()){
        pGexMainWindow->getUndoStack()->clear();
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("End CGexReport::CGexReport ").toLatin1().constData());

#if defined(DEBUG_WARNING_SECTION)
    GetReportLogList().addReportLog(QString("Message 1"),GS::Gex::ReportLog::ReportError);
    GetReportLogList().addReportLog(QString("Message 2"),GS::Gex::ReportLog::ReportInformation);
    GetReportLogList().addReportLog(QString("Message 2"),GS::Gex::ReportLog::ReportWarning);
    GetReportLogList().addReportLog(QString("Message 4"),GS::Gex::ReportLog::ReportWarning);
    GetReportLogList().addReportLog(QString("Message 4"),GS::Gex::ReportLog::ReportError);
    GetReportLogList().addReportLog(QString("Message 6"),GS::Gex::ReportLog::ReportInformation);
    GetReportLogList().addReportLog(QString("Message 7"),GS::Gex::ReportLog::ReportWarning);
    GetReportLogList().addReportLog(QString("Message 8"),GS::Gex::ReportLog::ReportWarning);
    GetReportLogList().addReportLog(QString("Message 9"),GS::Gex::ReportLog::ReportInformation);
    GetReportLogList().addReportLog(QString("Message 10"),GS::Gex::ReportLog::ReportInformation);
    GetReportLogList().addReportLog(QString("Message 11"),GS::Gex::ReportLog::ReportError);
#endif

}

///////////////////////////////////////////////////////////
// Destructor : Closes file if not done,
// reset all private variables.
///////////////////////////////////////////////////////////
CGexReport::~CGexReport()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Deleting report %1...").arg( mId).toLatin1().constData());
    sNumberOfIntances--;

    clearInteractiveTestFilter();

    {
        QSqlDatabase lReportDB = QSqlDatabase::database(mReportDBName);
        if (lReportDB.isOpen())
            lReportDB.close();
    }
    QSqlDatabase::removeDatabase(mReportDBName);

    // Close files in case process was aborted.
    if(hReportFile != NULL)
        CloseReportFile();	// Close report file

    if(hAdvancedReport != NULL)
        CloseReportFile(hAdvancedReport);

    // Destroy 'ChartDirector' drawing object
    if(m_pChartDirector != NULL)
    {
        delete m_pChartDirector;
        m_pChartDirector = NULL;
    }

    if(m_pChartsInfo != NULL)
    {
        delete m_pChartsInfo;
        m_pChartsInfo = NULL;
    }

    while (getGroupsList().isEmpty() == false)
        delete getGroupsList().takeFirst();

    GS::Core::MultiLimitItem::sNbMaxLimits = 0;

    while (m_pTestCustomFieldsList.isEmpty() == false)
        delete m_pTestCustomFieldsList.takeFirst();

    if (m_pAdvancedEnterpriseReport)
    {
        delete m_pAdvancedEnterpriseReport;
        m_pAdvancedEnterpriseReport = NULL;
    }

    if (m_pScriptEngine)
    {
        delete m_pScriptEngine;
        m_pScriptEngine = NULL;
    }

    foreach(GS::Gex::ReportUnit* ru, mReportUnits.values())
        if (ru)
            delete ru;
    mReportUnits.clear();

    mReportLogList.clear();

    // Hervé: is it the right moment to delete the static overall Results table
    if (CTestResult::sResults)
    {
        free(CTestResult::sResults);
        CTestResult::sResults=0;
    }

    QMap<QString, StdfDataFile*>::iterator lIter(mStdfDataFiles.begin()), lIterEnd(mStdfDataFiles.end());
    for(;lIter!= lIterEnd;++lIter)
    {
        delete lIter.value();
    }
    mStdfDataFiles.clear();
    mOrderedStdfDataFiles.clear();
}

void CGexReport::setReportName(const QString &lReportName)
{
    m_strEvaluatedReportName = lReportName;
}

QString CGexReport::ExecQuery(const QString &query, QVector< QVector<QVariant> > &results)
{
    if (mReportDBName.isEmpty())
        return "error : no db connection";

    if (query.isEmpty())
        return "error : query empty";

    QSqlQuery q("", QSqlDatabase::database(mReportDBName));
    //QSqlQuery q("", mReportDB);
    //mReportDB.

    bool b=q.exec(query);
    if (!b)
        return "error : "+ q.lastError().text();

    //if (q.size()==0)
      //  return "ok : but no results";
    // do me : do we parse results now or do we send the result to the client of the function ?

    QVector<QVariant> m;
    QSqlRecord rec = q.record();
    for(int i=0; i<rec.count(); i++)
        m.append( rec.fieldName(i) );
    results.append(m);

    m.clear();
    while(q.next())
    {
        for (int i=0; i<rec.count(); i++)
            m.append(q.value(i));
        results.append(m);
        m.clear();
    }

    q.clear();

    return "ok";
}

bool CGexReport::isCompleted() const
{
    return mCompleted;
}

void CGexReport::setCompleted(bool completed)
{
    mCompleted = completed;
}

///////////////////////////////////////////////////////////
// reset all variables.
///////////////////////////////////////////////////////////
bool CGexReport::reset()        // reset all CGexReport attributes
{
    // Close files in case process was aborted.
    if(hReportFile != NULL)
        CloseReportFile();	// Close report file

    if(hAdvancedReport != NULL)
        CloseReportFile(hAdvancedReport);

    // Destroy 'ChartDirector' drawing object
    if(m_pChartDirector != NULL)
    {
        delete m_pChartDirector;
        m_pChartDirector	= new CChartDirector();		// ChartDirector drawing object
    }

    if(m_pChartsInfo != NULL)
    {
        delete m_pChartsInfo;
        m_pChartsInfo = new CGexChartOverlays;   // Holds layers when in interactive mode.
    }

    // Clear ScatterChart cache (Linear regression,...)
    GexScatterChart::clearCache();

    while (!getGroupsList().isEmpty())
        delete getGroupsList().takeFirst();

    while (!m_pTestCustomFieldsList.isEmpty())
        delete m_pTestCustomFieldsList.takeFirst();

    if (m_pScriptEngine)
    {
        delete m_pScriptEngine;

        #ifdef GEX_NO_JS_SOLARIS
        m_pScriptEngine = NULL;
        #else
        m_pScriptEngine = new QScriptEngine();
        #endif
    }

    if (m_pAdvancedEnterpriseReport)
    {
        delete m_pAdvancedEnterpriseReport;
        m_pAdvancedEnterpriseReport = NULL;
    }

    /// TODO : check who delete hReportFile, hAdvancedReport
    hReportFile = NULL;			// Makes sure handle is NULL.
    hAdvancedReport = NULL;		// Makes sure handle is NULL.
    m_pReportOptions = NULL;

    // Reset variables.
    mCompleted = false;
    lfTotalFileSize = 0;	// Total size of all files to process
    iScriptExitCode = GS::StdLib::Stdf::NoError;

    // Ensure all pointers are well initialized.
    m_tProcessTime.start();	// Current time => will be used to compute time spent processing file.

    // Html font size
    iHthmSmallFontSize = 3;			// Holds the 'small'font size to use in HTML pages...may be overwritten at run time depending of output format (HTML, DOC,...) and platform (PC, unix,...)
    iHthmNormalFontSize = 3;		// Holds the 'standard' font size to use in HTML pages...may be overwritten at run time depending of output format (HTML, DOC,...) and platform (PC, unix,...)
    iHthmSmallFontSizePixels = 10;	// 12pixels: Holds the 'small'font size to use in HTML pages: 12 = normal pixel size, 10 = small
    iHthmNormalFontSizePixels = 12;	// 12pixels: Holds the 'standard' font size to use in HTML pages

    iLinesPerFlatPage		   = MAX_STATSLINES_PER_FLAT_PAGE;			// Total number of lines per flat page (may vary if page is for Word, or PPT,...)
    iLinesPercentagePerFlatPage = MAX_PERCENTAGE_LINES_PER_WORD_PAGE;	// Total number of lines per flat page (may vary if page is for Word, or PPT,...)

    // MAP structures used if must ignore test# and only rely on test name.
    m_testNameMap.clear();

    // Set the status to false when starting a new report
    m_eAutoRepairOption = repairUndefined;

    m_strJsReportName = "";
    m_strJsReportSubFolders = "";
    m_strJsReportRootFolderAbsPath = "";
    m_strEvaluatedReportName = "";
    m_strEvaluatedReportSubFolders = "";
    m_strEvaluatedReportRootFolderAbsPath = "";
    m_strEvaluatedReportType = "";
    m_strReportGenerationMode = "";
    m_strUserDefRepotFileName = "";
    m_strLegacyReportAbsFilePath = "";
    m_strLegacyReportRootFolder = "";
    m_strOutputFormat = "";
    m_strEvaluatedReportRelativeFilePath = "";
    m_bUserAlreadyAsked = false;

    // everything went well !
    return true;
}


int CGexReport::addFile(int group,
                        QString fileName,
                        int processSite,
                        int processBins,
                        const char *rangeList,
                        QString mapTests,
                        QString waferToExtract,
                        double	temperature,
                        QString datasetName,
                        QString samplegroup)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("add file %1").arg( fileName).toLatin1().constData());
    GS::Gex::Engine::GetInstance().UpdateLabelStatus(QString(" Adding file %1").arg(fileName));
    QCoreApplication::processEvents();

    // Add file to group
    CGexGroupOfFiles *pGroup = (group < 0 || group >= getGroupsList().size()) ? NULL : getGroupsList().at(group);
    if(pGroup == NULL)
        return -1;

    // Add file to group.
    CGexFileInGroup *pFile = new CGexFileInGroup(pGroup, group, fileName,
        processSite,processBins, rangeList, mapTests, waferToExtract, temperature, datasetName, samplegroup);
    /// TO REVIEW : CGexFileInGroup constructor include parsing treatment !

    QMap<QString, StdfDataFile*>::iterator lIter=  mStdfDataFiles.find(pFile->strFileNameSTDF);
    if(lIter == mStdfDataFiles.end())
    {
        lIter = mStdfDataFiles.insert(pFile->strFileNameSTDF, new StdfDataFile(pFile->strFileNameSTDF,
                                                                               hAdvancedReport,
                                                                               m_pReportOptions,
                                                                               waferToExtract));

        mOrderedStdfDataFiles.push_back(mStdfDataFiles[pFile->strFileNameSTDF]);
    }

    lIter.value()->insertFileInGroup(processSite, pFile);


    if(getGroupsList().isEmpty())  // hack to check if user clicked abort button during file conversion
        return -1;

    // Add file to its group
    pGroup->pFilesList.append(pFile);

    // Return file ID
    pFile->lFileID = pGroup->pFilesList.count()-1;
    return pFile->lFileID;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a color to plot a chart for given group.
/////////////////////////////////////////////////////////////////////////////
QColor	CGexReport::GetChartingColor(int iGroupIndex)
{
    // Select color used to plot this chart
    switch((iGroupIndex-1) % 12)
    {
    case 0:
        return QColor(101,152,76);	// Light green
    case 1:
        return QColor(255,0,255);	// Magenta
    case 2:
        return QColor(0,0,128);		// Blue-grey
    case 3:
        return QColor(255,255,0);	// Yellow
    case 4:
        return QColor(0,255,255);	// Blue lagoon
    case 5:
        return QColor(128,0,128);	// Violet
    case 6:
        return QColor(128,0,0);		// Brown
    case 7:
        return QColor(255,153,0);	// brighter Brown-orange
    case 8:
        return QColor(0,0,255);		// Full blue
    case 9:
        return QColor(0,204,255);	// Light blue lagoon
    case 10:
        return QColor(0,0,0);		// Black
    case 11:
        return QColor(0,128,0);		// Green
    }
    // Should never happen
    return Qt::blue;
}

bool CGexReport::getProcessingFile() const
{
    return mProcessingFile;
}

///////////////////////////////////////////////////////////
// Return output location set in options
///////////////////////////////////////////////////////////
QString CGexReport::outputLocation(CReportOptions	*pReportOptions)
{
    QString strOutputLocation = pReportOptions->GetOption("output","location").toString();

    if (strOutputLocation == "specified")
        return pReportOptions->GetOption("output","location_path").toString();
    else if (strOutputLocation == "same")
        return QLatin1String("");
    else // if (strOutputLocation == "default" )
    {
        return QLatin1String("default");
    }
}


int CGexReport::validSamplesExecsOverGroups(unsigned int testNumber,
                                          int pinmapIndex,
                                          QString testName,
                                          QList<CGexGroupOfFiles*> groupsList)
{
    int countSamplesValidExecs = 0;
    CGexGroupOfFiles *  pGroup = NULL;
    CGexFileInGroup *   pFile = NULL;
    CTest*              testCell = NULL;

    QListIterator<CGexGroupOfFiles*> itGroupsList(groupsList);
    while (itGroupsList.hasNext())
    {
        // Find test cell: RESET list to ensure we scan list of the right group!
        pGroup	= itGroupsList.next();
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        if (pFile &&
         pFile->FindTestCell(testNumber,
                             pinmapIndex,
                             &testCell,
                             false,
                             false,
                             testName.toLatin1().data()) == 1)
        {
            countSamplesValidExecs += testCell->ldSamplesValidExecs;
        }
    }

    return countSamplesValidExecs;
}

void CGexReport::CreateAdvCharac1ChartImageEx(CGexChartOverlays * pChartsInfo,
                                              CTest * ptTestCell,
                                              int iChartSize,
                                              QString strImage)
{
    GexCharacBoxWhiskerChart nxpChart(iChartSize, 0, pChartsInfo);
    nxpChart.setViewportMode(m_pReportOptions->getAdvancedReportSettings());
    nxpChart.computeData(m_pReportOptions, ptTestCell);
    nxpChart.buildChart(GEX_CHARTSIZE_LARGE_X, GEX_CHARTSIZE_LARGE_Y);
    nxpChart.drawChart(strImage, GetImageCopyrightString());

//    GexCharacHistoChart nxpChart(iChartSize, pChartsInfo);
//    nxpChart.setViewportMode(m_pReportOptions->iAdvancedReportSettings);
//    nxpChart.computeData(m_pReportOptions, ptTestCell);
//    nxpChart.buildChart(GEX_CHARTSIZE_LARGE_X/6, GEX_CHARTSIZE_LARGE_Y);
//    nxpChart.drawChart(strImage, GetImageCopyrightString());
}

// return true a test condition exist with this name
bool CGexReport::IsExistingTestConditions(const QString& testCondition)
{
    return m_oConditionList.contains(testCondition);
}

//returns the number of test conditions defined
int CGexReport::GetTestConditionsCount() const
{
    return m_oConditionList.count();
}

// returns the Test Conditions name for the given index
QString CGexReport::GetTestConditions(int iIdx)
{
    if(iIdx>=0 && iIdx <m_oConditionList.count())
        return m_oConditionList[iIdx];
    else
        return QString();
}

// adds a new test conditions name to the report object
void CGexReport::AddTestConditions(const QString &strTestConditionName)
{
    if(!m_oConditionList.contains(strTestConditionName))
        m_oConditionList.append(strTestConditionName);
}

// reset the Test conditions list
void CGexReport::ClearTestConditions()
{
    m_oConditionList.clear();
}

CReportOptions *CGexReport::getReportOptions()
{
    return m_pReportOptions;
}

void CGexReport::setReportOptions(CReportOptions *poReportOptions)
{
    m_pReportOptions = poReportOptions;
}

GS::QtLib::NumberFormat *CGexReport::getNumberFormat()
{
    return &mNumberFormat;
}

GS::Gex::ReportLogList &CGexReport::GetReportLogList()
{
    return mReportLogList;
}


StdfDataFile*     CGexReport::FindStdfDataFile (const QString& name)
{
    QList<StdfDataFile*>::iterator lIter(mOrderedStdfDataFiles.begin()), lIterEnd(mOrderedStdfDataFiles.end());
    for(; lIter != lIterEnd; ++lIter)
    {
        if((*lIter)->getFileName().contains(name))
            return (*lIter);
    }
    return 0;
}

