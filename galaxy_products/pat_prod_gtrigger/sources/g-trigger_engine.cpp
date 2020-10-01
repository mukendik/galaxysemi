// TODO SPEKTRA FLOW
// 1. Check GTF generation code (DONE)
// 2. Replication (DONE)
// 3. Check summary log file generation (DONE)

#include <QString>
#include <QFileInfo>
#include <QDir>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Galaxy modules includes
#include <gstdl_utils_c.h>
#include <gqtl_log.h>
#include <pat_recipe.h>
#include <pat_options.h>

// includes from GEX
#include <converter_external_file.h>

// Local includes
#include "g-trigger_engine.h"
#include "profile.h"    // Handles .INI file (settings/options)
#include "cstdf.h"


#include <QThread>
#include <QSharedPointer>

// Regular expressions for log filename parsing
// Caption 1 is WaferID
// Caption 2 is Date
// FETTEST.
// Example: X32606.5-09.dat_2008Apr09.110225_gexmo.log
#define FETTEST_LOG_REGEXP                                    "[^\\.]+\\.\\d+-(\\d+)\\.dat_([^\\.]+\\.\\d+)_gexmo\\.log"
#define FETTEST_LOG_REGEXP_WITHOUT_SPLITLOT                   "-(\\d+)\\.dat_([^\\.]+\\.\\d+)_gexmo\\.log"
// EAGLE.
// Example: Y03K123_1_11_P_ETS170758_04022008.log
#define EAGLE_LOG_REGEXP                                      "[^_]+_\\d+_(\\d+)_._ETS(\\d+_\\d+)\\.log"
#define EAGLE_LOG_REGEXP_WITHOUT_SPLITLOT                     "_(\\d+)_._ETS(\\d+_\\d+)\\.log"
// Example: D33H005_6_21_P_ETS211328_11112013.std
#define EAGLE_STDF_REGEXP                                     "(.*_ETS)(\\d+_\\d+)\\.std.*"
// SPEKTRA.
// Example: B15K209.1_W01.Dta.CSV_2012Feb15.015902_gexmo.log
#define SPEKTRA_LOG_REGEXP                                    "[^\\.]+\\.\\d+_w(\\d+)\\.dta.csv_([^\\.]+\\.\\d+)_gexmo\\.log"
#define SPEKTRA_LOG_REGEXP_WITHOUT_SPLITLOT                   "_w(\\d+)\\.dta.csv_([^\\.]+\\.\\d+)_gexmo\\.log"

namespace GS
{
namespace GTrigger
{

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GTriggerEngine::GTriggerEngine(const QString &AppName, const QString &AppPath)
    : mAppName(AppName),
      mAppPath(AppPath),
      mTimer(this),
      mQA(false)
{
    // Init variables
    mFirstCall = true;
    mSettingsModified = true;
    mDeleteIncompleteWafers = false;
    mAddSiteLocationToInfOutputDir = false;
    mLogLevel = 5;
    mMergeTestRetest = false;
}

bool GTriggerEngine::OnStart()
{
    // Startup status
    LogMessage(QString("Starting %1 engine thread (%2)").arg(mAppName).arg((intptr_t)QThread::currentThreadId()));
    LogMessage(QString("Application path is %1").arg(mAppPath));

    // Set timer to check for .PRN summary file periodically
    connect(&mTimer, SIGNAL(timeout()),this, SLOT(OnTimerEvent()));

    // Load settings from INI file.
    LoadINI();

    // Startup status
    LogMessage(QString("Checking for summary files every 2 seconds (Fettest summary .PRN, Eagle summary STDF, or Spektra summary .CNT.CSV)"));

    // Start checking for .PRN incoming file in 2 seconds.
    OnResetTimer();

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys the object and frees any allocated resources
/////////////////////////////////////////////////////////////////////////////////////
GTriggerEngine::~GTriggerEngine()
{
    // Stopping status
    LogMessage(QString("Stopping %1 engine thread (%2)").arg(mAppName).arg((intptr_t)QThread::currentThreadId()));
}

///////////////////////////////////////////////////////////
// Sets variable with value from INI, and checks if value
// changed from previous read
///////////////////////////////////////////////////////////
void GTriggerEngine::SetVariable(QString & strVariable, const QString & strValue)
{
    if(strVariable!=strValue)
        mSettingsModified = true;
    strVariable = strValue.trimmed();
}

///////////////////////////////////////////////////////////
// Sets variable with value from INI, and checks if value
// changed from previous read
///////////////////////////////////////////////////////////
void GTriggerEngine::SetVariable(int & nVariable, const int & nValue)
{
    if(nVariable!=nValue)
        mSettingsModified = true;
    nVariable = nValue;
}

///////////////////////////////////////////////////////////
// Sets variable with value from INI, and checks if value
// changed from previous read
///////////////////////////////////////////////////////////
void GTriggerEngine::SetVariable(float & fVariable, const float & fValue)
{
    if(fVariable!=fValue)
        mSettingsModified = true;
    fVariable = fValue;
}

///////////////////////////////////////////////////////////
// Sets variable with value from INI, and checks if value
// changed from previous read
///////////////////////////////////////////////////////////
void GTriggerEngine::SetVariable(bool & bVariable, const bool & bValue)
{
    if(bVariable!=bValue)
        mSettingsModified = true;
    bVariable = bValue;
}

///////////////////////////////////////////////////////////
// Reload INI file
///////////////////////////////////////////////////////////
void GTriggerEngine::LoadINI(void)
{
    char    szString[2048];
    QString strIniFile, strString;
    int     nValue;
    float   fValue;
    bool    bValue;

    // Build ini file name
    strIniFile += "g-trigger.ini";
    if(QFile::exists(strIniFile) == false)
    {
        // Fails finding .ini file; try hard-coded path...
        strIniFile = "C:\\Program Files\\Galaxy PAT-Server\\g-trigger.ini";
    }


    if(mFirstCall)
    {
        strString = "Loading .INI: " + strIniFile;
        LogMessage(strString);
    }

    // Load default fields
    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Input_PRN").toLatin1().data(),QString("c:/vishay").toLatin1().data(),szString,1024,strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mPrnFolder, strString);

    // FetTest HVM .prn files...
    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Input_PRN_HVM").toLatin1().data(),QString("c:/vishay").toLatin1().data(),szString,1024,strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mPrnFolderHVM, strString);

    // FetTest NO-PAT summary files...
    get_private_profile_string(QString("Folders").toLatin1().data(),
                               QString("Input_PRN_NO_PAT").toLatin1().data(),
                               QString("c:/vishay").toLatin1().data(),
                               szString,1024,(char*)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mPrnFolderNoPat, strString);

    // FetTest HVM NO-PAT summary files
    get_private_profile_string(QString("Folders").toLatin1().data(),
                               QString("Input_PRN_NO_PAT_HVM").toLatin1().data(),
                               QString("c:/vishay").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mPrnFolderNoPatHVM, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Input_Retest_PRN").toLatin1().data(),QString("c:/vishay/pat_retest").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mPrnRetestFolder, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Input_STDF").toLatin1().data(),QString("c:/vishay").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfInFolder, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Output_PAT_STDF").toLatin1().data(),QString("c:/vishay/output").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mOutFolderStdf, strString);

    get_private_profile_string(QString("Eagle_Folders").toLatin1().data(),QString("Input_Eagle_SUM").toLatin1().data(),QString("c:/vishay/eagle").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfEagleSumFolder, strString);

    get_private_profile_string(QString("Eagle_Folders").toLatin1().data(),QString("Input_Eagle_STDF").toLatin1().data(),QString("c:/vishay/eagle").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfEagleInFolder, strString);

    get_private_profile_string(QString("Eagle_Folders").toLatin1().data(),QString("Output_Eagle_PAT_STDF").toLatin1().data(),QString("c:/vishay/output").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfEagleOutFolderPat, strString);

    get_private_profile_string(QString("Spektra_Folders").toLatin1().data(),QString("Input_Spektra_CNT").toLatin1().data(),QString("c:/vishay/spektra").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfSpektraCntFolder, strString);

    get_private_profile_string(QString("Spektra_Folders").toLatin1().data(),QString("Input_Spektra_STDF").toLatin1().data(),QString("c:/vishay/spektra").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfSpektraInFolder, strString);

    get_private_profile_string(QString("Spektra_Folders").toLatin1().data(),QString("Output_Spektra_PAT_STDF").toLatin1().data(),QString("c:/vishay/output").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mStdfSpektraOutFolderPat, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Output_PAT_ENG_STDF").toLatin1().data(),QString("c:/vishay/output").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mOutFolderEngStdf, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Output_PAT_Log").toLatin1().data(),QString("c:/vishay/output/log").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mOutFolderLog, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Output_PAT_INF").toLatin1().data(),QString("c:/vishay/output/inf").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mOutFolderInf, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Recipes_PAT").toLatin1().data(),QString("c:/vishay/recipe").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mRecipeFolder, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),
                               QString("Log").toLatin1().data(),QString("").toLatin1().data(),szString,
                               1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    SetVariable(mLog, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Production").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    if(strString.isEmpty())
        strString = mOutFolderStdf; // If no production folder specified, write summary.log into output folder home page.
    SetVariable(mProduction, strString);

    get_private_profile_string(QString("Folders").toLatin1().data(),QString("Inspection_Maps_Folder").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.endsWith("/") || strString.endsWith("\\"))
        strString.truncate(strString.length()-1);   // Remove leading '/' or '\'
    if(!strString.isEmpty())
        SetVariable(mInspectionMapsFolder, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Shell").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mShell, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Shell_Fet_NoPAT").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mShell_Fet_NoPAT, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Shell_Fet_NoPAT_HVM").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mShell_Fet_NoPAT_HVM, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Shell_HVM").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mShell_HVM, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Output_Pat_Composite_Maps_Subfolder").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mOutput_Pat_Composite_Maps_Subfolder, strString);

    get_private_profile_string(QString("Options").toLatin1().data(),QString("Shell_Post_Composite").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mShell_Post_Composite, strString);

    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("Delay_Fet_NoPAT_LogFiles").toLatin1().data(),1,(char *)strIniFile.toLatin1().data());
    SetVariable(mDelay_Fet_NoPAT_LogFiles, nValue);

    // Delete .PRN when processed?
    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("DeletePRN").toLatin1().data(),0,(char *)strIniFile.toLatin1().data());
    SetVariable(mDeletePrn, nValue);

    // Do not process .PRN too recent (eg < 180 sec)
    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("DelayPRN").toLatin1().data(),180,(char *)strIniFile.toLatin1().data());
    SetVariable(mPrnDelay, nValue);

    // Do not generate summary.log too soon(eg wait 180 sec)
    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("DelaySummary").toLatin1().data(),180,(char *)strIniFile.toLatin1().data());
    SetVariable(mSummaryDelay, nValue);

    // Update TimeStamp info in PAT-Man STDF file created?
    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("UpdateTimeFields").toLatin1().data(),1,(char *)strIniFile.toLatin1().data());
    SetVariable(mUpdateTimeFields, nValue);

    // Get Gross die ratio that should be used to consider a wafer is complete
    fValue = 1.0;
    get_private_profile_string(QString("Options").toLatin1().data(),QString("ValidWaferThreshold_GrossDieRatio").toLatin1().data(),QString("1.0").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    if(sscanf(szString, "%g", &fValue) != 1)
        fValue = 1.0;
    SetVariable(mValidWaferThreshold_GrossDieRatio, fValue);

    // Get flag specifying if incomplete wafers should be removed
    bValue = false;
    get_private_profile_string(QString("Options").toLatin1().data(),QString("DeleteIncompleteWafers").toLatin1().data(),QString("0").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.trimmed() == "1")
        bValue = true;
    SetVariable(mDeleteIncompleteWafers, bValue);

    // Add Site-Location to INF output path?
    bValue = false;
    get_private_profile_string(QString("Options").toLatin1().data(),QString("AppendSiteLocationToInfOutpuPath").toLatin1().data(),QString("0").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.trimmed() == "1")
        bValue = true;
    SetVariable(mAddSiteLocationToInfOutputDir, bValue);

    // Subfolder for delayed files
    get_private_profile_string(QString("Options").toLatin1().data(),QString("Delayed_Files_Subfolder").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mDelayed_Files_Subfolder, strString);

    // Subfolder for rejected files
    get_private_profile_string(QString("Options").toLatin1().data(),QString("Rejected_Files_Subfolder").toLatin1().data(),QString("").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    SetVariable(mRejected_Files_Subfolder, strString);

    // Log level
    nValue = get_private_profile_int(QString("Options").toLatin1().data(),QString("LogLevel").toLatin1().data(),5,(char *)strIniFile.toLatin1().data());
    SetVariable(mLogLevel, nValue);

    // MapMergeScript
    get_private_profile_string(const_cast<char*>("Files"),
                               const_cast<char*>("MapMergeScript"),
                               const_cast<char*>(""),
                               szString, 1024,
                               strIniFile.toUtf8().data());
    strString = szString;
    SetVariable(mMapMergeScript, strString);

    // OutputTriggerFileFormat
    get_private_profile_string(const_cast<char*>("Options"),
                               const_cast<char*>("OutputTriggerFileFormat"),
                               const_cast<char*>("js"),
                               szString, 1024,
                               strIniFile.toUtf8().data());
    if (::strcmp(szString, "gtf") == 0)
    {
        this->SetVariable(mOutputTriggerFileFormat,
                          static_cast<int>(gtfFormat));
    }
    else
    {
        this->SetVariable(mOutputTriggerFileFormat,
                          static_cast<int>(javascriptFormat));
    }

    // Merge test/retest wafers?
    bValue = false;
    get_private_profile_string(QString("Options").toLatin1().data(),QString("MergeTestRetest").toLatin1().data(),QString("0").toLatin1().data(),szString,1024,(char *)strIniFile.toLatin1().data());
    strString = szString;
    if(strString.trimmed() == "1")
        bValue = true;
    SetVariable(mMergeTestRetest, bValue);

    // Startup status
    if(mFirstCall || mSettingsModified)
    {
        // Change the log level
        if((mLogLevel>=0) && (mLogLevel<=7))
        {
            strString = QString("GTRIGGER_LOGLEVEL is %1").arg(mLogLevel);
            //GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().constData());
            LogMessage(strString);
        }

        if(!mFirstCall)
        {
            strString = "Modified .INI file re-loaded: " + strIniFile;
            LogMessage(strString);
        }

        // QT3 -> QT4: cleasr ListView
        // CHECKME
        //while(listBox_Settings->count() > 0)
        //    delete listBox_Settings->takeItem(0);

        InsertSettingsLog("######## Environment variables");
        // Check if Production PROMIS file available
        char *ptChar = getenv("GEX_PROMIS_DATA_PATH");
        if(ptChar != NULL)
        {
            if(QFile::exists(ptChar) == false)
            {
                strString = QString("GEX_PROMIS_DATA_PATH environment variable pointing to missing file (%1)")
                        .arg(QString(ptChar));
                LogMessage(strString, eWarning);
                InsertSettingsLog(QString("**WARNING** : %1").arg(strString));
            }
            else
            {
                strString = "GEX_PROMIS_DATA_PATH = ";
                strString += ptChar;
                InsertSettingsLog(strString);
            }
        }

        // Check if BINMAP file available
        ptChar = getenv("GEX_FETTEST_BINMAP_FILE");
        if(ptChar != NULL)
        {
            if(QFile::exists(ptChar) == false)
            {
                strString = QString("GEX_FETTEST_BINMAP_FILE environment variable pointing to missing file (%1)")
                        .arg(QString(ptChar));
                LogMessage(strString, eWarning);
                InsertSettingsLog(QString("**WARNING** : %1").arg(strString));
            }
            else
            {
                strString = "GEX_FETTEST_BINMAP_FILE = ";
                strString += ptChar;
                InsertSettingsLog(strString);
            }
        }

        // If Engineering PROMIS file exists, tell it.
        ptChar = getenv("GEX_PROMIS_ENG_DATA_PATH");
        if(ptChar != NULL)
        {
            if(QFile::exists(ptChar) == false)
            {
                strString = QString("GEX_PROMIS_ENG_DATA_PATH environment variable pointing to missing file (%1)")
                        .arg(QString(ptChar));
                LogMessage(strString, eWarning);
                InsertSettingsLog(QString("**WARNING** : %1").arg(strString));
            }
            else
            {
                strString = "GEX_PROMIS_ENG_DATA_PATH = ";
                strString += ptChar;
                InsertSettingsLog(strString);
            }
        }

        InsertSettingsLog("");
        strString = "######## INI settings from " + strIniFile;
        InsertSettingsLog(strString);
        InsertSettingsLog("---- FETTEST FOLDERS ----");
        strString = "Input_PRN = " + mPrnFolder;
        InsertSettingsLog(strString);
        strString = "Input_PRN_HVM = " + mPrnFolderHVM;
        InsertSettingsLog(strString);
        // Make sure converter_external_file.xml file exists in HVM .prn spooling folder
        if(!QFile::exists(QDir::toNativeSeparators(mPrnFolderHVM + "/converter_external_file.xml")))
        {
            strString = QString("%1 is missing!")
                    .arg(QString(QDir::toNativeSeparators(mPrnFolderHVM + "/converter_external_file.xml")));
            LogMessage(strString, eWarning);
        }
        strString = "Input_PRN_NO_PAT = " + mPrnFolderNoPat;
        InsertSettingsLog(strString);
        strString = "Input_PRN_NO_PAT_HVM = " + mPrnFolderNoPatHVM;
        InsertSettingsLog(strString);
        strString = "Input_Retest_PRN = " + mPrnRetestFolder;
        InsertSettingsLog(strString);
        strString = "Input_STDF = " + mStdfInFolder;
        InsertSettingsLog(strString);
        strString = "Output_PAT_STDF = " + mOutFolderStdf;
        InsertSettingsLog(strString);
        strString = "Output_PAT_ENG_STDF = " + mOutFolderEngStdf;
        InsertSettingsLog(strString);

        InsertSettingsLog("");
        InsertSettingsLog("---- EAGLE FOLDERS ----");
        strString = "Input_Eagle_SUM = " + mStdfEagleSumFolder;
        InsertSettingsLog(strString);
        strString = "Input_Eagle_STDF = " + mStdfEagleInFolder;
        InsertSettingsLog(strString);
        strString = "Output_Eagle_PAT_STDF = " + mStdfEagleOutFolderPat;
        InsertSettingsLog(strString);

        InsertSettingsLog("");
        InsertSettingsLog("---- SPEKTRA FOLDERS ----");
        strString = "Input_Spektra_CNT = " + mStdfSpektraCntFolder;
        InsertSettingsLog(strString);
        strString = "Input_Spektra_STDF = " + mStdfSpektraInFolder;
        InsertSettingsLog(strString);
        strString = "Output_Spektra_PAT_STDF = " + mStdfSpektraOutFolderPat;
        InsertSettingsLog(strString);

        InsertSettingsLog("");
        InsertSettingsLog("---- COMMON FOLDERS ----");
        strString = "Recipes_PAT = " + mRecipeFolder;
        InsertSettingsLog(strString);
        strString = "Output_PAT_Log = " + mOutFolderLog;
        InsertSettingsLog(strString);
        strString = "Output_PAT_INF = " + mOutFolderInf;
        if(mAddSiteLocationToInfOutputDir)
            strString += "/<Site-Location>";
        InsertSettingsLog(strString);
        strString = "Log = " + mLog;
        InsertSettingsLog(strString);
        strString = "Production = " + mProduction;
        InsertSettingsLog(strString);
        strString = "Inspection_Maps_Folder = " + mInspectionMapsFolder;
        InsertSettingsLog(strString);

        InsertSettingsLog("");
        InsertSettingsLog("---- OPTIONS ----");
        strString = "DeletePRN= " + QString::number(mDeletePrn);
        InsertSettingsLog(strString);
        strString = "DelayPRN= " + QString::number(mPrnDelay);
        InsertSettingsLog(strString);
        strString = "DelaySummary= " + QString::number(mSummaryDelay);
        InsertSettingsLog(strString);
        strString = "UpdateTimeFields= " + QString::number(mUpdateTimeFields);
        InsertSettingsLog(strString);
        strString = "ValidWaferThreshold_GrossDieRatio= " + QString::number(mValidWaferThreshold_GrossDieRatio);
        InsertSettingsLog(strString);
        strString = "DeleteIncompleteWafers= ";
        if(mDeleteIncompleteWafers)
            strString += "1";
        else
            strString += "0";
        InsertSettingsLog(strString);
        strString = "AppendSiteLocationToInfOutpuPath= ";
        if(mAddSiteLocationToInfOutputDir)
            strString += "1";
        else
            strString += "0";
        InsertSettingsLog(strString);
        strString = "Shell= " + mShell;
        InsertSettingsLog(strString);
        strString = "Shell_Fet_NoPAT= " + mShell_Fet_NoPAT;
        InsertSettingsLog(strString);
        strString = "Shell_Post_Composite= " + mShell_Post_Composite;
        InsertSettingsLog(strString);
        strString = "Shell_Fet_NoPAT_HVM= " + mShell_Fet_NoPAT_HVM;
        InsertSettingsLog(strString);
        strString = "Shell_HVM= " + mShell_HVM;
        InsertSettingsLog(strString);
        strString = "Output_Pat_Composite_Maps_Subfolder= " + mOutput_Pat_Composite_Maps_Subfolder;
        InsertSettingsLog(strString);
        strString = "Delay_Fet_NoPAT_LogFiles= " + QString::number(mDelay_Fet_NoPAT_LogFiles);
        InsertSettingsLog(strString);
        strString = "Delayed_Files_Subfolder= " + mDelayed_Files_Subfolder;
        InsertSettingsLog(strString);
        strString = "Rejected_Files_Subfolder= " + mRejected_Files_Subfolder;
        InsertSettingsLog(strString);
        strString = QString("LogLevel= %1").arg(mLogLevel);
        InsertSettingsLog(strString);
        strString = "MergeTestRetest= ";
        if(mMergeTestRetest)
            strString += "1";
        else
            strString += "0";
        InsertSettingsLog(strString);
        mFirstCall = false;
        mSettingsModified = false;
    }
}

///////////////////////////////////////////////////////////
// Reset timer
///////////////////////////////////////////////////////////
void GTriggerEngine::OnResetTimer(bool bLaunchNow/*=false*/)
{
    // Reset timer (immediate or 2 seconds single-shot timer)
    mTimer.setSingleShot(true);
    if(bLaunchNow)
        mTimer.start();
    else
        mTimer.start(2000);
}

///////////////////////////////////////////////////////////
// Error handling: write to log file.
///////////////////////////////////////////////////////////
void GTriggerEngine::LogMessage(const QString &Message,LogMsgType MsgType/*=eInfo*/)
{
    // Get current time/date
    QDateTime lTime = QDateTime::currentDateTime();

    // Build message
    QString lTextMessage = lTime.toString("d MMM yyyy h:mm:ss ap: ");
    switch(MsgType)
    {
    default:
    case eInfo:
        lTextMessage += "[Info] " + Message;
        break;
    case eWarning:
        lTextMessage += "[**Warning**] " + Message;
        break;
    case eError:
        lTextMessage += "[**Error**] " + Message;
        break;
    }

    // Write to log file if Log path defined.
    if(!mLog.isEmpty())
    {
        // Build path to log file
        QFile   fLog(QString("%1/g-trigger-%2.log").arg(mLog).arg(lTime.toString("yyyy.MM.dd")));
        if(fLog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
        {
            QTextStream hLogFile(&fLog);
            hLogFile << "\t" << lTextMessage << endl;
            fLog.close();
        }
    }

    // Also insert error into GUI.
    InsertStatusLog(lTextMessage, false);
}

///////////////////////////////////////////////////////////
// DEBUG: write to log file.
///////////////////////////////////////////////////////////
void GTriggerEngine::LogMessage_Debug(const QString &strErrorMessage)
{
    // If no Log path defined, or log level < 7, quietly exit.
    if(mLog.isEmpty() || (mLogLevel < 7))
        return;

    QString strTextMessage;
    QDateTime cTime = QDateTime::currentDateTime();

    // Build message
    strTextMessage = cTime.toString("d MMM yyyy h:mm:ss ap: ");
    strTextMessage += "[**DEBUG**] " + strErrorMessage;

    // Build path to log file
    QString strLogFile = mLog + "/" + "g-trigger-" + cTime.toString("yyyy.MM.dd") + "_debug.log";

    QFile   fLog(strLogFile);
    if(fLog.open(QIODevice::WriteOnly|QIODevice::Append|QIODevice::Text))
    {
        QTextStream hLogFile(&fLog);
        hLogFile << "\t" << strTextMessage << endl;
        fLog.close();
    }
}

///////////////////////////////////////////////////////////
// Insert line into status log.
///////////////////////////////////////////////////////////
void GTriggerEngine::InsertStatusLog(const QString & strItem, bool bAddTimestamp/*=true*/)
{
    QDateTime lTime = QDateTime::currentDateTime();

    QString lTextMessage;
    if(bAddTimestamp)
        lTextMessage = lTime.toString("d MMM yyyy h:mm ap: ");
    lTextMessage += strItem;

    // emit signal for GUI
    emit sStatusLog(lTextMessage);
}

///////////////////////////////////////////////////////////
// Insert line into settings log.
///////////////////////////////////////////////////////////
void GTriggerEngine::InsertSettingsLog(const QString & strItem)
{
    // emit signal for GUI
    emit sSettingsLog(strItem);
}

///////////////////////////////////////////////////////////
// Check for Summary file ready for processing...
// Return 'false' if none available (nothing to process)
///////////////////////////////////////////////////////////
bool GTriggerEngine::CheckForSummaryFileAvailable(QString & strSummaryFolder, const char *szFilter)
{
    QDir                  cDir;
    QStringList::Iterator   itFile;
    QString               strSummaryFile;

    QString     strFilePath, strTextMessage;
    QFileInfo   cFileInfo;
    QDateTime   cDataTime;
    int         iDeltaSec;

    // Set summary folder
    mSummaryList.m_strSummaryFolder = strSummaryFolder;

    // Get list of files to comvert
    cDir.setPath(strSummaryFolder);

    LogMessage_Debug( QString("Checking folder for summary files in %1 with filter of %2").arg(strSummaryFolder).arg(szFilter) ) ;

    // Retrieve list of s files found in folder
    QStringList strlFiles;
    if (mQA)
    {
        strlFiles = cDir.entryList(QStringList(QString(szFilter)),
                                   QDir::Files, QDir::Name);
    }
    else
    {
        strlFiles = cDir.entryList(QStringList(QString(szFilter)),
                                   QDir::Files, QDir::Time | QDir::Reversed);
    }

    // Do not keep files too recent (15 secs old or less)
    for(itFile = strlFiles.begin(); itFile != strlFiles.end(); itFile++)
    {
        // Get summary file in list
        strSummaryFile = *itFile;
        strFilePath = strSummaryFolder + "/" + strSummaryFile;
        cFileInfo.setFile(strFilePath);

        LogMessage_Debug(QString("Checking file -- %1").arg(strSummaryFile)) ;

        // Check when last modified
        // SJM -- seems like we SHOULD use lastModified() not created .. this could be bad .. or both ... ??
        cDataTime = cFileInfo.created();

        // Old enough?
        iDeltaSec = cDataTime.secsTo(QDateTime::currentDateTime());
        if(iDeltaSec >= mPrnDelay)
        {
            strSummaryFile = strSummaryFolder + "|" + *itFile;
            if(mSummaryList.indexOf(strSummaryFile) == -1)
            {
                LogMessage_Debug(QString("File added to list to be process")) ;
                mSummaryList.append(strSummaryFile);
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////
// Check if summary .log files to create
///////////////////////////////////////////////////////////
void GTriggerEngine::CheckSummaryLogsCreation(QString &strFolder)
{
    QDir                    cDir;
    QStringList::Iterator   it;

    LogMessage_Debug(QString("Checking Summary Log Creation in folder %1").arg(strFolder));

    // Retrieve list of .log files found in folder
    cDir.setPath(strFolder);
    cDir.setFilter(QDir::Files);
    QStringList strLogList = cDir.entryList(QStringList(QString("*.log")));

    // If no .log file, nothing to do!
    if(strLogList.count() <= 0)
        return;

    // Extract list of lots for this product
    QString     strPattern, strRegExp, strLotID, strSplitNb, strSplitLotID, strTextMessage;
    QStringList strLotsChecked;
    QRegExp     clRegExp;

    clRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    for (it = strLogList.begin(); it != strLogList.end(); ++it )
    {
        // LOG
        strTextMessage = QString("Check summary log creation for %1").arg(*it);
        //GEXLOG(7, strTextMessage.toLatin1().data());
        LogMessage_Debug(strTextMessage);

        strSplitLotID = "";

        // Check if log file matching FetTest naming convention
        // Example: X32606.5-09.dat_2008Apr09.110225_gexmo.log
        clRegExp.setPattern(FETTEST_LOG_REGEXP);
        if(clRegExp.indexIn(*it) != -1)
        {
            // Compute SplitlotID (ie X32606.5) , pattern (ie X32606.5-*.log) and regExp (ie X32606\.5-\d+\.dat_[^\.]+\.\d+_gexmo.log)
            strSplitLotID = (*it).section('-',0,0);
            strLotID = strSplitLotID.section('.',0,0);
            strSplitNb = strSplitLotID.section('.',1,1);

            strPattern = strSplitLotID + "-*.log";
            strRegExp = strLotID + "\\.";
            strRegExp += strSplitNb + FETTEST_LOG_REGEXP_WITHOUT_SPLITLOT;

            // Build .log summary file name (unless already up-to-date)
            if(!strSplitLotID.isEmpty() && (strLotsChecked.indexOf(strSplitLotID) == -1))
            {
                CreateSummaryLog(strFolder, strPattern, strRegExp, strLotID, strSplitLotID, eFetTest);

                // Ensure we do not check this lot# twice during this call
                strLotsChecked.append(strSplitLotID);
            }
        }

        // Check if log file matching Eagle naming convention
        // Example: Y03K123_1_11_P_ETS170758_04022008.log
        clRegExp.setPattern(EAGLE_LOG_REGEXP);
        if(clRegExp.indexIn(*it) != -1)
        {
            // Compute SplitlotID (ie Y03K123.1) , pattern (ie Y03K123_1-*.log) and regExp (ie Y03K123_1_\d+_._ETS\d+_\d+.log)
            strPattern = (*it).section('_',0,1);
            strSplitLotID = strPattern;
            strSplitLotID.replace('_', '.');
            strLotID = strSplitLotID.section('.',0,0);
            strSplitNb = strSplitLotID.section('.',1,1);

            strRegExp = strPattern + EAGLE_LOG_REGEXP_WITHOUT_SPLITLOT;
            strPattern += "_*.log";

            // Build .log summary file name (unless already up-to-date)
            if(!strSplitLotID.isEmpty() && (strLotsChecked.indexOf(strSplitLotID) == -1))
            {
                CreateSummaryLog(strFolder, strPattern, strRegExp, strLotID, strSplitLotID, eEagle);

                // Ensure we do not check this lot# twice during this call
                strLotsChecked.append(strSplitLotID);
            }
        }

        // Check if log file matching Spektra naming convention
        // Example: B15K209.1_W01.Dta.CSV_2012Feb15.015902_gexmo.log
        clRegExp.setPattern(SPEKTRA_LOG_REGEXP);
        if(clRegExp.indexIn(*it) != -1)
        {
            // Compute SplitlotID (ie B15K209.1) , pattern (ie B15K209.1-*.log) and regExp (ie B15K209\.1_\d+\.dta.csv_[^\.]+\.\d+_gexmo.log)
            strSplitLotID = (*it).section('_',0,0);
            strLotID = strSplitLotID.section('.',0,0);
            strSplitNb = strSplitLotID.section('.',1,1);

            strPattern = strSplitLotID + "_*.log";
            strRegExp = strLotID + "\\.";
            strRegExp += strSplitNb + SPEKTRA_LOG_REGEXP_WITHOUT_SPLITLOT;

            // Build .log summary file name (unless already up-to-date)
            if(!strSplitLotID.isEmpty() && (strLotsChecked.indexOf(strSplitLotID) == -1))
            {
                CreateSummaryLog(strFolder, strPattern, strRegExp, strLotID, strSplitLotID, eSpektra);

                // Ensure we do not check this lot# twice during this call
                strLotsChecked.append(strSplitLotID);
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Extract date/time from Eagle log file into DateTime object
// Example of log file: Y03K123_1_11_P_ETS170758_04022008.log
// Example of summary file: D20D422_1_LotSummary_ETS193746_07152013.std
///////////////////////////////////////////////////////////
void GTriggerEngine::extractDateTime_Eagle(QDateTime &cDateTime,const QString &FileName, const QString &RegExp)
{
    // LOG
    QString strTextMessage = QString("Extract date/time from Eagle file %1").arg(FileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    QRegExp clRegExp(RegExp, Qt::CaseInsensitive);
    if(clRegExp.indexIn(FileName) == -1)
    {
        // Failed parsing date....should never occur
        cDateTime = QDateTime::currentDateTime();
        return;
    }

    // Parse text: eg: 170758_0402200 (format is hhmmss_MMddyyyy)
    QString strDateTime = clRegExp.cap(2);
    int     iYear,iMonth,iDay,iHour,iMin,iSec;
    char    cChar;
    // GCORE-4754 : File format for ETS was changed to YYYYMMDD_HHMMSS
    if(sscanf(strDateTime.toLatin1().constData(),"%4d%2d%2d%c%2d%2d%2d", &iYear, &iMonth, &iDay, &cChar, &iHour, &iMin, &iSec) != 7)
    {
        // Failed parsing date....should never occur
        cDateTime = QDateTime::currentDateTime();
        return;
    }

    QDate cDate(iYear,iMonth,iDay);
    QTime cTime(iHour,iMin,iSec);
    cDateTime.setDate(cDate);
    cDateTime.setTime(cTime);
}

///////////////////////////////////////////////////////////
// Extract date/time from Eagle STDF file into DateTime object
// Also extract root of the filename before time stamp
// Example of stdf file: D33H005_6_21_P_ETS211328_11112013.std
///////////////////////////////////////////////////////////
bool GTriggerEngine::extractDateTime_EagleStdf(QDateTime &cDateTime, QString &FileNameRoot, const QString &FileName)
{
    // LOG
    QString strTextMessage = QString("Extract date/time and root name from Eagle file %1").arg(FileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    QRegExp clRegExp(EAGLE_STDF_REGEXP, Qt::CaseInsensitive);
    if(clRegExp.indexIn(FileName) == -1)
    {
        // Failed parsing date....
        strTextMessage = QString("Failed extracting fields from Eagle file %1 using regexp %2").arg(FileName)
                .arg(EAGLE_STDF_REGEXP);
        //GEXLOG(7, strTextMessage.toLatin1().data());
        LogMessage_Debug(strTextMessage);
        return false;
    }

    // Extract root file name
    FileNameRoot = clRegExp.cap(1);

    // Extract time/date info: eg: 211328_11112013 (format is hhmmss_MMddyyyy)
    QString strDateTime = clRegExp.cap(2);
    int     iYear,iMonth,iDay,iHour,iMin,iSec;
    char    cChar;
    if(sscanf(strDateTime.toLatin1().constData(),"%2d%2d%2d%c%2d%2d%4d", &iHour, &iMin, &iSec, &cChar, &iMonth, &iDay, &iYear) != 7)
    {
        // Failed parsing date....
        strTextMessage = QString("Failed extracting fields from Eagle file %1 using regexp %2").arg(FileName)
                .arg(EAGLE_STDF_REGEXP);
        //GEXLOG(7, strTextMessage.toLatin1().data());
        LogMessage_Debug(strTextMessage);
        return false;
    }

    QDate cDate(iYear,iMonth,iDay);
    QTime cTime(iHour,iMin,iSec);
    cDateTime.setDate(cDate);
    cDateTime.setTime(cTime);

    return true;
}

///////////////////////////////////////////////////////////
// Extract date/time from FetTest log file into DateTime object
// Example of log file: X32606.5-09.dat_2008Apr09.110225_gexmo.log
///////////////////////////////////////////////////////////
void GTriggerEngine::extractDateTime_FetTest(QDateTime &cDateTime,const QString &FileName, const QString & RegExp)
{
    // LOG
    QString strTextMessage = QString("Extract date/time from FetTest log file %1").arg(FileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    QRegExp clRegExp(RegExp, Qt::CaseInsensitive);
    if(clRegExp.indexIn(FileName) == -1)
    {
        // Failed parsing date....should never occur
        cDateTime = QDateTime::currentDateTime();
        return;
    }

    QString strDateTime = clRegExp.cap(2);

    // Parse text: eg: 2007Jul28.064354 (format is YYYYMMMDD.HHMMSS)
    // GCORE-5450: Format can now be YYYYMMDD.HHMMSS so allow for either ...
    char    szMonth[5];
    char    cChar;
    int     iYear,iMonth,iDay,iHour,iMin,iSec;
    if (strDateTime.at(4).isDigit() ||
        sscanf(strDateTime.toLatin1().constData(), "%4d%3s%2d%c%2d%2d%2d",
               &iYear, szMonth, &iDay, &cChar, &iHour, &iMin, &iSec) != 7)
    {
        // Failed parsing date....should never occur
        // Actually it can now with the new optional format.. so try the new one ..
        if(sscanf(strDateTime.toLatin1().constData(),"%4d%2d%2d%c%2d%2d%2d", &iYear, &iMonth, &iDay, &cChar, &iHour, &iMin, &iSec) != 7)
        {
            // Failed parsing date....this one shouldn't happen ..
            strTextMessage = QString("Failed extracting fields from FET file %1 using YYYYMMDD.HHMMSS format").arg(FileName) ;
            //GEXLOG(7, strTextMessage.toLatin1().data());
            LogMessage_Debug(strTextMessage);

            cDateTime = QDateTime::currentDateTime();
            return;
        }

    } else {
        // Old format worked --
        // Convert String Month to Numeric one 01 - 12
        iMonth = 0;
        if(!qstricmp(szMonth,"Jan"))
            iMonth = 1;
        if(!qstricmp(szMonth,"Feb"))
            iMonth = 2;
        if(!qstricmp(szMonth,"Mar"))
            iMonth = 3;
        if(!qstricmp(szMonth,"Apr"))
            iMonth = 4;
        if(!qstricmp(szMonth,"May"))
            iMonth = 5;
        if(!qstricmp(szMonth,"Jun"))
            iMonth = 6;
        if(!qstricmp(szMonth,"Jul"))
            iMonth = 7;
        if(!qstricmp(szMonth,"Aug"))
            iMonth = 8;
        if(!qstricmp(szMonth,"Sep"))
            iMonth = 9;
        if(!qstricmp(szMonth,"Oct"))
            iMonth = 10;
        if(!qstricmp(szMonth,"Nov"))
            iMonth = 11;
        if(!qstricmp(szMonth,"Dec"))
            iMonth = 12;
    }
    LogMessage_Debug(QString("year=%1 month=%2 day=%3 hour=%4 min=%5 sec=%6").
                     arg(iYear).arg(iMonth).arg(iDay).
                     arg(iHour).arg(iMin).arg(iSec));

    QDate cDate(iYear,iMonth,iDay);
    QTime cTime(iHour,iMin,iSec);
    cDateTime.setDate(cDate);
    cDateTime.setTime(cTime);
}

///////////////////////////////////////////////////////////
// Extract date/time from Spektra log file into DateTime object
// Example of log file: B15K209.1_W01.Dta.CSV_2012Feb15.015902_gexmo.log
///////////////////////////////////////////////////////////
void GTriggerEngine::extractDateTime_Spektra(QDateTime &cDateTime,const QString &FileName, const QString &RegExp)
{
    // LOG
    QString strTextMessage = QString("Extract date/time from Spektra log file %1").arg(FileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    QRegExp clRegExp(RegExp, Qt::CaseInsensitive);
    if(clRegExp.indexIn(FileName) == -1)
    {
        // Failed parsing date....should never occur
        cDateTime = QDateTime::currentDateTime();
        return;
    }

    QString strDateTime = clRegExp.cap(2);

    // Parse text: eg: 2012Feb15.015902 (format is YYYYMMMDD.HHMMSS)
    char    szMonth[5];
    char    cChar;
    int     iYear,iMonth,iDay,iHour,iMin,iSec;
    if (strDateTime.at(4).isDigit() ||
        sscanf(strDateTime.toLatin1().constData(), "%4d%3s%2d%c%2d%2d%2d",
        &iYear, szMonth, &iDay, &cChar, &iHour, &iMin, &iSec) != 7)
    {
        // Failed parsing date....should never occur
        // Actually it can now with the new optional format.. so try the new one ..
        if(sscanf(strDateTime.toLatin1().constData(),"%4d%2d%2d%c%2d%2d%2d", &iYear, &iMonth, &iDay, &cChar, &iHour, &iMin, &iSec) != 7)
        {
            // Failed parsing date....this one shouldn't happen ..
            strTextMessage = QString("Failed extracting fields from FET file %1 using YYYYMMDD.HHMMSS format").arg(FileName) ;
            //GEXLOG(7, strTextMessage.toLatin1().data());
            LogMessage_Debug(strTextMessage);

            cDateTime = QDateTime::currentDateTime();
            return;
        }
    } else {
        // Old format parser worked -- convert month name into numeric one ..
        iMonth = 0;
        if(!qstricmp(szMonth,"Jan"))
            iMonth = 1;
        if(!qstricmp(szMonth,"Feb"))
            iMonth = 2;
        if(!qstricmp(szMonth,"Mar"))
            iMonth = 3;
        if(!qstricmp(szMonth,"Apr"))
            iMonth = 4;
        if(!qstricmp(szMonth,"May"))
            iMonth = 5;
        if(!qstricmp(szMonth,"Jun"))
            iMonth = 6;
        if(!qstricmp(szMonth,"Jul"))
            iMonth = 7;
        if(!qstricmp(szMonth,"Aug"))
            iMonth = 8;
        if(!qstricmp(szMonth,"Sep"))
            iMonth = 9;
        if(!qstricmp(szMonth,"Oct"))
            iMonth = 10;
        if(!qstricmp(szMonth,"Nov"))
            iMonth = 11;
        if(!qstricmp(szMonth,"Dec"))
            iMonth = 12;
    }
    LogMessage_Debug(QString("year=%1 month=%2 day=%3 hour=%4 min=%5 sec=%6").
                     arg(iYear).arg(iMonth).arg(iDay).
                     arg(iHour).arg(iMin).arg(iSec));

    QDate cDate(iYear,iMonth,iDay);
    QTime cTime(iHour,iMin,iSec);
    cDateTime.setDate(cDate);
    cDateTime.setTime(cTime);
}

///////////////////////////////////////////////////////////
// Extract date/time from FetTest NoPAT file into DateTime object
// Example of date/time string: 05/20/08 08:15:07
///////////////////////////////////////////////////////////
bool GTriggerEngine::extractDateTime_FetTest_NoPAT(QDateTime &cDateTime,QString &strDateTime)
{
    // LOG
    QString strTextMessage = QString("Extract date/time from FetTest NoPAT file %1").arg(strDateTime);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    // Parse text: eg: 05/20/08 08:15:07 (format is MM/DD/YY  HH:MM:SS)
    int iYear,iMonth,iDay,iHour,iMin,iSec;
    if(sscanf(strDateTime.toLatin1().constData(),"%2d%*c%2d%*c%2d%*c%2d%*c%2d%*c%2d", &iMonth, &iDay, &iYear, &iHour, &iMin, &iSec) != 6)
    {
        // Failed parsing date....should never occur
        cDateTime = QDateTime::currentDateTime();
        return false;
    }

    // Add 2000 to year if needed
    if(iYear < 2000)
        iYear += 2000;

    QDate cDate(iYear,iMonth,iDay);
    QTime cTime(iHour,iMin,iSec);
    cDateTime.setDate(cDate);
    cDateTime.setTime(cTime);

    return true;
}

///////////////////////////////////////////////////////////
// Build summary .log file (one per lot)
///////////////////////////////////////////////////////////
void GTriggerEngine::CreateSummaryLog(QString & strFolder, QString & strPattern, QString & strRegExp, QString & strLotID, QString & strSplitLotID, TesterType eTesterType)
{
    QDir        cDir;
    QString     lString, lMessage;

    // DEBUG LOG
    lMessage = QString("Create summary log for lot %1").arg(strLotID);
    //GEXLOG(7, lMessage.toLatin1().data());
    LogMessage_Debug(lMessage);

    // See all .log files for this LotID, and see if summary.log is the most recent...if not, must update it!
    cDir.setPath(strFolder);
    cDir.setFilter(QDir::Files);
    QStringList strLogList = cDir.entryList(QStringList(strPattern));

    // If no .log file, nothing to do!
    if(strLogList.count() <= 0)
        return;

    // For each log file, check if matching regular expression, and check date
    // GCORE-1301: introduce cPrevMostRecent to avoid missing any PAT LOG
    QFileInfo   cFileInfo;
    QDateTime   cDateTime, cMostRecentDate, cPrevMostRecent;
    QDateTime   cCurrentTime = QDateTime::currentDateTime();
    QString     strFilePath, lMostRecentPatLog;
    QRegExp     clRegExp(strRegExp, Qt::CaseInsensitive);

    cMostRecentDate.setTime_t(0);
    cPrevMostRecent.setTime_t(0);

    // Get/Purge last modification map (remove if older than 1 day)
    if(mLastModifiedLogForSummary.contains(strSplitLotID))
    {
        cPrevMostRecent = mLastModifiedLogForSummary.value(strSplitLotID);
        if(cPrevMostRecent.daysTo(cCurrentTime) > 1)
        {
            mLastModifiedLogForSummary.remove(strSplitLotID);
            cPrevMostRecent.setTime_t(0);
        }
    }

    QStringList::Iterator it;
    for(it = strLogList.begin(); it != strLogList.end(); )
    {
        lString = strFolder + "/" + *it;
        cFileInfo.setFile(lString);
        cDateTime = cFileInfo.lastModified();

        if(clRegExp.indexIn(*it) == -1)
            it = strLogList.erase(it);
        // GCORE-1301: Ignore files that are prior previous most recent (during previous generation of this
        // summary log file)
        else if(cDateTime <= cPrevMostRecent)
            it = strLogList.erase(it);
        else
        {
            if(cDateTime >= cMostRecentDate)
            {
                lMostRecentPatLog = lString;
                cMostRecentDate = cDateTime;
            }

            // DEBUG LOG
            lMessage = QString("File %1: lmd = %2, lmd of most recent log file = %3")
                    .arg(lString).arg(cDateTime.toString()).arg(cMostRecentDate.toString());
            LogMessage_Debug(lMessage);

            ++it;
        }
    }

    // If no .log file, nothing to do!
    if(strLogList.count() <= 0)
        return;

    // If some files too recent, do not create summary yet...
    if(cMostRecentDate.secsTo(cCurrentTime) < mSummaryDelay)
        return;

    // Check if need to update/create the summary.log file.
    cDir.setPath(mProduction);
    QString strLotSummary = mProduction + "/" + strSplitLotID + "-summary.log";

    // GCORE-1301: if previous last modofication exists, re-create summary log, as files still in the list
    // are more recent than this last modification
    if(QFile::exists(strLotSummary) && (!mLastModifiedLogForSummary.contains(strSplitLotID)))
    {
        // If last file modified was summary.log, then nothing to do!
        cFileInfo.setFile(strLotSummary);
        cDateTime = cFileInfo.lastModified();

        // DEBUG LOG
        lMessage = QString("Existing summary file %1: lmd = %2")
                .arg(strLotSummary).arg(cDateTime.toString());
        LogMessage_Debug(lMessage);

        // Ensure we recreate the summary file for the next 120 seconds after the last .log file found....
        if(cMostRecentDate.secsTo(cDateTime) > 120)
        {
            // LOG
            lMessage = QString("Existing summary file %1 (%2) is more recent than last PAT log file %3 (%4)")
                    .arg(strLotSummary).arg(cDateTime.toString())
                    .arg(lMostRecentPatLog).arg(cMostRecentDate.toString());
            LogMessage_Debug(lMessage);
            return;
        }
    }

    // DEBUG LOG
    lMessage = QString("Write/re-write summary log file %1").arg(strLotSummary);
    //GEXLOG(7, lMessage.toLatin1().data());
    LogMessage_Debug(lMessage);

    // Need to create/update the summary.log file...read each data file to extract the information wanted (eg: wafer#, yield info, etc)
    QFile           cLogFile;
    QTextStream     hLogFile;
    long            ldTotalDies;
    QString         strKeyword, strErrorMessage, strRecipe;
    QStringList     strLogCells;
    SummaryInfo     clSummaryInfo(mOutFolderLog, mProduction, strSplitLotID, strLotID);
    WaferInfo       *pWafer;
    bool            bOk;

    for (it = strLogList.begin(); it != strLogList.end(); ++it )
    {
        // Reset some variables
        strErrorMessage = strRecipe = "";

        // Read each log file (except summary.log one)
        lString = *it;
        strFilePath = strFolder + "/" + lString;
        cLogFile.setFileName(strFilePath);

        if(cLogFile.open(QIODevice::ReadOnly) == false)
            goto next_wafer;    // Failed opening .log file...see next one.

        // Read Tasks definition File
        hLogFile.setDevice(&cLogFile);  // Assign file handle to data stream

        // Create buffer to hold info.
        pWafer = new WaferInfo;

        // Save log file timestamp info. (extract from string file name)
        if(eTesterType == eFetTest)
            extractDateTime_FetTest(pWafer->cDateTime,lString, QString(FETTEST_LOG_REGEXP));
        else if(eTesterType == eEagle)
            extractDateTime_Eagle(pWafer->cDateTime,lString, QString(EAGLE_LOG_REGEXP));
        else if(eTesterType == eSpektra)
            extractDateTime_Spektra(pWafer->cDateTime,lString, QString(SPEKTRA_LOG_REGEXP));

        do
        {
            // Extract line
            lString = hLogFile.readLine();
            strLogCells = lString.split(QString(","), QString::SkipEmptyParts);
            strKeyword = strLogCells[0];

            // Check if valid PAT log file
            if(strKeyword == "Action")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }

                if(strLogCells[1] == "PAT")
                {
                    pWafer->bValidWafer = true;
                    pWafer->eWaferType = GS::GTrigger::ePatWafer;
                }
                else if(strLogCells[1] == "WAFER_EXPORT")
                {
                    pWafer->bValidWafer = true;
                    pWafer->eWaferType = GS::GTrigger::eNoPatWafer;
                    pWafer->strPassFail = " P ";
                }
                else
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // GrossDie count
            if((strKeyword == "Stdf_GrossDiesPerWafer") && (strLogCells.size() > 1))
                pWafer->iGrossDie = strLogCells[1].toLong(&bOk);;

            // Extract Error Message
            if((strKeyword == "ErrorMessage") && (strLogCells.size() > 1))
                strErrorMessage = strLogCells[1];

            // Extract Product
            if((strKeyword == "Product") && (strLogCells.size() > 1))
                clSummaryInfo.m_strProduct = strLogCells[1];

            // Extract Program
            if((strKeyword == "Program") && (strLogCells.size() > 1))
                clSummaryInfo.m_strProgram = strLogCells[1];

            // Extract Recipe
            if((strKeyword == "RecipeFile") && (strLogCells.size() > 1))
            {
                strRecipe = strLogCells[1];
                if(!strRecipe.isEmpty())
                    clSummaryInfo.m_strRecipe = strRecipe;
            }

            // Extract Wafer#
            if(strKeyword == "WaferID")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
                pWafer->lWaferID = strLogCells[1].toLong(&bOk);
                if(!bOk)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // Extract Die count
            if(strKeyword == "Stdf_TotalDies")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
                pWafer->lTotalDies = strLogCells[1].toLong(&bOk);
                if(!bOk)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // Extract Good Die count: NoPAT (use PrePAT variables)
            if(strKeyword == "GoodParts")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
                pWafer->lTotalGood_PrePAT = strLogCells[1].toLong(&bOk);
                if(!bOk)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // Extract Good Die count: Pre-PAT
            if(strKeyword == "GoodParts_BeforePAT")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
                pWafer->lTotalGood_PrePAT = strLogCells[1].toLong(&bOk);
                if(!bOk)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // Extract Good Die count: Post-PAT
            if(strKeyword == "GoodParts_AfterPAT")
            {
                if(strLogCells.size() <= 1)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
                pWafer->lTotalGood_PostPAT = strLogCells[1].toLong(&bOk);
                if(!bOk)
                {
                    delete pWafer;
                    goto next_wafer;    // Invalid field
                }
            }

            // Exytract Yield limit info
            if((strKeyword == "PatYieldLimit") && (strLogCells.size() > 1))
            {
                lString = strLogCells[1];   // eg: "0.05 % < 100.00% (alarm threshold)", "7.89 % > 0.00% (alarm threshold)"
                if(lString.contains("<"))
                    clSummaryInfo.m_strAlarmInfo = lString.section('<',1);  // Keep text AFTER the '<' sign
                else
                    clSummaryInfo.m_strAlarmInfo = lString.section('>',1);  // Keep text AFTER the '>' sign
            }

            // Extract PAT Alarm flag
            if((strKeyword == "PatAlarm") && (strLogCells.size() > 1))
            {
                if(strLogCells[1] == "No")
                    pWafer->strPassFail = " P ";
                else
                    pWafer->strPassFail = "*F*";
            }

        }
        while(hLogFile.atEnd() == false);
        cLogFile.close();

        // Compute yield info: see if use GrossDie count instead of STDF die count
        if(pWafer->iGrossDie > 0)
            ldTotalDies = pWafer->iGrossDie;
        else
            ldTotalDies = pWafer->lTotalDies;

        // Compute yield
        if(ldTotalDies)
        {
            pWafer->lfYield_PrePAT  = (100.0*pWafer->lTotalGood_PrePAT)/ldTotalDies;
            if(pWafer->eWaferType == GS::GTrigger::ePatWafer)
            {
                pWafer->lfYield_PostPAT = (100.0*pWafer->lTotalGood_PostPAT)/ldTotalDies;
                pWafer->lfDeltaYield = fabs(pWafer->lfYield_PrePAT - pWafer->lfYield_PostPAT);
            }
        }
        else
            pWafer->bValidWafer = false;

        // If PAT-ERROR reported in .log file, do not include file in summary log file!
        if(strErrorMessage.indexOf("*PAT Error*") >= 0)
            pWafer->bValidWafer = false;

        // Check if file read included all fields or if PAT task failed...
        if(pWafer->bValidWafer && (pWafer->lWaferID == -1))
        {
            // Failure...so extract wafer# from file name, and flag this wafer as failing one!
            if(eTesterType == eFetTest)
            {
                // Example: X32606.5-09.dat_2008Apr09.110225_gexmo.log
                QRegExp clRegExp(QString(FETTEST_LOG_REGEXP), Qt::CaseInsensitive);
                if(clRegExp.indexIn(*it) != -1)
                    lString = clRegExp.cap(1);
            }
            else if(eTesterType == eEagle)
            {
                // Example: Y03K123_1_11_P_ETS170758_04022008.log
                QRegExp clRegExp(QString(EAGLE_LOG_REGEXP), Qt::CaseInsensitive);
                if(clRegExp.indexIn(*it) != -1)
                    lString = clRegExp.cap(1);
            }
            else if(eTesterType == eSpektra)
            {
                // Example: B15K209.1_W01.Dta.CSV_2012Feb15.015902_gexmo.log
                QRegExp clRegExp(QString(SPEKTRA_LOG_REGEXP), Qt::CaseInsensitive);
                if(clRegExp.indexIn(*it) != -1)
                    lString = clRegExp.cap(1);
            }
            pWafer->lWaferID = lString.toLong();

            // If root cause=recipe missing, flag it!
            if((pWafer->eWaferType == GS::GTrigger::ePatWafer) && (strRecipe.isEmpty()))
                pWafer->strPassFail = "*F*";
        }

        // Add wafer info to list...unless invalid entry, or more recent entry exists!
        if(!pWafer->bValidWafer || !clSummaryInfo.Add(pWafer))
            delete pWafer;

next_wafer:;
    }

    // Create log file
    if(clSummaryInfo.WriteSummaryLogFile(strLotSummary, mAppName))
    {
        mLastModifiedLogForSummary.insert(strSplitLotID, cMostRecentDate);
        LogMessage("Summary log file created for splitlot " + strSplitLotID + " (" + strLotSummary + ").");
    }
}

///////////////////////////////////////////////////////////
// Read upto 256 characters string from STDF record
///////////////////////////////////////////////////////////
void GTriggerEngine::ReadStringToField(CStdf *pStdfFile,char *szField)
{
    char    szString[257];  // A STDF string is 256 bytes long max!
    QString strValue;

    // Empties string.
    *szField=0;

    // Read string from STDF file.
    if(pStdfFile->ReadString(szString)  != CStdf::NoError)
        return;

    // Security: ensures we do not overflow destination buffer !
    szString[256] = 0;
    strValue = szString;
    strValue = strValue.trimmed();
    strcpy(szField,strValue.toLatin1().constData());
}

///////////////////////////////////////////////////////////
// Read STDF MIR record so to extract header info: Lot#, product name.
///////////////////////////////////////////////////////////
bool GTriggerEngine::GetStdfHeaderData(QString &StdfFileName,QString &LotID,QString &SubLotID,
                                       QString &ProductID,QDateTime &StartTime)
{
    // LOG
    QString lTextMessage = QString("Read STDF MIR/WIR record from file %1").arg(StdfFileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(lTextMessage);

    ProductID = "unknown";
    SubLotID = "unknown";

    CStdf               lStdfHandle;
    StdfRecordReadInfo  lStdfRecordHeader;
    char                szString[257];
    unsigned char       bData;
    int                 wData;
    long                lData;

    int iStatus = lStdfHandle.Open(StdfFileName.toLatin1().constData(),STDF_READ,50000L);
    if(iStatus != CStdf::NoError)
    {
        // Failed openning STDF file....exception!
        LogMessage("Failed reading ProductID, file: "+StdfFileName,eError);
        return false;
    }

    do
    {
        // Read one record from STDF file.
        iStatus = lStdfHandle.LoadRecord(&lStdfRecordHeader);
        if(iStatus != CStdf::NoError)
        {
            // Failed reading STDF file....exception!
            LogMessage("Failed reading record (probably corrupted), file: "+StdfFileName,eError);
            lStdfHandle.Close();
            return false;
        }

        // Process STDF record read: read MIR/WIR records only.
        if(lStdfRecordHeader.iRecordType == 1 && lStdfRecordHeader.iRecordSubType == 10)
        {
            // MIR found, get data
            switch(lStdfRecordHeader.iStdfVersion)
            {
            case 3: // MIR STDF V3
                lStdfHandle.ReadByte(&bData);                   // CPU type
                lStdfHandle.ReadByte(&bData);                   // STDF Version
                lStdfHandle.ReadByte(&bData);                   // mode_code
                lStdfHandle.ReadByte(&bData);                   // stat #
                lStdfHandle.ReadByte(&bData);                   // TEST_COD char 1
                lStdfHandle.ReadByte(&bData);                   // TEST_COD char 2
                lStdfHandle.ReadByte(&bData);                   // TEST_COD char 3
                lStdfHandle.ReadByte(&bData);                   // rtst_code
                lStdfHandle.ReadByte(&bData);                   // PROT_COD
                lStdfHandle.ReadByte(&bData);                   // cmode_code
                lStdfHandle.ReadDword(&lData);                  // Setup_T
                lStdfHandle.ReadDword(&lData);                  // Start_T
                StartTime.setTime_t(lData);
                ReadStringToField(&lStdfHandle,szString);       // LotID
                LotID = szString;
                ReadStringToField(&lStdfHandle,szString);   // PART_TYP: ProductID
                ProductID = szString;
                return true;

            case 4: // MIR STDF V4
                lStdfHandle.ReadDword(&lData);                  // Setup_T
                lStdfHandle.ReadDword(&lData);                  // Start_T
                StartTime.setTime_t(lData);
                lStdfHandle.ReadByte(&bData);                  // station #
                lStdfHandle.ReadByte(&bData);                   // mode_code
                lStdfHandle.ReadByte(&bData);                   // rtst_code
                lStdfHandle.ReadByte(&bData);                   // prot_cod #
                lStdfHandle.ReadWord(&wData);                   // burn_time
                lStdfHandle.ReadByte(&bData);                   // cmode_code
                ReadStringToField(&lStdfHandle,szString);       // LOT_ID
                LotID = szString;
                ReadStringToField(&lStdfHandle,szString);       // PART_TYP: ProductID
                ProductID = szString;
                ReadStringToField(&lStdfHandle,szString);       // NODE_NAME
                ReadStringToField(&lStdfHandle,szString);       // TSTR_NAM
                ReadStringToField(&lStdfHandle,szString);       // JOB_NAME
                ReadStringToField(&lStdfHandle,szString);       // JOB_REV
                ReadStringToField(&lStdfHandle,szString);       // SBLOT_ID
                SubLotID = szString;
                return true;

            default:
                // Failed openning STDF file....exception!
                LogMessage("Failed reading ProductID, file: "+StdfFileName,eError);
                return false;
            }
        }
    }
    while(1);   // Loop until MIR or end of file found
}

///////////////////////////////////////////////////////////
// Open file, retries until timout reached.
///////////////////////////////////////////////////////////
bool GTriggerEngine::openFile(QFile &fFile,QIODevice::OpenMode openMode,QString &strFileName, int iTimeoutSec/*=15*/)
{
    // LOG
    QString strTextMessage = QString("Open file %1").arg(strFileName);
    //GEXLOG(7, strTextMessage.toLatin1().data());
    LogMessage_Debug(strTextMessage);

    fFile.setFileName(strFileName);

    QTime   cTime,cWait;
    QTime   cTimeoutTime = QTime::currentTime();
    cTimeoutTime = cTimeoutTime.addSecs(iTimeoutSec);

    do
    {
        // Open file
        if(fFile.open(openMode) == true)
            return true;    // Success opening file.

        // Failed accessing to file....then wait 15msec & retry.
        cTime = QTime::currentTime();
        cWait = cTime.addMSecs(15);
        do
        {
            cTime = QTime::currentTime();
        }
        while(cTime < cWait);
    }
    while(cTime < cTimeoutTime);

    // Timeout error: failed to open file.
    return false;
}

///////////////////////////////////////////////////////////
// Returns Product name associated with a given .RUN file.
///////////////////////////////////////////////////////////
bool GTriggerEngine::getProductName(QString &strSummaryFile,QString &strRunName,bool &bGalaxyRetest,
                                    bool &bDisablePromis,bool &bEngineeringRecipe,WaferFilesList & WaferList)
{
    int lWaferNb = -1;

    // Clear
    bGalaxyRetest = false;

    // Extract .RUN file name from .PRN content
    QFile       fPrnFile(strSummaryFile);
    QTextStream hPrn;
    QString     strString;

    // LOG
    strString = QString("Extracting product name for %1").arg(strSummaryFile);
    //GEXLOG(7, strString.toLatin1().data());
    LogMessage_Debug(strString);

    if(fPrnFile.open(QIODevice::ReadOnly) == false)
        return false;

    hPrn.setDevice(&fPrnFile);
    do
    {
        strString = hPrn.readLine();

        // If this .PRN was created by Galaxy (for retest), then we know the STDF file list follows!
        if(strString.indexOf(QString("GALAXY_RETEST_STDF_FILE")) >= 0)
        {
            bGalaxyRetest = true;
            // Extract STDF file name.
            strString = mStdfInFolder + "/" + strString.section("GALAXY_RETEST_STDF_FILE ", 1);
            WaferFiles lWaferFiles(lWaferNb, GS::GTrigger::ePatWafer);
            lWaferFiles.AddDataFile(strString);
            WaferList.insert(lWaferNb, lWaferFiles);
            lWaferNb--;
        }

        if(strString.indexOf(QString("GALAXY_RETEST_PROMIS_DISABLED")) >= 0)
        {
            strString = strString.section(' ', 1,1);    // Extract Promis disable status (1 or 0)
            bDisablePromis = (bool) strString.toInt();
        }

        if(strString.indexOf(QString("GALAXY_RETEST_ENG_RECIPE")) >= 0)
        {
            strString = strString.section(' ', 1,1);    // Do not use the production recipe but the Engineering instead.
            bEngineeringRecipe = (bool) strString.toInt();
        }

        if(strString.indexOf(QString("RUN FILE=")) >= 0)
        {
            // E.G Line: "REM =PEAA20FBQ D/S PROGRAM                       RUN FILE="DR9B2047.RUN"
            strString = strString.section("RUN FILE=\"", 1,1);  // Extract run file name
            strRunName = strString.section('.',0,0);            // get run file name (without file extension)
            if(strRunName.length() > 0)
                return true;
            else
                return false;
        }
    }
    while((hPrn.atEnd() == false));

    return false;
}

///////////////////////////////////////////////////////////
// Build name of recipe file to use in GTF.
///////////////////////////////////////////////////////////
QString GTriggerEngine::getRecipeFile(const QString &ProductID, const QString &RunName, const bool EngRecipe,
                                      const ProdFlow Flow)
{
    // LOG
    LogMessage_Debug(QString("Get Recipe for product %1").arg(ProductID));

    QString lRecipeName_Json, lRecipeName_Csv;

    // First check if we find a JSON recipe
    if(getRecipeFile(ProductID, RunName, EngRecipe, Flow, GS::Gex::PATRecipeIO::JSON, lRecipeName_Json))
        return lRecipeName_Json;

    // No JSON recipe, check if we find a CSV recipe
    if(getRecipeFile(ProductID, RunName, EngRecipe, Flow, GS::Gex::PATRecipeIO::CSV, lRecipeName_Csv))
        return lRecipeName_Csv;


    // No recipe exists... force default JSON name so PAT-Man will fail on it and report the expected recipe name
    return lRecipeName_Json;
}

///////////////////////////////////////////////////////////
// Build name of recipe file to use in GTF.
///////////////////////////////////////////////////////////
bool GTriggerEngine::getRecipeFile(const QString &ProductID, const QString &RunName, const bool EngRecipe,
                                   const ProdFlow Flow, const GS::Gex::PATRecipeIO::RecipeFormat RecipeFormat,
                                   QString &RecipeFile)
{
    QString     lString, lEng, lExt;
    QDir        lDir;
    QStringList lRecipesList;
    int         lIndex, lHighestVersionID;
    int         lMinorVersion=0, lHighestMinorVersion=0;
    int         lMajorVersion=0, lHighestMajorVersion=0;

    // Check if recipe file name to hold a Engineerning sub-string.
    if(EngRecipe)
        lEng = "_Eng";
    else
        lEng = "";

    // Check which extension to look for
    if(RecipeFormat == GS::Gex::PATRecipeIO::JSON)
        lExt = "json";
    else
        lExt = "csv";

    // Try#1: <recipe_folder>/product_program_Version*_patman_config.<ext>
    if(Flow != eFlow_Hvm_Spektra_Pat)
    {
        RecipeFile = QString("%1_%2%3_Version*_patman_config.%4").arg(ProductID).arg(RunName).arg(lEng).arg(lExt);
        lDir.setPath(mRecipeFolder);
        lDir.setFilter(QDir::Files);
        lRecipesList = lDir.entryList(QStringList(RecipeFile));
        if(lRecipesList.count() > 1)
        {
            // We need to get the highest version#
            lHighestVersionID = 0;
            lHighestMajorVersion=lHighestMinorVersion=0;
            for(lIndex = 0; lIndex < (int)lRecipesList.count(); lIndex++)
            {
                // Extract version# from file name.
                lString = lRecipesList[lIndex];
                lString = lString.section("Version",1);
                if(lString.isEmpty() == false)
                {
                    // Extract version# and keep highest
                    if(sscanf(lString.toLatin1().constData(),"%d.%d",&lMajorVersion,&lMinorVersion) == 2)
                    {
                        if(lMajorVersion > lHighestMajorVersion)
                        {
                            lHighestVersionID = lIndex; // Keep track of recipe file name with highest version#
                            lHighestMajorVersion = lMajorVersion;
                            lHighestMinorVersion = lMinorVersion;
                        }
                        else if((lMajorVersion == lHighestMajorVersion) && (lMinorVersion > lHighestMinorVersion))
                        {
                            lHighestVersionID = lIndex; // Keep track of recipe file name with highest version#
                            lHighestMajorVersion = lMajorVersion;
                            lHighestMinorVersion = lMinorVersion;
                        }
                    }
                }
            }
            RecipeFile = mRecipeFolder + "/" + lRecipesList[lHighestVersionID];
            return true;
        }
        else if(lRecipesList.count() == 1)
        {
            // Only one recipe, format is <recipe_folder>/product_program_VersionXXX_patman_config.<ext>
            RecipeFile = mRecipeFolder + "/" + lRecipesList[0];
            return true;
        }
    }

    // Try#2: <recipe_folder>/product_Version*_patman_config.<ext>
    if(Flow != eFlow_Hvm_FetTest_Pat)
    {
        RecipeFile = QString("%1_Version*_patman_config.%2").arg(ProductID).arg(lExt);
        lDir.setPath(mRecipeFolder);
        lDir.setFilter(QDir::Files);
        lRecipesList = lDir.entryList(QStringList(RecipeFile));
        if(lRecipesList.count() > 1)
        {
            // We need to get the highest version#
            lHighestVersionID = 0;
            lHighestMajorVersion=lHighestMinorVersion=0;
            for(lIndex = 0; lIndex < (int)lRecipesList.count(); lIndex++)
            {
                // Extract version# from file name.
                lString = lRecipesList[lIndex];
                lString = lString.section("Version",1);
                if(lString.isEmpty() == false)
                {
                    // Extract version# and keep highest
                    if(sscanf(lString.toLatin1().constData(),"%d.%d",&lMajorVersion,&lMinorVersion) == 2)
                    {
                        if(lMajorVersion > lHighestMajorVersion)
                        {
                            lHighestVersionID = lIndex; // Keep track of recipe file name with highest version#
                            lHighestMajorVersion = lMajorVersion;
                            lHighestMinorVersion = lMinorVersion;
                        }
                        else if((lMajorVersion == lHighestMajorVersion) && (lMinorVersion > lHighestMinorVersion))
                        {
                            lHighestVersionID = lIndex; // Keep track of recipe file name with highest version#
                            lHighestMajorVersion = lMajorVersion;
                            lHighestMinorVersion = lMinorVersion;
                        }
                    }
                }
            }
            RecipeFile = mRecipeFolder + "/" + lRecipesList[lHighestVersionID];
            return true;
        }
        else if(lRecipesList.count() == 1)
        {
            // Only one recipe, format is <recipe_folder>/product_VersionXXX_patman_config.<ext>
            RecipeFile = mRecipeFolder + "/" + lRecipesList[0];
            return true;
        }
    }

    // Try#3: <recipe_folder>/product_program_patman_config.<ext>
    if(Flow != eFlow_Hvm_Spektra_Pat)
    {
        RecipeFile = QString("%1/%2_%3%4_patman_config.%5").arg(mRecipeFolder).arg(ProductID).arg(RunName)
                .arg(lEng).arg(lExt);
        if(QFile::exists(RecipeFile))
            return true;
    }

    // Try#4: <recipe_folder>/product_patman_config.<ext>
    if(Flow != eFlow_Hvm_FetTest_Pat)
    {
        RecipeFile = QString("%1/%2%3_patman_config.%4").arg(mRecipeFolder).arg(ProductID).arg(lEng).arg(lExt);
        if(QFile::exists(RecipeFile))
            return true;
    }

    // No recipe found... (still RecipeFile contains expected file name)
    return false;
}

///////////////////////////////////////////////////////////
// Check if composite PAT is enabled in the recipe file.
///////////////////////////////////////////////////////////
bool GTriggerEngine::isCompositePatEnabled(const QString & RecipeFile)
{
    // Read recipe
    QSharedPointer<GS::Gex::PATRecipeIO> lRecipeIO(GS::Gex::PATRecipeIO::CreateRecipeIo(RecipeFile));
    if (lRecipeIO.isNull())
        return false;

    GS::Gex::PATRecipe lPatRecipe;

    if (lRecipeIO->Read(lPatRecipe) == false)
        return false;

    return (lPatRecipe.GetOptions().GetExclusionZoneEnabled());
}

///////////////////////////////////////////////////////////
// Get list of wafer# that belong to a given split-lot + get Promis info (Primis LotID, GrossDie count, Geometry name)
///////////////////////////////////////////////////////////
bool GTriggerEngine::getPROMIS_WafersInSplitLot(const QString &strFolder,const QString &strLotID,
                                                CPromisTextFile &cPromis,WaferFilesList & WaferList,bool bEngPromis,
                                                QString & strPromisLine, QString & strPromisFormat,
                                                const WaferType Type /*= GS::GTrigger::eUnknownWafer*/)
{
    strPromisLine="";

    // GET PROMIS FILE AND FORMAT
    QString strString, strPromisFile, strErrorMsg;

    // LOG
    strString = QString("Get list of wafers from PROMIS for lot %1").arg(strLotID);
    //GEXLOG(7, strString.toLatin1().data());
    LogMessage_Debug(strString);

    // IN: strPath contains the location where the converter_external_file.xml is stored
    // IN: strType must be "final" or "wafer"
    // IN: strMode must be "prod" or "eng"
    // OUT: strErrorMsg contains the error message is any
    // OUT: strFileName and strFileFormat contain file path and format information
    if(!ConverterExternalFile::GetPromisFile(strFolder,"wafer",bEngPromis?"eng":"prod",strPromisFile,strPromisFormat,strErrorMsg))
    {
        // LEGACY: support legacy env. variables
        // Check if 'GEX_PROMIS_DATA_PATH' environment variable is set
        char *ptChar=NULL;
        if(bEngPromis)
            ptChar = getenv("GEX_PROMIS_ENG_DATA_PATH");    // Look into Engineering PROMIS file
        else
            ptChar = getenv("GEX_PROMIS_DATA_PATH");        // Look into PROMIS file
        if(ptChar)
        {
            strPromisFile=QString(ptChar);
            strPromisFormat=QString("vishay-lvm");
        }
    }

    if(strPromisFile.isEmpty())
    {
        LogMessage_Debug("Promis filename is empty");
        return false;   // Not set...quietly return
    }
    else
    {
        LogMessage_Debug(QString("Promis filename : %1").arg(strPromisFile));
    }

    // Open Promis data file....try opening it for 2 secs. before timing out.
    QTime cTimeoutTime = QTime::currentTime();
    cTimeoutTime = cTimeoutTime.addSecs(2);
    QTime cTime,cWait;

    bool    bSuccessOpen=false;
    QFile   cFile(strPromisFile);
    do
    {
        // Open PROMIS file in Read Only mode.
        if(cFile.open(QIODevice::ReadOnly))
        {
            bSuccessOpen = true;
            break;
        }

        // Failed accessing to file....then wait 150msec & retry.
        cWait = cTime.addMSecs(150);
        do
        {
            cTime = QTime::currentTime();
        }
        while(cTime < cWait);
    }
    while(cTime < cTimeoutTime);


    // Failed having access to the resource for over 2 secs....
    if(!bSuccessOpen)
    {
        LogMessage_Debug("Open failed");
        return false;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Read file, see if can find matching entry, extract required fields
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // "vishay-lvm" file format:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // <Lot_ID.Wafer_ID>, <LotID.splitlot>, <Qty>, <Fabsite>, <Equipment_id>, <Partid>,
    // <Geometry>,<GrossDiePerWafer>,<DieX>, <DieY>, <FlatOrientation>, <TestSite>
    // X32606.10,X32606.5,3,FB3,WEG03,DSPBJLT150AFOA6A.02,PBJL150AFOA,1200,1.14,1.58,270,KS
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // "vishay-hvm" file format:
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // <Lot_ID.Wafer_ID>, <LotID.splitlot>, <number of wafers>, <Fab Location>, <Die Part>,
    // <Geom_Name>,<GrossDie>, <DieX>, <DieY>, <Flat Orientation>,<site Location>
    // A26K345.1,A26K345.1,25,TOWER,566209TS,HVNFAKCD6T,233,3.51,3.55,90,MN
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QTextStream   hFile(&cFile);    // Assign file handle to data stream
    QString       strLine;
    QStringList   strCells;
    bool          bOk;

    // Make sure Geometry name is empty, it will be used to check if entry found in PROMIS file
    cPromis.strGeometryName = "";

    // Promis split-lot already known?
    if(cPromis.strPromisLotID.isEmpty())
    {
        // Only Lot.Wafer# is know....
        do
        {
            strLine = hFile.readLine();
            strString = strLine.toUpper();
            strCells = strString.split(QString(","),QString::KeepEmptyParts);
            if((strCells.count() >= 8)&& (strCells[0].trimmed() == strLotID))
            {
                // Save PROMIS line
                strPromisLine = strLine;

                // We've found the right line
                int nIndex=1;
                cPromis.strPromisLotID = strCells[nIndex++].trimmed();          // Promis split LOT ID to which this lot + wafer belongs
                cPromis.iWafersInSplit = strCells[nIndex++].trimmed().toInt(&bOk);  // Total wafers in spplit-lot
                cPromis.strFacilityID = strCells[nIndex++].trimmed();       // FabSite (eg: SCFAB)
                if(strPromisFormat=="vishay-lvm")
                    cPromis.strEquipmentID = strCells[nIndex++].trimmed();    // Equipment ID (eg: 3301400)
                cPromis.strDsPartNumber = strCells[nIndex++].trimmed();         // DS Part Number (eg: DSANBADT40BCAA6A.04)

                // Extract Geometry (same for all wafers in lot)
                cPromis.strGeometryName = strCells[nIndex++].trimmed();     // Geometry name.

                // Extract gross die (same for all wafers in lot)
                cPromis.iGrossDie = strCells[nIndex++].trimmed().toInt(&bOk);   // GrossDie count

                if(strCells.count() > nIndex) cPromis.lfDieW = strCells[nIndex++].trimmed().toFloat();   // DieX size
                if(strCells.count() > nIndex) cPromis.lfDieH = strCells[nIndex++].trimmed().toFloat();   // DieY size
                if(strCells.count() > nIndex) cPromis.iFlat = strCells[nIndex++].trimmed().toInt();          // flat orientation
                if(strCells.count() > nIndex) cPromis.strSiteLocation = strCells[nIndex++].trimmed();        // Site Location (ie KS)

                if(!bOk)
                {
                    LogMessage_Debug("Parse failed");
                    return false;
                }
                break;
            }
        }
        while(hFile.atEnd() == false);
    }
    else
    {
        // Split-lot already known...seek to relevant line.
        do
        {
            strLine = hFile.readLine();
            strString = strLine.toUpper();
            strCells = strString.split(QString(","), QString::KeepEmptyParts);
            if((strCells.count() >= 8)&& (strCells[1].trimmed() == cPromis.strPromisLotID))
            {
                // Save PROMIS line
                strPromisLine = strLine;

                int nIndex=2;
                cPromis.iWafersInSplit = strCells[nIndex++].trimmed().toInt(&bOk);  // Total wafers in spplit-lot
                cPromis.strFacilityID = strCells[nIndex++].trimmed();       // FabSite (eg: SCFAB)
                if(strPromisFormat=="vishay-lvm")
                    cPromis.strEquipmentID = strCells[nIndex++].trimmed();    // Equipment ID (eg: 3301400)
                cPromis.strDsPartNumber = strCells[nIndex++].trimmed();         // DS Part Number (eg: DSANBADT40BCAA6A.04)

                // Extract Geometry (same for all wafers in lot)
                cPromis.strGeometryName = strCells[nIndex++].trimmed();     // Geometry name.

                // Extract gross die (same for all wafers in lot)
                cPromis.iGrossDie = strCells[nIndex++].trimmed().toInt(&bOk);   // GrossDie count

                if(strCells.count() > nIndex) cPromis.lfDieW = strCells[nIndex++].trimmed().toFloat();      // DieX size
                if(strCells.count() > nIndex) cPromis.lfDieH = strCells[nIndex++].trimmed().toFloat();      // DieY size
                if(strCells.count() > nIndex) cPromis.iFlat = strCells[nIndex++].trimmed().toInt();       // flat orientation
                if(strCells.count() > nIndex) cPromis.strSiteLocation = strCells[nIndex++].trimmed();           // Site Location (ie KS)

                if(!bOk)
                {
                    LogMessage_Debug("Parse failed");
                    return false;
                }
                break;
            }
        }
        while(hFile.atEnd() == false);
    }


    // Check if LotID not in PROMIS file....
    if(cPromis.strGeometryName.isEmpty())
    {
        LogMessage_Debug("Geometry name is empty");
        return false;
    }

    // Rewind file and identify ALL wafers matching this Promis Spli-Lot.
    cFile.reset();
    hFile.setDevice(&cFile);

    // Identify all wafers in this split-lot.
    unsigned int    lWaferNb;
    bool            lOk;
    do
    {
        strString = hFile.readLine();
        strString = strString.toUpper();
        strCells = strString.split(QString(","), QString::KeepEmptyParts);
        if((strCells.count() >= 8)&& (strCells[1].trimmed() == cPromis.strPromisLotID))
        {
            // We've found the right line: <lot>.<wafer#>.Eg: X32606.10
            lWaferNb = strCells[0].trimmed().section('.',1).toUInt(&lOk);
            if(lOk && !WaferList.contains(lWaferNb))
            {
                WaferFiles lWaferFiles(lWaferNb, Type);
                WaferList.insert(lWaferNb, lWaferFiles);
            }
        }
    }
    while(hFile.atEnd() == false);

    cFile.close();

    // Set Promis LotID
    WaferList.SetPromisLotID(cPromis.strPromisLotID);

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Move all Eagle data files (not the summary files,
// which will trigger the GTF creation)
///////////////////////////////////////////////////////////
void GTriggerEngine::MoveDataFiles_Eagle(void)
{
    // Example of filename to move: Y03K123_1_16_N_ETS171357_04022008.std
    // Example of filename not to move: X50M012_1_LotSummary_ETS160857_01312008.std
    QString strDataFile = "*_?_ETS*.std";

    // Find first file matching this criteria...may not exist if not tested yet!
    QDir        cDir;
    QStringList strlFileList;
    cDir.setPath(mStdfEagleSumFolder);
    cDir.setFilter(QDir::Files);
    strlFileList = cDir.entryList(QStringList(strDataFile));

    // LOG
    if(!strlFileList.isEmpty())
    {
        QString strTextMessage = QString("Moving EAGLE files from %1").arg(mStdfEagleSumFolder);
        //GEXLOG(7, strTextMessage.toLatin1().data());
        LogMessage_Debug(strTextMessage);
    }

    // Check if env variable for replication is set
    char *ptChar = getenv("GEX_SERVER_INPUT_REPLICATION_FOLDER");

    // Move all Eagle non-summary matching files
    QStringList::Iterator   itFile;
    QString                 strMoveEagleFile;
    unsigned int            uiFilesMoved = 0;
    for(itFile = strlFileList.begin(); itFile != strlFileList.end(); itFile++)
    {
        if((*itFile).toLower().indexOf("_lotsummary_") == -1)
        {
            strDataFile = mStdfEagleSumFolder + "/" + (*itFile);

            // Check if replication required (if so, all input files should be copied to specified folder)
            if(ptChar != NULL)
            {
                // Copy file for replication
                QFileInfo   clFileInfo(strDataFile);
                QDir        clDir;
                strMoveEagleFile = ptChar;
                strMoveEagleFile += "/" + clFileInfo.dir().dirName();
                if(!clDir.exists(strMoveEagleFile))
                    clDir.mkdir(strMoveEagleFile);
                strMoveEagleFile += "/" + clFileInfo.fileName();
                ut_CopyFile(strDataFile.toLatin1().constData(), strMoveEagleFile.toLatin1().constData(), UT_COPY_USETEMPNAME);
            }

            // Move file (delete dest file if exists)
            strMoveEagleFile = mStdfEagleInFolder + "/" + (*itFile);
            if(cDir.exists(strMoveEagleFile))
                cDir.remove(strMoveEagleFile);
            cDir.rename(strDataFile,strMoveEagleFile);

            uiFilesMoved++;
        }
    }

    if(uiFilesMoved)
        LogMessage("EAGLE data files: total of "+QString::number(uiFilesMoved) +" files moved.");
}

///////////////////////////////////////////////////////////
// Create as many trigger files as Wafers (STDF files)
///////////////////////////////////////////////////////////
void GTriggerEngine::CreateTriggers(QString &strSummaryFile)
{
    // Check if FetTest .PRN summary file, or Eagle STDF summary file...
    if(strSummaryFile.endsWith(QString(".prn"),Qt::CaseInsensitive))
        CreateTriggers_FetTest(strSummaryFile); // FET-TEST
    else
        if(strSummaryFile.indexOf(QString("_LotSummary_"),0,Qt::CaseInsensitive) > 0)
            CreateTriggers_Eagle(strSummaryFile);   // Eagle
        else
            if(strSummaryFile.indexOf(QString(".cnt.csv"),0,Qt::CaseInsensitive) > 0)
                CreateTriggers_Spektra(strSummaryFile); // Spektra
}

///////////////////////////////////////////////////////////
// Timer callback: every X seconds check for .PRN summary file
///////////////////////////////////////////////////////////
void GTriggerEngine::OnTimerEvent()
{
    // SJM -- Initialize the timer check date/time only once -- first time ..
    // BG -- Just revert to previous code before stupid bug introduced by myself.
    //       Could also be a member initialized in the constructor.
    static QDateTime cLastSummaryCheck = QDateTime::currentDateTime();

    // Load settings from INI file.
    LoadINI();

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Check if summary.log files to create...only checks every minute!
    ////////////////////////////////////////////////////////////////////////////////////////////
    QDateTime dt = QDateTime::currentDateTime();

    if(std::abs(dt.secsTo(cLastSummaryCheck)) > 60)
    {
        CheckSummaryLogsCreation(mOutFolderLog);
        cLastSummaryCheck = dt;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Check for .PRN NO-PAT flow files...
    ////////////////////////////////////////////////////////////////////////////////////////////
    CheckForFetTestSummary_NoPatFlow(mPrnFolderNoPat);
    CheckForFetTestSummary_NoPatFlow(mPrnFolderNoPatHVM);

    // Move all Eagle data files (not the summary files, which will trigger the GTF creation)
    MoveDataFiles_Eagle();

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Check for .PRN files to process...
    ////////////////////////////////////////////////////////////////////////////////////////////
    QString strSummaryFile;
    if(!mSummaryList.GetNextSummaryFile(strSummaryFile) && !CheckForSummaryFiles(strSummaryFile))
    {
        if (mQA) {
            LogMessage("exit");
            thread()->exit(EXIT_SUCCESS);
            return;
        }
        // Reset timer and come back in X seconds...
        OnResetTimer();
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Fettest .PRN or Eagle Summary file available: Process it!
    ////////////////////////////////////////////////////////////////////////////////////////////
    QString strString = "Processing: " + strSummaryFile;
    LogMessage(strString);

    // Generate trigger files for all wafers associated with this FetTest .PRN or Eagle summary file or Spektra CNT files
    CreateTriggers(strSummaryFile);

    // Immediately check for another .PRN file to process....if any.
    if (mSummaryList.count() > 0)
    {
        OnResetTimer(true);
    }
    else
    {
        if (mQA) {
            exit(EXIT_SUCCESS);
        }
        OnResetTimer();
    }
}

bool GTriggerEngine::CheckForSummaryFiles(QString & strSummaryFile)
{
    // Clear list of files
    mSummaryList.clear();

    LogMessage_Debug(QString("Checking various places for Summary Files..")) ;

    // Check for standard PRN files
    CheckForSummaryFileAvailable(mPrnFolder, "*.PRN");
    // Check for standard HVM PRN files
    CheckForSummaryFileAvailable(mPrnFolderHVM, "*.PRN");
    // Check for Retest .PRN (retest folder).
    CheckForSummaryFileAvailable(mPrnRetestFolder, "*.PRN");
    // Check for Eagle files
    CheckForSummaryFileAvailable(mStdfEagleSumFolder, "*_LotSummary_*.std");
    // Check for Spektra files
    CheckForSummaryFileAvailable(mStdfSpektraCntFolder, "*.cnt.csv");

    // Get first file in list.
    return(mSummaryList.GetNextSummaryFile(strSummaryFile));
}

void GTriggerEngine::ParsingError(QString & strErrorMessage, CInputFile *pclInputFile/*=NULL*/, bool bFileRejected/*=false*/)
{
    // Add file name and line number to error message?
    if(pclInputFile)
    {
        if(pclInputFile->m_pclFile)
            strErrorMessage += " (File=" + pclInputFile->m_strFileName + ", stopped at line " + QString::number(pclInputFile->m_uiLineNb) + ")";
        else
            strErrorMessage += " (File=" + pclInputFile->m_strFileName + ")";
    }

    // Log error message
    LogMessage(strErrorMessage, eError);

    // Rename/Close input file ?
    if(pclInputFile)
    {
        QDir        clDir;
        QFileInfo   clFileInfo(pclInputFile->m_strFileName);
        QString     strDelayedDir, strRejectDir;
        QString     strDestFileName, strLogFileName;

        // Create directories for delayed/rejected files?
        if(!mDelayed_Files_Subfolder.isEmpty())
        {
            strDelayedDir = clFileInfo.path() + "/" + mDelayed_Files_Subfolder;
            if(!clDir.exists(strDelayedDir))
                clDir.mkdir(strDelayedDir);
        }
        if(!mRejected_Files_Subfolder.isEmpty())
        {
            strRejectDir = clFileInfo.path() + "/" + mRejected_Files_Subfolder;
            if(!clDir.exists(strRejectDir))
                clDir.mkdir(strRejectDir);
        }

        // Rejected or delayed?
        if(bFileRejected)
        {
            if(!strRejectDir.isEmpty())
            {
                strDestFileName = strRejectDir + "/" + clFileInfo.fileName();
                strLogFileName = strDestFileName + ".log";
            }
            else
            {
                strDestFileName = pclInputFile->m_strFileName + ".bad";
                strLogFileName = pclInputFile->m_strFileName + ".log";
            }
        }
        else
        {
            if(!strDelayedDir.isEmpty())
            {
                strDestFileName = strDelayedDir + "/" + clFileInfo.fileName();
                strLogFileName = strDestFileName + ".log";
            }
            else
            {
                strDestFileName = pclInputFile->m_strFileName + ".delay";
                strLogFileName = pclInputFile->m_strFileName + ".log";
            }
        }

        // Close input file
        pclInputFile->Close();

        // Create log file
        QFile clLogFile(strLogFileName);
        if(clLogFile.open(QIODevice::WriteOnly))
        {
            QTextStream streamLogFile(&clLogFile);
            streamLogFile << strErrorMessage;
            clLogFile.close();
        }

        // Rename file
        clDir.remove(strDestFileName);
        clDir.rename(pclInputFile->m_strFileName,strDestFileName);
    }
}

// Extract data from .CNT.CSV file
bool GTriggerEngine::GetCntData(const QString & strSpektraCntFile, CInputFile *pclInputFile, QString & strLotID)
{
    QString strLine, strMessage;

    // Fail reading file, keep for next trial.
    if(pclInputFile->Open() == false)
        return false;

    // Read CNT File
    bool        bStatus, bFileCorrupted=false;
    QStringList strlCells;

    // Check first line
    // ie.: "CNT File Name,B15K209.1_W24.CNT"
    bStatus = pclInputFile->NextLine(strLine);
    strlCells = strLine.split(",", QString::SkipEmptyParts);
    if((strlCells.size() != 2) || (strlCells.at(0).toLower() != "cnt file name") || (strlCells.at(1).toLower()+".csv" != strSpektraCntFile.toLower()))
    {
        // Failed to read .CNT.CSV or get LotID in file...
        strMessage = "Failed reading .CNT.CSV file";
        ParsingError(strMessage, pclInputFile, true);
        return false;
    }

    bStatus = pclInputFile->NextLine(strLine);
    while(bStatus && !bFileCorrupted)
    {
        strlCells = strLine.split(",", QString::SkipEmptyParts);
        if(strlCells.size() == 2)
        {
            if(strlCells.at(0).toLower() == "lot name")
                strLotID = strlCells[1];
        }
        bStatus = pclInputFile->NextLine(strLine);
    };

    // Close input file
    pclInputFile->Close();

    // Check if LotID found
    if(strLotID.isEmpty())
    {
        // Failed to read .CNT.CSV or get LotID in file...
        strMessage = "No Lot ID found in .CNT.CSV file";
        ParsingError(strMessage, pclInputFile, true);
        return false;
    }

    return true;
}

// Create composite map trigger file. Use only PAT flow wafers.
bool GTriggerEngine::CreateCompositeTrigger(const QString & CompositeRootFolder, const WaferFilesList &WaferList,
                                            const QString & RecipeFileName, const QString & TimeStamp,
                                            QString & CompositeMapFileName, const QString & ProductID)
{
    // Build composite trigger folder name
    QString lCompositeFolder, lCompositeTriggerFileName;
    if(mOutput_Pat_Composite_Maps_Subfolder.isEmpty())
        lCompositeFolder = QString("%1/zpat").arg(CompositeRootFolder);
    else
        lCompositeFolder = QString("%1/%2").arg(CompositeRootFolder).arg(mOutput_Pat_Composite_Maps_Subfolder);

    // Make sure folder exists
    QDir clDir;
    clDir.mkpath(lCompositeFolder);

    // Build composite trigger file name
    lCompositeTriggerFileName = QString("%1/%2_Composite_%3.gtf").arg(lCompositeFolder).arg(WaferList.PromisLotID())
            .arg(TimeStamp);

    // Build composite map file name
    CompositeMapFileName = QString("%1/%2_Composite_%3.txt").arg(lCompositeFolder).arg(WaferList.PromisLotID())
            .arg(TimeStamp);

    // Open composite trigger file
    QFile lFile(lCompositeTriggerFileName);
    if(!lFile.open(QIODevice::WriteOnly))
        return false;

    // Use text stream to write to file
    QTextStream lFileStream(&lFile);

    lFileStream << "<GalaxyTrigger>" << endl;
    lFileStream << "Action=COMPOSITE_PAT" << endl;
    lFileStream << "#Define output files/locations" << endl;
    lFileStream << QString("CompositeFile=%1").arg(QDir::cleanPath(CompositeMapFileName)) << endl;
    lFileStream << QString("LogFile=%1").arg(QDir::cleanPath(mOutFolderLog)) << endl;
    lFileStream << "#Define input files/locations" << endl;
    lFileStream << QString("Recipe=%1").arg(QDir::cleanPath(RecipeFileName)) << endl;
    // Overload Product with Geometry name
    lFileStream << "#Overload Product name (with Geometry)" << endl;
    lFileStream << "Product = " + ProductID << endl;


    WaferFilesList::const_iterator lWaferIt;
    QStringList lFiles;
    for(lWaferIt = WaferList.constBegin(); lWaferIt != WaferList.constEnd(); ++lWaferIt)
    {
        const WaferFiles & lWaferFiles = lWaferIt.value();
        if(lWaferFiles.GetWaferType() == GS::GTrigger::ePatWafer)
        {
            lWaferFiles.DataFiles(lFiles);
            for(int lIndex = 0; lIndex < lFiles.size(); ++lIndex)
            {
                lFileStream << "<wafer>" << endl;
                lFileStream << QString("WaferID=%1").arg(lWaferFiles.WaferNb()) << endl;
                lFileStream << QString("DataSource=%1").arg(QDir::cleanPath(lFiles.at(lIndex))) << endl;
                if(mMergeTestRetest)
                {
                    // Add all files for this wafer in the same GTF (files are sorted by date)
                    while(++lIndex < lFiles.size())
                        lFileStream << QString("DataSource=%1").arg(QDir::cleanPath(lFiles.at(lIndex))) << endl;
                }
                lFileStream << "</wafer>" << endl;
            }
        }
    }

    lFileStream << QString("Shell=%1").arg(mShell_Post_Composite) << endl;
    lFileStream << "</GalaxyTrigger>" << endl;

    lFile.close();

    return true;
}

// Create composite map trigger file. Use only PAT flow wafers.
bool GTriggerEngine::AddOptionalSourceToGtf(QTextStream & Trigger, const QString & LotID, const QString & WaferID)
{
    // Check if an optional source file exists. Convention is <LotID>_<WaferID>.kla
    QString lFileName = QString("%1/%2_%3.kla").arg(mInspectionMapsFolder).arg(LotID).arg(WaferID);
    if(!QFileInfo::exists(lFileName))
    {
        // Try with using a numeric WaferID (no leading 0)
        bool lOk;
        unsigned int lWaferNb = WaferID.toUInt(&lOk);
        if(!lOk)
            return false;
        lFileName = QString("%1/%2_%3.kla").arg(mInspectionMapsFolder).arg(LotID).arg(lWaferNb);
        if(!QFileInfo::exists(lFileName))
            return false;
    }

    // Add optional source line to GTF file
   Trigger << "# Optional source file" << endl;
   Trigger << "OptionalSource=" +  lFileName << endl;
   Trigger << "MergeMaps=true" << endl;

   return true;
}

/******************************************************************************!
 * \fn AddOptionalSourceToJs
 * \brief Create composite map trigger file. Use only PAT flow wafers.
 ******************************************************************************/
bool
GTriggerEngine::AddOptionalSourceToJs(QTextStream& Trigger,
                                      const QString& LotID,
                                      const QString& WaferID)
{
    // Check if an optional source file exists
    // Convention is <LotID>_<WaferID>.kla
    QString lFileName = QString("%1/%2_%3.kla").
        arg(mInspectionMapsFolder).
        arg(LotID).
        arg(WaferID);
    if (! QFileInfo::exists(lFileName))
    {
        // Try with using a numeric WaferID (no leading 0)
        bool lOk;
        unsigned int lWaferNb = WaferID.toUInt(&lOk);
        if (! lOk)
        {
            return false;
        }
        lFileName = QString("%1/%2_%3.kla").
            arg(mInspectionMapsFolder).
            arg(LotID).
            arg(lWaferNb);
        if (! QFileInfo::exists(lFileName))
        {
            return false;
        }
    }

    // Add optional source line to GTF file
    Trigger << "    // Optional source file" << endl;
    Trigger << "    OptionalSource: '" + lFileName << "'," << endl;
    Trigger << "    MergeMaps: 'true'," << endl;

    return true;
}

} // namespace GS
} // namespace GTrigger