#include <QApplication>
#include <QProgressBar>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "datapump_taskdata.h"
#include "browser_dialog.h"
#include "gex_shared.h"
#include "scheduler_engine.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "gexmo_constants.h"
#include "db_engine.h"
#include "report_build.h"
#include "mo_task.h"
#include "db_external_database.h"
#include "product_info.h"
#include "engine.h"
#include "message.h"
#include "admin_engine.h"

// main.cpp
extern GexMainwindow *	pGexMainWindow;
extern QProgressBar *	GexProgressBar;	// Handle to progress bar in status bar

const QString GexMoDataPumpTaskData::sPreScriptAttrName="PreScript";
const QString GexMoDataPumpTaskData::sPostScriptAttrName="PostScript";
const QString GexMoDataPumpTaskData::sPostScriptActivatedAttrName="PostScriptActivated";
const QString GexMoDataPumpTaskData::sPreScriptActivatedAttrName="PreScriptActivated";

const QString GexMoDataPumpTaskData::sStatisticalAgentsConfigurationADRAttrName = "StatAgentsConfigurationADRName";

const QString GexMoDataPumpTaskData::sDefaultPreScript="/*\n"\
        "Enter here any JavaScript codes you'd like to run before processing. \n"\
        "Most of the singleton objects available in the Quantix JS API are available. \n"\
        "*/";
const QString GexMoDataPumpTaskData::sDefaultPostScript="/*\n"\
        "Enter here any JavaScript codes you'd like to run after processing. \n"\
        "Most of the singleton objects available in the Quantix JS API are available. \n"\
        "*/";

const QString GexMoDataPumpTaskData::sIllegalScript="/* This feature is disabled (no Administration database found,...) */";

const QString GexMoDataPumpTaskData::sDefaultDataPumpPreScript="/*\n"\
        "Enter here any JavaScript codes you'd like to run just before insertion. \n"\
        "Most of the singleton objects available in the Quantix JS API are available.\n"\
        "The key content object to configure this insertion is called 'CurrentGSKeysContent' \n"\
        "Example:\n"\
        "    if (....)\n"\
        "        CurrentGSKeysContent.Set('key', 'value');\n"\
        "Status values (integer type):\n"\
        "    0 - Passed: File can be processed\n"\
        "    1 - Failed: File must be rejected (eg: file corrupted,...)\n"\
        "    2 - FailedValidationStep: File not corrupted but doesn't match with the validation step\n"\
        "    3 - Delay: Process failed but file not corrupted (eg: copy problem, etc), delay insertion to try again later\n"\
        "*/\n"\
        "CurrentGSKeysContent.Set('Status',0);\n"\
        "CurrentGSKeysContent.Set('Error','');";

GexMoDataPumpTaskData::GexMoDataPumpTaskData(QObject* parent): TaskProperties(parent)
{
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        strEmailFrom = GEX_EMAIL_YIELD_MAN;

    eSortFlags          = QDir::Time;
    bScanSubFolders     = false;
    uiDataType          = 0;
    bExecutionWindow    = false;
    bCheckYield         = false;
    iAlarmLevel         = 0;
    lMinimumyieldParts  = 0;
    bHtmlEmail          = false;
    iPostImport         = GEXMO_POSTIMPORT_RENAME;          // Rename good files to " .read"
    iPostImportFailure  = GEXMO_BAD_POSTIMPORT_RENAME;      // Rename bad files to ".bad"
    iPostImportDelay    = GEXMO_DELAY_POSTIMPORT_LEAVE;     // Leave delayed files unchanged
    uiDataType          = GEX_DATAPUMP_DATATYPE_TEST;

    iFrequency = GEXMO_TASKFREQ_30MIN;                      // Task frequency.
    iDayOfWeek = 0;                                         // Day of Week to execute task (0= Monday, ...6=Sunday)

    strImportFileExtensions=GS::Gex::ConvertToSTDF::GetListOfSupportedFormat().join(";");	// *.stdf;*.wat;*.pcm;*.gdf etc...
    // Monitoring ALSO accepts incoming .GTF (Galaxy Trigger File) files!
    strImportFileExtensions += ";";
    strImportFileExtensions += GEX_VALID_TRIGGERFILES_EXT;

    //  Other options
    bRejectSmallSplitlots_NbParts       = false;
    uiRejectSmallSplitlots_NbParts      = 0;
    bRejectSmallSplitlots_GdpwPercent   = false;
    lfRejectSmallSplitlots_GdpwPercent  = 0.9;
    bExecuteBatchAfterInsertion         = false;            // Execute batch after insertion
    nMaxPartsForTestResultInsertion     = -1;               // Max parts over which raw test results will not be inserted
    bRejectFilesOnPassBinlist           = false;            // Disable option to reject files with PASS hard bins not in specified list
    strPassBinListForRejectTest         ="1";

    mPriority = 1;
}

void GexMoDataPumpTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();
    // Insert new values
    SetPrivateAttribute("Title",strTitle);
    SetPrivateAttribute("DataPath",strDataPath);
    SetPrivateAttribute("ScanSubFolders",bScanSubFolders ? "YES" : "NO");
    SetPrivateAttribute("SortBy",QString(((eSortFlags & QDir::Time) ? "Time" : "Name")) + QString(((eSortFlags & QDir::Reversed) ? "|Desc" : "")));
    SetPrivateAttribute("ImportFileExtensions",strImportFileExtensions);
    SetPrivateAttribute("Frequency",QString::number(iFrequency));
    SetPrivateAttribute("DayOfWeek",QString::number(iDayOfWeek));
    SetPrivateAttribute("Database",strDatabaseTarget);
    SetPrivateAttribute("DataType",QString::number(uiDataType));
    SetPrivateAttribute("TestingStage",strTestingStage);
    SetPrivateAttribute("ExecWindow",bExecutionWindow ? "YES" : "NO");
    SetPrivateAttribute("StartTime",QString::number(cStartTime.hour()) + "," + QString::number(cStartTime.minute()));
    SetPrivateAttribute("StopTime",QString::number(cStopTime.hour()) + "," + QString::number(cStopTime.minute()));
    SetPrivateAttribute("PostImport",QString::number(iPostImport));
    SetPrivateAttribute("PostImportFolder",strPostImportMoveFolder);
    SetPrivateAttribute("PostImportFailure",QString::number(iPostImportFailure));
    SetPrivateAttribute("PostImportFailureFolder",strPostImportFailureMoveFolder);
    SetPrivateAttribute("PostImportDelay",QString::number(iPostImportDelay));
    SetPrivateAttribute("PostImportDelayFolder",strPostImportDelayMoveFolder);
    SetPrivateAttribute("CheckYield",bCheckYield ? "YES" : "NO");
    SetPrivateAttribute("YieldBins",strYieldBinsList);
    SetPrivateAttribute("AlarmLevel",QString::number(iAlarmLevel));
    SetPrivateAttribute("MinimumParts",QString::number(lMinimumyieldParts));
    SetPrivateAttribute("Emails",strEmailNotify);
    SetPrivateAttribute("EmailFrom",strEmailFrom);
    SetPrivateAttribute("EmailFormat",bHtmlEmail ? "HTML" : "TEXT");
    SetPrivateAttribute("RejectSmallSplitlots_NbParts",bRejectSmallSplitlots_NbParts ? QString::number(uiRejectSmallSplitlots_NbParts) : "-1");
    SetPrivateAttribute("RejectSmallSplitlots_Gdpw_Percent",bRejectSmallSplitlots_GdpwPercent ? QString::number(lfRejectSmallSplitlots_GdpwPercent, 'f', 2) : "-1");
    SetPrivateAttribute("ExecuteBatchAfterInsertion",bExecuteBatchAfterInsertion ? "YES" : "NO");
    SetPrivateAttribute("BatchToExecuteAfterInsertion",strBatchToExecuteAfterInsertion);
    SetPrivateAttribute("MaxPartsForTestResultInsertion",QString::number(nMaxPartsForTestResultInsertion));
    SetPrivateAttribute("RejectFilesOnPassBinlist",bRejectFilesOnPassBinlist ? "YES" : "NO");
    SetPrivateAttribute("PassBinlistForRejectTest",strPassBinListForRejectTest);
    SetPrivateAttribute("Priority",QVariant(mPriority));
}

GexMoDataPumpTaskData &GexMoDataPumpTaskData::operator=(const GexMoDataPumpTaskData &copy)
{
    if(this != &copy)
    {
        strTitle                = copy.strTitle;
        strDataPath             = copy.strDataPath;
        bScanSubFolders         = copy.bScanSubFolders;
        eSortFlags              = copy.eSortFlags;
        strImportFileExtensions = copy.strImportFileExtensions;
        strDatabaseTarget       = copy.strDatabaseTarget;
        uiDataType              = copy.uiDataType;
        strTestingStage         = copy.strTestingStage;
        iFrequency              = copy.iFrequency;
        iDayOfWeek              = copy.iDayOfWeek;
        bExecutionWindow        = copy.bExecutionWindow;

        iPostImport             = copy.iPostImport;
        strPostImportMoveFolder = copy.strPostImportMoveFolder;
        iPostImportFailure      = copy.iPostImportFailure;
        strPostImportFailureMoveFolder = copy.strPostImportFailureMoveFolder;
        iPostImportDelay        = copy.iPostImportDelay;
        strPostImportDelayMoveFolder = copy.strPostImportDelayMoveFolder;

        cStartTime              = copy.cStartTime;
        cStopTime               = copy.cStopTime;
        bCheckYield             = copy.bCheckYield;
        strYieldBinsList        = copy.strYieldBinsList;
        iAlarmLevel             = copy.iAlarmLevel;
        lMinimumyieldParts      = copy.lMinimumyieldParts;
        strEmailFrom            = copy.strEmailFrom;
        strEmailNotify          = copy.strEmailNotify;
        bHtmlEmail              = copy.bHtmlEmail;

        bRejectSmallSplitlots_NbParts       = copy.bRejectSmallSplitlots_NbParts;
        uiRejectSmallSplitlots_NbParts      = copy.uiRejectSmallSplitlots_NbParts;
        bRejectSmallSplitlots_GdpwPercent   = copy.bRejectSmallSplitlots_GdpwPercent;
        lfRejectSmallSplitlots_GdpwPercent  = copy.lfRejectSmallSplitlots_GdpwPercent;
        bExecuteBatchAfterInsertion         = copy.bExecuteBatchAfterInsertion;
        strBatchToExecuteAfterInsertion     = copy.strBatchToExecuteAfterInsertion;
        nMaxPartsForTestResultInsertion     = copy.nMaxPartsForTestResultInsertion;
        bRejectFilesOnPassBinlist           = copy.bRejectFilesOnPassBinlist;
        strPassBinListForRejectTest         = copy.strPassBinListForRejectTest;

        mPriority               = copy.mPriority;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}

