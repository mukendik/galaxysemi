///////////////////////////////////////////////////////////
// Scripting commands to control GEX.
///////////////////////////////////////////////////////////

#include <qregexp.h>
#include <sstream>
#include <unistd.h>

#include "gex_version.h"
#include "gex_shared.h"
#include "engine.h"
#include "zcsl.hpp"

#include <gqtl_sysutils.h>

// Holds the message thrown by the script exception...
#include "../report_build.h"
#include "../report_options.h"
#include "../interactive_charts.h"		// Layer Style class.
#include "../drill_chart.h"
#include "../script_wizard.h"
#include "../scripting_io.h"
#include "../gex_web.h"
#include "../gex_report.h"
#include "../db_engine.h"
#include "../db_transactions.h"
#include "../gex_file_in_group.h"
#include "../gex_group_of_files.h"
#include "../export_ascii_dialog.h"			// USed for Toolbox: STDF Dump function.
#include "export_ascii_input_file.h"
#include "export_ascii_input_filelist.h"
#include "../export_csv.h"
#include "../db_external_database.h"
#include "../classes.h"
#include <gqtl_log.h>
#include "product_info.h"
#include "tb_merge_retest_dialog.h"
#include "tb_merge_retest.h"
#include "gex_database_entry.h"
#include "charac_line_chart_template.h"
#include "charac_box_whisker_template.h"
#include "ofr_controller.h"

#ifdef unix
#include <unistd.h>
#endif

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
//#include "DebugMemory.h"

// main
extern GexMainwindow	*pGexMainWindow;
extern CReportOptions	ReportOptions;	// Holds options (report_build.h)
extern CGexReport *gexReport;			// Handle to report class
extern GexScriptEngine*	pGexScriptEngine;

// in settings_dialog.cpp
extern QString ImportTestListFromFile(const QString &strTestListFile);
extern bool checkProfessionalFeature(bool bProductionReleaseOnly);

static	GexDatabaseQuery			cQuery;				// Holds Query parameters.
static	CSTDFtoASCII *				ptStdfToAscii=NULL;	// Toolbox: Dump STDF file
CGexSingleChart *			s_pChartStyle;		// Holds Chart style (color, line style & width,...)
static	GS::Gex::ExportAscii_InputFileList	m_pInputFiles;

CBinColor	cBinColor;			// Holds the Bin# and color.
static	int	m_lInteractiveTestX;		// Test in X to plot (interactive mode only)
static	int	m_lInteractiveTestPmX;		// Test Pinmap in X to plot (interactive mode only)
static	int	m_lGroupX;					// GroupID to use in X
static	int	m_lInteractiveTestY;		// Test in Y to plot (interactive mode only)
static	int	m_lInteractiveTestPmY;		// Test Pinmap in Y to plot (interactive mode only)
static	int	m_lGroupY;					// GroupID to use in X
static	int	m_InteractiveLayer;			// Keeps track of total layers inserted by the scripts.
float m_fCslVersion=0.0f;			// Csl version

static QMap<QString, QString> staticMapMergeOptions;

extern ZString gexOptions(ZCsl* csl);

////////////////////////////////////////////////////////////////////////////////
// Return handle to cQuery local structure.
////////////////////////////////////////////////////////////////////////////////
const GexDatabaseQuery &getScriptQuery(void)
{
    return cQuery;
}


// To be fixed !!!!
//////////////////////////////////////////////////////////////////////////////////
//// Update the ASP/HTML page that lists the HTML reports available + return its name
//// Call : gexUpdateWebReportsList();
//////////////////////////////////////////////////////////////////////////////////
//static QString gexUpdateWebReportsList(void)
//{
//    // Only applies to ExaminatorWeb!
////    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GEX_DATATYPE_ALLOWED_DATABASEWEB)
////        return "";

//    // Build full path to the HTML page to build
//    QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
//    strShowPage += GEX_HTMLPAGE_WEB_REPORTS;

//    pGexMainWindow->mGexWeb->htmlBuildAdminReports(strShowPage);

//    // Return path the the ASP/HTML page created.
//    return strShowPage;
//}

////////////////////////////////////////////////////////////////////////////////
// Update the ASP/HTML page that lists the databases available + return its name
// Call : gexUpdateWebDatabasesList();
////////////////////////////////////////////////////////////////////////////////
static QString gexUpdateWebDatabasesList(void)
{
    // Only applies to ExaminatorWeb!
//    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GEX_DATATYPE_ALLOWED_DATABASEWEB)
//        return "";

    // Build full path to the HTML page to build
    QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
    strShowPage += GEX_HTMLPAGE_WEB_DATABASES;

    pGexMainWindow->mGexWeb->htmlBuildAdminDatabases(strShowPage);

    // Return path the the ASP/HTML page created.
    return strShowPage;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexConvertSTDF: Convert file to STDF (Examinator-MONITORING ONLY)
// Call : gexConvertSTDF(QString strInputFile);
////////////////////////////////////////////////////////////////////////////////
static ZString gexConvertSTDF(ZCsl* csl)
{
    // Only applies to Examinator-Monitoring!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isYieldMan() && !GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
      return 0;

    int argc = csl->get("argCount").asInt();

    GS::Gex::ConvertToSTDF  StdfConvert;	// To Convert to STDF
    QString                 strFileNameSTDF;		// To receive STDF file name created.
    QString                 strErrorMessage;
    bool                    bFileCreated;
    const char *            szInputFile	= csl->get("input_file").constBuffer();		// Pointer to input file to convert.

    // build output file name to create
    if(argc == 2)
    {
        // The script has specified a output file...
        strFileNameSTDF = csl->get("output_file").constBuffer();
    }
    else
    {
        strFileNameSTDF = szInputFile;
        strFileNameSTDF += "_gexmo.std";
    }

    // Convert file to STDF
    int nConvertStatus = StdfConvert.Convert(true,false,false, true, szInputFile, strFileNameSTDF,"",bFileCreated,strErrorMessage);
    GSLOG(SYSLOG_SEV_DEBUG, QString("StdfConvert.Convert returned %1").arg( nConvertStatus).toLatin1().constData());
    // If conversion failed...return error code
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
        return 0;

    return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Function gexConvertCSV: Convert file to CSV (Examinator-MONITORING ONLY)
// Call : gexConvertCSV(QString strInputFile);
////////////////////////////////////////////////////////////////////////////////
static ZString gexConvertCSV(ZCsl* csl)
{
   // Only applies to Examinator-Monitoring!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isYieldMan() && ! GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
    {
      GSLOG(SYSLOG_SEV_WARNING, "This function is available when Monitoring feature is activated. Aborting.");
      return 0;
    }

    int argc = csl->get("argCount").asInt();

    GS::Gex::ConvertToSTDF  StdfConvert;	// To Convert to STDF
    CSTDFtoCSV              CsvConvert;
    QString                 strFileNameSTDF;		// To receive STDF file name created.
    QString                 strFileNameCSV;			// To receive CSV file name created.
    QString                 strErrorMessage;
    const char *            szInputFile = csl->get("input_file").constBuffer();
    bool                    bStdfFileCreated;

    // Intermediate STDF file to create (unless already STDF)
    strFileNameSTDF = szInputFile;
    strFileNameSTDF += "_gexmo.std";

    // build output file name to create
    if(argc == 2)
    {
        // The script has specified a output file...
        strFileNameCSV = csl->get("output_file").constBuffer();
    }
    else
    {
        strFileNameCSV = szInputFile;
        strFileNameCSV += "_gexmo.csv";
    }

    // Convert file to STDF
    bool bResult;
    int nConvertStatus = StdfConvert.Convert(true,false,false, true, szInputFile, strFileNameSTDF,"",bStdfFileCreated,strErrorMessage);
    GSLOG(SYSLOG_SEV_DEBUG, QString("StdfConvert.Convert returned %1").arg( nConvertStatus).toLatin1().constData());
    // If conversion failed...return error code
    if(nConvertStatus == GS::Gex::ConvertToSTDF::eConvertError)
        return 0;

    // If input file was already a STDF file...
    if(!bStdfFileCreated)
        strFileNameSTDF = szInputFile;

    // STDF convertion successful, so now convert to CSV!
    bResult = CsvConvert.Convert(strFileNameSTDF, strFileNameCSV);
    GSLOG(SYSLOG_SEV_DEBUG, QString("CsvConvert.Convert returned %1").arg( bResult?"true":"false").toLatin1().constData());

    // Delete intermediate STDF file (if created)
    if(bStdfFileCreated)
        unlink(strFileNameSTDF.toLatin1().constData());

    // If conversion failed...return error code
    if(bResult == false)
        return 0;

    return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Function gexBuildReport: Generates report (data analysis)
// Call : gexBuildReport(bool ShowReport);
////////////////////////////////////////////////////////////////////////////////
static ZString gexBuildReport(ZCsl* csl)
{
    const char* szShowPage=0;	// Report page to show once report ready

    // Number of arguments
    int argc = csl->get("argCount").asInt();

    // If a list of Html sections NOT to create is given, get list of flags...
    if(argc <= 0)
        szShowPage = "home";	// if no argument, assume to show HOME page
    else
        szShowPage = csl->get("show_page").constBuffer();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("gexBuildReport %1").arg( szShowPage).toLatin1().constData());

    if(argc >= 2)
    {
        //	List of HTML sections NOT to create (used when multi-pass involved such as for 'guard-banding'
        ReportOptions.iHtmlSectionsToSkip = csl->get("sections_ignored").asInt();
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("iHtmlSectionsToSkip = %1").arg(ReportOptions.iHtmlSectionsToSkip).toLatin1().data() );
    }
    // Get report created (some sections may not be created...depends of 'iHtmlSkip')
    QString r(100, '?'); // just to be sure we wont need more mem to fill it

    r = gexReport->BuildReport(&ReportOptions);

    if (r.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("build report failed : %1").arg( r).toLatin1().constData());
        WriteScriptMessage(r, true);
        // case 6419: is there another way to inform
        // the script caller that the build report failed ?
        // ?
        ZTHROWEXC(r.toLatin1().data());
        return 0;
    }

    ShowReportOnCompletion(szShowPage);

// To be fixed
//    // If ExaminatorWeb running, update the reports table
//    gexUpdateWebReportsList();

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexGenerateDynamicReport: Generates OFR report
// Call : gexGenerateDynamicReport(QString  json_file_name, QString report_file_name, Qstring report_format);
////////////////////////////////////////////////////////////////////////////////
static ZString gexGenerateDynamicReport(ZCsl* csl)
{
    const char* json_file_name=0;
    const char* report_file_name=0;
    const char* report_format=0;

    // Number of arguments
    int argc = csl->get("argCount").asInt();

    if(argc < 2)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Not enough parameters : %1").arg(argc).toLatin1().constData());
    }

    json_file_name = csl->get("json_file_name").constBuffer();
    report_file_name = csl->get("report_file_name").constBuffer();

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("gexGenerateDynamicReport with json file %1 and report file name %2")
                                    .arg(json_file_name)
                                    .arg(report_file_name).toLatin1().constData());

    // Get report created (some sections may not be created...depends of 'iHtmlSkip')
    QString lReturn(100, '?'); // just to be sure we wont need more mem to fill it
    if(argc == 2)
    {
        QMap< QString,  QString> lParams;
        QString lFormat("pdf");
        lParams.insert("report_format", lFormat);
        if (!GS::Gex::OFR_Controller::GetInstance()->GenerateReport(json_file_name, report_file_name, lParams))
            lReturn = "error generating report";
        else
            lReturn = "Ok";
    }
    else if (argc == 3)
    {
        report_format = csl->get("report_format").constBuffer();
        QMap<QString, QString> lParams;
        lParams.insert(QString("report_format"), report_format);
        if (!GS::Gex::OFR_Controller::GetInstance()->GenerateReport(json_file_name, report_file_name, lParams))
            lReturn = "error generating report";
        else
            lReturn = "Ok";
    }

    if (lReturn.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_WARNING, lReturn.toLatin1().constData());
        WriteScriptMessage(lReturn, true);
        // case 6419: is there another way to inform
        // the script caller that the build report failed ?
        // ?
        ZTHROWEXC(lReturn.toLatin1().data());
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexGenerateDynamicReport: Generates OFR report
// Call : gexGenerateDynamicReport(QString  json_file_name, QString report_file_name, Qstring report_format);
////////////////////////////////////////////////////////////////////////////////
static ZString gexAnalyseMode(ZCsl* csl)
{
    QString lAnalyseMode=0;

    lAnalyseMode = QString(csl->get("analyse_mode").constBuffer());

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("analyseMode with mode %1").arg(lAnalyseMode).toLatin1().constData());

    // Get report created (some sections may not be created...depends of 'iHtmlSkip')
    QString lReturn("OK"); // just to be sure we wont need more mem to fill it
    if (lAnalyseMode.compare("SingleFile") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::SingleFile);
    else if (lAnalyseMode.compare("MergeFiles") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::MergeFiles);
    else if (lAnalyseMode.compare("CompareFiles") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::CompareFiles);
    else if (lAnalyseMode.compare("CompareGroupOfFiles") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::CompareGroupOfFiles);
    else if (lAnalyseMode.compare("SingleDataset") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::SingleDataset);
    else if (lAnalyseMode.compare("CompareDatasets") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::CompareDatasets);
    else if (lAnalyseMode.compare("MergeDatasets") == 0)
        pGexMainWindow->SetAnalyseMode(GexMainwindow::MergeDatasets);
    else
    {
        pGexMainWindow->SetAnalyseMode(GexMainwindow::InvalidAnalyse);
        lReturn = "error of the analyse mode";
    }

    if (lReturn.startsWith("error"))
    {
        GSLOG(SYSLOG_SEV_WARNING, lReturn.toLatin1().constData());
        WriteScriptMessage(lReturn, true);
        ZTHROWEXC(lReturn.toLatin1().data());
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ExmaminatorWeb ONLY.
// Function gexWeb: Let's user perform ExaminatorWeb management functions
// Call : gexWeb(action, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexWeb(ZCsl* csl)
{
    const char *	szAction;		// Action to perform
    const char *	szField		= "";	// Field name
    const char *	szValue		= "";	// Field value
    bool			bStatus;

    // If NOT ExaminatorWEB, return!
//    if(GS::LPPlugin::ProductInfo::getInstance()->getProductID() != GEX_DATATYPE_ALLOWED_DATABASEWEB)
//        return 1;	// Silent return.

    // Get number of parameters given
    int argc = csl->get("argCount").asInt();

    szAction = csl->get("action").constBuffer();
    if(argc >= 2)
        szField = csl->get("field").constBuffer();
    if(argc >= 3)
        szValue = csl->get("value").constBuffer();

    // Import all files in $home/<szValue> folder
    if(!qstricmp(szAction,"db_import"))
    {
        // Build full path to the import folder (non-recursive: meaning only files in this folder are imported, not the one in sub-folders).
        QStringList strCorruptedFiles,strProcessedFiles;
        QString strShowPage;
        QString strFolder = pGexMainWindow->strExaminatorWebUserHome + "/";
        strFolder += szValue;

        // Call import method. ImportFolder(DatabaseLogicName,ImportPath,bRecursive,bDeleteAfterImport)
        QString strErrorMessage;
        bStatus = GS::Gex::Engine::GetInstance().GetDatabaseEngine().ImportFolder(szField,strFolder,false,strProcessedFiles,&strCorruptedFiles,strErrorMessage,true);

        if(bStatus == true)
        {
            // Import SUCCESSFUL
            strShowPage = pGexMainWindow->strExaminatorWebUserHome;
            strShowPage += GEX_HELP_FOLDER;
            strShowPage += GEX_HTMLPAGE_WEB_IMPORT;

            // Update the ASP/HTML page that lists the databases+size
            gexUpdateWebDatabasesList();
        }
        else
        {
            // Import FAILURES: Create HTML report in the $home/user
            strShowPage = pGexMainWindow->strExaminatorWebUserHome;
            strShowPage += GEX_DATABASE_WEB_EXCHANGE;
            strShowPage += GEX_HTMLPAGE_WEB_FAILIMPORT;

            pGexMainWindow->mGexWeb->htmlImportFailure(strShowPage,&strCorruptedFiles);
        }

        gexReport->setLegacyReportName(strShowPage);
        // Have it shown when script is executed!
        ShowReportOnCompletion("real_html");
        return 1;	// No error.
    }

    // Rebuild databases HTML admin page
    if(!qstricmp(szAction,"db_update_html"))
    {
        // Update the ASP/HTML page that lists the databases available + return its name
        QString strShowPage = gexUpdateWebDatabasesList();

        gexReport->setLegacyReportName(strShowPage);
        // Have it shown when script is executed!
        ShowReportOnCompletion("real_html");
        return 1;	// No error.
    }

// To be fixed !!!
//    // Rebuild reports HTML admin page
//    if(!qstricmp(szAction,"rpt_update_html"))
//    {
//        // Update the ASP/HTML page that lists the reports available + return its name
//        QString strShowPage = gexUpdateWebReportsList();

//        gexReport->setLegacyReportName(strShowPage);
//        // Have it shown when script is executed!
//        ShowReportOnCompletion("real_html");
//        return 1;	// No error.
//    }

    // Delete specified report+update Reports table HTML page.
    if(!qstricmp(szAction,"rpt_delete"))
    {
        // Normalise report name to report folder name
        QString strReportName = szField;
        CGexSystemUtils::NormalizeString(strReportName);

        // Build path to report folder to erase.
        QString strReportPath = pGexMainWindow->strExaminatorWebUserHome;
        strReportPath += GEX_DATABASE_REPORTS;
        strReportPath += strReportName;

        // Delete folder
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(strReportPath);
        // Remove logical entry from the XML definition file.
        pGexMainWindow->mGexWeb->RemoveReportEntry(szField);

// To be fixed !!!
        // Update the ASP/HTML page that lists the reports available + return its name
        QString strShowPage = "";//gexUpdateWebReportsList();

        gexReport->setLegacyReportName(strShowPage);
        // Have it shown when script is executed!
        ShowReportOnCompletion("real_html");

        return 1;	// No error.
    }

    // Zip + download a report entry....for now doesn't do anything!
    if(!qstricmp(szAction,"rpt_zip"))
    {
        // Build full path to the HTML page to build
        QString strShowPage = pGexMainWindow->strExaminatorWebUserHome + GEX_DATABASE_WEB_EXCHANGE;
        strShowPage += GEX_HTMLPAGE_WEB_REPORTS;

        gexReport->setLegacyReportName(strShowPage);
        // Have it shown when script is executed!
        ShowReportOnCompletion("real_html");

        return 1;	// No error.
    }


    // Create a database entry
    if(!qstricmp(szAction,"db_create"))
    {
        // Fill database creation fields.
        GexDatabaseEntry cDatabaseEntry(0);
        cDatabaseEntry.SetHoldsFileCopy(true);
        cDatabaseEntry.SetCompressed(false);
        cDatabaseEntry.SetLocalDB(true);
        cDatabaseEntry.SetSummaryOnly(false);
        cDatabaseEntry.SetBlackHole(false);
        cDatabaseEntry.SetExternal(false);


        cDatabaseEntry.SetLogicalName(szField);
        cDatabaseEntry.SetDescription(szValue);
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().SaveDatabaseEntry(&cDatabaseEntry);

        // Update the ASP/HTML page that lists the databases
        gexUpdateWebDatabasesList();

        return 1;	// No error.
    }

    // Delete a database entry
    if(!qstricmp(szAction,"db_delete"))
    {
        GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteDatabaseEntry(szField);

        // Update the ASP/HTML page that lists the databases
        gexUpdateWebDatabasesList();

        return 1;	// No error.
    }


    ZTHROWEXC("invalid parameter :"+ZString(szAction));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameter: allows to preset parameter info (limits, markers,...)
////////////////////////////////////////////////////////////////////////////////
static int gexParameterEntry(QString strTest,QString strName,QString strField,QString strValue,QString strLabel,QString &strError)
{
    if(gexReport == NULL)
        return 1;	// Just in case!

    // Test with custom limits for guard banding analysis.
    CTestCustomize* pTest;
    pTest = new CTestCustomize();

    // Save test name
    pTest->strTestName = strName;

    // bLimitWhatIfFlag: bit0=1 (no low limit forced), bit1=1 (no high limit forced)
    pTest->bLimitWhatIfFlag = CTEST_LIMITFLG_NOLTL | CTEST_LIMITFLG_NOHTL;

    // List of tests in this group.
    if(sscanf(strTest.toLatin1().constData(),"%d%*c%d",&(pTest->lTestNumber),&(pTest->lPinmapIndex)) < 2)
        pTest->lPinmapIndex= GEX_PTEST;

    if(strField.endsWith("Marker",Qt::CaseInsensitive))
    {
        // Marker value is in format: <Marker_Position> <line_width> <R G B color> <layer#>
        // eg: '10.5 2 255 0 0'
        // note: if RGB colrs are - &-1 -1, then color to be used is same as layer charting color
        double lfPos;
        int	iLine,iR,iG,iB,iLayer=-1;
        if(strValue.startsWith("-1.#IND", Qt::CaseInsensitive) ||
           strValue.startsWith("-1.#INF", Qt::CaseInsensitive) ||
           strValue.startsWith("1.#IND", Qt::CaseInsensitive) ||
           strValue.startsWith("1.#INF", Qt::CaseInsensitive))
            return 1;	// Invalid marker, ignore it

        if(sscanf(strValue.toLatin1().constData(),"%lf %d %d %d %d %d",&lfPos,&iLine,&iR,&iG,&iB,&iLayer) < 5)
        {
            strError = "Marker 'value' syntax error";
            return -1;
        }
        pTest->ptMarker = new TestMarker();
        pTest->ptMarker->lfPos = lfPos;
        pTest->ptMarker->iLine = iLine;
        pTest->ptMarker->cColor = QColor(iR,iG,iB);
        pTest->ptMarker->strLabel = strLabel;
        pTest->ptMarker->iLayer = iLayer;
    }
    else
    if(strField.endsWith("Viewport",Qt::CaseInsensitive))
    {
        // Viweport value is in format: <LowX> <HighX> [<LowY> <HighY>]
        // eg: '10.5 50.3 -5 -2'
        pTest->ptChartOptions = new CGexTestChartingOptions();

        double dLowX=0;
        double dLowY=0;
        double dHighX=0;
        double dHighY=0;

        int iParams = sscanf(strValue.toLatin1().constData(),"%lf %lf %lf %lf", &dLowX, &dHighX, &dLowY, &dHighY);


        // Check if valid number of parameters specified.
        if(iParams != 2 &&  iParams != 4)
        {
            strError = "Viewport 'value' syntax error";
            return -1;
        }

        // Set flag 'valid X view port'
        pTest->ptChartOptions->setCustomViewportX(true);
        pTest->ptChartOptions->setLowX(dLowX);
        pTest->ptChartOptions->setHighX(dHighX);

        // If viewport includes Y scales (for scatter plot only), then save info
        if(iParams == 4)
        {
            pTest->ptChartOptions->setCustomViewportY(true);
            pTest->ptChartOptions->setLowY(dLowY);
            pTest->ptChartOptions->setHighY(dHighY);
        }
        else
            pTest->ptChartOptions->setCustomViewportY(false);
    }
    else
    {
        strError = "invalid parameter :" + strField;
        return -1;
    }

    // Add this test to the list of 'test with custom limits/markers,...'
    gexReport->m_pTestCustomFieldsList.append(pTest);

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameter: allows to preset parameter info (limits, markers,...)
////////////////////////////////////////////////////////////////////////////////
static ZString gexParameter(ZCsl* csl)
{
    QString			strErrorMsg;
    const char *	szTest	= csl->get("parameter").constBuffer();		// Test# eg: 10.35
    const char *	szName	= csl->get("name").constBuffer();			// Test name
    const char *	szField = csl->get("field").constBuffer();			// Field. eg: LL or HL, or marker, or viewport
    const char *	szValue = csl->get("value").constBuffer();			// Field value (eg: limit, or marker color & width)
    const char *	szLabel = csl->get("label").constBuffer();			// Field label/comment. E.g: maker name if field = 'marker'

    if(gexParameterEntry(szTest,szName,szField,szValue,szLabel,strErrorMsg) < 0)
        ZTHROWEXC(ZString(strErrorMsg.toLatin1().constData()));

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexParameterFile: allows to preset a list of parameter info (limits, markers,...)
// listed in a file
////////////////////////////////////////////////////////////////////////////////
static ZString gexParameterFile(ZCsl* csl)
{
    QString			strString;
    const char *	szFileName = csl->get("parameter").constBuffer();	// File name where parameters are listed

    // Check file exists
    if(QFile(szFileName).exists() == false)
        ZTHROWEXC("File doesn't exist :"+ZString(szFileName));

    // Open file and parse it.
    QFile f(szFileName);
    if(!f.open( QIODevice::ReadOnly ))
    {
        strString = "Failed reading file:" + QString(szFileName);
        ZTHROWEXC(ZString(strString.toLatin1().constData()));
    }

    // Assign file I/O stream
    QString strTestNumber,strTestName,strField,strValue,strLabel,strErrorMsg;
    QTextStream hFile(&f);
    do
    {
        // Read line.
        strString = hFile.readLine();
        strString = strString.trimmed();

        // Check if comment line
        if(strString.startsWith("#"))
        {
            // comment line...simply do nothing! Ignore this line!
        }
        else
        if(strString.startsWith("gexParameter"))
        {
            // Parse 'gexParameter' line!
            // Format:   gexParameter('14798','iddq_dev_flash_tst_vCEG PstStrs:92 1','Marker','6.76033e-006 2 255 0 255 1','+N');
            strString.replace("\\'","'");
            strTestNumber = strString.section('\'',1,1);
            strTestName = strString.section("','",1,1);
            strField = strString.section("','",2,2);
            strValue = strString.section("','",3,3);
            strLabel = strString.section("','",4,4);

            if(gexParameterEntry(strTestNumber,strTestName,strField,strValue,strLabel,strErrorMsg) < 0)
                ZTHROWEXC(ZString(strErrorMsg.toLatin1().constData()));

        }
    }
    while(hFile.atEnd() == false);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexToolbox: Toolbox functions 'eg: STDF Dump to ASCII, etc...)
////////////////////////////////////////////////////////////////////////////////
static ZString gexToolbox(ZCsl* csl)
{
    const char *	szAction	= "";	// Toolbox action
    const char *	szField		= "";	// Field. eg: 'name'
    const char *	szValue		= "";	// Field value (eg: 'c:/files/file.stdf')
    const char *	szOptional	= "";	// Optional Field value (eg: 'c:/output-folder')

    if(checkProfessionalFeature(true) == false)
        return 0;

    // Create object
    if(ptStdfToAscii == NULL)
    {
        ptStdfToAscii = new CSTDFtoASCII(false);
        // Clear list of input files
        m_pInputFiles.clear();
        // Default: dump ALL records
        ptStdfToAscii->SetProcessRecord(true);
    }

    int argc	= csl->get("argCount").asInt();
    szAction	= csl->get("action").constBuffer();
    szField		= csl->get("field").constBuffer();

    if(argc >= 3)
        szValue = csl->get("value").constBuffer();
    if(argc >= 4)
        szOptional = csl->get("optional").constBuffer();

    if(!qstricmp(szAction,"stdf_dump"))
    {
        if(!qstricmp(szField,"records"))
        {
            // Define STDF records to dump
            if(!qstricmp(szValue,"clear"))
            {
                // Clear all records
                ptStdfToAscii->SetProcessRecord(false);
                return 1;
            }
            if(!qstricmp(szValue,"all"))
            {
                // Set all records
                ptStdfToAscii->SetProcessRecord(true);
                return 1;
            }
            if(!qstricmp(szValue,"ATR"))
            {
                // Dump of ATR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_ATR);
                return 1;
            }
            if(!qstricmp(szValue,"BPS"))
            {
                // Dump of BPS records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_BPS);
                return 1;
            }
            if(!qstricmp(szValue,"DTR"))
            {
                // Dump of DTR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_DTR);
                return 1;
            }
            if(!qstricmp(szValue,"EPS"))
            {
                // Dump of EPS records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_EPS);
                return 1;
            }
            if(!qstricmp(szValue,"FAR"))
            {
                // Dump of FAR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_FAR);
                return 1;
            }
            if(!qstricmp(szValue,"FTR"))
            {
                // Dump of FTR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_FTR);
                return 1;
            }
            if(!qstricmp(szValue,"GDR"))
            {
                // Dump of GDR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_GDR);
                return 1;
            }
            if(!qstricmp(szValue,"HBR"))
            {
                // Dump of HBR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_HBR);
                return 1;
            }
            if(!qstricmp(szValue,"MIR"))
            {
                // Dump of MIR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_MIR);
                return 1;
            }
            if(!qstricmp(szValue,"MPR"))
            {
                // Dump of MPR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_MPR);
                return 1;
            }
            if(!qstricmp(szValue,"MRR"))
            {
                // Dump of MRR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_MRR);
                return 1;
            }
            if(!qstricmp(szValue,"PCR"))
            {
                // Dump of PCR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PCR);
                return 1;
            }
            if(!qstricmp(szValue,"PGR"))
            {
                // Dump of PGR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PGR);
                return 1;
            }
            if(!qstricmp(szValue,"PIR"))
            {
                // Dump of PIR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PIR);
                return 1;
            }
            if(!qstricmp(szValue,"PLR"))
            {
                // Dump of PLR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PLR);
                return 1;
            }
            if(!qstricmp(szValue,"PMR"))
            {
                // Dump of PMR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PMR);
                return 1;
            }
            if(!qstricmp(szValue,"PRR"))
            {
                // Dump of PRR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PRR);
                return 1;
            }
            if(!qstricmp(szValue,"PTR"))
            {
                // Dump of PTR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_PTR);
                return 1;
            }
            if(!qstricmp(szValue,"RDR"))
            {
                // Dump of RDR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_RDR);
                return 1;
            }
            if(!qstricmp(szValue,"SBR"))
            {
                // Dump of SBR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_SBR);
                return 1;
            }
            if(!qstricmp(szValue,"SDR"))
            {
                // Dump of SDR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_SDR);
                return 1;
            }
            if(!qstricmp(szValue,"TSR"))
            {
                // Dump of TSR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_TSR);
                return 1;
            }
            if(!qstricmp(szValue,"WCR"))
            {
                // Dump of WCR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_WCR);
                return 1;
            }
            if(!qstricmp(szValue,"WIR"))
            {
                // Dump of WIR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_WIR);
                return 1;
            }
            if(!qstricmp(szValue,"WRR"))
            {
                // Dump of WRR records
                ptStdfToAscii->SetProcessRecord(GQTL_STDF::Stdf_Record::Rec_WRR);
                return 1;
            }
            ZTHROWEXC("invalid record field :"+ZString(szField));
        }
        else
        if(!qstricmp(szField,"style"))
        {
            // DUMP header will display the style selected
            ptStdfToAscii->SetSizeFileLimitOptionText(szValue);

            // Define dump style
            if(!qstricmp(szValue,"short"))
            {
                // Dump in the short form: only record names
                ptStdfToAscii->SetFieldFilter(GQTL_STDF::Stdf_Record::FieldFlag_None);
                return 1;
            }
            if(!qstricmp(szValue,"detailed"))
            {
                // Dump in the detailed form: list all record fields
                ptStdfToAscii->SetFieldFilter(GQTL_STDF::Stdf_Record::FieldFlag_Present);
                return 1;
            }
            ZTHROWEXC("invalid style :"+ZString(szField));
        }
        else
        if(!qstricmp(szField,"size"))
        {
            // Define maximum dump size (0=no limit)
            long	ldSize;
            if(sscanf(szValue,"%ld",&ldSize) != 1)
                ldSize = 0;
            ptStdfToAscii->SetSizeFileLimit(ldSize);
        }
        else
        if(!qstricmp(szField,"consecutive"))
        {
            // Dump consecutive records of same type
            if(!qstricmp(szValue,"yes"))
            {
                // Dump in the short form: only record names
                ptStdfToAscii->SetConsecutiveRecordOption(true);
                return 1;
            }
            if(!qstricmp(szValue,"no"))
            {
                //: DO NOT dump consecutive records of same type
                ptStdfToAscii->SetConsecutiveRecordOption(false);
                return 1;
            }
            ZTHROWEXC("invalid value :"+ZString(szField));
        }
        else
        if(!qstricmp(szField,"file"))
        {
            // Add STDF name to the dump list
            m_pInputFiles.Append(szValue,szOptional);
            return 1;
        }
        else
        if(!qstricmp(szField,"dump"))
        {
            // Trigger dump action now!
            QString strError;
            GS::Gex::ExportAscii_FileInfo	*pclFileInfo = m_pInputFiles.FirstStdf(GS::Gex::ExportAscii_FileInfo::FileType_STDF_V4);

            // If no file dumped, tell it!
            if(pclFileInfo == NULL)
            {
                strError = "'stdf_dump': No file dumped. Invalid input format?";
                WriteScriptMessage(strError,true);	// Text + EOL
            }

            while(pclFileInfo)
            {
                // Convert files
                if (pclFileInfo->Convert(*ptStdfToAscii, "") != true)
                {
                    // Error Dumping file, display file & error
                    ptStdfToAscii->GetLastError(strError);
                    QString strMessage;
                    strMessage.sprintf("Error dumping STDF file to ASCII.\nFile: %s", strError.toLatin1().constData());
                   // Send error message to Script Window.
                   WriteScriptMessage(strMessage,true);	// Text + EOL
                }

                pclFileInfo = m_pInputFiles.NextStdf(GS::Gex::ExportAscii_FileInfo::FileType_STDF_V4);
            }

            // Clear list of input files
            m_pInputFiles.clear();

            // Delete Dump object.
            delete ptStdfToAscii;
            ptStdfToAscii = NULL;

            return 1;
        }
        ZTHROWEXC("invalid field :"+ZString(szField));

    }
    ZTHROWEXC("invalid action :"+ZString(szAction));

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexFavoriteScripts: Let's user define the GEX list of 'Favorite Scripts'
// Call : gexFavoriteScripts(action [,path, title);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFavoriteScripts(ZCsl* csl)
{
    const char *	szAction;	// Action: 'clear' or 'insert'
    const char *	szPath;	// Script path
    const char *	szTitle;	// Script description Title
    int				argc = csl->get("argCount").asInt();

    QString strMessage;

    switch (argc)
    {
      case 1:
            // If only one argument, must be 'clear'
            szAction = csl->get("action").constBuffer();
            if(!qstricmp(szAction,"clear"))
            {
                EraseFavoriteList();
                return 1;	// No error.
            }
            strMessage = QString("gexFavoriteScripts('%1',...): invalid parameter").arg(szAction);
            WriteScriptMessage(strMessage,true);
        break;

      case 3:
            // If only one argument, must be 'insert'
            szAction = csl->get("action").constBuffer();
            szPath   = csl->get("path").constBuffer();
            szTitle  = csl->get("title").constBuffer();
            if(!qstricmp(szAction,"insert"))
            {
                InsertFavoriteList(szPath,szTitle);
                return 1;	// No error.
            }
            strMessage = QString("gexFavoriteScripts('%1','%2','%3'): invalid parameter").arg(szAction,szPath,szTitle);
            WriteScriptMessage(strMessage,true);
            break;

      default:
            strMessage = QString("gexFavoriteScripts(...): invalid number of parameters");
            WriteScriptMessage(strMessage,true);
            break;
   } // switch
    return 0;
} // gexFavoriteScripts

////////////////////////////////////////////////////////////////////////////////
// Function gexFile: Add file to group.
// Call : gexFile(group_id, action, file, site, process [,range,...]);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFile(ZCsl* csl)
{
    long			lGroupID;	// Group_ID to which this file will belong
    const char *	szAction=0;	// Action
    const char*     szFile=0;	// STDF file to process
    const char *	szSite=0;	// Site(s) to process
    const char *	szProcess=0; // Type of parts to process.
    const char *	szRangeList=0;	// Range of parts/bins to process
    const char *	szMapTests			= "";	// .CSV Test number mapping file
    const char *	szWaferToExtract	= ""; // WaferID to extract.
    const char *	szDatasetName		= ""; // Dataset name if any associated with file...
    const char *	szSampleGroup       = ""; // Sample group associated with the file
    double			lfTemperature		= -1000;	// Testing temperature (if applicable).

    int		iProcessSite;
    int		iProcessBins=0;

    lGroupID	= csl->get("group_id").asLong();
    szAction	= csl->get("action").constBuffer();
    szFile		= csl->get("file").constBuffer();
    szSite		= csl->get("site").constBuffer();
    szProcess	= csl->get("process").constBuffer();

  GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("gex file %1").arg( szFile?szFile:"?").toLatin1().constData());

    if(qstricmp(szAction,"insert"))
        ZTHROWEXC("invalid parameter :"+ZString(szAction));

    // We are processing data file
    gexReport->SetProcessingFile(true);

    // Check sites to process
    if(!qstricmp(szSite,"all"))
        iProcessSite = -1;	// Will process ALL sites
    else
    {
        // Check which specific site# is given
        iProcessSite = (long) csl->get("site").asLong();
    }

    // Check for type of parts to process
    if(!qstricmp(szProcess,"all"))
        iProcessBins = GEX_PROCESSPART_ALL;
    else if(!qstricmp(szProcess,"allparts_except"))
        iProcessBins = GEX_PROCESSPART_EXPARTLIST;
    else if(!qstricmp(szProcess,"good"))
        iProcessBins = GEX_PROCESSPART_GOOD;
    else if(!qstricmp(szProcess,"fails"))
        iProcessBins = GEX_PROCESSPART_FAIL;
    else if(!qstricmp(szProcess,"parts"))
        iProcessBins = GEX_PROCESSPART_PARTLIST;
    else if(!qstricmp(szProcess,"bins"))
        iProcessBins = GEX_PROCESSPART_SBINLIST;
    else if(!qstricmp(szProcess,"allbins_except"))
        iProcessBins = GEX_PROCESSPART_EXSBINLIST;
    else if(!qstricmp(szProcess,"hbins"))
        iProcessBins = GEX_PROCESSPART_HBINLIST;
    else if(!qstricmp(szProcess,"allhbins_except"))
        iProcessBins = GEX_PROCESSPART_EXHBINLIST;
    else if(!qstricmp(szProcess,"odd_parts"))
        iProcessBins = GEX_PROCESSPART_ODD;
    else if(!qstricmp(szProcess,"even_parts"))
        iProcessBins = GEX_PROCESSPART_EVEN;
    else if(!qstricmp(szProcess,"first_instance"))
        iProcessBins = GEX_PROCESSPART_FIRSTINSTANCE;
    else if(!qstricmp(szProcess,"last_instance"))
        iProcessBins = GEX_PROCESSPART_LASTINSTANCE;
    else if(!qstricmp(szProcess,"parts_inside"))
        iProcessBins = GEX_PROCESSPART_PARTSINSIDE;
    else if(!qstricmp(szProcess,"parts_outside"))
        iProcessBins = GEX_PROCESSPART_PARTSOUTSIDE;
    else if(!qstricmp(szProcess,"no_samples"))
        iProcessBins = GEX_PROCESSPART_NO_SAMPLES;
    else
        ZTHROWEXC("invalid type of parts to process parameter :"+ZString(szProcess));

    switch(iProcessBins)
    {
        case GEX_PROCESSPART_EXPARTLIST:
        case GEX_PROCESSPART_PARTLIST:
        case GEX_PROCESSPART_SBINLIST:
        case GEX_PROCESSPART_EXSBINLIST:
        case GEX_PROCESSPART_HBINLIST:
        case GEX_PROCESSPART_EXHBINLIST:
        case GEX_PROCESSPART_PARTSINSIDE:
        case GEX_PROCESSPART_PARTSOUTSIDE:
            // A range of parts-bins must have been specified
            // Check if range specified
            if(csl->get("argCount").asInt() < 6)
                ZTHROWEXC("missing parameter : range");
            szRangeList = csl->get("range").constBuffer();

            if (iProcessBins == GEX_PROCESSPART_PARTSINSIDE || iProcessBins == GEX_PROCESSPART_PARTSOUTSIDE)
            {
                if (CGexRangeCoord(szRangeList).isValid() == false)
                    ZTHROWEXC("invalid range value: " + ZString(szRangeList));
            }
            break;

        default: // Allow any test#
            szRangeList = "0 to 2147483647";	// Test is a 31bits number.
            break;
    }

    // if Test Mapping file defined...
    if(csl->get("argCount").asInt() >= 7)
        szMapTests = csl->get("maptests").constBuffer();

    // Wafer to extractID defined...
    if(csl->get("argCount").asInt() >= 8)
        szWaferToExtract = csl->get("extractwafer").constBuffer();

    // Testing temperature if known...
    if(csl->get("argCount").asInt() >= 9)
    {
        QString strString(csl->get("temperature").constBuffer());
        strString = strString.trimmed();
        bool	bOkay;
        lfTemperature = strString.toDouble(&bOkay);
        if(!bOkay)
            lfTemperature = -1000;	// If invalid or unknown temperature.
    }
    // Datasetname defined...
    if(csl->get("argCount").asInt() >= 10)
        szDatasetName = csl->get("datasetname").constBuffer();

    if(csl->get("argCount").asInt() >= 11)
        szSampleGroup = csl->get("samplegroup").constBuffer();

    // Adds file to given group.
    return gexReport->addFile(lGroupID,szFile,iProcessSite,iProcessBins,szRangeList,szMapTests,szWaferToExtract,lfTemperature,szDatasetName,szSampleGroup);
}

////////////////////////////////////////////////////////////////////////////////
// Function gexGroup: Reset/Create new group.
// Call : gexGroup(action,value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexGroup(ZCsl* csl)
{
    int				argc		= csl->get("argCount").asInt();		// Number of arguments.
    const char *	szAction	= csl->get("action").constBuffer();	// action type
    const char *	szValue		= csl->get("value").constBuffer();	// Action parameter

    if(!qstricmp(szAction,"reset"))
    {
        // Delete all existing structures...
        if(gexReport != NULL)
            delete gexReport;
        gexReport = new CGexReport;
        gexReport->setReportOptions(NULL);	// Will remain NULL unless a report is created...
        if (pGexScriptEngine)
        {
            QScriptValue sv = pGexScriptEngine->newQObject((QObject*)gexReport);
            if (!sv.isNull())
                pGexScriptEngine->globalObject().setProperty("GSReport", sv);
            else
                GSLOG(SYSLOG_SEV_WARNING, "Cannot register new gex report in script engine");
        }
        // Dataset source results from inserting files (not a Query).
        ReportOptions.bQueryDataset = false;

        // Reset query structure
        cQuery.clear();
        return 1;
    }
    else
    if(!qstricmp(szAction,"insert"))
    {
        // Create new group
        int iGroupID = gexReport->addGroup(szValue);
        ZString sGroupID = iGroupID;

        // Dataset source results from inserting files (not a Query).
        ReportOptions.bQueryDataset = false;
        return sGroupID;
    }
    else
    if(!qstricmp(szAction,"insert_query"))
    {
        // Query (list of files to process)
        QStringList sFilesMatchingQuery;
        QString		strFilePath;
        QString     lErrorMessage;
        int			iGroupID=0;

        // Dataset source results from running a Query.
        ReportOptions.bQueryDataset = true;

        cQuery.strTitle = szValue;

        QList<GexDatabaseQuery> dbQueries = GS::Gex::Engine::GetInstance().GetDatabaseEngine()
                .QuerySplit(cQuery, lErrorMessage);

        if (dbQueries.isEmpty() && !lErrorMessage.isEmpty())
        {
            ZTHROWEXC(lErrorMessage.toLatin1().constData());
            return 0;
        }

        QStringList::iterator               itFiles;
        QList<GexDatabaseQuery>::iterator   itQuery;

        for (itQuery = dbQueries.begin(); itQuery != dbQueries.end(); ++itQuery)
        {
            GexDatabaseQuery dbQueryTmp(*itQuery);

            // Find all files matching query into the group.
            sFilesMatchingQuery = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFiles(&dbQueryTmp, lErrorMessage);

            // Make sure at least 1 file is returned
            if(sFilesMatchingQuery.isEmpty())
            {
                GSLOG(SYSLOG_SEV_ERROR, "warning in QuerySelectFiles() : No data matching query criteria.");
                WriteScriptMessage(QString("Warning : No data matching query criteria in Database ")
                                           +dbQueryTmp.strDatabaseLogicalName
                                           +QString(". TestingStage: ")
                                           +dbQueryTmp.strDataTypeQuery
                                           + ", "
                                           + dbQueryTmp.strlSqlFilters.join(", "), true);
                if(!lErrorMessage.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().data());
                    WriteScriptMessage(QString("Warning : ")
                                       + lErrorMessage, true);
                    ZTHROWEXC(lErrorMessage.toLatin1().constData());
                    return 0;
                }
            }

            // 1: Compute total size of files in query (so to see if use Summary instead of data files!).
            QFile	cFile;
            dbQueryTmp.lfQueryFileSize = 0;
            for (itFiles = sFilesMatchingQuery.begin(); itFiles != sFilesMatchingQuery.end(); ++itFiles )
            {
                // Adds file to given group.
                strFilePath = (*itFiles);	// Absolute path to the file...

                // Check if file exists...if not, probably it is compressed (then add .gz extension)
                if(QFile::exists(strFilePath) == false)
                    strFilePath += ".gz";

                cFile.setFileName(strFilePath);
                dbQueryTmp.lfQueryFileSize += (double) cFile.size();
            }

            // Create group to hold ALL files matching the query criteria
            if(argc > 2)
                iGroupID = (long) csl->get("group_id").asLong();
            else
            {
                if (!gexReport)
                {
                    GSLOG(SYSLOG_SEV_WARNING, "gexReport NULL");
                    ZTHROWEXC("gexReport NULL");
                }
                iGroupID= gexReport->addGroup(dbQueryTmp.strTitle, &dbQueryTmp);
            }

            // 2: Add all files of the query to the group.
            for (itFiles = sFilesMatchingQuery.begin(); itFiles != sFilesMatchingQuery.end(); ++itFiles )
            {
                // Adds file to given group.
                strFilePath = (*itFiles);	// Absolute path to the file...

                // Check if file exists...if not, probably it is compressed (then add .gz extension)...unless 'Summary' database
                if((QFile::exists(strFilePath) == false) && (dbQueryTmp.bSummaryOnly == false))
                    strFilePath += ".gz";

                // Add file to group (if it exists!)
                if(QFile::exists(strFilePath) || dbQueryTmp.bSummaryOnly)
                {
                    gexReport->addFile(iGroupID,
                        strFilePath,
                        dbQueryTmp.iSite,
                        dbQueryTmp.iProcessData,
                        dbQueryTmp.strProcessData.toLatin1().data(),
                        dbQueryTmp.strMapFile,
                        dbQueryTmp.strWaferID);
                }
            }

            // 3: add the conditions to the group
            CGexGroupOfFiles * pGroup = (iGroupID < 0 || iGroupID >= gexReport->getGroupsList().size()) ? NULL : gexReport->getGroupsList().at(iGroupID);
            if(pGroup)
            {
                foreach (const QString &condition, dbQueryTmp.mTestConditions.keys())
                {
                    pGroup->AddTestConditions(condition,
                                              dbQueryTmp.mTestConditions.value(condition));
                }
            }
        }

        // Reset query structure
        cQuery.clear();

        // Return groupID
        ZString sGroupID = iGroupID;
        return sGroupID;
    }
    else if(!qstricmp(szAction,"declare_condition"))
    {
        // Create new condition
        gexReport->AddTestConditions(szValue);
        return 0;
    }
    else
        ZTHROWEXC("invalid parameter :"+ZString(szAction));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//------- Extract FILTER name + value  --------
////////////////////////////////////////////////////////////////////////////////
/*static int setQueryFilter(const char *szSection, const char *szField)
{
    // Burning time filter
    if(!qstricmp(szSection,"dbf_burnin_time"))
    {
        sscanf(szField,"%d",&cQuery.iBurninTime);
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_BURNIN;
        return 1;
    }
    // Data origin filter
    if(!qstricmp(szSection,"dbf_data_origin"))
    {
        cQuery.strDataOrigin = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_ORIGIN;
        return 1;
    }
    // Facility location filter
    if(!qstricmp(szSection,"dbf_facility_id"))
    {
        cQuery.strFacilityID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_FACILITY;
        return 1;
    }
    // FamilyID filter
    if(!qstricmp(szSection,"dbf_family_id"))
    {
        cQuery.strFamilyID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_FAMILY;
        return 1;
    }
    // Floor Location filter
    if(!qstricmp(szSection,"dbf_floor_id"))
    {
        cQuery.strFloorID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_FLOOR;
        return 1;
    }
    // Testing/step frequency filter
    if(!qstricmp(szSection,"dbf_freq_id"))
    {
        cQuery.strFrequencyStep = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_FREQUENCYSTEP;
        return 1;
    }
    // LoadBoard name filter
    if(!qstricmp(szSection,"dbf_loadboard_id"))
    {
        cQuery.strLoadBoardName = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDNAME;
        return 1;
    }
    // LoadBoard type (brand/family) filter
    if(!qstricmp(szSection,"dbf_loadboard_type"))
    {
        cQuery.strLoadBoardType = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOADBOARDTYPE;
        return 1;
    }
    // DIB name filter
    if(!qstricmp(szSection,"dbf_dib_id"))
    {
        cQuery.strDibName = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_DIBNAME;
        return 1;
    }
    // DIB type (brand/family) filter
    if(!qstricmp(szSection,"dbf_dib_type"))
    {
        cQuery.strDibType = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_DIBTYPE;
        return 1;
    }
    // Lot ID filter
    if(!qstricmp(szSection,"dbf_lot_id"))
    {
        cQuery.strLotID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_LOT;
        return 1;
    }
    // Operator name filter
    if(!qstricmp(szSection,"dbf_operator_name"))
    {
        cQuery.strOperator = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_OPERATOR;
        return 1;
    }
    // Package type filter
    if(!qstricmp(szSection,"dbf_package_type"))
    {
        cQuery.strPackageType = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PACKAGE;
        return 1;
    }
    // Prober name filter
    if(!qstricmp(szSection,"dbf_prober_id"))
    {
        cQuery.strProberName = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROBERNAME;
        return 1;
    }
    // Prober type (brand) filter
    if(!qstricmp(szSection,"dbf_prober_type"))
    {
        cQuery.strProberType = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROBERTYPE;
        return 1;
    }
    // Product name filter
    if(!qstricmp(szSection,"dbf_product_id"))
    {
        cQuery.strProductID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PRODUCT;
        return 1;
    }
    // Program name filter
    if(!qstricmp(szSection,"dbf_program_name"))
    {
        cQuery.strJobName = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROGRAMNAME;
        return 1;
    }
    // Program revision filter
    if(!qstricmp(szSection,"dbf_program_rev"))
    {
        cQuery.strJobRev = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROGRAMREVISION;
        return 1;
    }
    // Sublot ID filter
    if(!qstricmp(szSection,"dbf_retest_nbr"))
    {
        cQuery.strRetestNbr = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_RETESTNBR;
        return 1;
    }
    // Sublot ID filter
    if(!qstricmp(szSection,"dbf_sublot_id"))
    {
        cQuery.strSubLotID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_SUBLOT;
        return 1;
    }
    // Testing temperature filter
    if(!qstricmp(szSection,"dbf_temperature"))
    {
        cQuery.strTemperature = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_TEMPERATURE;
        return 1;
    }
    // Tester name filter
    if(!qstricmp(szSection,"dbf_tester_name"))
    {
        cQuery.strNodeName = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTERNAME;
        return 1;
    }
    // Tester type (brand/family) filter
    if(!qstricmp(szSection,"dbf_tester_type"))
    {
        cQuery.strTesterType = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTERTYPE;
        return 1;
    }
    // Testing code filter
    if(!qstricmp(szSection,"dbf_testing_code"))
    {
        cQuery.strTestingCode = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_TESTCODE;
        return 1;
    }
    // Process name filter
    if(!qstricmp(szSection,"dbf_process_id"))
    {
        cQuery.strProcessID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_PROCESS;
        return 1;
    }
    // Tester Hardware testing site# filter
    if(!qstricmp(szSection,"dbf_site_nbr"))
    {
        cQuery.iSite = -1;
        int r=sscanf(szField, "%d", &cQuery.iSite);
        if (r==EOF)
            GSLOG(SYSLOG_SEV_ERROR, "can't interpret 'dbf_site_nbr'");
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_SITENBR;
        return 1;
    }

    // WaferID name filter
    if(!qstricmp(szSection,"dbf_wafer_id"))
    {
        cQuery.strWaferID = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_WAFERID;
        return 1;
    }

    // User1 name filter
    if(!qstricmp(szSection,"dbf_user_1"))
    {
        cQuery.strUser1 = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER1;
        return 1;
    }

    // User2 name filter
    if(!qstricmp(szSection,"dbf_user_2"))
    {
        cQuery.strUser2 = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER2;
        return 1;
    }

    // User3 name filter
    if(!qstricmp(szSection,"dbf_user_3"))
    {
        cQuery.strUser3 = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER3;
        return 1;
    }

    // User4 name filter
    if(!qstricmp(szSection,"dbf_user_4"))
    {
        cQuery.strUser4 = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER4;
        return 1;
    }

    // User5 name filter
    if(!qstricmp(szSection,"dbf_user_5"))
    {
        cQuery.strUser5 = szField;
        cQuery.uFilterFlag |= GEX_QUERY_FLAG_USER5;
        return 1;
    }

    // Invalid filter name
    return 0;
}*/

////////////////////////////////////////////////////////////////////////////////
// Function gexQuery: Let's user define the Query content
// Call : gexQuery(section, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexQuery(ZCsl* csl)
{
    const char *	szValue		= NULL;									// Field value
    const char *	szSection	= csl->get("section").constBuffer();	// Section name to setup
    if (!szSection)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Section NULL");
        return 0;
    }
    const char *	szField		= csl->get("field").constBuffer();		// Field name to setup
    if (!szField)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Field NULL");
        return 0;
    }
    int				argc		= csl->get("argCount").asInt();			// Number of arguments.

    // We are processing datasets (DB queries)
    gexReport->SetProcessingFile(false);

    if ( QString(szSection)=="db_granularity" && !GS::LPPlugin::ProductInfo::getInstance()->isGenealogyAllowed())
    {
        GSLOG(SYSLOG_SEV_WARNING, "forbidden gexQuery command regarding to the license");
        //"error : This feature is not activated in your license."
        /*
        WriteScriptMessage(QString(
            "This script requires the Genealogy module. Please contact "+QString(GEX_EMAIL_SALES)+" for more information.")
            , true);
            */
        ZTHROWEXC("This script requires the Genealogy module. Please contact Quantix for more information.");
        return 0;
    }

    // Report folder name (or CSV file name)
    if(!qstricmp(szSection,"db_report"))
    {
        // Save Title string (may include spaces or any other characters).
        QString strTitle = szField;

        // Check if title not empty
        if(strTitle.trimmed().isEmpty())
                ZTHROWEXC("empty report name specified in db_report() command");

        // Normalized: Filter string to be file-name/folder-name compatible.
        ReportOptions.strReportTitle = strTitle;
        strTitle.replace(QRegExp("[^A-Za-z0-9]"), "_" );
        ReportOptions.strReportNormalizedTitle = strTitle;
        return 1;
    }

    // Database Location
    if(!qstricmp(szSection,"db_location"))
    {
        if(!qstricmp(szField,"[Local]"))
            cQuery.bLocalDatabase = true;
        else
            cQuery.bLocalDatabase = false;
        return 1;
    }

    // Database Logical Name
    if(!qstricmp(szSection,"db_name"))
    {
        cQuery.strDatabaseLogicalName = szField;
        return 1;
    }

    // Data type to focus on in query (wafer sort, final test, e-test...:only applies to remote SQL databases)
    if(!qstricmp(szSection,"db_data_type"))
    {
        cQuery.strDataTypeQuery = szField;
        return 1;
    }

    if(!qstricmp(szSection,"db_consolidated"))
    {
        //ZException exc("db_consolidated");
        //ZTHROWEXC("gexQuery: db_consolidated\n");
        //ZTrace::writeMsg(ZException::iFile, ZException::iLine, "gexQuery: db_consolidated\n");
        if(!qstricmp(szField,"true"))
            cQuery.bConsolidatedExtraction = true;
        else
            cQuery.bConsolidatedExtraction = false;

        GSLOG(SYSLOG_SEV_DEBUG, QString("gexQuery: db_consolidated '%1'")
            .arg(cQuery.bConsolidatedExtraction?"true":"false")
            .toLatin1().data() );
        return 1;
    }


    // Query type: database or offline
    if(!qstricmp(szSection,"db_offline_query"))
    {
        if(!qstricmp(szField,"true"))
            cQuery.bOfflineQuery = true;
        else
            cQuery.bOfflineQuery = false;
        return 1;
    }

    // Time period to consider
    if(!qstricmp(szSection,"db_period"))
    {
        int iIndex=0;
        do
        {
            if(gexTimePeriodChoices[iIndex] == 0)
            {
                ZTHROWEXC("invalid parameter :"+ZString(szField));
                return 0;
            }
            // Check if Time period keyword found...
            if(!qstricmp(szField,gexTimePeriodChoices[iIndex]))
                break;
            // Not found yet...move to next keyword entry
            iIndex++;
        }
        while(1);

        // According to date window type, compute From-To dates
        cQuery.calendarFrom = cQuery.calendarTo = QDate::currentDate();
        cQuery.calendarFrom_Time = QTime(0,0,0);
        cQuery.calendarTo_Time = QTime(23,59,59);
        cQuery.iTimePeriod = iIndex;
        int r=-1;
        int NFactor=1;
        char TimeStep[256]; //char* TimeStep=(char*)malloc(256);
        switch(iIndex)
        {
            case GEX_QUERY_TIMEPERIOD_ALLDATES:
            case GEX_QUERY_TIMEPERIOD_TODAY:
            default:
            break;
            case GEX_QUERY_TIMEPERIOD_LAST2DAYS:
                cQuery.calendarFrom = cQuery.calendarFrom.addDays(-1);
            break;
            case GEX_QUERY_TIMEPERIOD_LAST3DAYS:
                cQuery.calendarFrom = cQuery.calendarFrom.addDays(-2);
            break;
            case GEX_QUERY_TIMEPERIOD_LAST7DAYS:
                cQuery.calendarFrom = cQuery.calendarFrom.addDays(-6);
            break;
            case GEX_QUERY_TIMEPERIOD_LAST14DAYS:
                cQuery.calendarFrom = cQuery.calendarFrom.addDays(-13);
            break;
            case GEX_QUERY_TIMEPERIOD_LAST31DAYS:
                cQuery.calendarFrom = cQuery.calendarFrom.addDays(-30);
            break;
            case GEX_QUERY_TIMEPERIOD_LAST_N_X:
                // CodeMe
                if(argc < 3)
                {
                    ZTHROWEXC("missing parameter : date range");
                    return 0;
                }
                szValue = csl->get("value").constBuffer();
                if (szValue)
                    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("csl value : %1").arg( szValue).toLatin1().constData());
                NFactor=-1;
                r=sscanf(szValue, "%d %s", &NFactor, TimeStep);	// &TimeStep
                if (r==EOF) // || !NFactor || !TimeStep)
                {
                    GSLOG(SYSLOG_SEV_ERROR, QString("LAST N X parameter parsing failed : '%1'").arg( szValue).toLatin1().constData());
                    ZTHROWEXC("bad parameter : last N X params");
                    return 0;
                }
                GSLOG(SYSLOG_SEV_WARNING, QString("checkMe : NFactor=%1 NStep=%2").arg( NFactor).arg( TimeStep).toLatin1().constData());
                cQuery.iTimeNFactor = NFactor;

                if (QString(TimeStep)=="days")
                {
                    cQuery.calendarFrom = cQuery.calendarFrom.addDays( -NFactor );
                    cQuery.m_eTimeStep = GexDatabaseQuery::DAYS;
                }
                else if (QString(TimeStep)=="weeks")
                {
                    cQuery.calendarFrom = cQuery.calendarFrom.addDays( -NFactor*7 );
                    cQuery.m_eTimeStep = GexDatabaseQuery::WEEKS;
                }
                else if (QString(TimeStep)=="months")
                {
                    cQuery.calendarFrom = cQuery.calendarFrom.addMonths( -NFactor );
                    cQuery.m_eTimeStep = GexDatabaseQuery::MONTHS;
                }
                else if (QString(TimeStep)=="quarters")
                {
                    cQuery.calendarFrom = cQuery.calendarFrom.addMonths( -NFactor*4 );
                    cQuery.m_eTimeStep = GexDatabaseQuery::QUARTERS;
                }
                else if (QString(TimeStep)=="years")
                {
                    cQuery.calendarFrom = cQuery.calendarFrom.addYears( -NFactor );
                    cQuery.m_eTimeStep = GexDatabaseQuery::YEARS;
                }
                else
                {
                    QString f=QString("gexQuery failed : Last N step parameter unknown : '%1'").arg(TimeStep);
                    GSLOG(SYSLOG_SEV_ERROR, f.toLatin1().data());
                    ZTHROWEXC( f.toLatin1().data() );
                    return 0;
                }
                GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" calendar From = %1").arg(
                       cQuery.calendarFrom.toString(Qt::TextDate).toLatin1().data()
                    ).toLatin1().constData());
            break;

            case GEX_QUERY_TIMEPERIOD_THISWEEK:
                // 'From' date must be last 'Monday' before current date.
                while(cQuery.calendarFrom.dayOfWeek() != 1)
                    cQuery.calendarFrom = cQuery.calendarFrom.addDays(-1);
            break;
            case GEX_QUERY_TIMEPERIOD_THISMONTH:
                // 'From' date must be 1st day of the month.
                int year;
                (cQuery.calendarFrom.year() <= 99)? (year = cQuery.calendarFrom.year() + 1900) : (year = cQuery.calendarFrom.year());
                cQuery.calendarFrom.setDate(year,cQuery.calendarFrom.month(),1);
            break;
            case GEX_QUERY_TIMEPERIOD_CALENDAR:
                // Extract <From> <To> dates: "YYYY MM DD YYYY MM DD"
                int iFromYear,iFromMonth,iFromDay;
                int iToYear,iToMonth,iToDay;
                // Check if range specified
                if(argc < 3)
                    ZTHROWEXC("missing parameter : date range");
                szValue = csl->get("value").constBuffer();
                sscanf(szValue,"%d %d %d %d %d %d",
                    &iFromYear,&iFromMonth,&iFromDay,
                    &iToYear,&iToMonth,&iToDay);
                if (iFromYear < 99) iFromYear += 1900;
                cQuery.calendarFrom.setDate(iFromYear,iFromMonth,iFromDay);
                if (iToYear < 99) iToYear += 1900;
                cQuery.calendarTo.setDate(iToYear,iToMonth,iToDay);
                QStringList strlTokens = QString(szValue).split(' ');
                if(strlTokens.count() >= 8)
                {
                    int nHour, nMin, nSec;
                    nHour = strlTokens[6].section(':',0,0).toUInt();
                    nMin = strlTokens[6].section(':',1,1).toUInt();
                    nSec = strlTokens[6].section(':',2,2).toUInt();
                    cQuery.calendarFrom_Time = QTime(nHour, nMin, nSec);
                    nHour = strlTokens[7].section(':',0,0).toUInt();
                    nMin = strlTokens[7].section(':',1,1).toUInt();
                    nSec = strlTokens[7].section(':',2,2).toUInt();
                    cQuery.calendarTo_Time = QTime(nHour, nMin, nSec);
                }
            break;
        }
        return 1;
    }

    // Data type filter
    if(!qstricmp(szSection,"db_data"))
    {
        // Check for type of parts to process
        if(!qstricmp(szField,"all"))
            cQuery.iProcessData = GEX_PROCESSPART_ALL;
        else if(!qstricmp(szField,"allparts_except"))
            cQuery.iProcessData = GEX_PROCESSPART_EXPARTLIST;
        else if(!qstricmp(szField,"good"))
            cQuery.iProcessData = GEX_PROCESSPART_GOOD;
        else if(!qstricmp(szField,"fails"))
            cQuery.iProcessData = GEX_PROCESSPART_FAIL;
        else if(!qstricmp(szField,"parts"))
            cQuery.iProcessData = GEX_PROCESSPART_PARTLIST;
        else if(!qstricmp(szField,"bins"))
            cQuery.iProcessData = GEX_PROCESSPART_SBINLIST;
        else if(!qstricmp(szField,"allbins_except"))
            cQuery.iProcessData = GEX_PROCESSPART_EXSBINLIST;
        else if(!qstricmp(szField,"hbins"))
            cQuery.iProcessData = GEX_PROCESSPART_HBINLIST;
        else if(!qstricmp(szField,"allhbins_except"))
            cQuery.iProcessData = GEX_PROCESSPART_EXHBINLIST;
        else if(!qstricmp(szField,"odd_parts"))
            cQuery.iProcessData = GEX_PROCESSPART_ODD;
        else if(!qstricmp(szField,"even_parts"))
            cQuery.iProcessData = GEX_PROCESSPART_EVEN;
        else if(!qstricmp(szField,"first_instance"))
            cQuery.iProcessData = GEX_PROCESSPART_FIRSTINSTANCE;
        else if(!qstricmp(szField,"last_instance"))
            cQuery.iProcessData = GEX_PROCESSPART_LASTINSTANCE;
        else if(!qstricmp(szField,"parts_inside"))
            cQuery.iProcessData = GEX_PROCESSPART_PARTSINSIDE;
        else if(!qstricmp(szField,"parts_outside"))
            cQuery.iProcessData = GEX_PROCESSPART_PARTSOUTSIDE;
        else if(!qstricmp(szField,"no_samples"))
            cQuery.iProcessData = GEX_PROCESSPART_NO_SAMPLES;
        else
            ZTHROWEXC("invalid parameter :"+ZString(szField));

        switch(cQuery.iProcessData)
        {
            case GEX_PROCESSPART_EXPARTLIST:
            case GEX_PROCESSPART_PARTLIST:
            case GEX_PROCESSPART_SBINLIST:
            case GEX_PROCESSPART_EXSBINLIST:
            case GEX_PROCESSPART_HBINLIST:
            case GEX_PROCESSPART_EXHBINLIST:
            case GEX_PROCESSPART_PARTSINSIDE:
            case GEX_PROCESSPART_PARTSOUTSIDE:
                // A range of parts-bins must have been specified
                // Check if range specified
                if(argc < 3)
                    ZTHROWEXC("missing parameter : range");
                szValue = csl->get("value").constBuffer();

                if (cQuery.iProcessData == GEX_PROCESSPART_PARTSINSIDE || cQuery.iProcessData == GEX_PROCESSPART_PARTSOUTSIDE)
                {
                    if (CGexRangeCoord(szValue).isValid() == false)
                        ZTHROWEXC("invalid range value: " + ZString(szValue));
                }
                break;

            default: // Allow any test#
                szValue = "0 to 2147483647";	// Test is a 31bits number.
                break;
        }
        cQuery.strProcessData = szValue;
        return 1;
    }

    // Minimum number of parts required in file for considering it.
    if(!qstricmp(szSection,"db_minimum_samples"))
    {
        if(sscanf(szField,"%ld",&cQuery.lMinimumPartsInFile) != 1)
            cQuery.lMinimumPartsInFile = 0;
        return 1;
    }

    // Mapping/Merge test numbers
    if(!qstricmp(szSection,"db_mapping"))
    {
        cQuery.strMapFile = szField;
        return 1;
    }

    // Test list to focus on (only used for Remote Database queries)
    if(!qstricmp(szSection,"db_testlist"))
    {
        cQuery.strTestList = szField;
        return 1;
    }

    // Plugin options (only used for Remote Database queries)
    if(!qstricmp(szSection,"db_plugin_options"))
    {
        cQuery.strOptionsString = szField;
        return 1;
    }

    // SQL filter
    if(!qstricmp(szSection,"dbf_sql"))
    {
        // Make sure we have a third parameter
        if(argc < 3)
            ZTHROWEXC("missing parameter : filter value");
        QString strFilter = szField;
        strFilter += "=";
        strFilter += csl->get("value").constBuffer();
        cQuery.strlSqlFilters.append(strFilter);
        return 1;
    }

    // Split field
    if(!qstricmp(szSection,"db_split"))
    {
        if(!qstricmp(szField,"dbf_sql"))
        {
            // Make sure we have a third parameter
            if(argc < 3)
                ZTHROWEXC("missing parameter : filter value");
            cQuery.mSplitFields.append(csl->get("value").constBuffer());
            return 1;
        }
        cQuery.mSplitFields.append(szField);
        return 1;
    }

    // NON-SQL filters
    if(cQuery.setQueryFilter(szSection,szField))
        return 1;

    // csl lib throw an exception if there is no value arg
    if (argc>2)
        szValue = csl->get("value").constBuffer();

    WriteScriptMessage(QString("Extra gexQuery(%1, %2, %3...) registered.").arg(szSection).arg(szField).arg(szValue?szValue:""), true);

    QList<QString> sl;
    sl.push_back(QString(szSection));
    sl.push_back(QString(szField));
    if (szValue)
        sl.push_back(QString(szValue));

    /*
    if (argc>2)
    {
        sl.push_back( QString(csl->get("value").constBuffer()) );
    }
    */
    cQuery.m_gexQueries.push_back(sl);
    return 1;
} // gexQuery

////////////////////////////////////////////////////////////////////////////////
// Function gexInteractive: Let's user prepare layers then jump into interactive mode!
// Call : gexOption(section, field, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexInteractive(ZCsl* csl)
{
    int				argc		= csl->get("argCount").asInt();			// Number of arguments.
    const char *	szSection	= csl->get("section").constBuffer();	// Section name to setup
    const char *	szField		= "";									// Field name to setup
    const char *	szValue		= "";									// Field value

    if(argc > 1)
        szField = csl->get("field").constBuffer();
    if(argc > 2)
        szValue = csl->get("value").constBuffer();

    if(checkProfessionalFeature(true) == false)
        return 0;

    // Check if valid handle to main GUI window
    if(pGexMainWindow == NULL)
        return 0;

    // RESET section
    if(!qstricmp(szSection,"reset"))
    {
        // Switch to Interactive page (have it visible)
        m_lGroupX = m_lGroupY = 0;

        // Define page to load after script completed...
        ShowReportOnCompletion("interactive_chart");

        // And force it to execute now as well (because Chart class is dynamically created and must be done now before inserting layers)
        pGexMainWindow->Wizard_DrillChart("");

        // Reset layer count
        m_InteractiveLayer = 0;
        return 1;
    }

    // Check if valid Interactive handle...
    if(pGexMainWindow->IsWizardChartAvalaible() == false)
        return 0;

    // Add layer section: 'setX','<test#>','iGroup'
    if(!qstricmp(szSection,"setX"))
    {
        // Get Testnumber & Pinamp in X
        if(sscanf(szField,"%d%*c%d",&m_lInteractiveTestX,&m_lInteractiveTestPmX) < 2)
            m_lInteractiveTestPmX= GEX_PTEST;

        // Get Group#
        if(sscanf(szValue,"%d",&m_lGroupX) != 1)
            m_lGroupX = 0;

        // Reset Y axis for now
        m_lInteractiveTestY = m_lInteractiveTestPmY = -1;
        m_lGroupY = 0;
        return 1;
    }

    // Add layer section: 'setY','<test#>','iGroup'
    if(!qstricmp(szSection,"setY"))
    {
        // Get Testnumber & Pinamp in Y
        if(sscanf(szField,"%d%*c%d",&m_lInteractiveTestY,&m_lInteractiveTestPmY) < 2)
            m_lInteractiveTestPmY= GEX_PTEST;

        // Get Group#
        if(sscanf(szValue,"%d",&m_lGroupY) != 1)
            m_lGroupY = 0;
        return 1;
    }

    // Trigger to Add layer
    if(!qstricmp(szSection,"addLayer"))
    {
        bool bClearLayers = (m_InteractiveLayer == 0) ? true: false;
        pGexMainWindow->LastCreatedWizardChart()->addChart(false,bClearLayers,
            m_lInteractiveTestX,m_lInteractiveTestPmX,"",m_lGroupX,
            m_lInteractiveTestY,m_lInteractiveTestPmY,"",m_lGroupY);

        // Keep track of layers inserted
        m_InteractiveLayer++;
        return 1;
    }

    // Show all layers inserted
    if(!qstricmp(szSection,"Chart"))
    {
        // Have GUI (statistics, style, etc) properly refreshed.
        pGexMainWindow->LastCreatedWizardChart()->exitInsertMultiLayers(m_lInteractiveTestX,m_lInteractiveTestPmX,"");

        if(!qstricmp(szField,"Histogram"))
            pGexMainWindow->LastCreatedWizardChart()->OnHistogram();
        else
        if(!qstricmp(szField,"Trend"))
            pGexMainWindow->LastCreatedWizardChart()->OnTrend();
        else
        if(!qstricmp(szField,"Scatter"))
            pGexMainWindow->LastCreatedWizardChart()->OnScatter();
        else
        {
            ZTHROWEXC("invalid parameter :"+ZString(szField));
        }

        return 1;
    }

    ZTHROWEXC("invalid parameter :"+ZString(szSection));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexChartStyle: Define Charting style section (Interactive Charts)
// Call : gexChartStyle(field, value);
////////////////////////////////////////////////////////////////////////////////
ZString gexChartStyle(const char * szField, const char * szValue)
{
    //GSLOG(SYSLOG_SEV_INFORMATIONAL, " %s %s", strParameterList.toLatin1().data(), bIsAdvancedReport?"(AdvReport)":"" );
    // Charting style section (Interactive Charts)
    int	iR,iG,iB;
    int iWidth;

    // Chart background color
    if(!qstricmp(szField,"bkg_color"))
    {
        sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
        ReportOptions.cBkgColor = QColor(iR,iG,iB);
        return 1;	// No error.
    }

    // Overlay curve's name on chart?
    if(!qstricmp(szField,"show_legend"))
    {
        if(!qstricmp(szValue,"1"))
            ReportOptions.bPlotLegend = true;
        else
            ReportOptions.bPlotLegend = false;
        return 1;	// No error.
    }

    if(!qstricmp(szField,"text_rotation"))
    {
        ReportOptions.mTextRotation = atoi(szValue);
        return 1;	// No error.
    }

    if (! qstricmp(szField, "show_qqline"))
    {
        if (! qstricmp(szValue, "1"))
        {
            ReportOptions.mPlotQQLine = true;
        }
        else
        {
            ReportOptions.mPlotQQLine = false;
        }
        return 1;
    }

    if(!qstricmp(szField,"total_bars"))
    {
        if(sscanf(szValue,"%d",&ReportOptions.mTotalBars) != 1)
            ReportOptions.mTotalBars = TEST_ADVHISTOSIZE;	// No value, force to default (40)
        if(ReportOptions.mTotalBars < 2)
            ReportOptions.mTotalBars = 2;
        if(ReportOptions.mTotalBars > 10000)
            ReportOptions.mTotalBars = 10000;
        return 1;	// No error.
    }

    if (!qstricmp(szField, "indexY_scale"))
    {
        ReportOptions.mYScale = atoi(szValue);
        return 1;
    }

    if (!qstricmp(szField, "custom"))
    {
        if (! qstricmp(szValue, "1"))
        {
            ReportOptions.mCustom = true;
        }
        else
        {
            ReportOptions.mCustom = false;
        }
        return 1;
    }

    if(!qstricmp(szField,"total_bars_custom"))
    {
        if(sscanf(szValue,"%d",&ReportOptions.mTotalBarsCustom) != 1)
            ReportOptions.mTotalBarsCustom = TEST_ADVHISTOSIZE;	// No value, force to default (40)
        if(ReportOptions.mTotalBarsCustom < 2)
            ReportOptions.mTotalBarsCustom = 2;
        if(ReportOptions.mTotalBarsCustom > 10000)
            ReportOptions.mTotalBarsCustom = 10000;
        return 1;	// No error.
    }


    // Plot Bars (applies to histograms only)
    if(!qstricmp(szField,"box_bars"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bBoxBars = true;
        else
            s_pChartStyle->bBoxBars = false;
        return 1;	// No error.
    }
    if(!qstricmp(szField,"box_3d_bars"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bBox3DBars = true;
        else
            s_pChartStyle->bBox3DBars = false;
        return 1;	// No error.
    }

    // Plot lines
    if(!qstricmp(szField,"stack"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->mIsStacked = true;
        else
            s_pChartStyle->mIsStacked = false;
        return 1;	// No error.
    }

    // Plot fitting curve
    if(!qstricmp(szField,"fitting_curve"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bFittingCurve = true;
        else
            s_pChartStyle->bFittingCurve = false;
        return 1;	// No error.
    }

    // Plot gaussian / bell curve
    if(!qstricmp(szField,"bell_curve"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bBellCurve = true;
        else
            s_pChartStyle->bBellCurve = false;
        return 1;	// No error.
    }

    // Box-Plot whisker type: Range or Q2 +/- 1.5*IQR
    if(!qstricmp(szField,"box_whisker"))
    {
        if(!qstricmp(szValue,"range"))
        {
            s_pChartStyle->iWhiskerMode= GEX_WHISKER_RANGE;	// Range
            return 1;	// No error.
        }
        else
            if(!qstricmp(szValue,"q1q3"))
            {
            s_pChartStyle->iWhiskerMode= GEX_WHISKER_Q1Q3;	// Q1-1.5*IQR, Q3+1.5*IQR
            return 1;	// No error.
        }
        else
            if(!qstricmp(szValue,"iqr"))
            {
            s_pChartStyle->iWhiskerMode= GEX_WHISKER_IQR;	// Median+/-1.5IQR
            return 1;	// No error.
        }
    }

    // Plot lines
    if(!qstricmp(szField,"lines"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bLines = true;
        else
            s_pChartStyle->bLines = false;
        return 1;	// No error.
    }

    // Plot Spots
    if(!qstricmp(szField,"spots"))
    {
        if(!qstricmp(szValue,"1"))
            s_pChartStyle->bSpots = true;
        else
            s_pChartStyle->bSpots = false;
        return 1;	// No error.
    }

    // Get line width
    if(!qstricmp(szField,"line_width"))
    {
        sscanf(szValue,"%d",&s_pChartStyle->iLineWidth);
        return 1;	// No error.
    }

    // Get line style (solid, dashed,...)
    if(!qstricmp(szField,"line_style"))
    {
        sscanf(szValue,"%d",&s_pChartStyle->iLineStyle);
        return 1;	// No error.
    }

    // Get spot style (cross, triangle, round,...)
    if(!qstricmp(szField,"spot_style"))
    {
        sscanf(szValue,"%d",&s_pChartStyle->iSpotStyle);
        return 1;	// No error.
    }

    // Get charting RGB colors
    if(!qstricmp(szField,"rgb_color"))
    {
        sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
        s_pChartStyle->cColor = QColor(iR,iG,iB);
        return 1;	// No error.
    }

    ///////////////////////////////////////
    // Markers
    ///////////////////////////////////////

    // Mean marker: line width & color
    if(!qstricmp(szField,"marker_mean"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setMeanLineWidth(iWidth);
        s_pChartStyle->setMeanColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // Median marker: line width & color
    if(!qstricmp(szField,"marker_median"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth, &iR,&iG,&iB);
        s_pChartStyle->setMedianLineWidth(iWidth);
        s_pChartStyle->setMedianColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // MIN marker: line width & color
    if(!qstricmp(szField,"marker_min"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setMinLineWidth(iWidth);
        s_pChartStyle->setMinColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // MAX marker: line width & color
    if(!qstricmp(szField,"marker_max"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setMaxLineWidth(iWidth);
        s_pChartStyle->setMaxColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // Limits marker: line width & color
    if(!qstricmp(szField,"marker_limits"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setLimitsLineWidth(iWidth);
        s_pChartStyle->setLimitsColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    if(!qstricmp(szField,"marker_speclimits"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setSpecLimitsLineWidth(iWidth);
        s_pChartStyle->setSpecLimitsColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // Limits marker: line width & color
    if(!qstricmp(szField,"marker_rolling_limits"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setRollingLimitsLineWidth(iWidth);
//        s_pChartStyle->setLimitsColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }



    // Limits marker: line width & color
    if(!qstricmp(szField,"marker_rolling_limits"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setRollingLimitsLineWidth(iWidth);
//        s_pChartStyle->setLimitsColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }


    // 2sigma marker: line width & color
    if(!qstricmp(szField,"marker_2sigma"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->set2SigmaLineWidth(iWidth);
        s_pChartStyle->set2SigmaColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // 3sigma marker: line width & color
    if(!qstricmp(szField,"marker_3sigma"))
    {
        sscanf(szValue,"%d %d %d %d ",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->set3SigmaLineWidth(iWidth);
        s_pChartStyle->set3SigmaColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // 6sigma marker: line width & color
    if(!qstricmp(szField,"marker_6sigma"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->set6SigmaLineWidth(iWidth);
        s_pChartStyle->set6SigmaColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    // 12sigma marker: line width & color
    if(!qstricmp(szField,"marker_12sigma"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->set12SigmaLineWidth(iWidth);
        s_pChartStyle->set12SigmaColor(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    if(!qstricmp(szField,"marker_QuartileQ1"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setQuartileQ1(iWidth);
        s_pChartStyle->setQuartileQ1Color(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    if(!qstricmp(szField,"marker_QuartileQ3"))
    {
        sscanf(szValue,"%d %d %d %d",&iWidth,&iR,&iG,&iB);
        s_pChartStyle->setQuartileQ3(iWidth);
        s_pChartStyle->setQuartileQ3Color(QColor(iR,iG,iB));
        return 1;	// No error.
    }

    if(!qstricmp(szField,"chart_layer"))
    {
        // Get Layer ID
        sscanf(szValue,"%d",&iR);

        // Check if reset layer list
        if(iR < 0)
        {
            while (!ReportOptions.pLayersStyleList.isEmpty())
                delete ReportOptions.pLayersStyleList.takeFirst();

            return 1;
        }

        // Allocate structure to copy style info
        CGexSingleChart	* pChartStyle = new CGexSingleChart(*s_pChartStyle);

        // Add structure to list of styles.
        ReportOptions.pLayersStyleList.append(pChartStyle);
        return 1;	// No error.
    }

    ZTHROWEXC("invalid parameter :"+ZString(szField));
    return 0;
}

static ZString gexChartStyle(ZCsl* csl)
{
    const char *	szField		= csl->get("field").constBuffer();						// Field name to setup
    const char *	szValue		= csl->get("value").constBuffer();						// Field value

    return gexChartStyle(szField, szValue);
}


////////////////////////////////////////////////////////////////////////////////
// Function gexBinStyle: Define Binning style section (custom bin colors to be used in Binning & Wafer map reports)
// Call : gexBinStyle(field, value);
////////////////////////////////////////////////////////////////////////////////
ZString gexSiteStyle(const char * szField, const char * szValue)
{
    int	iR,iG,iB;

    // Empty the list of custom style.
    if(!qstricmp(szField,"clear"))
    {
        ReportOptions.siteColorList.clear();
        return 1;	// No error.
    }

    // Read The Bin# list
    if(!qstricmp(szField,"site_list"))
    {
        cBinColor.cBinRange = new GS::QtLib::Range(szValue);
        return 1;	// No error.
    }

    // Read The Bin RGB color
    if(!qstricmp(szField,"site_color"))
    {
        sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
        cBinColor.cBinColor = QColor(iR,iG,iB);
        return 1;	// No error.
    }

    // Insert the BinColor definition to the Hard bin or Soft bin list
    if(!qstricmp(szField,"add_color"))
    {
        ReportOptions.siteColorList.append(cBinColor);
        return 1;	// No error.
    }

    ZTHROWEXC("invalid parameter :"+ZString(szField));
    return 0;
}


static ZString gexSiteStyle(ZCsl* csl)
{
    int				argc		= csl->get("argCount").asInt();							// Number of arguments.
    const char *	szField		= csl->get("field").constBuffer();						// Field name to setup
    const char *	szValue		= (argc > 1) ? csl->get("value").constBuffer() : "";	// Field value

    return gexSiteStyle(szField,szValue);
}

////////////////////////////////////////////////////////////////////////////////
// Function gexBinStyle: Define Binning style section (custom bin colors to be used in Binning & Wafer map reports)
// Call : gexBinStyle(field, value);
////////////////////////////////////////////////////////////////////////////////
ZString gexBinStyle(const char * szField, const char * szValue)
{
    int	iR,iG,iB;

    // Empty the list of custom style.
    if(!qstricmp(szField,"clear"))
    {
        ReportOptions.hardBinColorList.clear();
        ReportOptions.softBinColorList.clear();

        return 1;	// No error.
    }

    // Tells if custom colors to be used or not.
    if(!qstricmp(szField,"custom_colors"))
    {
        if(!qstricmp(szValue,"1"))
            ReportOptions.bUseCustomBinColors = true;
        else
            ReportOptions.bUseCustomBinColors = false;
        return 1;	// No error.
    }

    // Read The Bin# list
    if(!qstricmp(szField,"bin_list"))
    {
        cBinColor.cBinRange = new GS::QtLib::Range(szValue);
        return 1;	// No error.
    }

    // Read The Bin RGB color
    if(!qstricmp(szField,"bin_color"))
    {
        sscanf(szValue,"%d %d %d",&iR,&iG,&iB);
        cBinColor.cBinColor = QColor(iR,iG,iB);
        return 1;	// No error.
    }

    // Insert the BinColor definition to the Hard bin or Soft bin list
    if(!qstricmp(szField,"add_bin"))
    {
        if(!qstricmp(szValue,"soft_bin"))
            ReportOptions.softBinColorList.append(cBinColor);
        else
            ReportOptions.hardBinColorList.append(cBinColor);
        return 1;	// No error.
    }

    ZTHROWEXC("invalid parameter :"+ZString(szField));
    return 0;
}

static ZString gexBinStyle(ZCsl* csl)
{
    int				argc		= csl->get("argCount").asInt();							// Number of arguments.
    const char *	szField		= csl->get("field").constBuffer();						// Field name to setup
    const char *	szValue		= (argc > 1) ? csl->get("value").constBuffer() : "";	// Field value

    return gexBinStyle(szField,szValue);
}

////////////////////////////////////////////////////////////////////////////////
// Builds a test range from a test list string (or path to import file)
////////////////////////////////////////////////////////////////////////////////
CGexTestRange *createTestsRange(QString strParameterList,bool bAcceptRange=true, bool bIsAdvancedReport=false)
{
    // Check if the list is a file path...
    if(QFile::exists(strParameterList))
    {
        // The list must be imported from a file...
        strParameterList = ImportTestListFromFile(strParameterList);
    }

    // If the list doesn't allow specifying range (eg: scatter plot specieid tests pairs only), then to appropriate filtering!
    if(!bAcceptRange)
    {
        // We have a regular list...
        strParameterList = strParameterList.replace( " ", ","); // ensure only one type of delimiter!
        strParameterList = strParameterList.replace( ";", ","); // ensure only one type of delimiter!
        strParameterList = strParameterList.replace( "to", ",", Qt::CaseInsensitive); // remove 'to' strings (not case sensitive)
        strParameterList = strParameterList.replace( "-", ","); // remove '-' strings
        strParameterList = strParameterList.replace( "..", ","); // remove '..' strings
        strParameterList = strParameterList.replace( ",,",","); // simplify ',,' strings
    }
    // If this is an advanced report, we need to save the parameter list.
    if(bIsAdvancedReport)
        ReportOptions.strAdvancedTestList = strParameterList;

    // Create the range list structures from the string holdinng the test list.
    return new CGexTestRange(strParameterList.toLatin1().constData());
}

////////////////////////////////////////////////////////////////////////////////
// Function GetWaferMapSettings: Extract & save wafer map settings
////////////////////////////////////////////////////////////////////////////////
static int GetWaferMapSettings(const char *szField, const char *szValue,bool bDataMining)
{
    if(!qstricmp(szField,"disabled"))
    {
      ReportOptions.iWafermapType = GEX_WAFMAP_DISABLED;
      return 1;
    }
    if(!qstricmp(szField,"soft_bin"))
    {
      ReportOptions.iWafermapType = GEX_WAFMAP_SOFTBIN;

      if(ReportOptions.pGexWafermapRangeList)
          delete ReportOptions.pGexWafermapRangeList;
      ReportOptions.pGexWafermapRangeList = createTestsRange("1",true,false);

      if(bDataMining)
          ReportOptions.setAdvancedReportSettings (GEX_WAFMAP_SOFTBIN);
      return 1;
    }
    if(!qstricmp(szField,"stack_soft_bin"))
    {
        if(checkProfessionalFeature(true) == false)
      return 0;

        ReportOptions.iWafermapType = GEX_WAFMAP_STACK_SOFTBIN;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings( GEX_WAFMAP_STACK_SOFTBIN);

        return 1;
    }
    if(!qstricmp(szField,"hard_bin"))
    {
      ReportOptions.iWafermapType = GEX_WAFMAP_HARDBIN;

      if(ReportOptions.pGexWafermapRangeList)
          delete ReportOptions.pGexWafermapRangeList;
      ReportOptions.pGexWafermapRangeList = createTestsRange("1",true,false);

      if(bDataMining)
          ReportOptions.setAdvancedReportSettings ( GEX_WAFMAP_HARDBIN);
      return 1;
    }
    if(!qstricmp(szField,"stack_hard_bin"))
    {
      if(checkProfessionalFeature(true) == false)
      return 0;

      ReportOptions.iWafermapType = GEX_WAFMAP_STACK_HARDBIN;
      // Save list of tests
      if(ReportOptions.pGexWafermapRangeList)
          delete ReportOptions.pGexWafermapRangeList;
      ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
      if(bDataMining)
          ReportOptions.setAdvancedReportSettings( GEX_WAFMAP_STACK_HARDBIN);
      return 1;
    }
    if(!qstricmp(szField,"param_over_limits"))
    {
        ReportOptions.iWafermapType = GEX_WAFMAP_TESTOVERLIMITS;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings ( GEX_WAFMAP_TESTOVERLIMITS);
        return 1;
    }
    if(!qstricmp(szField,"stack_param_over_limits"))
    {
        if(checkProfessionalFeature(true) == false)
          return 0;

        ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TESTOVERLIMITS;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings( GEX_WAFMAP_STACK_TESTOVERLIMITS);

        return 1;
    }
    if(!qstricmp(szField,"param_over_range"))
    {
        ReportOptions.iWafermapType = GEX_WAFMAP_TESTOVERDATA;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings( GEX_WAFMAP_TESTOVERDATA);

        return 1;
    }
    if(!qstricmp(szField,"stack_param_over_range"))
    {
        if(checkProfessionalFeature(true) == false)
            return 0;

        ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TESTOVERDATA;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings(GEX_WAFMAP_STACK_TESTOVERDATA);

        return 1;
    }
    if(!qstricmp(szField,"param_passfail") || !qstricmp(szField,"test_passfail"))
    {
        ReportOptions.iWafermapType = GEX_WAFMAP_TEST_PASSFAIL;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        if (QString(szValue).startsWith("top"))
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_TOP_N_FAILTESTS;
            ReportOptions.iWafermapNumberOfTests=QString(szValue).section(' ',1,1).toInt();
            if (ReportOptions.iWafermapNumberOfTests==0)
                ReportOptions.iWafermapNumberOfTests=5;
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings( GEX_WAFMAP_TEST_PASSFAIL);

        return 1;
    }
    if(!qstricmp(szField,"stack_param_passfail") || !qstricmp(szField,"stack_test_passfail"))
    {
        if(checkProfessionalFeature(true) == false)
            return 0;

        ReportOptions.iWafermapType = GEX_WAFMAP_STACK_TEST_PASSFAIL;

        if(strstr(szValue,"all") != NULL)
        {
            ReportOptions.iWafermapTests = GEX_WAFMAP_ALL;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange("",true,false);
        }
        else
        {
            ReportOptions.iWafermapTests= GEX_WAFMAP_LIST;

            // Save list of tests
            if(ReportOptions.pGexWafermapRangeList)
                delete ReportOptions.pGexWafermapRangeList;
            ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
        }

        if(bDataMining)
            ReportOptions.setAdvancedReportSettings(GEX_WAFMAP_STACK_TEST_PASSFAIL);

        return 1;
    }

    if(!qstricmp(szField,"zonal_soft_bin"))
    {
      if(checkProfessionalFeature(true) == false)
          return 0;

      ReportOptions.iWafermapType = GEX_WAFMAP_ZONAL_SOFTBIN;
      // Save list of tests
      if(ReportOptions.pGexWafermapRangeList)
          delete ReportOptions.pGexWafermapRangeList;
      ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
      if(bDataMining)
          ReportOptions.setAdvancedReportSettings(GEX_WAFMAP_ZONAL_SOFTBIN);
      return 1;
    }
    if(!qstricmp(szField,"zonal_hard_bin"))
    {
      if(checkProfessionalFeature(true) == false)
          return 0;

      ReportOptions.iWafermapType = GEX_WAFMAP_ZONAL_HARDBIN;
      // Save list of tests
      if(ReportOptions.pGexWafermapRangeList)
          delete ReportOptions.pGexWafermapRangeList;
      ReportOptions.pGexWafermapRangeList = createTestsRange(szValue,true,false);
      if(bDataMining)
        ReportOptions.setAdvancedReportSettings(GEX_WAFMAP_ZONAL_HARDBIN);
      return 1;
    }

    // Invalid parameter string.
    return 0;
}

static ZString  gexGetEnv(ZCsl* csl)
{
    if (!csl)
        return 0;

    const char* env= csl->get("env").constBuffer();
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("gexGetEnv %1").arg( env?env:"?").toLatin1().constData());
    if (!env)
        return 0;
    QByteArray ba=qgetenv(env);
    if (ba.size()>0)
    {
        WriteScriptMessage(QString("GetEnv %1 : %2").arg(env).arg(QString(ba)), true );
        return ba.constData();
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function : Let's user define the csl version of this csl file
// Call : gexCslVersion(version);
////////////////////////////////////////////////////////////////////////////////
static ZString  gexCslVersion(ZCsl* csl)
{
    const char *	szVersion		= "";		// Section name to setup
    if (csl==NULL)
        return 0;

    //WriteScriptMessage( QString("in %1...").arg((char*)csl->last_loaded_file), true );
    GSLOG(SYSLOG_SEV_DEBUG, QString(" in %1").arg( (char*)csl->last_loaded_file).toLatin1().data() );

    /*
    //ZString ac=csl->get("argCount");
    int ac=csl->get("argCount").asInt();
    //if(ac.asInt() >= 1)
    if (ac<1)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "gexCslVersion : error : version arg not found !");
        ZTHROWEXC("invalid parameter :"+ZString(szVersion));
        //m_fCslVersion=1.0f;
        return 0;
    }
    */

    szVersion = csl->get("version").constBuffer();
    if (szVersion)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(" %1").arg(szVersion).toLatin1().data() );
    }
    else
    {
        GSLOG(SYSLOG_SEV_WARNING, " gexCslVersion() : error : no argument 'version' !");
        return 0;
    }

    if(sscanf(szVersion,"%f",&m_fCslVersion) != 1)
    {
        GSLOG(SYSLOG_SEV_WARNING, "error : can't read csl version given by gexCslVerison() function !");
        ZTHROWEXC("invalid parameter :"+ZString(szVersion));
        m_fCslVersion=0.0f; // which should be the default value ?
        return 0;
    }

    if (m_fCslVersion<GEX_MIN_CSL_VERSION)
    {
        GSLOG(SYSLOG_SEV_NOTICE, QString(" csl version %1 too old and no more supported !").arg(m_fCslVersion).toLatin1().data() );
        WriteScriptMessage( QString("Error : csl files version %1 too old and no more supported !").arg(m_fCslVersion), true );
        m_fCslVersion=0.0f;
        return 0;
    }

    if (m_fCslVersion>GEX_MAX_CSL_VERSION)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, QString(" csl version %1 not supported in this version of the product !")
               .arg(m_fCslVersion).toLatin1().data()  );
        WriteScriptMessage( QString("Error : csl version %1 not supported in this version of the product !").arg(m_fCslVersion), true );
        m_fCslVersion=0.0f;
        return 0;
    }

    if ((csl->last_loaded_file) && (m_fCslVersion<GEX_MAX_CSL_VERSION))
        // we have to bakjup the file as the next save will overwrite it
    {
        QString cop(QString("%1.bak").arg((char*)csl->last_loaded_file));
        if (!QFile::copy( QDir::cleanPath(QString("%1").arg((char*)csl->last_loaded_file)),
                          QDir::cleanPath(cop)
                        )
            )
        {
            if (!QFile::exists(cop))
            {
                WriteScriptMessage( QString(
                    "Warning : this csl file (%1) version (%2) is too old for this version of the product.\n"
                    "We have tried to do a backup but it was impossible."
                    ).arg((char*)csl->last_loaded_file).arg(m_fCslVersion), true
                );
                GSLOG(SYSLOG_SEV_DEBUG, " cannot save a backup for this csl");
            }
        }
        else
        {
            WriteScriptMessage( QString(
                    "A backup copy of this csl (%1) has been saved because its format is too old (%2)."
                                    ).arg((char*)csl->last_loaded_file).arg(m_fCslVersion), true );
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "gexCslVersion: a backup copy of this csl has been saved.");
        }
    }

    WriteScriptMessage( QString("gexCslVersion %1 for %2").arg(m_fCslVersion).arg((char*)csl->last_loaded_file),
                        true );

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function : Let's user reset all the options
// Call : gexResetOptions(param);
////////////////////////////////////////////////////////////////////////////////
static ZString  gexResetOptions(ZCsl* csl)
{
    if (!csl)
        return 0;

    const char *	szParam		= "";
    //int				argc = csl->get("argCount").asInt(); // Does NOT work when 1 arg

    szParam = csl->get("param").constBuffer();
    GSLOG(SYSLOG_SEV_DEBUG, QString(" gexResetOptions('%1'')").arg(szParam?szParam:"?").toLatin1().constData() );

    if (!szParam)
    {
        GSLOG(SYSLOG_SEV_WARNING, QString(" gexResetOptions() : error : bad parameter !").toLatin1().constData());
        WriteScriptMessage(QString("Warning : gexResetOptions(...) bad parameter !"), true);
        return 1; // should we return 0 ?
    }

    if(!qstricmp(szParam,"default"))
    {
        // Reset all options to default
        ReportOptions.Reset(true);
        return 1;	// No error.
    }
    else
    if(!qstricmp(szParam,"clear"))
    {
        // Reset all options to 0!
        ReportOptions.Reset(false);
        return 1;	// No error.
    }

    WriteScriptMessage(QString("Warning : gexResetOptions(...) unknown parameter '%1'.").arg(szParam), true);
    return 1;
}


////////////////////////////////////////////////////////////////////////////////
// Function gexReportType: Let's user define the report sections to create
// Call : gexReportType(section, type, value);
////////////////////////////////////////////////////////////////////////////////
static ZString gexReportType(ZCsl* csl)
{
    const char *	szSection		= "";		// Section name to setup
    const char *	szField			= "";		// Field name to setup
    const char *	szValue			= "";		// Field value
    const char *	szCustomValue	= "";		// Custom value used ie for Volume HL for volume axis viewport (adv Production Report)
    QString			strParameterList;	// Holds the copy of the value field
    QString			strTempString;
    int				argc = csl->get("argCount").asInt();

    if(argc >= 1)
        szSection = csl->get("section").constBuffer();

    if(argc >= 2)
        szField = csl->get("type").constBuffer();

    if(argc >= 3)
    {
        szValue = csl->get("value").constBuffer();
        // Get copy of the 'value' field in case it is a file path to a test list to import.
        strParameterList = szValue;
        strParameterList = strParameterList.simplified();	// Remove any duplicated spaces
    }

    if(argc >= 4)
        szCustomValue = csl->get("customvalue").constBuffer();

    QVector<QString> p; p.push_back(szSection);  p.push_back(strParameterList);
    if (QString(szCustomValue)!="")
        p.push_back(szCustomValue);
    ReportOptions.addReportUnit(QString(szSection), p);

    bool	bOemRelease=false;
    if(GS::LPPlugin::ProductInfo::getInstance()->isOEM())
        bOemRelease = true;

    if (!qstricmp(szSection, "adv_report_builder"))
    {
        if(!qstricmp(szField,"template"))
        {
            // Get Template file to process
            ReportOptions.setAdvancedReport(GEX_ADV_REPORT_BUILDER);
            ReportOptions.strTemplateFile = szValue;
            return 1;
        }
    }

    // Test statistics section
    if(!qstricmp(szSection,"stats"))
    {
        if(!qstricmp(szField,"disabled"))
        {
            ReportOptions.iStatsType = GEX_STATISTICS_DISABLED;
            return 1;
        }
        if(!qstricmp(szField,"all"))
        {
          ReportOptions.iStatsType = GEX_STATISTICS_ALL;
          return 1;
        }
        if(!qstricmp(szField,"fails"))
        {
          ReportOptions.iStatsType = GEX_STATISTICS_FAIL;
          return 1;
        }
        if(!qstricmp(szField,"outliers"))
        {
          ReportOptions.iStatsType = GEX_STATISTICS_OUTLIERS;
          return 1;
        }
        if(!qstricmp(szField,"tests"))
        {
            // Dummy user check: in case they select 'Histogram of specific tests'...then specify 'All' in test list!
            if(strstr(szValue,"all") != NULL)
            {
              ReportOptions.iStatsType = GEX_STATISTICS_ALL;
              return 1;
            }
          ReportOptions.iStatsType = GEX_STATISTICS_LIST;
          // Save list of tests
          if(ReportOptions.pGexStatsRangeList)
              delete ReportOptions.pGexStatsRangeList;
          ReportOptions.pGexStatsRangeList = createTestsRange(strParameterList);
          return 1;
        }
        if(!qstricmp(szField,"cp"))
        {
          ReportOptions.iStatsType = GEX_STATISTICS_BADCP;
          sscanf(szValue,"%lf",&ReportOptions.lfStatsLimit);
          return 1;
        }
        if(!qstricmp(szField,"cpk"))
        {
          ReportOptions.iStatsType = GEX_STATISTICS_BADCPK;
          sscanf(szValue,"%lf",&ReportOptions.lfStatsLimit);
          return 1;
        }
        if(!qstricmp(szField,"top_n_fails"))
        {
            ReportOptions.iStatsType = GEX_STATISTICS_TOP_N_FAILTESTS;
            sscanf(szValue,"%lf",&ReportOptions.lfStatsLimit);
            return 1;
        }

        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Wafer map section
    if(!qstricmp(szSection,"wafer"))
    {
        if(GetWaferMapSettings(szField,szValue,false))
            return 1;

        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Histogram section
    if(!qstricmp(szSection,"histogram"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.iHistogramType = GEX_HISTOGRAM_DISABLED;
          return 1;
        }

        // Forces standard histogram section to use Advanced histograms instead.
        if(!qstricmp(szField,"advanced"))
        {
          ReportOptions.bForceAdvancedHisto = true;
          return 1;
        }

        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.iHistogramType = GEX_HISTOGRAM_OVERLIMITS;
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
                return 1;
            }
            if (QString(szValue).startsWith("top"))
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_TOP_N_FAIL_TESTS;
                ReportOptions.iHistogramNumberOfTests = QString(szValue).section(' ',1,1).toInt();
                if (ReportOptions.iHistogramNumberOfTests==0)
                    ReportOptions.iHistogramNumberOfTests=10;
                GSLOG(SYSLOG_SEV_DEBUG, QString("gexReportType(): histogram : will show only the top %1 failtests")
                       .arg(ReportOptions.iHistogramNumberOfTests)
                       .toLatin1().data() );
                return 1;
            }
            ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
            // Save list of tests
            if(ReportOptions.pGexHistoRangeList)
                delete ReportOptions.pGexHistoRangeList;
            ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
            return 1;
        }
        if(!qstricmp(szField,"cumul_over_limits"))
        {
            ReportOptions.iHistogramType = GEX_HISTOGRAM_CUMULLIMITS;
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
                return 1;
            }
            if (QString(szValue).startsWith("top"))
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_TOP_N_FAIL_TESTS;
                ReportOptions.iHistogramNumberOfTests = QString(szValue).section(' ',1,1).toInt();
                if (ReportOptions.iHistogramNumberOfTests==0)
                    ReportOptions.iHistogramNumberOfTests=10;
                GSLOG(SYSLOG_SEV_DEBUG, QString("gexReportType: histogram : will show only the top %1 failtests").arg(
                    ReportOptions.iHistogramNumberOfTests).toLatin1().constData());
                return 1;
            }
            ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
            // Save list of tests
            if(ReportOptions.pGexHistoRangeList)
                delete ReportOptions.pGexHistoRangeList;
            ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.iHistogramType = GEX_HISTOGRAM_OVERDATA;
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
                return 1;
            }
            if (QString(szValue).startsWith("top"))
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_TOP_N_FAIL_TESTS;
                ReportOptions.iHistogramNumberOfTests = QString(szValue).section(' ',1,1).toInt();
                if (ReportOptions.iHistogramNumberOfTests==0)
                    ReportOptions.iHistogramNumberOfTests=10;
                GSLOG(SYSLOG_SEV_DEBUG, QString("gexReportType: histogram : will show only the top %1 failtests").arg(
                       ReportOptions.iHistogramNumberOfTests).toLatin1().constData());
                return 1;
            }
            ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
            // Save list of tests
            if(ReportOptions.pGexHistoRangeList)
                delete ReportOptions.pGexHistoRangeList;
            ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
            return 1;
        }
        if(!qstricmp(szField,"cumul_over_range"))
        {
            ReportOptions.iHistogramType = GEX_HISTOGRAM_CUMULDATA;
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
                return 1;
            }
            if (QString(szValue).startsWith("top"))
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_TOP_N_FAIL_TESTS;
                ReportOptions.iHistogramNumberOfTests = QString(szValue).section(' ',1,1).toInt();
                if (ReportOptions.iHistogramNumberOfTests==0)
                    ReportOptions.iHistogramNumberOfTests=10;
                GSLOG(SYSLOG_SEV_DEBUG, QString("gexReportType: histogram : will show only the top %1 failtests")
                      .arg( ReportOptions.iHistogramNumberOfTests).toLatin1().constData());
                return 1;
            }
            ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
            // Save list of tests
            if(ReportOptions.pGexHistoRangeList)
                delete ReportOptions.pGexHistoRangeList;
            ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.iHistogramType = GEX_HISTOGRAM_DATALIMITS;
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_ALL;
                return 1;
            }
            if (QString(szValue).startsWith("top"))
            {
                ReportOptions.iHistogramTests = GEX_HISTOGRAM_TOP_N_FAIL_TESTS;
                ReportOptions.iHistogramNumberOfTests = QString(szValue).section(' ',1,1).toInt();
                if (ReportOptions.iHistogramNumberOfTests==0)
                    ReportOptions.iHistogramNumberOfTests=10;
                GSLOG(SYSLOG_SEV_DEBUG, QString("gexReportType: histogram : will show only the top %1 failtests").arg(
                         ReportOptions.iHistogramNumberOfTests).toLatin1().constData());
                return 1;
            }
            ReportOptions.iHistogramTests= GEX_HISTOGRAM_LIST;
            // Save list of tests
            if(ReportOptions.pGexHistoRangeList)
                delete ReportOptions.pGexHistoRangeList;
            ReportOptions.pGexHistoRangeList = createTestsRange(strParameterList);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }
    // FTR correlation
    if(!qstricmp(szSection,"adv_ftr_correlation"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        ReportOptions.setAdvancedReport(GEX_ADV_FTR_CORRELATION);
        // Save list of tests
        if(ReportOptions.pGexAdvancedRangeList)
            delete ReportOptions.pGexAdvancedRangeList;
        ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
        return 1;
    }
    //Functional Histo
    if(!qstricmp(szSection,"adv_functional_histogram"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }


        if(!qstricmp(szField,"cycl_cnt"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_FUNCTIONAL);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_FUNCTIONAL_CYCL_CNT);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"rel_vadr"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_FUNCTIONAL);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_FUNCTIONAL_REL_VAD);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Histogram section
    if(!qstricmp(szSection,"adv_histogram"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_HISTOGRAM);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_HISTOGRAM_OVERLIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"cumul_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_HISTOGRAM);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_HISTOGRAM_CUMULLIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_HISTOGRAM);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_HISTOGRAM_OVERDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"cumul_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_HISTOGRAM);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_HISTOGRAM_CUMULDATA);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_HISTOGRAM);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_HISTOGRAM_DATALIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Trend section
    if(!qstricmp(szSection,"adv_trend"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_OVERLIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_OVERDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_DATALIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"difference"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_DIFFERENCE);

            // Save list of tests...should only be 2 tests.
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_mean"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_AGGREGATE_MEAN);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_sigma"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_AGGREGATE_SIGMA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_cp"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_AGGREGATE_CP);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_cpk"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_AGGREGATE_CPK);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"soft_bin_sublots"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_SOFTBIN_SBLOTS);

            // Save SoftBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"soft_bin_parts"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_SOFTBIN_PARTS);

            // Save SoftBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"hard_bin_sublots"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_HARDBIN_SBLOTS);

            // Save HardBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"hard_bin_parts"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_HARDBIN_PARTS);

            // Save HardBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"soft_bin_rolling"))
        {
            if(checkProfessionalFeature(true) == false)
                return 1;

            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_SOFTBIN_ROLLING);

            // Save SoftBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"hard_bin_rolling"))
        {
            if(checkProfessionalFeature(true) == false)
                return 1;

            ReportOptions.setAdvancedReport(GEX_ADV_TREND);
                    ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_HARDBIN_ROLLING);

            // Save HardBin# list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Scatter/Correlation section
    if(!qstricmp(szSection,"adv_correlation"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CORRELATION);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CORR_OVERLIMITS);

            // Save Test pairs list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CORRELATION);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CORR_OVERDATA);

            // Save Test pairs list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CORRELATION);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CORR_DATALIMITS);

            // Save Test pairs list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Boxplot section
    if(!qstricmp(szSection,"adv_probabilityplot"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }

        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROBABILITY_PLOT);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PROBPLOT_OVERLIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROBABILITY_PLOT);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PROBPLOT_OVERDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROBABILITY_PLOT);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PROBPLOT_DATALIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Boxplot section
    if(!qstricmp(szSection,"adv_boxplot_ex"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }

        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_BOXPLOT);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_BOXPLOT_OVERLIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_BOXPLOT);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_BOXPLOT_OVERDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_BOXPLOT);
            ReportOptions.setAdvancedReportSettings( GEX_ADV_BOXPLOT_DATALIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Multi-chart section
    if(!qstricmp(szSection,"adv_multichart"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }

        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport( GEX_ADV_MULTICHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_MULTICHART_OVERLIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_MULTICHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_MULTICHART_OVERDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_MULTICHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_MULTICHART_DATALIMITS);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Multi-chart section
    if(!qstricmp(szSection,"adv_charac1"))
    {
        if(!qstricmp(szField, "disabled"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_DISABLED);
            return 1;
        }
        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_CHARAC_BOXWHISKER_CHART);
            ReportOptions.setAdvancedReportSettings (GEX_ADV_CHARAC_CHART_OVERLIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_CHARAC_BOXWHISKER_CHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CHARAC_CHART_OVERDATA);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CHARAC_BOXWHISKER_CHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CHARAC_CHART_DATALIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }

        if(!qstricmp(szField,"chart"))
        {
            if (!qstricmp(szValue,"top_level_aggregate"))
            {
                if (ReportOptions.mBoxWhiskerTemplate == NULL)
                    ReportOptions.mBoxWhiskerTemplate = new GS::Gex::CharacBoxWhiskerTemplate();

                QString lCustomValue(szCustomValue);
                QString lCondition;
                QColor  lColor;

                int lIndex = lCustomValue.indexOf(";");

                while (lIndex != -1)
                {
                    QString lParam = lCustomValue.left(lIndex);
                    QString lField = lParam.section("=", 0, 0);
                    QString lValue = lParam.section("=", 1, 1);

                    if (lField == "--conditions")
                    {
                        lCondition = lValue;

                        if (lCondition.isEmpty())
                            ZTHROWEXC("Empty condition in charac box whisker template");
                    }
                    else if (lField == "--color")
                    {
                        lColor.setNamedColor(lValue);

                        if (lColor.isValid() == false)
                            ZTHROWEXC("Invalid color in charac box whisker template");
                    }
                    else
                        ZTHROWEXC("invalid parameter: "+ZString(szField)+" - " +
                                  ZString(szValue) + " - " + ZString(szCustomValue));

                    lCustomValue = lCustomValue.mid(lIndex+1);
                    lIndex = lCustomValue.indexOf(";");
                }

                if (lCondition.isEmpty() == false && lColor.isValid())
                    ReportOptions.mBoxWhiskerTemplate->SetTopLevelColor(lCondition,
                                                                        lColor);

                return 1;
            }
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Line chart characterization report
    if(!qstricmp(szSection,"adv_charac2"))
    {
        if(!qstricmp(szField, "disabled"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_DISABLED);
            return 1;
        }
        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_CHARAC_LINE_CHART);
            ReportOptions.setAdvancedReportSettings (GEX_ADV_CHARAC_CHART_OVERLIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReport (GEX_ADV_CHARAC_LINE_CHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CHARAC_CHART_OVERDATA);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CHARAC_LINE_CHART);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_CHARAC_CHART_DATALIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }

        if(!qstricmp(szField,"chart"))
        {
            if (ReportOptions.mLineChartTemplate == NULL)
                ReportOptions.mLineChartTemplate = new GS::Gex::CharacLineChartTemplate();

            if (!qstricmp(szValue,"variable"))
            {
                ReportOptions.mLineChartTemplate->SetVariable(szCustomValue);
                return 1;
            }

            if (!qstricmp(szValue,"serie_conditions"))
            {
                ReportOptions.mLineChartTemplate->SetSerieCondition(QString(szCustomValue).split("|", QString::SkipEmptyParts));
                return 1;
            }

            if (!qstricmp(szValue,"add_serie"))
            {
                GS::Gex::CharacLineChartSerie serie;
                QString lCustomValue(szCustomValue);

                if (ReportOptions.mLineChartTemplate->GetSerieConditions().count() == 0)
                    ZTHROWEXC("No conditions defined in the chart series");

                int lIndex = lCustomValue.indexOf(";");

                while (lIndex != -1)
                {
                    QString lParam = lCustomValue.left(lIndex);
                    QString lField = lParam.section("=", 0, 0);
                    QString lValue = lParam.section("=", 1, 1);

                    if (lField == "--conditions")
                        serie.SetConditions(lValue.split("|", QString::SkipEmptyParts));
                    else if (lField == "--color")
                    {
                        QColor color(lValue);

                        if (color.isValid() == false)
                            ZTHROWEXC("Invalid color defined in the series: " + ZString(lValue.toLatin1().data()));
                        else
                            serie.SetColor(color);
                    }
                    else if (lField == "--name")
                        serie.SetName(lValue);
                    else if (lField == "--variable")
                        serie.SetVariable(lValue);
                    else
                        ZTHROWEXC("invalid parameter: "+ZString(szField)+" - " +
                                  ZString(szValue) + " - " + ZString(szCustomValue));

                    lCustomValue = lCustomValue.mid(lIndex+1);
                    lIndex = lCustomValue.indexOf(";");
                }

                ReportOptions.mLineChartTemplate->AddSerieDefinition(serie);
                return 1;
            }
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Boxplot section
    if(!qstricmp(szSection,"adv_boxplot"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"tests"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_CANDLE_MEANRANGE);
            if(strstr(szValue,"all")!=NULL)
            {
                ReportOptions.setAdvancedReportSettings(GEX_ADV_ALL);
                // Save list of tests: Take them ALL!
                QString strAllTests;
                strAllTests = QString::number(GEX_MINTEST) + " to " + QString::number(GEX_MAXTEST);

                // Save Test list
                if(ReportOptions.pGexAdvancedRangeList)
                    delete ReportOptions.pGexAdvancedRangeList;
                ReportOptions.pGexAdvancedRangeList = createTestsRange(strAllTests,true,true);
                return 1;
            }
            else
            {
                if (QString(szField).startsWith("top"))
                 ReportOptions.setAdvancedReportSettings(GEX_ADV_TOP_N_FAILTESTS);
                else
                {
                 ReportOptions.setAdvancedReportSettings(GEX_ADV_LIST);
                 // Save Test list
                 if(ReportOptions.pGexAdvancedRangeList)
                  delete ReportOptions.pGexAdvancedRangeList;
                 ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
                }
            }
            return 1;
        }
        GSLOG(SYSLOG_SEV_ERROR, QString("Invalid gexReportType parameter %1").arg( szField).toLatin1().constData());
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Datalog section
    if(!qstricmp(szSection,"adv_datalog"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(strstr(szField,"all")!=NULL)
        {
            ReportOptions.setAdvancedReport(GEX_ADV_DATALOG);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_DATALOG_ALL);
            return 1;
        }
        if(!qstricmp(szField,"outliers"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_DATALOG);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_DATALOG_OUTLIER);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"fails"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_DATALOG);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_DATALOG_FAIL);
            return 1;
        }
        if(!qstricmp(szField,"tests"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_DATALOG);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_DATALOG_LIST);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"tests_only"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_DATALOG);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_DATALOG_RAWDATA);

            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced PRODUCTION Reports (YIELD) section
    if(!qstricmp(szSection,"adv_production_yield"))
    {
        if(checkProfessionalFeature(true) == false)
            return 1;

        // Specify information to trend
        if(!qstricmp(szField,"sublot"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_SBLOT);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        if(!qstricmp(szField,"lot"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_LOT);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        if(!qstricmp(szField,"group"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_GROUP);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        if(!qstricmp(szField,"daily"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_DAY);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        if(!qstricmp(szField,"weekly"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_WEEK);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        if(!qstricmp(szField,"monthly"))
        {
            ReportOptions.setAdvancedReport(GEX_ADV_PROD_YIELD);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PRODYIELD_MONTH);
            // Save Bin list
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;	// No error.
        }
        // Specify chart title
        if(!qstricmp(szField,"title"))
        {
            ReportOptions.strProdReportTitle = strParameterList; // Use strString instead of szValue, because szValue has been lower-cased
            return 1;	// No error.
        }
        // Yield Axis options
        if(!qstricmp(szField,"yield_axis"))
        {
            if(!qstricmp(szValue,"0-100"))
                ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_0_100;
            if(!qstricmp(szValue,"0-max"))
                ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_0_MAX;
            if(!qstricmp(szValue,"min-100"))
                ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_MIN_100;
            if(!qstricmp(szValue,"min-max"))
                ReportOptions.uiYieldAxis = GEX_ADV_PROD_YIELDAXIS_MIN_MAX;
            return 1;	// No error.
        }
        // Volume Axis options
        if(!qstricmp(szField,"volume_axis"))
        {
            if(!qstricmp(szValue,"0-max"))
                ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_0_MAX;
            if(!qstricmp(szValue,"0-custom"))
            {
                strTempString = szCustomValue;
                ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_0_CUSTOM;
                ReportOptions.uiVolumeHL = strTempString.toUInt();
            }
            if(!qstricmp(szValue,"min-max"))
                ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_MIN_MAX;
            if(!qstricmp(szValue,"min-custom"))
            {
                strTempString = szCustomValue;
                ReportOptions.uiVolumeAxis = GEX_ADV_PROD_VOLUMEAXIS_MIN_CUSTOM;
                ReportOptions.uiVolumeHL = strTempString.toUInt();
            }
            return 1;	// No error.
        }
        // Yield marker
        if(!qstricmp(szField,"yield_marker"))
        {
            strTempString = szValue;
            ReportOptions.uiYieldMarker = strTempString.toUInt();
            return 1;	// No error.
        }

        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Test to test correlation (for Test time optimization): Pearson's correlation value
    if(!qstricmp(szSection,"adv_pearson"))
    {
        ReportOptions.setAdvancedReport(GEX_ADV_PEARSON);
        if (!szField || QString(szField).isEmpty())
        {
            ReportOptions.setAdvancedReportSettings (GEX_ADV_PEARSON_OVERLIMITS);
            return 1;
        }

        if(!qstricmp(szField,"test_over_limits"))
        {
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PEARSON_OVERLIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"test_over_range"))
        {
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PEARSON_OVERDATA);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        if(!qstricmp(szField,"adaptive"))
        {
            ReportOptions.setAdvancedReportSettings(GEX_ADV_PEARSON_DATALIMITS);
            // Save list of tests
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strParameterList,true,true);
            return 1;
        }
        GSLOG(SYSLOG_SEV_WARNING, QString("Unknonw setting %1").arg( szField).toLatin1().constData());
        return 1;
    }

    // Now handled in mReportsMap
    if(!qstricmp(szSection,"adv_shift"))
    {
        GSLOG(SYSLOG_SEV_NOTICE, "gexReportType adv_shift");
        ReportOptions.setAdvancedReport(GEX_ADV_SHIFT);
        return 1;
    }

        // PAT Traceability report: extract text logged into STDF.DTR records during PAT off-line processing
    if(!qstricmp(szSection,"adv_pat"))
    {
        ReportOptions.setAdvancedReport(GEX_ADV_PAT_TRACEABILITY);

        if(!qstricmp(szField, "traceability_file"))
            ReportOptions.SetPATTraceabilityFile(strParameterList);

        return 1;
    }

    // PAT-Man: triggering a 'Outlier Removal' report generation (following the Outlier sweeping over a STDF file)
    if(!qstricmp(szSection,"adv_data_cleaning"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"enabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_OUTLIER_REMOVAL);
          return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Advanced Optimizer Diags section
    if(!qstricmp(szSection,"adv_optimizer"))
    {
        if(!qstricmp(szField,"disabled"))
        {
          ReportOptions.setAdvancedReport(GEX_ADV_DISABLED);
          return 1;
        }
        if(!qstricmp(szField,"all"))
        {
            //
            ReportOptions.setAdvancedReport(GEX_ADV_GO_DIAGNOSTICS);
            ReportOptions.setAdvancedReportSettings(GEX_ADV_TREND_OVERLIMITS);
            // Save list of tests: Take them ALL!
            QString strAllTests;
            strAllTests = QString::number(GEX_MINTEST) + " to " + QString::number(GEX_MAXTEST);
            if(ReportOptions.pGexAdvancedRangeList)
                delete ReportOptions.pGexAdvancedRangeList;
            ReportOptions.pGexAdvancedRangeList = createTestsRange(strAllTests);
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Interactive Drill section: DataMining-Yield Analysis
    if(!qstricmp(szSection,"drill_disabled"))
    {
          ReportOptions.iDrillReport = -1;
          return 1;
    }

    if(!qstricmp(szSection,"drill_what_if"))
    {
        ReportOptions.iDrillReport = GEX_DRILL_GUARDBANDING;
        ReportOptions.setAdvancedReport(GEX_ADV_GUARDBANDING);
        return 1;
    }

    if(!qstricmp(szSection,"drill_wafer"))
    {
        // Extract Drill type (structure shared with GEX-InstantReport)
        ReportOptions.iDrillReport = GEX_DRILL_WAFERPROBE;
        if(GetWaferMapSettings(szField,szValue,true))
            return 1;
    }
    if(!qstricmp(szSection,"drill_packaged"))
    {
        // Extract Drill type (structure shared with GEX-InstantReport)
        ReportOptions.iDrillReport = GEX_DRILL_PACKAGED;
        if(GetWaferMapSettings(szField,szValue,true))
            return 1;
    }
    if(!qstricmp(szSection,"drill_histo"))
    {
        // Extract Drill type (structure shared with GEX-InstantReport)
        ReportOptions.iDrillReport = GEX_DRILL_HISTOGRAM;
        return 1;
    }
    if(!qstricmp(szSection,"drill_trend"))
    {
        // Extract Drill type (structure shared with GEX-InstantReport)
        ReportOptions.iDrillReport = GEX_DRILL_TREND;
        return 1;
    }
    if(!qstricmp(szSection,"drill_correlation"))
    {
        // Extract Drill type (structure shared with GEX-InstantReport)
        ReportOptions.iDrillReport = GEX_DRILL_CORRELATION;
        return 1;
    }

    // MyReports section
    if(!qstricmp(szSection,"adv_my_report"))
    {
        if(!bOemRelease && checkProfessionalFeature(false) == false)
            return 0;

        if(!qstricmp(szField,"template"))
        {
            // Get Template file to process
            ReportOptions.setAdvancedReport(GEX_ADV_TEMPLATE);
            ReportOptions.strTemplateFile = szValue;
            return 1;
        }

        if(!qstricmp(szField,"disabled"))
        {
            // Get Template file to process
            ReportOptions.strTemplateFile = "";
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // AER section
    if(!qstricmp(szSection,"adv_report_center"))
    {
        if(!bOemRelease && checkProfessionalFeature(false) == false)
        {
            GSLOG(SYSLOG_SEV_NOTICE, "This report type (adv_report_center) needs professional feature on.");
            return 0;
        }
        if(!qstricmp(szField,"template"))
        {
            if (!QFile::exists(szValue))
              GSLOG(SYSLOG_SEV_WARNING, QString("gex Report type: Template file %1 does not exist.").arg(
                szValue).toLatin1().constData());
            // Get Template file to process
            ReportOptions.strReportCenterTemplateFile = szValue;
            return 1;
        }

        if(!qstricmp(szField,"disabled"))
        {
            // Get Template file to process
            ReportOptions.strReportCenterTemplateFile = "";
            return 1;
        }

        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

    // Global advanced options
    if(!qstricmp(szSection,"adv_global_options"))
    {
        if(!qstricmp(szField,"binning"))
        {
            // Get which binning to focus on (hard-bin or soft bin)
            if(!qstricmp(szValue,"hard_bin"))
                ReportOptions.bAdvancedUsingSoftBin = false;
            else
                ReportOptions.bAdvancedUsingSoftBin = true;
            return 1;
        }
        ZTHROWEXC("invalid parameter :"+ZString(szField));
    }

  QString m=QString("Invalid or unknown gexReportType : '%1'").arg(szSection);
  WriteScriptMessage(m, true);
  GSLOG(SYSLOG_SEV_ERROR, m.toLatin1().data() );
    ZTHROWEXC("invalid parameter :"+ZString(szSection));
    return 0;
} // gexReportType

////////////////////////////////////////////////////////////////////////////////
// Function gexFileInfo: Extracts info about a given data file
// Call : gexFileInfo(file_name, field);
////////////////////////////////////////////////////////////////////////////////
static ZString gexFileInfo(ZCsl* csl)
{
    int				group_id	= 0;
    int				file_id		= 0;
    const char *	szField		= csl->get("group_id").constBuffer();

    sscanf(szField,"%d",&group_id);
    szField = csl->get("file_id").constBuffer();
    sscanf(szField,"%d",&file_id);
    szField = csl->get("field").constBuffer();

    if(checkProfessionalFeature(true) == false)
        return 0;

    // Check if valid handle to main GUI window
    if(pGexMainWindow == NULL)
        return 0;

    // If processing error
    if(gexReport == NULL)
        return 0;

    CGexGroupOfFiles *pGroup;
    CGexFileInGroup *pFile;
    if((group_id < 0) || (group_id >= gexReport->getGroupsList().size()))
        return 0;
    pGroup = gexReport->getGroupsList().at(group_id);
    if(pGroup == NULL)
        return 0;

    if(file_id >= pGroup->pFilesList.size())
        return 0;
    pFile = pGroup->pFilesList.at(file_id);
    if(pFile == NULL)
        return 0;

    if(!qstricmp(szField,"file_name"))
    {
        // Return original file name.
        return pFile->strFileName.toLatin1().constData();
    }
    if(!qstricmp(szField,"stdf_file_name"))
    {
        // Return original file name.
        return pFile->strFileNameSTDF.toLatin1().constData();
    }
    if(!qstricmp(szField,"stdf_file_name"))
    {
        // Return original file name.
        return pFile->strFileNameSTDF.toLatin1().constData();
    }
    if(!qstricmp(szField,"program_name"))
    {
        // Program name.
        return pFile->getMirDatas().szJobName;
    }
    if(!qstricmp(szField,"program_version"))
    {
        // Program name.
        return pFile->getMirDatas().szJobRev;
    }
    if(!qstricmp(szField,"lot_id"))
    {
        // Lot ID.
        return pFile->getMirDatas().szLot;
    }
    if(!qstricmp(szField,"wafer_id"))
    {
        // Wafere ID.
        return pFile->getWaferMapData().szWaferID;
    }
    if(!qstricmp(szField,"tester_name"))
    {
        // Tester name.
        return pFile->getMirDatas().szNodeName;
    }
    if(!qstricmp(szField,"tester_type"))
    {
        // Tester type.
        return pFile->getMirDatas().szTesterType;
    }
    if(!qstricmp(szField,"product"))
    {
        // Part Type type.
        return pFile->getMirDatas().szPartType;
    }
    if(!qstricmp(szField,"operator"))
    {
        // Part Type type.
        return pFile->getMirDatas().szOperator;
    }
    if(!qstricmp(szField,"exec_type"))
    {
        // Exec type.
        return pFile->getMirDatas().szExecType;
    }
    if(!qstricmp(szField,"exec_version"))
    {
        // Exec version.
        return pFile->getMirDatas().szExecVer;
    }
    if(!qstricmp(szField,"good_parts"))
    {
        // Total good parts.
        return pFile->getPcrDatas().lGoodCount;
    }
    if(!qstricmp(szField,"bad_parts"))
    {
        // Total Failing parts.
        return pFile->getPcrDatas().lPartCount - pFile->getPcrDatas().lGoodCount;
    }
    if(!qstricmp(szField,"total_parts"))
    {
        // Total parts tested
        return pFile->getPcrDatas().lPartCount;
    }
    if(!qstricmp(szField,"facility_id"))
    {
        // Facility ID.
        return pFile->getMirDatas().szFacilityID;
    }
    if(!qstricmp(szField,"family_id"))
    {
        // Family ID.
        return pFile->getMirDatas().szFamilyID;
    }
    if(!qstricmp(szField,"floor_id"))
    {
        // Floor ID.
        return pFile->getMirDatas().szFloorID;
    }
    if(!qstricmp(szField,"flow_id"))
    {
        // Flow ID.
        return pFile->getMirDatas().szFlowID;
    }
    if(!qstricmp(szField,"handler_id"))
    {
        // Hanlder/Prober ID.
        return pFile->getMirDatas().szHandlerProberID;
    }
    if(!qstricmp(szField,"package_id"))
    {
        // Package ID.
        return pFile->getMirDatas().szPkgType;
    }
    if(!qstricmp(szField,"probecard_id"))
    {
        // Probe card ID.
        return pFile->getMirDatas().szProbeCardID;
    }
    if(!qstricmp(szField,"serial_id"))
    {
        // Serial ID.
        return pFile->getMirDatas().szSerialNumber;
    }
    if(!qstricmp(szField,"setup_id"))
    {
        // setup ID.
        return pFile->getMirDatas().szSetupID;
    }
    if(!qstricmp(szField,"spec_name"))
    {
        // spec name.
        return pFile->getMirDatas().szSpecName;
    }
    if(!qstricmp(szField,"spec_version"))
    {
        // spec name.
        return pFile->getMirDatas().szSpecVersion;
    }
    if(!qstricmp(szField,"sublot_id"))
    {
        // sublot ID.
        return pFile->getMirDatas().szSubLot;
    }
    if(!qstricmp(szField,"temperature"))
    {
        // Temperature ID.
        return pFile->getMirDatas().szTestTemperature;
    }
    if(!qstricmp(szField,"user_text"))
    {
        // Temperature ID.
        return pFile->getMirDatas().szUserText;
    }
    if(!qstricmp(szField,"aux_file"))
    {
        // Temperature ID.
        return pFile->getMirDatas().szAuxFile;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexReportSettings: allows to force some reports settings
////////////////////////////////////////////////////////////////////////////////
static ZString gexReportProperties(ZCsl* csl)
{
    QString strSection(csl->get("section").constBuffer());	// Section name
    QString strField(csl->get("field").constBuffer());		// Field name
    QString strValue(csl->get("value").constBuffer());		// Field value

    if (strSection == "output")
    {
        if (strField == "root_folder")
        {
            gexReport->setJsReportRootFolderAbsPath(strValue);
        }
        else if (strField == "sub_folders")
        {
            gexReport->setJsReportSubFolders(strValue);
        }
        else if (strField == "file_name")
        {
            gexReport->setJsReportFileName(strValue);
        }
        else if (strField == "generation_mode")
        {
            gexReport->setReportGenerationMode(strValue);
        }
    }

    return 1;
}

//////////////////////////////////////////////////////////////////////////////
// Function gexStdfToCSV: convert from stdf files list from stdf to csv.
// Call : gexStdfToCSV(const stdf_list);
//////////////////////////////////////////////////////////////////////////////

int gexStdfToCSV(const QStringList &sStdfList, const QString &strCSVDestPath)
{

    QDir oDir(strCSVDestPath);
    if(!oDir.exists())
        oDir.mkpath(strCSVDestPath);
    int iError = 0;
    foreach(const QString &strStdf, sStdfList){
        QFileInfo oSTDFInfo(strStdf);
        QString strCsvName = oSTDFInfo.fileName();
        strCsvName.replace(".stdf", ".csv");
        strCsvName = strCSVDestPath + QDir::separator() + strCsvName;
        oDir.remove(strCsvName);	// Make sure destination doesn't exist.
        CSTDFtoCSV      oCsvExport;			// To Convert to CSV
        bool bResult = oCsvExport.Convert(strStdf, strCsvName);
        if(!bResult){
            iError++;
            QString strErrorMessage = "*FAILED* to convert strCsvName" + oCsvExport.GetLastError();
            WriteScriptMessage(strErrorMessage,true);
        }
        oDir.remove(strStdf);
    }
    return iError;

}

////////////////////////////////////////////////////////////////////////////////
// Function gexGenStdfFromQuery: Generate STDF files from a query and return list
// generated stdf files separated by a comma.
// Call : gexGenStdfFromQuery(const value);
////////////////////////////////////////////////////////////////////////////////

static ZString gexGenCSVFromQuery(ZCsl *csl)
{
//        int				argc		= csl->get("argCount").asInt();		// Number of arguments.
//        const char *	szAction	= csl->get("action").constBuffer();	// action type
//        const char *	szValue		= csl->get("value").constBuffer();	// Action parameter
    QString strCSVDestPath = csl->get("value").constBuffer();
    QString lErrorMessage;

    // Query (list of files to process)
    QStringList sFilesMatchingQuery;

    // Dataset source results from running a Query.
    ReportOptions.bQueryDataset = true;

    QList<GexDatabaseQuery> dbQueries = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySplit(cQuery, lErrorMessage);

    if (dbQueries.isEmpty() && !lErrorMessage.isEmpty())
    {
        ZTHROWEXC(lErrorMessage.toLatin1().constData());
        return 0;
    }

    QList<GexDatabaseQuery>::iterator   itQuery;

    for (itQuery = dbQueries.begin(); itQuery != dbQueries.end(); ++itQuery)
    {
        GexDatabaseQuery dbQueryTmp = (*itQuery);

        // Find all files matching query into the group.
        sFilesMatchingQuery = GS::Gex::Engine::GetInstance().GetDatabaseEngine().QuerySelectFiles(&dbQueryTmp, lErrorMessage);

        // Make sure at least 1 file is returned
        if(sFilesMatchingQuery.isEmpty())
        {
                GSLOG(SYSLOG_SEV_ERROR, "warning in QuerySelectFiles() : No data matching query criteria.");
                WriteScriptMessage(QString("Warning : No data matching query criteria in Database ")
                                                                   +dbQueryTmp.strDatabaseLogicalName
                                                                   +QString(". TestingStage: ")
                                                                   +dbQueryTmp.strDataTypeQuery
                                                                   + ", "
                                                                   + dbQueryTmp.strlSqlFilters.join(", "), true);
                if(!lErrorMessage.isEmpty())
                {
                    GSLOG(SYSLOG_SEV_ERROR, lErrorMessage.toLatin1().data());
                    WriteScriptMessage(QString("Warning : ")
                                       + lErrorMessage, true);
                    ZTHROWEXC(lErrorMessage.toLatin1().constData());
                    return 0;
                }
        }
        else
        {
            QString     splitDirectory;
            QString     destinationPath = strCSVDestPath;
            int         idxFrom         = 0;
            int         idxBegin        = dbQueryTmp.strTitle.indexOf("[", idxFrom);
            int         idxEnd          = dbQueryTmp.strTitle.indexOf("]", idxBegin);

            // Looking for split field used
            while (idxBegin != -1 && idxEnd != -1)
            {
                splitDirectory   = dbQueryTmp.strTitle.mid(idxBegin+1, idxEnd-idxBegin-1);

                destinationPath += QDir::separator() + splitDirectory.replace(QRegExp("[^a-zA-Z0-9\\\\d\\\\s]"),"_");

                idxFrom     = idxEnd;
                idxBegin    = dbQueryTmp.strTitle.indexOf("[", idxFrom);
                idxEnd      = dbQueryTmp.strTitle.indexOf("]", idxBegin);
            }

            gexStdfToCSV(sFilesMatchingQuery, destinationPath);
        }
    }

    // Reset query structure
    cQuery.clear();

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexMerge: merge function )
////////////////////////////////////////////////////////////////////////////////
static ZString gexMerge(ZCsl* csl)
{

    // Only applies to Examinator-Monitoring!
    if(!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
  {
      GSLOG(SYSLOG_SEV_WARNING, "This function is available when Monitoring feature is activated. Aborting.");
      return 0;
  }

    //  'add_test_file', 'file path' => add a test file
    //  'add_retest_file','file path' => add a retest file
    //  sort , 'date' OR 'none' => sort the test and retest file by date or non
    //  if_more_retest_part, 'proceed' Or 'swap' Or 'cancel' => what to do if the retest contains more part then the test
    //  'out' , 'merged file destination' => the merged file path
    //  'merge' => execute the merge

    int argc = csl->get("argCount").asInt();

    const char *	szAction	= csl->get("action").constBuffer();
    const char *	szValue		= "";
    if(argc == 2)
        szValue = csl->get("value").constBuffer();

    if(!qstricmp(szAction,"add_test_file")){
        if(!QFile::exists(szValue)){
            ZTHROWEXC("File does not exist :"+ZString(szValue));
            return 0;
        }

        staticMapMergeOptions["add_test_file"]+= QString(";%1").arg(szValue);
        return 1;

    }else if(!qstricmp(szAction,"add_retest_file")){
        if(!QFile::exists(szValue)){
            ZTHROWEXC("File does not exist :"+ZString(szValue));
            return 0;
        }
        staticMapMergeOptions["add_retest_file"]+= QString(";%1").arg(szValue);
        return 1;

    }else if(!qstricmp(szAction,"sort")){
        if(!qstricmp(szValue,"date"))
            staticMapMergeOptions["sort"] = QString::number(1);
        else if(!qstricmp(szValue,"none"))
            staticMapMergeOptions["sort"] = QString::number(0);
        else {
            ZTHROWEXC("invalid field :"+ZString(szValue));
            return 0;
        }
        return 1;

    }else if(!qstricmp(szAction,"out")){

        staticMapMergeOptions["out"] = QString(szValue);
        return 1;
    }else if(!qstricmp(szAction,"if_more_retest_part")) {
        if(!qstricmp(szValue,"cancel")){
            ZTHROWEXC("Merge Canceled");
            return 0;
        }
        staticMapMergeOptions["if_more_retest_part"] = QString(szValue);
        return 1;

    }
    else if(!qstricmp(szAction,"merge"))
    {
        GexTbMergeRetest lMergeTool(false);
        QStringList strTest = staticMapMergeOptions["add_test_file"].split(";",QString::SkipEmptyParts);
        QStringList strReTest = staticMapMergeOptions["add_retest_file"].split(";",QString::SkipEmptyParts);
        int iRet = lMergeTool.merge(strTest, strReTest, staticMapMergeOptions["out"], (staticMapMergeOptions["sort"].toInt() == 1), staticMapMergeOptions["if_more_retest_part"]);
        staticMapMergeOptions.clear();
        if(iRet == GexTbMergeRetest::NoError)
            return 1;
        else
        {
            ZTHROWEXC(ZString(lMergeTool.GetErrorMessage().toLatin1().constData()));
            return 0;
        }
    }

    ZTHROWEXC("invalid action :"+ZString(szAction));
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Function gexCondition: add condition to group
////////////////////////////////////////////////////////////////////////////////
static ZString  gexCondition(ZCsl *csl)
{
    //gexCondition(group_id,<condition_name>,<condition_value>)
    long lGroupId = csl->get("group_id").asLong();
    QString strCondition = csl->get("condition_name").constBuffer();
    QString strValue = csl->get("condition_value").constBuffer();

    if((lGroupId < 0) || (lGroupId >= gexReport->getGroupsList().size()))
        return 0;

    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().at(lGroupId);
    pGroup->AddTestConditions(strCondition,strValue);

    return 0;
}


void ZCslInitLib_Gex(ZCsl* csl)
{
   ZString iFile("GexLib");
   std::istringstream init("const myVersion = 3.00;\n");
   csl->loadScript(iFile, &init);
   (*csl)
      .addFunc(
          iFile,
         "gexBuildReport([const show_page,const sections_ignored])",
          gexBuildReport)
      .addFunc(
          iFile,
         "gexConvertSTDF(const input_file, [const output_file])",
          gexConvertSTDF)
      .addFunc(
          iFile,
         "gexConvertCSV(const input_file, [const output_file])",
          gexConvertCSV)
      .addFunc(
          iFile,
         "gexWeb(const action, [const field, const value])",
          gexWeb)
      .addFunc(
          iFile,
         "gexParameter(const parameter, const name, const field, const value,const label,[const not_used])",
          gexParameter)
      .addFunc(
          iFile,
         "gexParameterFile(const parameter)",
          gexParameterFile)
      .addFunc(
          iFile,
         "gexToolbox(const action, const field, [const value, const optional])",
          gexToolbox)
      .addFunc(iFile, "gexAnalyseMode(const analyse_mode)",gexAnalyseMode)
      .addFunc(
          iFile,
         "gexFavoriteScripts(const action, [const path, const title])",
          gexFavoriteScripts)
      .addFunc(
          iFile,
         "gexFile(const group_id, const action, const file, const site, const process, [const range, const maptests, const extractwafer, const temperature, const datasetname, const samplegroup])",
          gexFile)
      .addFunc(
          iFile,
         "gexFileInfo(const group_id, const file_id, const field)",
          gexFileInfo)
      .addFunc(
          iFile,
         "gexGroup(const action, const value, [const group_id])",
          gexGroup)
      .addFunc(
          iFile,
         "gexOptions(const section, const field, const value, [const setvalue])",
          gexOptions)
      .addFunc(
          iFile,
         "gexInteractive(const section, [const field, const value])",
          gexInteractive)
      .addFunc(
          iFile,
         "gexQuery(const section, const field, [const value])",
          gexQuery)
      .addFunc(
          iFile,
         "gexReportType(const section, [const type, const value, const customvalue])",
          gexReportType)
      .addFunc( iFile, "gexCslVersion(const version)", gexCslVersion)
      .addFunc( iFile, "gexGetEnv(const env)", gexGetEnv)
      .addFunc( iFile, "gexResetOptions(const param)", gexResetOptions)
      .addFunc(
          iFile,
          "gexReport(const section, const field, const value)",
          gexReportProperties)
      .addFunc(
          iFile,
          "gexChartStyle(const field, const value)",
          gexChartStyle)
      .addFunc(
          iFile,
          "gexBinStyle(const field, [const value])",
          gexBinStyle)
      .addFunc(
          iFile,
          "gexSiteStyle(const field, [const value])",
          gexSiteStyle)
      .addFunc(
          iFile,
          "gexGenerateDynamicReport(const json_file_name, const report_file_name, [const report_format])",
          gexGenerateDynamicReport);

   (*csl)
      .addFunc(
          iFile,
          "gexGenCSVFromQuery(const value)",
          gexGenCSVFromQuery)
      .addFunc(
          iFile,
          "gexMerge(const action, [const value])",
          gexMerge)
      .addFunc(iFile,
          "gexCondition(const group_id, const condition_name, const condition_value )",
          gexCondition);

   // Erase previous report structure if any. Clean start!
    if(gexReport != NULL)
        delete gexReport;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new GexReport ").toLatin1().constData());
    gexReport = new CGexReport;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("setReportOptions(NULL) ").toLatin1().constData());
    gexReport->setReportOptions(NULL);	// Will remain NULL unless a report is created...

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new CGexSingleChart ").toLatin1().constData());
    s_pChartStyle = new CGexSingleChart;

} // ZCslInitLib

void ZCslCleanupLib_Gex(ZCsl* /*csl*/)
{
   // cleanup codes.
    if(ptStdfToAscii !=  NULL)
    {
        delete ptStdfToAscii;
        ptStdfToAscii=NULL;
    }

    if (s_pChartStyle)
    {
        delete s_pChartStyle;
        s_pChartStyle = NULL;
    }
    if (gexReport != NULL)
    {
        // delete gexReport;
        // gexReport = NULL;
    }
} // ZCslCleanupLib
