//#include <QMessageBox>
//#include <QFileDialog>
#include <QScriptValueIterator>

// Galaxy QT libraries
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "browser_dialog.h"
#include "engine.h"
#include "pickproduct_id_dialog.h"
#include "tb_merge_retest_dialog.h"
#include "tb_merge_retest.h"
#include "cbinning.h"
#include "gqtl_datakeys.h"
#include "db_engine.h"
#include "export_atdf.h"
#include "message.h"
#include "datapump/datapump_taskdata.h"
#include "outlierremoval_taskdata.h"
#include "status/status_taskdata.h"
#include "mo_email.h"
#include "mo_task.h"
#include "patman_lib.h"
#include "gexmo_constants.h"
#include "gex_file_in_group.h"
#include "gex_shared.h"
#include "gex_group_of_files.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "temporary_files_manager.h"
#include "product_info.h"
#include "csl/csl_engine.h"
#include "pat_external_map_details.h"
#include "pat_process_ws.h"
#include "pat_report_ws.h"
#include "pat_recipe_io.h"
#include "pat_definition.h"
#include "pat_engine.h"
#include "report_options.h"
#include "report_build.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "outlierremoval/outlierremoval_task.h"
#include "pat/pat_task.h"
#include "admin_engine.h"

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

// PAT alarms flag
#define GEX_PAT_ALARM_YIELD_LOSS_YIELD			0x01
#define GEX_PAT_ALARM_YIELD_LOSS_PARTS			0x02
#define GEX_PAT_ALARM_DISTRIBUTION_MISMATCH		0x04

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *	pGexMainWindow;
extern GexScriptEngine* pGexScriptEngine;

// in report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options
extern CGexReport *gexReport;				// Handle to report class

// script_wizard.h
extern void ConvertToScriptString(QString &strFile);

///////////////////////////////////////////////////////////
// Constructor: structure holding PAT task.
///////////////////////////////////////////////////////////
GexMoOutlierRemovalTaskData::GexMoOutlierRemovalTaskData(QObject* parent): TaskProperties(parent)
{
    strTitle = "default";           // Task title.
    strProductID = " ";             // ProductID to look for
    iAlarmType=0;                   // 0= Trigger alarm of given % of parts are fail. 1= Trigger alarm of given # of parts are fail
    lfAlarmLevel=100;               // Alarm kevel (if higher failure rate than this field, trigger alarm)
    lMinimumyieldParts=0;           // Minimum parts required to check the yield.
    strEmailNotify="";              // Email addresses to notify.
    bHtmlEmail=true;                // 'true' if email to be sent in HTML format
    bNotifyShapeChange=false;       // true=email notification if distribution change compared to historical data
    iEmailReportType=0	;           // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, CSV, Word,  PPT, PDF
    iNotificationType=0;            // 0= Report attached to email, 1= keep on server, only email its URL
    lCompositeExclusionZoneAlarm=5; // Composite Waferlot: Alarm level (number of rejected dies on exclusion zone map)
    lCompositeEtestAlarm=50;        // Composite Waferlot: Alarm level (maximum mismatch per wafer)

    // Default 'From' Email address.
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        strEmailFrom = GEX_EMAIL_YIELD_MAN;
}

void GexMoOutlierRemovalTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();
    // Insert new values
    SetPrivateAttribute("Title",strTitle);
    SetPrivateAttribute("Database",strDatabase);
    SetPrivateAttribute("TestingStage",strTestingStage);
    SetPrivateAttribute("ProductID",strProductID);
    SetPrivateAttribute("AlarmLevel",QString::number(lfAlarmLevel));
    SetPrivateAttribute("AlarmType",QString::number(iAlarmType));
    SetPrivateAttribute("MinimumParts",QString::number(lMinimumyieldParts));
    SetPrivateAttribute("NotifyShapeChange",(bNotifyShapeChange ? "1" : "0"));
    SetPrivateAttribute("CompositeEtestAlarm",QString::number(lCompositeEtestAlarm));
    SetPrivateAttribute("CompositeExclusionZoneAlarm",QString::number(lCompositeExclusionZoneAlarm));
    SetPrivateAttribute("Emails",strEmailNotify);
    SetPrivateAttribute("EmailFormat",(bHtmlEmail ? "HTML" : "TEXT"));
    SetPrivateAttribute("EmailReportType",QString::number(iEmailReportType));
    SetPrivateAttribute("NotificationType",QString::number(iNotificationType));
    SetPrivateAttribute("ExceptionLevel",QString::number(iExceptionLevel));
}

GexMoOutlierRemovalTaskData &GexMoOutlierRemovalTaskData::operator=(const GexMoOutlierRemovalTaskData &copy)
{
    if(this != &copy)
    {
        strTitle                = copy.strTitle;
        strDatabase             = copy.strDatabase;
        strTestingStage         = copy.strTestingStage;
        strProductID            = copy.strProductID;
        iAlarmType              = copy.iAlarmType;
        lfAlarmLevel            = copy.lfAlarmLevel;
        lMinimumyieldParts      = copy.lMinimumyieldParts;
        lCompositeEtestAlarm    = copy.lCompositeEtestAlarm;
        lCompositeExclusionZoneAlarm = copy.lCompositeExclusionZoneAlarm;
        strEmailFrom            = copy.strEmailFrom;
        strEmailNotify          = copy.strEmailNotify;
        bHtmlEmail              = copy.bHtmlEmail;
        bNotifyShapeChange      = copy.bNotifyShapeChange;
        iEmailReportType        = copy.iEmailReportType;
        iNotificationType       = copy.iNotificationType;
        iExceptionLevel         = copy.iExceptionLevel;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}
/*
///////////////////////////////////////////////////////////
// Check PAT Yield loss and trigger email if need be
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::CheckPatYieldLoss(GS::Gex::PATProcessing &cFields, double lfYieldLoss,
                                                 long lPatFailingParts, long lTotalParts,
                                                 int lDistributionMismatch,
                                                 GS::QtLib::DatakeysContent& dbKeysContent,
                                                 QString &strErrorMessage,
                                                 QString& lLogMessage)
{
    bool	bAlarm                      = false;
    QString	strYieldLoss                = "";
    QString	strDistributionShapeAlarm   = "";

    switch(cFields.iAlarmType)
    {
    case 0:	// Check if % of failures exceeds threshold
        if(lfYieldLoss > cFields.lfYieldAlarmLoss)
            bAlarm = true;

        cFields.strYieldThreshold = QString::number(lfYieldLoss,'f',2);
        cFields.strYieldThreshold += " %";

        // If yield limit defined, report it in text
        if(cFields.lfYieldAlarmLoss <= 100.0)
        {
            if(bAlarm)
                cFields.strYieldThreshold += " > ";	// Alarm
            else
                cFields.strYieldThreshold += " < ";	// No alarm
            cFields.strYieldThreshold += QString::number(cFields.lfYieldAlarmLoss,'f',2) + "% (alarm threshold)";
        }
        strYieldLoss += cFields.strYieldThreshold;
        break;

    case 1:	// Check if number of failing parts exceeds threshols
        if(lPatFailingParts > cFields.lfYieldAlarmLoss)
            bAlarm = true;

        cFields.strYieldThreshold = QString::number(lPatFailingParts);
        cFields.strYieldThreshold += " parts";

        // If yield limit defined, report it in text
        if(cFields.lfYieldAlarmLoss > 0)
        {
            if(bAlarm)
                cFields.strYieldThreshold += " > ";	// Alarm
            else
                cFields.strYieldThreshold += " < ";	// No alamrm

            cFields.strYieldThreshold += QString::number(cFields.lfYieldAlarmLoss,'f',0) + " (alarm threshold)";
        }

        strYieldLoss += cFields.strYieldThreshold;
        break;
    }

    // Check if Distribution shape change compared to historical data
    if(lDistributionMismatch > 0)
    {
        strDistributionShapeAlarm.sprintf("%d distribution changes detected (Distribution mismatch: Historical Data vs. current data)",lDistributionMismatch);
        bAlarm = true;	// Force alarm so email notification will be triggered.
    }

    // Save alarm status
    cFields.bPatAlarm = bAlarm;

    // Check if Keep PAT report on server or not (delete it)...
    bool	bErasePatReport=false;
    switch(cFields.iGenerateReport)
    {
        case GEXMO_OUTLIER_REPORT_ALWAYS:	// Always generate report...
        break;

        default:
        case GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM:
            // If not enough failure to generate a report, then quietly return!
            if(cFields.strOutputReportMode.startsWith("alarm", Qt::CaseInsensitive) == true && bAlarm == false)
                bErasePatReport = true;		// Erase report!
            break;
        case GEXMO_OUTLIER_REPORT_NEVER:	// Always erase report...
            bErasePatReport = true;
            break;
    }

    // Erase report if requested
    if(bErasePatReport)
    {
        // Remove report created (if any)
        QFileInfo cFileInfo(gexReport->reportAbsFilePath());	// Path to report name
        if(gexReport->reportAbsFilePath().endsWith(".htm",false) || gexReport->reportAbsFilePath().endsWith(".html",false))
            GS::Gex::Engine::GetInstance().GetDatabaseEngine().DeleteFolderContent(cFileInfo.absolutePath());	// Delete folder where report was created (if HTML report !)
        GS::Gex::Engine::RemoveFileFromDisk(gexReport->reportAbsFilePath());	// Delete file if exists
        return nReturn_AlarmFlag;
    }

    // Report kept...check if option set to rename it to same name as trigger file?
    if(cFields.iReportNaming == GEXMO_OUTLIER_REPORT_NAME_TRIGGERFILE)
    {
        QDir cDir;
        QFileInfo	cFileInfo(cFields.strTriggerFile);
        QString		strStriggerName = cFileInfo.baseName();	// Get trigger file name (without path, without extension)
        // Get full path to report folder
        cFileInfo.setFile(gexReport->reportAbsFilePath());
        QString strNewReportName = cFileInfo.absolutePath() + "/" + strStriggerName + "." + cFileInfo.suffix();

        GS::Gex::Engine::RemoveFileFromDisk(strNewReportName);	// Delete file if exists

        // Rename report name to use trigger base name.
        cDir.rename(gexReport->reportAbsFilePath(),strNewReportName);

        // Update report name buffer
        if (gexReport->reportGenerationMode() == "legacy")
            gexReport->setLegacyReportName(strNewReportName);
        else
            gexReport->setReportName(strStriggerName);
    }

    // Alarm: Create email message!
    QString strAttachment;
    QString	strEmailBody="";

    // Get Email spooling folder.
    CGexMoTaskItem *ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        strErrorMessage = "  > Reporting: failed to get Email spooling folder";
        return nReturn_AlarmFlag;
    }

    // If report to append, create it now
    if(cFields.strOutputReportFormat.isEmpty() == false)
    {
        // If report to be sent as an attachment, then tell where the report is located
        if(cFields.iNotificationType == 0)
            strAttachment = gexReport->reportAbsFilePath();
    }
    else
        strAttachment="";	// No report to attach...so clear report name currently stored in this variable.

    // Build email + send it.
    GexMoBuildEmailString	cEmailString;
    bool					bHtmlEmail = cFields.bHtmlEmail;	// 'true' if email to be sent in HTML format
    GexMoSendEmail			Email;
    QString					strFilePath;
    QString					strFrom,strTo,strSubject;
    QString					strProductID = dbKeysContent.Get("Product").toString();
    if(!cFields.m_strProductName.isEmpty())
        strProductID = cFields.m_strProductName;
    strTo = cFields.strEmailTo;
    if(cFields.strEmailFrom.isEmpty() == true)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            strFrom = GEX_EMAIL_PAT_MAN;
        else
            strFrom = GEX_EMAIL_YIELD_MAN;
    }
    else
        strFrom = cFields.strEmailFrom;
    if(cFields.strEmailTitle.isEmpty() == true)
        strSubject = "PAT-Man Yield Loss Alarm";
    else
        strSubject = cFields.strEmailTitle;

    // Message to report in history log window
    switch(cFields.iGenerateReport)
    {
        case GEXMO_OUTLIER_REPORT_ALWAYS:	// Always generate report...
            if(bAlarm)
                lLogMessage = "  > PAT low yield alarm. Report sent to: " + strTo;	// If alarm while report forced, flag it!
            break;

        default:
        case GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM:
            lLogMessage = "  > PAT low yield alarm. Report sent to: " + strTo;
            break;

        case GEXMO_OUTLIER_REPORT_NEVER:	// Must never happen since code must return from previous bloc!
            break;
    }

    // Build email!
    time_t lStartTime = dbKeysContent.Get("StartTime").toUInt();

    QString strHttpReportName;
    // Check if specify http path to report
    if (!ptStatusTask->ptStatus->reportHttpURL().isEmpty())
        strHttpReportName = ptStatusTask->ptStatus->reportHttpURL() + QDir::cleanPath(QDir::separator() + gexReport->reportRelativeFilePath());

    if(bHtmlEmail)
    {
        // HTML Header
        cEmailString.CreatePage(strSubject);

        // Table with Lot Info
        cEmailString.WriteHtmlOpenTable();

        // Write table
        cEmailString.WriteInfoLine("Testing date", TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
        cEmailString.WriteInfoLine("Product",strProductID);
        cEmailString.WriteInfoLine("Lot",dbKeysContent.Get("Lot").toString());
        cEmailString.WriteInfoLine("SubLot",dbKeysContent.Get("SubLot").toString());
        cEmailString.WriteInfoLine("WaferID",dbKeysContent.Get("Wafer").toString());

        if(strYieldLoss.isEmpty() == false)
            cEmailString.WriteInfoLine("Yield Loss",strYieldLoss,true);	// Highlight in red as it is a Yield Alarm!

        if(strDistributionShapeAlarm.isEmpty() == false)
            cEmailString.WriteInfoLine("Data Shape alarm",strDistributionShapeAlarm,true);	// Highlight in red as it is a Data shape change alarm

        cEmailString.WriteInfoLine();

        if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Tester",dbKeysContent.Get("TesterName").toString());

        if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Operator",dbKeysContent.Get("Operator").toString());

        if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Program name",dbKeysContent.Get("ProgramName").toString());

        if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Program revision",dbKeysContent.Get("ProgramRevision").toString());

        cEmailString.WriteInfoLine("Total parts tested",QString::number(lTotalParts));

        // Table with Lot Info
        cEmailString.WriteHtmlCloseTable();

        // If Full Report to be created
        if(cFields.strOutputReportFormat.isEmpty() == false)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(cFields.iNotificationType == 0)
                cEmailString.AddHtmlString("<br><b>Full report attached to this message!</b><br>\n");
            else
            {
                // Leave report on server, email URL only.
                cEmailString.AddHtmlString("<br><b>Full Report available on server at:<br>\n");

                if (!strHttpReportName.isEmpty())
                {
                    // Http hyperlink
                    cEmailString.AddHtmlString("<a href=\"");
                    cEmailString.AddHtmlString(strHttpReportName);
                    cEmailString.AddHtmlString("\">");
                    // Display File name
                    cEmailString.AddHtmlString(strHttpReportName);
                }
                else
                {
                    // File hyperlink
                    cEmailString.AddHtmlString("<a href=\"file:///");
                    cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                    cEmailString.AddHtmlString("\">");
                    // Display File name
                    cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                }
                // Close URL hyperlink
                cEmailString.AddHtmlString("</a>");

                // Close message
                cEmailString.AddHtmlString("</b><br><br>\n");
            }
        }
    }
    else
    {
        // Plain Text email.
        strEmailBody = "\n###############################################################\n";
        strEmailBody += QString("Testing date       : ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + QString("\n");
        strEmailBody += QString("Product            : ") + strProductID + QString("\n");
        strEmailBody += QString("Lot                : ") + dbKeysContent.Get("Lot").toString() + QString("\n");
        strEmailBody += QString("SubLot             : ") + dbKeysContent.Get("SubLot").toString() + QString("\n");
        strEmailBody += QString("WaferID            : ") + dbKeysContent.Get("Wafer").toString() + QString("\n");

        if(strYieldLoss.isEmpty() == false)
            strEmailBody += QString("Yield Loss         : ") + strYieldLoss;

        if(strDistributionShapeAlarm.isEmpty() == false)
            strEmailBody += QString("Data Shape alarm   : ") + strDistributionShapeAlarm;
        strEmailBody += "\n";
        strEmailBody += "###############################################################\n\n";

        // If Full Report to be created
        if(cFields.strOutputReportFormat.isEmpty() == false)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(cFields.iNotificationType == 0)
                strEmailBody += "Full report attached to this message!\n";
            else
            {
                // Leave report on server, email URL only.
                strEmailBody += "Full Report available on server at:\n  ";
                if (!strHttpReportName.isEmpty())
                    strEmailBody += strHttpReportName;
                else
                    strEmailBody += gexReport->reportAbsFilePath();
                strEmailBody += "\n";
            }
            strEmailBody += "\n";
        }

        if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
            strEmailBody += QString("Tester             : ") + dbKeysContent.Get("TesterName").toString() + QString("\n");

        if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
            strEmailBody += QString("Operator           : ") + dbKeysContent.Get("Operator").toString() + QString("\n");

        if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
            strEmailBody += QString("Program name       : ") + dbKeysContent.Get("ProgramName").toString() + QString("\n");

        if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
            strEmailBody += QString("Program revision   : ") + dbKeysContent.Get("ProgramRevision").toString() + QString("\n");

        strEmailBody += QString("Total parts tested : ") + QString::number(lTotalParts) + QString("\n");
    }

    // Close HTML email string if need be
    if(bHtmlEmail)
        strEmailBody = cEmailString.ClosePage();

    // We have a spooling folder: create email file in it!
    strFilePath = ptStatusTask->ptStatus->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;

    // Send email with Yield Monitoring alarm message + report.
    Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,bHtmlEmail,strAttachment);

    return nReturn_AlarmFlag;
}
*/

#ifdef GCORE15334

bool GS::Gex::SchedulerEngine::CheckForPATNotification(GS::Gex::PATProcessing &cFields,
                                                       long lTotalParts,
                                                       int lDistributionMismatch)
{
    bool    lNotify     = false;
    double  lYieldLoss  = 0.0;

    if (lTotalParts > 0)
    {
        cFields.mPatAlarm = false;

        CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();
        lYieldLoss = 100.0 * (double) lPatInfo->GetTotalPATFailingParts() / lTotalParts;

        if (lYieldLoss > 100)
            lYieldLoss = 100;

        switch(cFields.mAlarmType)
        {
            case 0:	// Check if % of failures exceeds threshold
                if(lYieldLoss > cFields.mYieldAlarmLoss)
                    cFields.mPatAlarm = true;

                cFields.mYieldThreshold = QString::number(lYieldLoss,'f',2);
                cFields.mYieldThreshold += " %";

                // If yield limit defined, report it in text
                if(cFields.mYieldAlarmLoss <= 100.0)
                {
                    if(cFields.mPatAlarm)
                        cFields.mYieldThreshold += " > ";	// Alarm
                    else
                        cFields.mYieldThreshold += " < ";	// No alarm
                    cFields.mYieldThreshold += QString::number(cFields.mYieldAlarmLoss,'f',2) + "% (alarm threshold)";
                }
                break;

            case 1:	// Check if number of failing parts exceeds threshols
                if(lPatInfo->GetTotalPATFailingParts() > cFields.mYieldAlarmLoss)
                    cFields.mPatAlarm = true;

                cFields.mYieldThreshold = QString::number(lPatInfo->GetTotalPATFailingParts());
                cFields.mYieldThreshold += " parts";

                // If yield limit defined, report it in text
                if(cFields.mYieldAlarmLoss > 0)
                {
                    if(cFields.mPatAlarm)
                        cFields.mYieldThreshold += " > ";	// Alarm
                    else
                        cFields.mYieldThreshold += " < ";	// No alamrm

                    cFields.mYieldThreshold += QString::number(cFields.mYieldAlarmLoss,'f',0) + " (alarm threshold)";
                }
                break;
        }

        // Check if Distribution shape change compared to historical data
        if(lDistributionMismatch > 0)
            cFields.mPatAlarm = true;	// Force alarm so email notification will be triggered.

        // Check if Keep PAT report on server or not (delete it)...
        switch(cFields.iGenerateReport)
        {
            case GEXMO_OUTLIER_REPORT_ALWAYS:	// Always generate report...
                lNotify = true;
                break;

            default:
            case GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM:
                // If not enough failure to generate a report, then quietly return!
                if(cFields.strOutputReportMode.startsWith("alarm", Qt::CaseInsensitive) == true &&
                   cFields.mPatAlarm == true)
                    lNotify = true;
                break;
            case GEXMO_OUTLIER_REPORT_NEVER:	// Always erase report...
                break;
        }
    }

    return lNotify;
}

bool GS::Gex::SchedulerEngine::SendPATNotification(GS::Gex::PATProcessing &cFields,
                                                   int lDistributionMismatch,
                                                   long lTotalParts,
                                                   GS::QtLib::DatakeysContent &dbKeysContent,
                                                   QString &strErrorMessage, QString &lLogMessage)
{
    // Alarm: Create email message!
    QString strAttachment;
    QString	strEmailBody;

    // Get Email spooling folder.
    CGexMoTaskStatus * ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        strErrorMessage = "  > Reporting: failed to get Email spooling folder";
        return false;
    }

    // If report to append, create it now
    if(cFields.strOutputReportFormat.isEmpty() == false)
    {
        // If report to be sent as an attachment, then tell where the report is located
        if(cFields.iNotificationType == 0)
            strAttachment = gexReport->reportAbsFilePath();
    }

    // Build email + send it.
    GexMoBuildEmailString	cEmailString;
    bool					bHtmlEmail = cFields.bHtmlEmail;	// 'true' if email to be sent in HTML format
    GexMoSendEmail			Email;
    QString					strFilePath;
    QString					strFrom,strTo,strSubject;
    QString					strProductID = dbKeysContent.Get("Product").toString();
    QString                 lDistributionShapeAlarm;

    if(!cFields.m_strProductName.isEmpty())
        strProductID = cFields.m_strProductName;

    if (lDistributionMismatch > 0)
        lDistributionShapeAlarm.sprintf("%d distribution changes detected (Distribution mismatch: Historical Data vs. current data)",
                                        lDistributionMismatch);

    strTo = cFields.strEmailTo;

    if(cFields.strEmailFrom.isEmpty() == true)
    {
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            strFrom = GEX_EMAIL_PAT_MAN;
        else
            strFrom = GEX_EMAIL_YIELD_MAN;
    }
    else
        strFrom = cFields.strEmailFrom;

    if(cFields.strEmailTitle.isEmpty() == true)
        strSubject = "PAT-Man Yield Loss Alarm";
    else
        strSubject = cFields.strEmailTitle;

    // Message to report in history log window
    switch(cFields.iGenerateReport)
    {
        case GEXMO_OUTLIER_REPORT_ALWAYS:	// Always generate report...
            // If alarm while report forced, flag it!
            if(cFields.mPatAlarm)
                lLogMessage = "  > PAT low yield alarm. Report sent to: " + strTo;
            break;

        default:
        case GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM:
            lLogMessage = "  > PAT low yield alarm. Report sent to: " + strTo;
            break;

        case GEXMO_OUTLIER_REPORT_NEVER:	// Must never happen since code must return from previous bloc!
            break;
    }

    // Build email!
    time_t lStartTime = dbKeysContent.Get("StartTime").toUInt();

    QString strHttpReportName;
    // Check if specify http path to report
    if (!ptStatusTask->GetProperties()->reportHttpURL().isEmpty())
        strHttpReportName = ptStatusTask->GetProperties()->reportHttpURL() + QDir::cleanPath(QDir::separator() + gexReport->reportRelativeFilePath());

    if(bHtmlEmail)
    {
        // HTML Header
        cEmailString.CreatePage(strSubject);

        // Table with Lot Info
        cEmailString.WriteHtmlOpenTable();

        // Write table
        cEmailString.WriteInfoLine("Testing date",TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
        cEmailString.WriteInfoLine("Product",strProductID);
        cEmailString.WriteInfoLine("Lot",dbKeysContent.Get("Lot").toString());
        cEmailString.WriteInfoLine("SubLot",dbKeysContent.Get("SubLot").toString());
        cEmailString.WriteInfoLine("WaferID",dbKeysContent.Get("Wafer").toString());

        // Highlight in red as it is a Yield Alarm!
        if(cFields.mYieldThreshold.isEmpty() == false)
            cEmailString.WriteInfoLine("Yield Loss", cFields.mYieldThreshold, cFields.mPatAlarm);

        // Highlight in red as it is a Data shape change alarm
        if(lDistributionShapeAlarm.isEmpty() == false)
            cEmailString.WriteInfoLine("Data Shape alarm", lDistributionShapeAlarm, true);

        cEmailString.WriteInfoLine();

        if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Tester",dbKeysContent.Get("TesterName").toString());

        if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Operator",dbKeysContent.Get("Operator").toString());

        if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Program name",dbKeysContent.Get("ProgramName").toString());

        if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
            cEmailString.WriteInfoLine("Program revision",dbKeysContent.Get("ProgramRevision").toString());

        cEmailString.WriteInfoLine("Total parts tested",QString::number(lTotalParts));

        // Table with Lot Info
        cEmailString.WriteHtmlCloseTable();

        // If Full Report to be created
        if(cFields.strOutputReportFormat.isEmpty() == false)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(cFields.iNotificationType == 0)
                cEmailString.AddHtmlString("<br><b>Full report attached to this message!</b><br>\n");
            else
            {
                // Leave report on server, email URL only.
                cEmailString.AddHtmlString("<br><b>Full Report available on server at:<br>\n");

                if (!strHttpReportName.isEmpty())
                {
                    // Http hyperlink
                    cEmailString.AddHtmlString("<a href=\"");
                    cEmailString.AddHtmlString(strHttpReportName);
                    cEmailString.AddHtmlString("\">");
                    // Display File name
                    cEmailString.AddHtmlString(strHttpReportName);
                }
                else
                {
                    // File hyperlink
                    cEmailString.AddHtmlString("<a href=\"file:///");
                    cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                    cEmailString.AddHtmlString("\">");
                    // Display File name
                    cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                }
                // Close URL hyperlink
                cEmailString.AddHtmlString("</a>");

                // Close message
                cEmailString.AddHtmlString("</b><br><br>\n");
            }
        }
    }
    else
    {
        // Plain Text email.
        strEmailBody = "\n###############################################################\n";
        strEmailBody += QString("Testing date       : ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + QString("\n");
        strEmailBody += QString("Product            : ") + strProductID + QString("\n");
        strEmailBody += QString("Lot                : ") + dbKeysContent.Get("Lot").toString() + QString("\n");
        strEmailBody += QString("SubLot             : ") + dbKeysContent.Get("SubLot").toString() + QString("\n");
        strEmailBody += QString("WaferID            : ") + dbKeysContent.Get("Wafer").toString() + QString("\n");

        if(cFields.mYieldThreshold.isEmpty() == false)
            strEmailBody += QString("Yield Loss         : ") + cFields.mYieldThreshold;

        if(lDistributionShapeAlarm.isEmpty() == false)
            strEmailBody += QString("Data Shape alarm   : ") + lDistributionShapeAlarm;

        strEmailBody += "\n";
        strEmailBody += "###############################################################\n\n";

        // If Full Report to be created
        if(cFields.strOutputReportFormat.isEmpty() == false)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(cFields.iNotificationType == 0)
                strEmailBody += "Full report attached to this message!\n";
            else
            {
                // Leave report on server, email URL only.
                strEmailBody += "Full Report available on server at:\n  ";
                if (!strHttpReportName.isEmpty())
                    strEmailBody += strHttpReportName;
                else
                    strEmailBody += gexReport->reportAbsFilePath();
                strEmailBody += "\n";
            }
            strEmailBody += "\n";
        }

        if(dbKeysContent.Get("TesterName").toString().isEmpty() == false)
            strEmailBody += QString("Tester             : ") + dbKeysContent.Get("TesterName").toString() + QString("\n");

        if(dbKeysContent.Get("Operator").toString().isEmpty() == false)
            strEmailBody += QString("Operator           : ") + dbKeysContent.Get("Operator").toString() + QString("\n");

        if(dbKeysContent.Get("ProgramName").toString().isEmpty() == false)
            strEmailBody += QString("Program name       : ") + dbKeysContent.Get("ProgramName").toString() + QString("\n");

        if(dbKeysContent.Get("ProgramRevision").toString().isEmpty() == false)
            strEmailBody += QString("Program revision   : ") + dbKeysContent.Get("ProgramRevision").toString() + QString("\n");

        strEmailBody += QString("Total parts tested : ") + QString::number(lTotalParts) + QString("\n");
    }

    // Close HTML email string if need be
    if(bHtmlEmail)
        strEmailBody = cEmailString.ClosePage();

    // We have a spooling folder: create email file in it!
    strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;

    // Send email with Yield Monitoring alarm message + report.
    return Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,bHtmlEmail,strAttachment);
}

/******************************************************************************!
 * \fn SetProperties
 ******************************************************************************/
void
GS::Gex::SchedulerEngine::SetProperties(GS::Gex::PATProcessing& lPP,
                                        const QString& lErrorMessage,
                                        const GS::Gex::PAT::ExternalMapDetails& lExternalMapDetails)
{
    CGexGroupOfFiles* pGroup = NULL;
    CGexFileInGroup* pFile = NULL;
    CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();
    bool lRecipeExists = QFile::exists(lPP.strRecipeFile);

    lPP.Set("PatmanRev", GS::Gex::Engine::GetInstance().Get("AppFullName"));
    lPP.Set("Action", "PAT");
    lPP.Set("ErrorMessage", lErrorMessage);
    if (gexReport && gexReport->getGroupsList().count())
    {
        pGroup = (gexReport->getGroupsList().isEmpty()) ? NULL : gexReport->getGroupsList().first();
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // Output datalog file
        if(lPP.mOutputDatalogFormat == GEX_TPAT_DLG_OUTPUT_NONE)
        {
            lPP.Set("DataOutput", "none");
        }
        QString lSetUpTime =
            ((pFile->getMirDatas().lSetupT > 0) ? TimeStringUTC(pFile->getMirDatas().lSetupT): "n/a.\n");
        lPP.Set("SetupTime", lSetUpTime);

        QString lStartTime =
            ((pFile->getMirDatas().lStartT > 0) ? TimeStringUTC(pFile->getMirDatas().lStartT): "n/a.\n");
        lPP.Set("StartTime", lStartTime);

        QString lEndTime =
            ((pFile->getMirDatas().lEndT > 0) ? TimeStringUTC(pFile->getMirDatas().lEndT): "n/a.\n");
        lPP.Set("EndTime", lEndTime);

        if (lPP.m_strProductName.isEmpty())
        {
            lPP.Set("Product", pFile->getMirDatas().szPartType);
        }
        lPP.Set("DesignRev",  pFile->getMirDatas().szDesignRev);
        lPP.Set("Program", pFile->getMirDatas().szJobName);
        lPP.Set("ProgramRev", pFile->getMirDatas().szJobRev);
        lPP.Set("Lot", pFile->getMirDatas().szLot);
        lPP.Set("SubLot", pFile->getMirDatas().szSubLot);
        lPP.Set("WaferID", pFile->getWaferMapData().szWaferID);
        if (QFile::exists(gexReport->reportAbsFilePath()))
        {
            lPP.Set("Report", gexReport->reportAbsFilePath());
        }
        else
        {
            lPP.Set("Report", "None");
        }
    }
    if (lRecipeExists)
    {
        if (lPatInfo)
        {
            QString lOutputString;
            // SPAT failing Soft/Hardbin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().iFailStatic_SBin)
                    + "," + QString::number(lPatInfo->GetRecipeOptions().iFailStatic_HBin);
            lPP.Set("SPAT_FailBin", lOutputString);

            // DPAT rule failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().iFailDynamic_SBin)
                    + "," + QString::number(lPatInfo->GetRecipeOptions().iFailDynamic_HBin);
            lPP.Set("DPAT_FailBin", lOutputString);

            // MVPAT rule failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().GetMVPATSoftBin())
                    + "," + QString::number(lPatInfo->GetRecipeOptions().GetMVPATHardBin());
            lPP.Set("MVPAT_FailBin", lOutputString);

            // DPAT NNR failing bin (Nearest Neighbor Residual)
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().GetNNRSoftBin())
                    + "," + QString::number(lPatInfo->GetRecipeOptions().GetNNRHardBin());
            lPP.Set("DPAT_NNR", lOutputString);

            // DPAT GDBN failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().mGDBNPatSBin)
                    + "," + QString::number(lPatInfo->GetRecipeOptions().mGDBNPatHBin);
            lPP.Set("DPAT_BadNeighbor", lOutputString);

            // DPAT Clustering failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().mClusteringPotatoSBin)
                    + "," + QString::number(lPatInfo->GetRecipeOptions().mClusteringPotatoHBin);
            lPP.Set("DPAT_Clustering", lOutputString);

            // DPAT Reticle failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().GetReticleSoftBin())
                    + "," + QString::number(lPatInfo->GetRecipeOptions().GetReticleHardBin());
            lPP.Set("DPAT_Reticle", lOutputString);

            // DPAT Reticle failing bin
            lOutputString = QString::number(lPatInfo->GetRecipeOptions().iCompositeZone_SBin)
                    + "," + QString::number(lPatInfo->GetRecipeOptions().iCompositeZone_HBin);
            lPP.Set("DPAT_ZPAT", lOutputString);
        }
    }
    // If file processed
    if (lRecipeExists && pFile)
    {
        if (lExternalMapDetails.mComputed == true)
        {
            lPP.Set("Map_TotalDies", lExternalMapDetails.mTotalDies);
            lPP.Set("Map_STDF_DiesLocMatching", lExternalMapDetails.mTotalMatchingMapDiesLoc );
            // KLA=Pass, STDF = Fail
            lPP.Set("Map_STDF_MismatchingMapPassBin", lExternalMapDetails.mTotalMismatchingMapPassBin );
            // KLA=Fail, STDF = Pass
            lPP.Set("Map_STDF_MismatchingMapFailBin", lExternalMapDetails.mTotalMismatchingMapFailBin );
            // number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map
            lPP.Set("GoodPartsMap_DPAT_Failures", lExternalMapDetails.mGoodPartsMapDPATFailures );
            // number of good die from the input wafer map that do not have matching STDF data
            lPP.Set("GoodPartsMap_STDF_Missing", lExternalMapDetails.mGoodPartsMapSTDFMissing );
        }
        lPP.Set("Stdf_Rows", pFile->getWaferMapData().SizeX);
        lPP.Set("Stdf_Columns", pFile->getWaferMapData().SizeY);
        if (lPatInfo)
        {
            lPP.Set("Stdf_TotalDies", QString::number(lPatInfo->GetSTDFTotalDies()));
            lPP.Set("Stdf_PartsWithoutDatalog", QString::number(lPatInfo->lPartsWithoutDatalog));
        }
        // Report wafermap type: Wafer or Strip
        lPP.Set("Stdf_WafermapType",
                           (pFile->getWaferMapData().bStripMap) ? QString("StripMap") : QString("WaferMap"));
        if (lPatInfo)
        {
            // Bin classes
            CBinning* ptBinCell = lPatInfo->GetSTDFSoftBins();
            QString lBinOriginalName;
            while (ptBinCell)
            {
                lBinOriginalName += ptBinCell->strBinName + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("BinName_Original", lBinOriginalName);

            // Bin ID
            ptBinCell = lPatInfo->GetSTDFSoftBins();
            QString lBinNumberOriginal;
            while (ptBinCell)
            {
                lBinNumberOriginal += QString::number(ptBinCell->iBinValue) + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("Bin#_Original", lBinNumberOriginal);

            // Bin count
            ptBinCell = lPatInfo->GetSTDFSoftBins();
            QString lBinTotalCount;
            while (ptBinCell)
            {
                lBinTotalCount += QString::number(ptBinCell->ldTotalCount) + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("BinCount_Original", lBinTotalCount);

            // Bin name
            ptBinCell = lPatInfo->GetSoftBins();
            lBinOriginalName = "";
            while (ptBinCell)
            {
                lBinOriginalName += ptBinCell->strBinName + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("BinName_PAT", lBinOriginalName);

            // Bin#
            ptBinCell = lPatInfo->GetSoftBins();
            lBinNumberOriginal = "";
            while (ptBinCell)
            {
                lBinNumberOriginal += QString::number(ptBinCell->iBinValue) + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("Bin#_PAT", lBinNumberOriginal);

            // Bin count
            ptBinCell = lPatInfo->GetSoftBins();
            lBinTotalCount = "";
            while (ptBinCell)
            {
                lBinTotalCount += QString::number(ptBinCell->ldTotalCount) + ",";
                ptBinCell = ptBinCell->ptNextBin;
            };
            lPP.Set("BinCount_PAT", lBinTotalCount);

            lPP.Set(GS::Gex::PATProcessing::sKeyTotalGoodBeforePAT, lPatInfo->GetTotalGoodPartsPrePAT());
            lPP.Set("TotalParts", (unsigned)lPatInfo->GetSTDFTotalDies());
            lPP.Set("TotalGoodAfterPAT", lPatInfo->GetTotalGoodPartsPostPAT());
            lPP.Set("TotalPATFails", lPatInfo->GetTotalPATFailingParts());
            lPP.Set("PPATFails", lPatInfo->GetTotalPPATFailingParts());
            lPP.Set("MVPATFails", lPatInfo->GetMVPATPartCount());
            lPP.Set("NNRPATFails", lPatInfo->mNNROutliers.count());
            lPP.Set("IDDQPATFails", lPatInfo->mIDDQOutliers.count());
            lPP.Set("GDBNPATFails", lPatInfo->mGDBNOutliers.count());
            lPP.Set("ClusteringPATFails", lPatInfo->mClusteringOutliers.count());
            lPP.Set("ZPATFails", lPatInfo->mZPATOutliers.count());
            lPP.Set("ReticlePATFails", lPatInfo->mReticleOutliers.count());
        }
    }
}

/******************************************************************************!
 * \fn CreatePatLogFile
 ******************************************************************************/
void GS::Gex::SchedulerEngine::CreatePatLogFile(GS::Gex::PATProcessing &lPATProcessing,
                                                const GS::Gex::PAT::ExternalMapDetails& lExternalMapDetails)
{
    GSLOG(SYSLOG_SEV_NOTICE, QString("Create PAT log file: %1 %2")
           .arg(lPATProcessing.strLogFilePath)
           .arg(lPATProcessing.m_strLogFileName).toLatin1().data() );
    // Check if log file disabled
    if(lPATProcessing.strLogFilePath.isEmpty())
        return;

    // Check where to create the log file.
    QFileInfo cFileInfo(lPATProcessing.strTriggerFile);
    if(lPATProcessing.strLogFilePath == ".")
    {
        // Extract path from trigger file processed
        lPATProcessing.m_strLogFileName = cFileInfo.absolutePath();
    }
    else
        lPATProcessing.m_strLogFileName = lPATProcessing.strLogFilePath;

    // Add original trigger name + '.log' extension...unless string already includes full log file name!
    QDir cDir;
    if(cDir.exists(lPATProcessing.m_strLogFileName) == true)
    {
        if(lPATProcessing.m_strLogFileName.endsWith("/") == false)
            lPATProcessing.m_strLogFileName += "/";
        lPATProcessing.m_strLogFileName +=  cFileInfo.completeBaseName();   //Qt3: cFileInfo.baseName(true);
        lPATProcessing.m_strLogFileName += ".log";
    }

    lPATProcessing.m_strTemporaryLog = lPATProcessing.m_strLogFileName + ".tmp";

    // Open Log file
    lPATProcessing.m_LogFile.setFileName(lPATProcessing.m_strTemporaryLog);
    if(!lPATProcessing.m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;	// Failed creating log file

    // Get current date & time
    QDateTime dt = GS::Gex::Engine::GetInstance().GetClientDateTime();

    // Assign file I/O stream
    lPATProcessing.m_hLogFile.setDevice(&lPATProcessing.m_LogFile);

    // Write log file
    lPATProcessing.m_hLogFile << "# PAT-Man log file" << endl;
    lPATProcessing.m_hLogFile << "#" << endl;
    lPATProcessing.m_hLogFile << "# Global info" << endl;
    lPATProcessing.m_hLogFile << "Date," << dt.toString("dd MMMM yyyy hh:mm:ss") << endl;	// date: '21 January 2006 21:23:05'
    lPATProcessing.Set("Date", dt.toString("dd MMMM yyyy hh:mm:ss"));

    // PAT-Man version used
    lPATProcessing.m_hLogFile << "PatmanRev,"
        << lPATProcessing.Get("PatmanRev").toString() << endl;

    // PAT log file
    lPATProcessing.m_hLogFile << "Action,PAT" << endl;

    // PAT Error message (if any)
    lPATProcessing.m_hLogFile << "ErrorMessage," << lPATProcessing.Get("ErrorMessage").toString() << endl;

    // If file processed, report info
    CGexGroupOfFiles	*pGroup=NULL;
    CGexFileInGroup		*pFile=NULL;
    bool				bRecipeExists   = QFile::exists(lPATProcessing.strRecipeFile);
    CPatInfo *          lPatInfo        = PATEngine::GetInstance().GetContext();

    if(gexReport && gexReport->getGroupsList().count())
    {
        // Get pointer to first group & first file (we always have them exist)
        pGroup = (gexReport->getGroupsList().isEmpty())?NULL:gexReport->getGroupsList().first();
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // Output datalog file
        QString strOutputDatalogFile;
        switch(lPATProcessing.mOutputDatalogFormat)
        {
            case GEX_TPAT_DLG_OUTPUT_NONE:
                strOutputDatalogFile = "none";
                break;

            case GEX_TPAT_DLG_OUTPUT_STDF:
//                strOutputDatalogFile = pFile->strFileName;
                strOutputDatalogFile = lPatInfo->GetOutputDataFilename();
                break;

            case GEX_TPAT_DLG_OUTPUT_ATDF:
//                QFileInfo cFileInfo(pFile->strFileName);
//                strOutputDatalogFile=cFileInfo.dirPath() + "/" + cFileInfo.baseName(true) + ".atd";
                strOutputDatalogFile = lPatInfo->GetOutputDataFilename();
                break;
        }


        // List Lot details
        // STDF file processed.
        lPATProcessing.m_hLogFile << "DataOutput," << strOutputDatalogFile << endl;
        lPATProcessing.Set("DataOutput", strOutputDatalogFile);
        // Map file
        if(QFile::exists(lPATProcessing.strOptionalSource))
        {
            lPATProcessing.m_hLogFile << "OptionalOutput," << lPATProcessing.strOptionalOutput << endl;
            lPATProcessing.Set("OptionalOutput", lPATProcessing.strOptionalOutput);
        }

        lPATProcessing.m_hLogFile << "SetupTime," << lPATProcessing.Get("SetupTime").toString();
        lPATProcessing.m_hLogFile << "StartTime," << lPATProcessing.Get("StartTime").toString();
        lPATProcessing.m_hLogFile << "EndTime,"<< lPATProcessing.Get("EndTime").toString();

        if(!lPATProcessing.m_strProductName.isEmpty())
        {
            lPATProcessing.m_hLogFile << "Product," << lPATProcessing.m_strProductName << endl;
            lPATProcessing.Set("Product", lPATProcessing.m_strProductName);
        }
        else
        {
            lPATProcessing.m_hLogFile << "Product," << lPATProcessing.Get("Product").toString() << endl;
        }
        lPATProcessing.m_hLogFile << "DesignRev," << lPATProcessing.Get("DesignRev").toString() << endl;
        lPATProcessing.m_hLogFile << "Program," << lPATProcessing.Get("Program").toString() << endl;
        lPATProcessing.m_hLogFile << "ProgramRev," << lPATProcessing.Get("ProgramRev").toString() << endl;
        lPATProcessing.m_hLogFile << "Lot," << lPATProcessing.Get("Lot").toString() << endl;
        lPATProcessing.m_hLogFile << "SubLot," << lPATProcessing.Get("SubLot").toString() << endl;
        lPATProcessing.m_hLogFile << "WaferID," << lPATProcessing.Get("WaferID").toString() << endl;

         if(QFile::exists(gexReport->reportAbsFilePath()))
         {
            lPATProcessing.m_hLogFile << "Report," << lPATProcessing.Get("Report").toString() << endl;  // Report created
         }
         else
         {
            lPATProcessing.m_hLogFile << "Report,<none>" << endl;  // No report available
         }
    }

    // Reports if Yield alarm
    QString lPatAlarm("No");

    if(lPATProcessing.mPatAlarm)
        lPatAlarm = "Yes";
    lPATProcessing.m_hLogFile << "PatAlarm," << lPatAlarm << endl;
    lPATProcessing.Set("PatAlarm", lPatAlarm);


    // Report Yield threshod
    lPATProcessing.m_hLogFile << "PatYieldLimit," << lPATProcessing.mYieldThreshold << endl;
    lPATProcessing.Set("PatYieldLimit", lPATProcessing.mYieldThreshold);

    // Report Yield loss limit origin
    bool    lOk     = false;
    int     lSource = lPATProcessing.property(GS::PAT::Trigger::sPATYieldLimitSource).toInt(&lOk);

    lPATProcessing.m_hLogFile << "PatYieldLimitSource,";

    if (lOk)
    {
        switch (lSource)
        {
            case GS::Gex::PATProcessing::PYLFromRecipe:
                lPATProcessing.m_hLogFile << "Recipe" << endl;
                break;

            case GS::Gex::PATProcessing::PYLFromTrigger:
                lPATProcessing.m_hLogFile << "TriggerFile" << endl;
                break;

            case GS::Gex::PATProcessing::PYLFromTask:
                lPATProcessing.m_hLogFile << "OutlierRemovalTask[";
                lPATProcessing.m_hLogFile << lPATProcessing.property(GS::PAT::Trigger::sPATOutlierRemovalTask).toString();
                lPATProcessing.m_hLogFile << "]" << endl;
                break;

            default:
                lPATProcessing.m_hLogFile << "Unknown" << endl;
                break;
        }
    }
    else
        lPATProcessing.m_hLogFile << "Unknown" << endl;

    if (bRecipeExists)
    {
        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "# Recipe details" << endl;
        lPATProcessing.m_hLogFile << "RecipeFile," << lPATProcessing.strRecipeFile << endl;
        lPATProcessing.Set("RecipeFile", lPATProcessing.strRecipeFile);
        if (lPatInfo)
        {
            lPATProcessing.m_hLogFile << "SPAT_FailBin," << lPATProcessing.Get("SPAT_FailBin").toString() << endl;
            lPATProcessing.m_hLogFile << "DPAT_FailBin," << lPATProcessing.Get("DPAT_FailBin").toString() << endl;
            lPATProcessing.m_hLogFile << "MVPAT_FailBin," << lPATProcessing.Get("MVPAT_FailBin").toString() << endl;
            lPATProcessing.m_hLogFile << "DPAT_NNR," << lPATProcessing.Get("DPAT_NNR").toString() << endl;
            lPATProcessing.m_hLogFile << "DPAT_BadNeighbor," << lPATProcessing.Get("DPAT_BadNeighbor").toString()
                << endl;
            lPATProcessing.m_hLogFile << "DPAT_Clustering," << lPATProcessing.Get("DPAT_Clustering").toString() << endl;
            lPATProcessing.m_hLogFile << "DPAT_Reticle," << lPATProcessing.Get("DPAT_Reticle").toString() << endl;
            lPATProcessing.m_hLogFile << "DPAT_ZPAT," << lPATProcessing.Get("DPAT_ZPAT").toString() << endl;
        }
        else
            lPATProcessing.m_hLogFile << "# No active PAT done/found"<<endl;
    }

    // If composite input file defined, list it
    if(lPATProcessing.strCompositeFile.isEmpty() == false)
    {
        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "# Composite multi-wafer PAT (holds exclusion zones, 3D neighborhood)" << endl;
        lPATProcessing.m_hLogFile << "CompositeFile," << lPATProcessing.strCompositeFile << endl;
        lPATProcessing.Set("CompositeFile", lPATProcessing.strCompositeFile);
    }

    // If Reticle Location input file defined, list it
    if(lPATProcessing.mReticleStepInfo.isEmpty() == false)
    {
        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "# Reticle Step Information:" << endl;
        lPATProcessing.m_hLogFile << "ReticleStepInfo," << lPATProcessing.mReticleStepInfo << endl;
        lPATProcessing.Set("ReticleStepInfo", lPATProcessing.mReticleStepInfo);
    }

    // If file processed
    if(bRecipeExists && (pFile!=NULL))
    {
        if(lExternalMapDetails.mComputed == true)
        {
            lPATProcessing.m_hLogFile << "#" << endl;
            lPATProcessing.m_hLogFile << "# Optional Input WaferMap details" << endl;
            lPATProcessing.m_hLogFile << "Map_TotalDies," << lPATProcessing.Get("Map_TotalDies").toString() << endl;
            lPATProcessing.m_hLogFile << "Map_STDF_DiesLocMatching,"
                << lPATProcessing.Get("Map_STDF_DiesLocMatching").toString() << endl;
            // KLA=Pass, STDF = Fail
            lPATProcessing.m_hLogFile << "Map_STDF_MismatchingMapPassBin,"
                << lPATProcessing.Get("Map_STDF_MismatchingMapPassBin").toString() << endl;
            // KLA=Fail, STDF = Pass
            lPATProcessing.m_hLogFile << "Map_STDF_MismatchingMapFailBin,"
                << lPATProcessing.Get("Map_STDF_MismatchingMapFailBin").toString() << endl;
            // number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
            lPATProcessing.m_hLogFile << "GoodPartsMap_DPAT_Failures,"
                << lPATProcessing.Get("GoodPartsMap_DPAT_Failures").toString() <<endl;
            // number of good die from the input wafer map that do not have matching STDF data.
            lPATProcessing.m_hLogFile << "GoodPartsMap_STDF_Missing,"
                << lPATProcessing.Get("GoodPartsMap_STDF_Missing").toString() << endl;
            /////////////////////////////////////////////////////////////////////////
            // If optional Input MAP, report its binnings...
            /////////////////////////////////////////////////////////////////////////
            if(lExternalMapDetails.mBinCountBeforePAT.count() > 0)
            {
                QMap<int,int>::ConstIterator itMap;

                lPATProcessing.m_hLogFile << "#" << endl;
                lPATProcessing.m_hLogFile << "# Input MAP Original Binning info (Before PAT applied)" << endl;
                // Display Bin classes
                lPATProcessing.m_hLogFile << "Map_Bin#_Original,";
                for ( itMap = lExternalMapDetails.mBinCountBeforePAT.constBegin();
                      itMap != lExternalMapDetails.mBinCountBeforePAT.constEnd(); ++itMap )
                {
                    // Display Bin ID
                    lPATProcessing.m_hLogFile << itMap.key() << ",";

                }
                lPATProcessing.m_hLogFile <<  endl;

                // Display Bin count
                lPATProcessing.m_hLogFile << "Map_BinCount_Original,";
                for ( itMap = lExternalMapDetails.mBinCountBeforePAT.constBegin();
                      itMap != lExternalMapDetails.mBinCountBeforePAT.constEnd(); ++itMap )
                {
                    lPATProcessing.m_hLogFile << itMap.value() << ",";
                }
                lPATProcessing.m_hLogFile << endl;
            }

            if(lExternalMapDetails.mBinCountAfterPAT.count() > 0)
            {
                QMap<int,int>::ConstIterator itMap;

                lPATProcessing.m_hLogFile << "#" << endl;
                lPATProcessing.m_hLogFile << "# Input MAP Original Binning info (After PAT applied)" << endl;
                // Display Bin classes
                lPATProcessing.m_hLogFile << "Map_Bin#_PAT,";
                for ( itMap = lExternalMapDetails.mBinCountAfterPAT.constBegin();
                      itMap != lExternalMapDetails.mBinCountAfterPAT.constEnd(); ++itMap )
                {
                    // Display Bin ID
                    lPATProcessing.m_hLogFile << itMap.key() << ",";
                }
                lPATProcessing.m_hLogFile << endl;

                // Display Bin count
                lPATProcessing.m_hLogFile << "Map_BinCount_PAT,";
                for ( itMap = lExternalMapDetails.mBinCountAfterPAT.constBegin();
                      itMap != lExternalMapDetails.mBinCountAfterPAT.constEnd(); ++itMap )
                {
                    lPATProcessing.m_hLogFile << itMap.value() << ",";
                }
                lPATProcessing.m_hLogFile << endl;
            }
        }

        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "# STDF Wafer details" << endl;
        if (pFile)
        {
            lPATProcessing.m_hLogFile << "Stdf_Rows," << lPATProcessing.Get("Stdf_Rows").toString() << endl;
            lPATProcessing.m_hLogFile << "Stdf_Columns,"<< lPATProcessing.Get("Stdf_Columns").toString() << endl;
        }
        if (lPatInfo)
        {
            lPATProcessing.m_hLogFile << "Stdf_TotalDies," << lPATProcessing.Get("Stdf_TotalDies").toString() << endl;
            lPATProcessing.m_hLogFile << "Stdf_PartsWithoutDatalog,"
                << lPATProcessing.Get("Stdf_PartsWithoutDatalog").toString() << endl;
        }

        // Get pointer to first group & first file (we always have them exist)
        pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
        if (pGroup)
            pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        lPATProcessing.m_hLogFile << "Stdf_WafermapType," << lPATProcessing.Get("Stdf_WafermapType").toString() << endl;

        if(lPATProcessing.iGrossDiePerWafer > 0)
            lPATProcessing.m_hLogFile << "Stdf_GrossDiesPerWafer," << lPATProcessing.iGrossDiePerWafer << endl;

        if(lPATProcessing.strWaferFileFullName.isEmpty() == false)
            lPATProcessing.m_hLogFile << "WafMapFile," << lPATProcessing.strWaferFileFullName << endl;	// Wafermap file created (if any)

        lPATProcessing.m_hLogFile << "#" << endl;

        if (lPatInfo)
        {
            lPATProcessing.m_hLogFile << "# STDF Original Binning info (Before PAT applied)" << endl;
            lPATProcessing.m_hLogFile << "BinName_Original," << lPATProcessing.Get("BinName_Original").toString() << endl;
            lPATProcessing.m_hLogFile << "Bin#_Original," << lPATProcessing.Get("Bin#_Original").toString() << endl;
            lPATProcessing.m_hLogFile << "BinCount_Original," << lPATProcessing.Get("BinCount_Original").toString() << endl;
            lPATProcessing.m_hLogFile << "#" << endl;
            lPATProcessing.m_hLogFile << "# STDF Binning info (After PAT applied)" << endl;
            lPATProcessing.m_hLogFile << "BinName_PAT," << lPATProcessing.Get("BinName_PAT").toString() << endl;
            lPATProcessing.m_hLogFile << "Bin#_PAT," << lPATProcessing.Get("Bin#_PAT").toString() << endl;
            lPATProcessing.m_hLogFile << "BinCount_PAT," << lPATProcessing.Get("BinCount_PAT").toString() << endl;
        }

        if (lPatInfo)
        {
            // Include few yield related info
            lPATProcessing.m_hLogFile << "#" << endl;
            lPATProcessing.m_hLogFile << "# Yield details" << endl;
            // Total STDF good dies
            lPATProcessing.m_hLogFile << "GoodParts_BeforePAT," << lPatInfo->GetTotalGoodPartsPrePAT() << endl;
            // Total STDF bad dies
            lPATProcessing.m_hLogFile << "GoodParts_AfterPAT,"  << lPatInfo->GetTotalGoodPartsPostPAT() << endl;
            // Total DPAT & SPAT dies.
            lPATProcessing.m_hLogFile << "GoodParts_DPAT_Failures,"  << lPatInfo->GetTotalPPATFailingParts() << endl;
            // Total MVPAT dies.
            lPATProcessing.m_hLogFile << "GoodParts_MVPAT_Failures,"  << lPatInfo->GetMVOutliers().count() << endl;
            // Total NNR rejects
            lPATProcessing.m_hLogFile << "GoodParts_NNR_Failures,"  << lPatInfo->mNNROutliers.count() << endl;
            // Total IDDQ rejects
            lPATProcessing.m_hLogFile << "GoodParts_IDDQ_Failures,"  << lPatInfo->mIDDQOutliers.count() << endl;
            // Total Good die in bad neigbhorhood
            lPATProcessing.m_hLogFile << "GoodParts_GDBN_Failures,"  << lPatInfo->mGDBNOutliers.count() << endl;
            // Total Good die rejected by Reticle rule
            lPATProcessing.m_hLogFile << "GoodParts_Reticle_Failures,"  << lPatInfo->mReticleOutliers.count() << endl;
            // Total Good die rejected by Clustering 'Potato' rule
            lPATProcessing.m_hLogFile << "GoodParts_Clustering_Failures,"  << lPatInfo->mClusteringOutliers.count() << endl;
            // Total Good die rejected by Clustering 'Potato' rule
            lPATProcessing.m_hLogFile << "GoodParts_ZPAT_Failures,"  << lPatInfo->mZPATOutliers.count() << endl;

            /////////////////////////////////////////////////////////////////////////
            // Include some various info
            /////////////////////////////////////////////////////////////////////////
            lPATProcessing.m_hLogFile << "#" << endl;
            lPATProcessing.m_hLogFile << "# Parameter details" << endl;
            QStringList strlUnknownShapeTests;
            long	iTotalUnknownShapes=0;
            int		iIndex;

            tdIterPATDefinitions    it(lPatInfo->GetUnivariateRules());
            const CPatDefinition *  ptPatDef = NULL;
            QList <int>             lSites;

            while(it.hasNext())
            {
                ptPatDef = it.next().value();

                lSites = ptPatDef->GetDynamicSites();
                for(iIndex = 0;iIndex < lSites.count(); iIndex++)
                {
                    // Check if this test & site has Distribution shape = Unknown
                    if(ptPatDef->mDynamicLimits[lSites.at(iIndex)].mDistributionShape == PATMAN_LIB_SHAPE_UNKNOWN)
                    {
                        strlUnknownShapeTests += ",T" + QString::number(ptPatDef->m_lTestNumber);
                    }
                }
            }
            strlUnknownShapeTests.removeDuplicates();	// Ensure no test duplicates.
            // Total DPAT Tests processed that had a Unknown shape distribution
            lPATProcessing.m_hLogFile << "DPAT_Unknown_Shape," << iTotalUnknownShapes;
            for(iIndex=0;iIndex<strlUnknownShapeTests.count();iIndex++)
                lPATProcessing.m_hLogFile << strlUnknownShapeTests[iIndex];
            lPATProcessing.m_hLogFile << endl;

            // Report DPAT dies + Tests details
            if(lPATProcessing.bLogDPAT_Details)
            {
                // Header text
                lPATProcessing.m_hLogFile << "#" << endl;
                lPATProcessing.m_hLogFile << "# DPAT Test details" << endl;
                lPATProcessing.m_hLogFile << "# DPAT_Die_Details,,Part ID,Bin,DieX,DieY,Site,Tests" << endl;

                QList<CPatFailingTest>::iterator itPart;
                CPatFailingTest			cFailTest;
                tdIterPatOutlierParts	itOutlierParts(lPatInfo->m_lstOutlierParts);
                CPatOutlierPart *		pOutlierPart = NULL;

                while(itOutlierParts.hasNext())
                {
                    pOutlierPart = itOutlierParts.next();

                    lPATProcessing.m_hLogFile << "DPAT_Die_Details,";
                    lPATProcessing.m_hLogFile << pOutlierPart->strPartID << ",";
                    lPATProcessing.m_hLogFile << QString::number(pOutlierPart->iPatSBin) << ",";
                    lPATProcessing.m_hLogFile << QString::number(pOutlierPart->iDieX) << ",";
                    lPATProcessing.m_hLogFile << QString::number(pOutlierPart->iDieY) << ",";
                    lPATProcessing.m_hLogFile << QString::number(pOutlierPart->iSite) << ",";

                    for ( itPart = pOutlierPart->cOutlierList.begin(); itPart != pOutlierPart->cOutlierList.end(); ++itPart )
                    {
                        cFailTest = *itPart;
                        lPATProcessing.m_hLogFile << "T" << cFailTest.mTestNumber;

                        if (cFailTest.mPinIndex >= 0)
                            lPATProcessing.m_hLogFile << cFailTest.mPinIndex;

                        lPATProcessing.m_hLogFile << ",";
                    }

                    lPATProcessing.m_hLogFile << endl;
                };
            }

            // Report NNR dies + Tests details
            if(lPATProcessing.bLogNNR_Details)
            {
                tdIterGPATOutliers                  itNNR(lPatInfo->mNNROutliers);
                CPatOutlierNNR *					ptNRR_OutlierPart   = NULL;
                int	iSite,iTestCount;

                // Header text
                lPATProcessing.m_hLogFile << "#" << endl;
                lPATProcessing.m_hLogFile << "# NNR Test details" << endl;
                lPATProcessing.m_hLogFile << "# NNR_Die_Details,,Part ID,Bin,DieX,DieY,Site,Tests" << endl;

                while (itNNR.hasNext())
                {
                    // Scan list of NNR outlier tests...and find the one matching this die location.
                    // Keep track of NNR test failures in same die.
                    iTestCount = 0;
                    foreach(ptNRR_OutlierPart, lPatInfo->pNNR_OutlierTests)
                    {
                        if(ptNRR_OutlierPart->mDieX == itNNR.value().mDieX &&
                           ptNRR_OutlierPart->mDieY == itNNR.value().mDieY)
                        {
                            // Keep track of total NNR failures in same die
                            iTestCount++;

                            // Get site# used for testing given die.
                            iSite = gexReport->getSiteForDie(ptNRR_OutlierPart->mDieX, ptNRR_OutlierPart->mDieY);

                            // Report test details failing NNR for that die
                            if(iTestCount == 1)
                            {
                                // First NNR test exception in die.
                                lPATProcessing.m_hLogFile << "NNR_Die_Details,";
                                lPATProcessing.m_hLogFile << QString::number(ptNRR_OutlierPart->mDieX) << ",";
                                lPATProcessing.m_hLogFile << QString::number(ptNRR_OutlierPart->mDieY) << ",";
                                lPATProcessing.m_hLogFile << QString::number(iSite) << ",";
                            }
                            // Write test info
                            lPATProcessing.m_hLogFile << "T" << ptNRR_OutlierPart->mTestNumber << ",";
                        }
                    }
                    lPATProcessing.m_hLogFile << endl;
                }
            }

            // Report IDDQ-Delta dies + Tests details
            if(lPATProcessing.bLogIDDQ_Delta_Details)
            {
                tdIterGPATOutliers      itIDDQ(lPatInfo->mIDDQOutliers);
                CPatOutlierIDDQ_Delta * ptIDDQ_OutlierPart  = NULL;
                int                     iSite;
                int                     iTestCount;

                // Header text
                lPATProcessing.m_hLogFile << "#" << endl;
                lPATProcessing.m_hLogFile << "# IDDQ-Delta Test details" << endl;
                lPATProcessing.m_hLogFile << "# IDDQ_Delta_Die_Details,,Part ID,Bin,DieX,DieY,Site,Tests" << endl;

                while (itIDDQ.hasNext())
                {
                    itIDDQ.next();

                    // Scan list of IDDQ-Delta outlier tests...and find the one matching this die location.
                    // Keep track of IDDQ-Delta test failures in same die.
                    iTestCount = 0;
                    foreach(ptIDDQ_OutlierPart, lPatInfo->pIDDQ_Delta_OutlierTests)
                    {
                        if(ptIDDQ_OutlierPart->iDieX == itIDDQ.value().mDieX &&
                           ptIDDQ_OutlierPart->iDieY == itIDDQ.value().mDieY)
                        {
                            // Keep track of total IDDQ-Delta failures in same die
                            iTestCount++;

                            // Get site# used for testing given die.
                            iSite = gexReport->getSiteForDie(ptIDDQ_OutlierPart->iDieX,
                                                             ptIDDQ_OutlierPart->iDieY);

                            // Report test details failing NNR for that die
                            if(iTestCount == 1)
                            {
                                // First NNR test exception in die.
                                lPATProcessing.m_hLogFile << "IDDQ_Delta_Die_Details,";
                                lPATProcessing.m_hLogFile << QString::number(ptIDDQ_OutlierPart->iDieX) << ",";
                                lPATProcessing.m_hLogFile << QString::number(ptIDDQ_OutlierPart->iDieY) << ",";
                                lPATProcessing.m_hLogFile << QString::number(iSite) << ",";
                            }
                            // Write test-pair info
                            lPATProcessing.m_hLogFile << "T" << ptIDDQ_OutlierPart->lTestNumber1 << " / ";
                            lPATProcessing.m_hLogFile << "T" << ptIDDQ_OutlierPart->lTestNumber1 << ",";
                        }
                    }
                    lPATProcessing.m_hLogFile << endl;
                }
            }
        } // if PatInfo
    }// If file processed


    // Add contents of trigger file for traceability purpose
    QFile cTriggerFile(lPATProcessing.strTriggerFile);
    if(cTriggerFile.open(QIODevice::ReadOnly))
    {
        // Assign file I/O stream
        QTextStream hTriggerFile(&cTriggerFile);
        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "#" << endl;
        lPATProcessing.m_hLogFile << "# Trigger file processed: " << lPATProcessing.strTriggerFile << endl;
        lPATProcessing.m_hLogFile << "#" << endl;
        do
            lPATProcessing.m_hLogFile << "# " << hTriggerFile.readLine() << endl;
        while(hTriggerFile.atEnd() == false);
        hTriggerFile.setDevice(0);
        cTriggerFile.close();
    }

    // Close file
    lPATProcessing.m_hLogFile.setDevice(0);
    lPATProcessing.m_LogFile.close();

    // Rename log file to final name
    cDir.remove(lPATProcessing.m_strLogFileName);
    cDir.rename(lPATProcessing.m_strTemporaryLog,lPATProcessing.m_strLogFileName);
}

#endif
///////////////////////////////////////////////////////////
// Create Composite PAT Log file (if enabled)
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::CreateCompositePatLogFile(CGexCompositePatProcessing &lCompositePatProcess)
{
    // Check if log file disabled
    if(lCompositePatProcess.strLogFilePath.isEmpty())
        return;

    // Check where to create the log file.
    QFileInfo cFileInfo(lCompositePatProcess.strTriggerFile);
    if(lCompositePatProcess.strLogFilePath == ".")
    {
        // Extract path from trigger file processed
        lCompositePatProcess.m_strLogFileName = cFileInfo.absolutePath();
    }
    else
        lCompositePatProcess.m_strLogFileName = lCompositePatProcess.strLogFilePath;

    // Add original trigger name + '.log' extension
    if(lCompositePatProcess.m_strLogFileName.endsWith("/") == false)
        lCompositePatProcess.m_strLogFileName += "/";
    lCompositePatProcess.m_strLogFileName += cFileInfo.completeBaseName();
    lCompositePatProcess.m_strLogFileName += ".log";

    lCompositePatProcess.m_strTemporaryLog = lCompositePatProcess.m_strLogFileName + ".tmp";

    // Open Log file
    lCompositePatProcess.m_LogFile.setFileName(lCompositePatProcess.m_strTemporaryLog);
    if(!lCompositePatProcess.m_LogFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;	// Failed creating log file

    // Assign file I/O stream
    lCompositePatProcess.m_hLogFile.setDevice(&lCompositePatProcess.m_LogFile);

    // Write log file
    lCompositePatProcess.m_hLogFile << "# PAT-Man log file" << endl;

    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Get curret date & time
    QDateTime dt = GS::Gex::Engine::GetInstance().GetClientDateTime();

    // List Lot details
    lCompositePatProcess.m_hLogFile << "#" << endl;
    lCompositePatProcess.m_hLogFile << "# Global info" << endl;
    lCompositePatProcess.m_hLogFile << "Date," << dt.toString("dd MMMM yyyy hh:mm:ss") << endl;	// date: '21 January 2006 21:23:05'
    lCompositePatProcess.setProperty("Date", dt.toString("dd MMMM yyyy hh:mm:ss"));

    // PAT-Man version used
    lCompositePatProcess.m_hLogFile << "PatmanRev,"
                       << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
    lCompositePatProcess.setProperty("PatmanRev", GS::Gex::Engine::GetInstance().Get("AppFullName"));

    // COMPOSITE PAT log file
    lCompositePatProcess.m_hLogFile << "Action,COMPOSITE_PAT" << endl;
    lCompositePatProcess.setProperty("Action", "COMPOSITE_PAT");

    // Product ID processed.
    QString lProduct = pFile->getMirDatas().szPartType;
    if(!lCompositePatProcess.strProductName.isEmpty())
        lProduct = lCompositePatProcess.strProductName;
    lCompositePatProcess.m_hLogFile << "Product," << lProduct << endl;
    lCompositePatProcess.setProperty("Product", lProduct);

     // Lot ID processed.
    lCompositePatProcess.m_hLogFile << "LotID," << pFile->getMirDatas().szLot << endl;
    lCompositePatProcess.setProperty("LotID", pFile->getMirDatas().szLot);
}

///////////////////////////////////////////////////////////
// Close Composite PAT Log file (if enabled)
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::CloseCompositePatLogFile(CGexCompositePatProcessing &cFields)
{
    cFields.m_hLogFile << endl << "###################### END OF FILE ########################" << endl;

    // Close file
    cFields.m_hLogFile.setDevice(0);
    cFields.m_LogFile.close();

    // Rename log file to final name
    QDir cDir;
    cDir.remove(cFields.m_strLogFileName);
    cDir.rename(cFields.m_strTemporaryLog,cFields.m_strLogFileName);
}

///////////////////////////////////////////////////////////
// Create Composite PAT Report file (if enabled)
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::CreateCompositePatReportFile(CGexCompositePatProcessing &cFields)
{
    // Get pointer to first group & first file (we always have them exist)
    CGexGroupOfFiles *pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    CGexFileInGroup *pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

    // Create
    gexReport->buildReportArborescence(&ReportOptions,cFields.strOutputReportFormat);

    // Get reportname created by Examinator (usually <path>/index.htm)
    cFields.m_strReportFileName = gexReport->reportAbsFilePath();
    cFields.m_strTemporaryReport = cFields.m_strReportFileName + ".tmp";

    // Get handle to Report File  created
    cFields.m_hReportFile = gexReport->getReportFile();
    if(cFields.m_hReportFile == NULL)
        return;	// No handle: no report to be created! (eg: PAT processing a lot with Z-PAT)

    // Write HTML header, do NOT write standard Examinator TOC (table of contents) as we have a custom one to write.
    if( ReportOptions.isReportOutputHtmlBased() )
        gexReport->WriteHeaderHTML(cFields.m_hReportFile,"#000000","#FFFFFF","",true,false);	// Default: Text is Black

    // If PowerPoint slide to create, set its name
    QString strString = "PAT-Man: Composite Wafer-Lot";
    gexReport->SetPowerPointSlideName(strString);

    // Write HTML title
    gexReport->WriteHtmlSectionTitle(cFields.m_hReportFile, strString, strString);

    // List Lot details
    gexReport->WriteHtmlOpenTable(98,2);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)

    gexReport->WriteInfoLine("PatmanRev",
                             GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );		// PAT-Man version used
    gexReport->WriteInfoLine("Action","COMPOSITE_PAT");			// COMPOSITE PAT report file
    // Product ID processed.
    if(!cFields.strProductName.isEmpty())
        gexReport->WriteInfoLine("Product", cFields.strProductName.toLatin1().constData());
    else
        gexReport->WriteInfoLine("Product", pFile->getMirDatas().szPartType);
    // Lot ID processed.
    gexReport->WriteInfoLine("LotID",pFile->getMirDatas().szLot);

    // Close table
    fprintf(cFields.m_hReportFile,"</table>\n<br>\n");

    // Write table of Hyperlinks.
    QString of=ReportOptions.GetOption("output", "format").toString();
    //if (ReportOptions.iOutputFormat & GEX_OPTION_OUTPUT_HTML_BASED)
    if (ReportOptions.isReportOutputHtmlBased()) //(of=="HTML"||of=="DOC"||of=="PDF"||of=="PPT"||of=="INTERACTIVE")
    {
        fprintf(cFields.m_hReportFile,"<p align=\"left\">Sections created:</p>\n");
        fprintf(cFields.m_hReportFile,"<blockquote>\n");
        fprintf(cFields.m_hReportFile,"<p align=\"left\"><img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#comp_individual\">Individual wafers: STDF and E-test discrepancies</a><br>\n");
        fprintf(cFields.m_hReportFile,"<img border=\"0\" src=\"../images/action.png\" align=\"center\" width=\"17\" height=\"17\"> <a href=\"#comp_exclusion_zone\">Exclusion zone: Dies to reject on all wafers</a><br>\n");
        fprintf(cFields.m_hReportFile,"</blockquote>\n");
    }

    // Write this page as a slide (image)
    gexReport->WritePageBreak();

    // Set bookmark to individual wafers
    fprintf(cFields.m_hReportFile,"<a name=\"comp_individual\"</a>\n");
}

///////////////////////////////////////////////////////////
// Close Composite PAT Report file (if enabled)
///////////////////////////////////////////////////////////
void GS::Gex::SchedulerEngine::CloseCompositePatReportFile(CGexCompositePatProcessing &cFields)
{
    // Write HTML file
    fprintf(cFields.m_hReportFile,"</body>\n");
    fprintf(cFields.m_hReportFile,"</html>\n");

    gexReport->CloseReportFile(cFields.m_hReportFile);
    cFields.m_hReportFile = NULL;

    // Rename HTML Report file to final name
    QDir cDir;
    cDir.rename(cFields.m_strTemporaryReport,cFields.m_strReportFileName);

    // If Non-HTML output (eg: Word, PDF, etc), call relevant converter.
    gexReport->ReportPostProcessingConverter();
}


#ifdef GCORE15334

void GS::Gex::SchedulerEngine::BuildRecipeFileFullName(GS::Gex::PATProcessing &cFields,
                                                       GS::QtLib::DatakeysContent& dbKeysContent)
{
    // If a custom recipe file is specified, use it!
    QString strRecipe = cFields.strRecipeFile.trimmed();
    if((strRecipe.isEmpty() == false) || QFile::exists(strRecipe))
        return;

    QString strProductID = dbKeysContent.Get("Product").toString();
    if(!cFields.m_strProductName.isEmpty())
        strProductID = cFields.m_strProductName;

    // 1: Get file path to the configuration file: <databases>/.patman/<productID>
    cFields.strRecipeFile = GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString();
    cFields.strRecipeFile += GEX_DATABASE_FOLDER;
    cFields.strRecipeFile += GEXMO_PATMAN_FOLDER;
    cFields.strRecipeFile += "/" + strProductID;

    // a) try to find config file: <product>_<jobname>_<jobrevision>
    QString strString = cFields.strRecipeFile;
    strString += "_" + dbKeysContent.Get("ProgramName").toString();
    strString += "_" + dbKeysContent.Get("ProgramRevision").toString();
    strString += "_patman_config.csv";
    if(QFile::exists(strString))
        cFields.strRecipeFile = strString;	// Config file found!
    else
    {
        // b) If config file not found, try:  <product>_<jobname>
        strString = cFields.strRecipeFile;
        strString += "_" + dbKeysContent.Get("ProgramName").toString();
        strString += "_" + dbKeysContent.Get("ProgramRevision").toString();
        strString += "_patman_config.csv";
        if(QFile::exists(strString))
            cFields.strRecipeFile = strString;	// Config file found!
        else
        {
            // c) If config file not found, try:  <product>
            cFields.strRecipeFile += "_patman_config.csv";
            if(QFile::exists(strString) == false)
                cFields.strRecipeFile = "";	// No recipe exist!
        }
    }
}

#endif
///////////////////////////////////////////////////////////
// Merge N*STDF files together...
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::MergeInputFiles(GS::Gex::PATProcessing& cFields,
                                               QString& strTestDataFile,
                                               QString& strErrorMessage)
{
    GexTbMergeRetest lMergeTool(false);

    // Destination file
    QString strOutput = cFields.mOutputFolderSTDF;
    QFileInfo cFileInfo(cFields.strSources.first());
    // Build output folder since not specified...will be same folder as input data files
    if(strOutput.isEmpty())
        strOutput = cFileInfo.absolutePath();
    if(strOutput.endsWith("/") == false)
        strOutput += "/";
    strOutput += cFileInfo.completeBaseName();
    strOutput += "_merged.std";

    // Remove temporary merged STDF file if already exists..
    GS::Gex::Engine::RemoveFileFromDisk(strOutput);

    // Merge files (all samples data, ignore summary sections and HBR/SBR)
    //cMergeDialog.comboBoxMergingMode->setCurrentItem(0);	// Set to sort files to merge by: DATE.
    int	iStatus = lMergeTool.MergeSamplesFiles(cFields.strSources,strOutput,false,true);

    // Check Status and display message accordingly.
    if(iStatus == GexTbMergeRetest::NoError)
    {
        strTestDataFile = strOutput;
        return true;	// Success
    }
    else
    {
        strErrorMessage = lMergeTool.GetErrorMessage();
        return false;	// Error
    }
}

#ifdef GCORE15334

QString GS::Gex::SchedulerEngine::ExecutePatProcessing(GS::Gex::PATProcessing *cFields)
{
    QString lErrorMessage;

    if (cFields)
    {
#ifndef QT_DEBUG
        if (GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        {
#endif
            ExecutePatProcessing(*cFields, lErrorMessage, NULL);

            if (lErrorMessage.isEmpty())
                lErrorMessage = "ok";
            else if (lErrorMessage.startsWith("error", Qt::CaseInsensitive) == false)
                lErrorMessage = "error: " + lErrorMessage;
#ifndef QT_DEBUG
        }
        else
            lErrorMessage = "error: function allowed only in PATMan";
#endif

    }
    else
        lErrorMessage = "error: PATProcessing object null";

    return lErrorMessage;
}

QString GS::Gex::SchedulerEngine::ExecutePatProcessing(
        GS::Gex::PATProcessing &cFields,
        QString &strErrorMessage,
        CGexMoTaskPatPump *lPATPump)
{
    if (GS::Gex::Engine::GetInstance().CheckForLicense()!=0)
    {
       strErrorMessage = "error: bad license";
       GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecutePatProcessing: Error executing PAT processing", strErrorMessage);
       return strErrorMessage;
    }

    if (!GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
    {
       strErrorMessage = "error: function not allowed in this product";
       GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecutePatProcessing: Error executing PAT processing", strErrorMessage);
       return strErrorMessage;
    }

    //if (!isActivated())
      //  return "error: scheduler not activated";

    // Check if multiple input files. If so MERGE them first.
    QString strTestDataFile;
    QString strLocalError, strTemp;
    //FIXME: not used ?
    //bool bMergeStdf=false;

    if (GS::Gex::PATProcessWS::CreateSTDFDataFile(cFields, strTestDataFile, strErrorMessage) == false)
    {
        GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecutePatProcessing: Error executing PAT processing", strErrorMessage);
        return strErrorMessage;
    }

    // Load the structure with the data file MIR info.
    GS::QtLib::DatakeysContent dbKeysContent;
    GS::Gex::Engine::GetInstance().GetDatabaseEngine().ExtractFileKeys(strTestDataFile,dbKeysContent);

    //ExtraType and ExtraName from SDR ?
    cFields.setProperty("ExtraType", dbKeysContent.Get("ExtraType"));
    cFields.setProperty("ExtraName", dbKeysContent.Get("ExtraName"));

    // Operator from MIR
    cFields.setProperty("Operator", dbKeysContent.Get("Operator"));

    cFields.setProperty("Product", dbKeysContent.Get("Product"));
    cFields.setProperty("Process", dbKeysContent.Get("Process"));
    cFields.setProperty("ProcessID", dbKeysContent.Get("ProcessID"));

    cFields.setProperty("Wafer", dbKeysContent.Get("Wafer"));
    cFields.setProperty("WaferNotch", dbKeysContent.Get("WaferNotch"));
    cFields.setProperty("WaferSize", dbKeysContent.Get("WaferSize"));
    cFields.setProperty("WaferUnits", dbKeysContent.Get("WaferUnits"));
    cFields.setProperty("WaferCenterX", dbKeysContent.Get("WaferCenterX"));
    cFields.setProperty("WaferCenterY", dbKeysContent.Get("WaferCenterY"));
    cFields.setProperty("WaferPosX", dbKeysContent.Get("WaferPosX") );
    cFields.setProperty("WaferPosY", dbKeysContent.Get("WaferPosY") );
    cFields.setProperty("WaferStartTime", dbKeysContent.Get("WaferStartTime") );
    cFields.setProperty("WaferFinishTime", dbKeysContent.Get("WaferFinishTime") );

    cFields.setProperty("ChipSizeX", dbKeysContent.Get("ChipSizeX"));
    cFields.setProperty("ChipSizeY", dbKeysContent.Get("ChipSizeY"));

    //cFields.setProperty("WaferNb", dbKeysContent.Get("WaferNb")); // WaferNb has disappeared ?
    cFields.setProperty("Lot", dbKeysContent.Get("Lot"));
    cFields.setProperty("FinishTime", dbKeysContent.Get("FinishTime"));
    cFields.setProperty("SetupTime", dbKeysContent.Get("SetupTime"));
    cFields.setProperty("StartTime", dbKeysContent.Get("StartTime"));
    cFields.setProperty("Station", dbKeysContent.Get("Station"));
    cFields.setProperty("ProdData", dbKeysContent.Get("ProdData"));
    cFields.setProperty("DataType", dbKeysContent.Get("DataType"));

    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Registering Quantix JS DTR: '%1'")
          .arg(dbKeysContent.property("GalaxySemiJSDTR").toString())
          .toLatin1().data());
    cFields.setProperty("GalaxySemiJSDTR", dbKeysContent.property("GalaxySemiJSDTR"));

    // Get handle to PAT task.
    CGexMoTaskOutlierRemoval cDefaultTask;
    cDefaultTask.SetProperties(new GexMoOutlierRemovalTaskData(NULL));

    CGexMoTaskOutlierRemoval *ptOutlierTask = NULL;
    QString	strProductID = dbKeysContent.Get("Product").toString();
    if(!cFields.m_strProductName.isEmpty())
        strProductID = cFields.m_strProductName;
    ptOutlierTask = GetProductOutlierInfo(strProductID);
    if(ptOutlierTask == NULL)
    {
        // No PAT Task for the given productID...check if a 'default' one exists
        ptOutlierTask = GetProductOutlierInfo("default");
        if(ptOutlierTask == NULL)
        {
            // Give pointer to default PAT task so file van be processed (but no email alarm notification possible)
            ptOutlierTask = &cDefaultTask;
            if(strProductID.isEmpty())
            {
                // Data file doesn't have a ProductID!..check if a default PAT Task exists for such files...
                strLocalError = "PAT processing warning.";
                strLocalError += "\n\t> Trigger file : " + cFields.strTriggerFile;
                strLocalError += "\n\t> Data file    : " + cFields.strSources.first();
                strLocalError += "\n\t> * Warning *  : Missing ProductID in data file (MIR.part_typ), and no 'default' PAT task defined. Yield alarm notification & reports disabled.";
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(strTemp,strLocalError);
            }
            else
            {
                strLocalError = "PAT processing warning.";
                strLocalError += "\n\t> Trigger file : " + cFields.strTriggerFile;
                strLocalError += "\n\t> Data file    : " + cFields.strSources.first();
                strLocalError += "\n\t> * Warning *  : No PAT task defined for product: " + strProductID +" . Yield alarm notification & reports disabled.";
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().UpdateLogError(strTemp,strLocalError);
            }
        }
    }

    // Lock purge of temporary files
    GS::Gex::Engine::GetInstance().GetTempFilesManager().lock();

    // Process PAT
    // PAT-17
    //strErrorMessage += ExecuteOutlierRemovalTask(ptOutlierTask,dbKeysContent,true,cFields, strTestDataFile);
    strErrorMessage += ExecuteOutlierRemovalTask(ptOutlierTask, lPATPump, dbKeysContent, cFields, strTestDataFile);

    // Purge temporary files
    GS::Gex::Engine::GetInstance().GetTempFilesManager().unlock();
    GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

    CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();
    if(lPatInfo)
    {
        // If no error found, generated file list is cleared
        // otherwise, if any errors occured, generated files are deleted
        if(strErrorMessage.isEmpty())
        {
            lPatInfo->CleanupGeneratedFileList(false);
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("INFO","ExecutePatProcessing: PAT processing executed without error", strErrorMessage);
        }
        else
        {
            lPatInfo->CleanupGeneratedFileList(true);
            GS::Gex::Engine::GetInstance().GetAdminEngine().Log("ERROR","ExecutePatProcessing: Error executing PAT processing", strErrorMessage);
        }
    }

    return strErrorMessage;
}

#endif

void dumpWafmap(QString strWaferFileFullName,CWaferMap *pWafMap,bool bAsciiMap)
{
    // Create file.
    QFile file(strWaferFileFullName); // Write the text to the file (IO_Translate to have CR or CR-LF for end-of-line depending of the OS platform)
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return;	// Failed writing to wafermap file.

    // Write Wafermap file
    QTextStream hWafermapFile;
    hWafermapFile.setDevice(&file);	// Assign file handle to data stream

    // Header section
    hWafermapFile << "TSMC" << endl;						// Header line
    hWafermapFile << "Product" << endl;		// Product name
    hWafermapFile << "WaferID" << endl;	// WaferID
    hWafermapFile << "File.TSM" << endl;				// Wafermap file name created (eg: A4523403.TSM)

    // Wafermap
    int	iLine,iCol,iBinCode;
    for(iLine = 0; iLine < pWafMap->SizeY; iLine++)
    {
        // Processing a wafer line.
        for(iCol = 0; iCol < pWafMap->SizeX; iCol++)
        {
            // Get PAT-Man binning at location iRow,iCol.
            iBinCode = pWafMap->getWafMap()[(iCol+(iLine*pWafMap->SizeX))].getBin();
            if(bAsciiMap)
            {
                switch(iBinCode)
                {
                case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                    hWafermapFile << ".";
                    break;

                case '1':		// GOOD die
                    hWafermapFile << "1";
                    break;

                case '@':		// Visual reject die
                    hWafermapFile << "@";
                    break;

                default:	// FAILING die
                    hWafermapFile << "X";
                    break;
                }
            }
            else
            {
                // Binary map (STDF)
                switch(iBinCode)
                {
                case GEX_WAFMAP_EMPTY_CELL:	// -1: Die not tested
                    hWafermapFile << ".";
                    break;

                case 1:		// GOOD die
                    hWafermapFile << "1";
                    break;

                default:	// FAILING die
                    hWafermapFile << "X";
                    break;
                }
            }
        }
        // Insert line break
        hWafermapFile << endl;
    }
}

///////////////////////////////////////////////////////////
// Execute PAT processing on a file...
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::CreateCompositePatResultFile(CGexCompositePatProcessing &cFields,QString &strErrorMessage)
{
    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;

    if (gexReport == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "NULL Report object");

        strErrorMessage = "No report object found";
        return false;
    }

    if (gexReport->getGroupsList().isEmpty() == false)
        pGroup = gexReport->getGroupsList().first();

    if (pGroup && pGroup->pFilesList.isEmpty() == false)
        pFile  = pGroup->pFilesList.first();
    else
    {
        GSLOG(SYSLOG_SEV_ERROR, "No Datasets found");

        strErrorMessage = "No Datasets found";
        return false;
    }

    // If no composite file name specified, create in default location (were STDF files are)
    if(cFields.strCompositeFile.isEmpty() == true)
    {
        // Build composite file name: <stdf_file_path>/<LotID>_composite.pat
        QFileInfo cFileInfo(pFile->strFileNameSTDF);
        cFields.strCompositeFile = cFileInfo.absolutePath();
        cFields.strCompositeFile += "/" + QString(pFile->getMirDatas().szLot);
        cFields.strCompositeFile += "_composite_pat.txt";
    }

    // Create Composite file
    QFile file(cFields.strCompositeFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
        strErrorMessage = "Failed to create composite file (disk protection?): " + cFields.strCompositeFile;
        return false;	// Failed writing to composite file.
    }

    // Open file
    QTextStream hCompositeFile;
    hCompositeFile.setDevice(&file);	// Assign file handle to data stream

    // Check if valid header...or empty!
    hCompositeFile << "<composite_pat>" << endl << endl;

    // Write Exclusion mask (based on stacked wafer)
    int	iLine,iCol,iDieCount;
    CPatInfo * lPatInfo = PATEngine::GetInstance().GetContext();
    double	lfPatternYieldLevel = lPatInfo->GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold;	// Yield level threshold to
    int iDieCountThreshold = (int) ceil(
                ((double)(pGroup->cStackedWaferMapData.iTotalWafermaps) *
                 lfPatternYieldLevel)
                /100.0);
    hCompositeFile << "# Exclusion mask Wafer map" << endl;
    hCompositeFile << "<Exclusion_mask>" << endl;
    hCompositeFile << "LowYieldLimit:" << lfPatternYieldLevel << " %" << endl;
    hCompositeFile << "BinList:" << lPatInfo->GetRecipeOptions().pBadBinsZPAT_List->GetRangeList() << endl;
    hCompositeFile << "iRowMin:" << pGroup->cStackedWaferMapData.iLowDieY << endl;
    hCompositeFile << "iColMin:" << pGroup->cStackedWaferMapData.iLowDieX << endl;
    hCompositeFile << "iSizeX:" << pGroup->cStackedWaferMapData.SizeX << endl;
    hCompositeFile << "iSizeY:" << pGroup->cStackedWaferMapData.SizeY << endl;
    for(iLine = 0; iLine < pGroup->cStackedWaferMapData.SizeY; iLine++)
    {
        // Starting line
        hCompositeFile << "RowData:";

        // Processing a wafer line.
        for(iCol = 0; iCol < pGroup->cStackedWaferMapData.SizeX; iCol++)
        {
            // Get PAT-Man binning at location iRow,iCol.
            iDieCount = pGroup->cStackedWaferMapData.cWafMap[(iCol+(iLine*pGroup->cStackedWaferMapData.SizeX))].ldCount;
            if(iDieCount == GEX_WAFMAP_EMPTY_CELL)
                hCompositeFile << ". ";
            else
            {
                // Check if BadDie count reaching or higher than threshold (if so, reject die).
                if(iDieCount >= iDieCountThreshold)
                    hCompositeFile << "0 ";	// Reject die
                else
                    hCompositeFile << "1 ";	// Keep die
            }
        }
        // Insert line break
        hCompositeFile << endl;
    }
    hCompositeFile << "</Exclusion_mask>" << endl << endl;

    // Write individual maps of each wafer (so 3D neighborhood can be conducted)
    int		iBinValue;
    QString	strBinValue;
    QListIterator<CGexFileInGroup*>	itFilesList(pGroup->pFilesList);

    while(itFilesList.hasNext())
    {
        pFile  = itFilesList.next();

        hCompositeFile << "<wafer>" << endl;
        hCompositeFile << "WaferID:" << pFile->getWaferMapData().szWaferID << endl;
        hCompositeFile << "iRowMin:" << pFile->getWaferMapData().iLowDieX << endl;
        hCompositeFile << "iColMin:" << pFile->getWaferMapData().iLowDieY << endl;
        hCompositeFile << "iSizeX:" << pFile->getWaferMapData().SizeX << endl;
        hCompositeFile << "iSizeY:" << pFile->getWaferMapData().SizeY << endl;
        for(iLine = 0; iLine < pFile->getWaferMapData().SizeY; iLine++)
        {
            // Starting line
            hCompositeFile << "RowData:";

            // Processing a wafer line.
            for(iCol = 0; iCol < pFile->getWaferMapData().SizeX; iCol++)
            {
                // Get PAT-Man binning at location iRow,iCol.
                iBinValue = pFile->getWaferMapData().getWafMap()[(iCol+(iLine*pFile->getWaferMapData().SizeX))].getBin();
                if(iBinValue == GEX_WAFMAP_EMPTY_CELL)
                    hCompositeFile << ".... ";
                else
                {
                    // Wafermap reports Bin 0-0xffff
                    if(iBinValue > 0xFFFF)
                        iBinValue = 0xFFFF;
                    strBinValue.sprintf("%4X ",iBinValue);
                    hCompositeFile << strBinValue;	// Keep die
                }
            }
            // Insert line break
            hCompositeFile << endl;
        }
        hCompositeFile << "</wafer>" << endl << endl;
    };

    hCompositeFile << "</composite_pat>" << endl << endl;

    // Success
    return true;
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool GS::Gex::SchedulerEngine::CompositePatMergeDataFiles(CGexCompositePatProcessing &cFields,QString &strErrorMessage)
{
    // Create script that will read data file + compute all statistics (but NO report created)
    QString strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    //FIXME: should not need to do that
    //if (strScriptFile.isEmpty())
    //{
    //    GS::Gex::Engine::GetInstance().InitScriptsPath();
    //    strScriptFile = GS::Gex::Engine::GetInstance().GetAssistantScript();
    //}
    FILE *      hFile       = fopen(strScriptFile.toLatin1().constData(),"w");

    PATEngine::GetInstance().CreateContext();
    CPatInfo *  lPatInfo    = PATEngine::GetInstance().GetContext();

    if(hFile == NULL)
    {
        strErrorMessage = "Failed to create script file: " + strScriptFile;
        return false;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        GEX_ASSERT(false);
        strErrorMessage = QString("Failed to write option section");
        return false;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");
    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");

    // Insert each file to process (merge them so to create the stacked wafermap)

    GexTbMergeRetest lMergeTool(false);
    CGexCompositePatWaferInfo	cWafer;
    CompositeWaferMaps::Iterator it;
    QString strOutput;
    QFileInfo	cFileInfo;
    for ( it = cFields.cWaferMaps.begin(); it != cFields.cWaferMaps.end(); ++it )
    {
        cWafer = *it;

        if(cWafer.strSources.count() > 1)
        {
            // Define where to merge files, & file name to use
            cFileInfo.setFile(cWafer.strSources.first());
            strOutput = cFileInfo.absolutePath();
            if(strOutput.endsWith("/") == false)
                strOutput += "/";
            strOutput += "wafer_" + QString::number(cWafer.iWaferID);
            strOutput += "_merged.std";


            // Merge files (all samples data, ignore summary sections and HBR/SBR)
            //cMergeDialog.comboBoxMergingMode->setCurrentItem(0);	// Set to sort files to merge by: DATE.
            if(lMergeTool.MergeSamplesFiles(cWafer.strSources,strOutput,false,true) != GexTbMergeRetest::NoError)
            {
                // Remove temporary merged STDF file created.
                GS::Gex::Engine::RemoveFileFromDisk(strOutput);

                strErrorMessage = lMergeTool.GetErrorMessage();

                return false;	// Error occured
            }
            else
                GS::Gex::Engine::GetInstance().GetTempFilesManager().addFile(strOutput, TemporaryFile::BasicCheck); // This temporary merged file must be erased after processing!

            // Keep track of file created.
            cWafer.strDataFile = strOutput;
        }
        else
            cWafer.strDataFile = cWafer.strSources.first();	// Only one STDF input file; no merge!
        strOutput = cWafer.strDataFile;

        // update Data file name to process
        cFields.cWaferMaps[cWafer.iWaferID] = cWafer;

        // insert file name into script to execute: only condirer Bin results, ignore samples!
        ConvertToScriptString(strOutput);
        fprintf(hFile,"  gexFile(group_id,'insert','%s','All','no_samples',' ','');\n",
                strOutput.toLatin1().constData());
    }

    // Close script section listing files to merge.
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");
    fprintf(hFile,"  gexOptions('sorting','none','');\n");						// Disable file sorting so they appear in the waferID order.
    fprintf(hFile,"  gexOptions('output','format','html');\n");					// Avoids editor to be launched if output options is set to Word or Spreadsheet CSV!
    if(lPatInfo->GetRecipeOptions().bZPAT_SoftBin)
        fprintf(hFile,"  gexReportType('wafer','stack_soft_bin','1');\n");			// Build Wafer on SOFT-Bin1
    else
        fprintf(hFile,"  gexReportType('wafer','stack_hard_bin','1');\n");			// Build Wafer on HARD-Bin1
    fprintf(hFile,"  gexReportType('stats','disabled');\n");
    fprintf(hFile,"  gexReportType('histogram','disabled');\n");
    fprintf(hFile,"  gexOptions('pareto','section','disabled');\n");
    fprintf(hFile,"  gexOptions('binning','section','disabled');\n");

    QStringList lstMirrorOptions;
    // Swap X axis
    if(cFields.bSwapDieX)
        lstMirrorOptions << "mirror_x";

    // Swap Y axis
    if(cFields.bSwapDieY)
        lstMirrorOptions << "mirror_y";

    if(lstMirrorOptions.count() > 0)
        fprintf(hFile,"  gexOptions('wafer','visual_options','%s');\n", lstMirrorOptions.join("|").toLatin1().data());			// Mirror wafermap over X axis

    // Retest policy
    if(cFields.bRetest_HighestBin)
        fprintf(hFile,"  gexOptions('wafer','retest_policy','highest_bin');\n");	// Retest policy: promote highest bin
    else
        fprintf(hFile,"  gexOptions('wafer','retest_policy','last_bin');\n");		// Retest policy: keep last bin (default)

    fprintf(hFile,"  gexOptions('output','format','html');\n");					// Avoids editor to be launched if output options is set to Word or Spreadsheet CSV!
    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!
    fprintf(hFile,"  gexBuildReport('home','0');\n");
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    GS::Gex::CSLStatus lStatus = GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile);
    if (lStatus.IsFailed())
    {
        strErrorMessage = "  > Failed to analyse data files: " + lStatus.GetErrorMessage();

        // Erase all temporary files created (the murged files if any)
        GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

        return false;
    }

    // Erase all temporary files created (the murged files if any)
    GS::Gex::Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

    return true;
}

QString GS::Gex::SchedulerEngine::ExecuteCompositePat(QScriptValue lFields)
{
    GSLOG(SYSLOG_SEV_NOTICE, "Execute Composite Pat ");
    if (lFields.isNull())
        return "error: parameter null";
    if (!lFields.isArray())
        return "error: parameter not an array";
    if (!lFields.isObject())
        return "error: parameter not an object";

    CGexCompositePatProcessing lCompositePatProc;
    QScriptValueIterator it(lFields);
    while (it.hasNext())
    {
         it.next();
         if (!lCompositePatProc.Set(it.name(), it.value().toString()))
             return "error: fail to set "+it.name();
    }

    QString lErrorMessage;
    ExecuteCompositePatProcessing(lCompositePatProc,lErrorMessage);

    return lErrorMessage;
}

QString GS::Gex::SchedulerEngine::ExecuteCompositePatProcessing(CGexCompositePatProcessing &cFields,
                                                                QString &strErrorMessage)
{
    GSLOG(SYSLOG_SEV_NOTICE, "Execute CompositePat Processing...");

    // Read recipe file so to get settings (eg: Exclusion zone yield threshold, etc...)
    QSharedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(cFields.strRecipeFile));
    // Zied: dont use pointer before testing it...
    if (lRecipeIO.isNull())
        return "error: cannot create a recipe IO for "+cFields.strRecipeFile;

    lRecipeIO->SetRecipeName(cFields.strRecipeFile);

    PATEngine::GetInstance().CreateContext();

    CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();
    if (lPatInfo == NULL)
    {
        lPatInfo = new CPatInfo();
    }
    else
        lPatInfo->clear();

    if (lRecipeIO->Read(lPatInfo->GetRecipe()) == false)
    {
        strErrorMessage = lRecipeIO->GetErrorMessage();
        return strErrorMessage;
    }

    //  Analyze (& stack) all wafers in wafer lot.
    if(!CompositePatMergeDataFiles(cFields,strErrorMessage))
        return strErrorMessage;


    // Have the STDF file analysed so we can create a summary version!...Unless Corporate database: then NO summary can be created (would cause a crash because the FTP script is already running)!
    bool bSendEmailAlarm_DieMismatch=false;	// Set to 'true' if alarm detected (sends email)
    bool bSendEmailAlarm_ExclusionZone=false;	// Set to 'true' if alarm detected (sends email)

    CGexGroupOfFiles *pGroup=0;
    CGexFileInGroup	*pFile=0;
    CWaferMap WaferMapEtest;	// To hold E-Test Wafermap data resulting from STDF file analysis.
    QString		strImageName ;
    QString		strImageFile;
    QString	strString,strTitle;
    QString	strMismatch;	// HTML text listing mismatching bins.
    int	iStdfNotchLocation=6;	// Used to store Wafermap flat/notch. Initialized to 6 (Down) in case missing for first file to process
    int	iEtestNotchLocation=6;	// Used to store Wafermap flat/notch. Initialized to 6 (Down) in case missing for first file to process
    int	iIndex,iX,iY,iDieLocX,iDieLocY;
    int	iEtestBin,iStdfBin;		// Used to compare Bin value between Etest wafer map and STDF wafer map
    int	iTotalDies;
    int	iTotalMismatch;
    int	iGoodCount;
    int	iRejectCount;
    int	iVisualRejectCount;
    int	iMaxMismatch=0;			// to hold the maximum of e-test vs STDf die mismatch over all wafers.
    CGexCompositePatWaferInfo cWafer;

    strErrorMessage = "No data file selected";
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return strErrorMessage;	// Error: probably missing input file from a merge command
    pFile = pGroup->pFilesList.isEmpty() ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return strErrorMessage;	// Error: probably missing input file from a merge command

    // Clear error message
    strErrorMessage = "";

    // Open log file to hold status
    CreateCompositePatLogFile(cFields);

    // If caller is PAT-Man, get handle to relevant task.
    CGexMoTaskOutlierRemoval *ptOutlierTask = NULL;
    QString			strProductID = pFile->getMirDatas().szPartType;
    if(!cFields.strProductName.isEmpty())
        strProductID = cFields.strProductName;
    ptOutlierTask = GetProductOutlierInfo(strProductID);
    // If no PAT Task for the given productID...check if a 'default' one exists
    if(ptOutlierTask == NULL)
        ptOutlierTask = GetProductOutlierInfo("default");
    if(ptOutlierTask != NULL)
        cFields.strOutputReportFormat = getOutputReportFormat(ptOutlierTask->GetProperties()->iEmailReportType);
    else
        cFields.strOutputReportFormat = getOutputReportFormat(0);	// force HTML.

    // Create report header file.
    CreateCompositePatReportFile(cFields);

    // If no report to create, only build ZPAT map mask
    QListIterator<CGexFileInGroup*>	itFilesList(pGroup->pFilesList);
    CompositeWaferMaps::Iterator it = cFields.cWaferMaps.begin();
    if(cFields.m_hReportFile == NULL)
    {
        // Close Log file.
        CloseCompositePatLogFile(cFields);
        goto create_zpat_map;
    }
    // Now that the STDF files have been merged, overload wafermap with Inkless E-test wafermap
    // point to first wafer definition (as defined in trigger file); order of STDF files in memory is the same.
    while(itFilesList.hasNext())
    {
        pFile = itFilesList.next();

        // Check / Overload STDF wafermap
        cWafer = *it;	// Get Wafer details

        // Clear varables used during the compare
        iTotalDies = 0;			// Total dies.
        iGoodCount = 0;			// To count toal good bins.
        iTotalMismatch = 0;		// total dies that mismatch
        iRejectCount = 0;		// To count total rejects
        iVisualRejectCount= 0;  // To couont total visual rejects
        strMismatch="";			// Holds HTML text listing dies discrepancies

        // Write Wafer header info in log file
        cFields.m_hLogFile << endl << "##### Wafermaps analysis (STDF, E-Test) #####" << endl;	// STDF file processed.
        cFields.m_hLogFile << "WaferID," << cWafer.iWaferID << endl;							// STDF file processed.

        // Write Wafer header in HTML flat report
        strString.sprintf("WaferID_%d",cWafer.iWaferID);
        strTitle.sprintf("Lot %s - Wafer: %d",pFile->getMirDatas().szLot,cWafer.iWaferID);
        gexReport->WriteHtmlSectionTitle(cFields.m_hReportFile, strString, strTitle);

        // If Visual inspection wafermap defined, load it so to compare it with STDF map.
        if(QFile::exists(cWafer.strOptionalSource) == false)
        {
            // Log file
            cFields.m_hLogFile << " Matching Status, Passed (No E-Test data file defined)" << endl;

            // Html section
            fprintf(cFields.m_hReportFile,"<p><b>No E-Test</b> data file to compare with STDF wafermap.</p>\n");

            // Save STDF notch location
            iStdfNotchLocation = pFile->getWaferMapData().GetWaferNotch();

            // Rotate STDF wafer until we match the same notch as the visual inspection file
            while(pFile->getWaferMapData().GetWaferNotch() != iEtestNotchLocation)
                pFile->getWaferMapData().RotateWafer();
        }
        else
        {
            // E-Test data file deinfed: Load its map
            WaferMapEtest.loadFromEtestfile(cWafer.strOptionalSource);

            // Save STDF & E-test notch locations
            iStdfNotchLocation = pFile->getWaferMapData().GetWaferNotch();
            iEtestNotchLocation = WaferMapEtest.GetWaferNotch();

            // Rotate STDF wafer until we match the same notch as the visual inspection file
            while(pFile->getWaferMapData().GetWaferNotch() != iEtestNotchLocation)
                pFile->getWaferMapData().RotateWafer();

            // Check if both wafers have the same size
            if(WaferMapEtest.SizeX != pFile->getWaferMapData().SizeX || WaferMapEtest.SizeY != pFile->getWaferMapData().SizeY)
            {
                // Report Error in log file.
                cFields.m_hLogFile << " Wafermap size mismatch: ";
                cFields.m_hLogFile << " E-test (" << WaferMapEtest.SizeX << "x" << WaferMapEtest.SizeY << ") ";
                cFields.m_hLogFile << " STDF (" << pFile->getWaferMapData().SizeX << "x" << pFile->getWaferMapData().SizeY << ")" << endl;
                iTotalMismatch = WaferMapEtest.SizeX*WaferMapEtest.SizeY;
                goto wafer_summary_status;
            }

            // Check if matching dies
            for(iIndex = 0; iIndex < WaferMapEtest.SizeX*WaferMapEtest.SizeY; iIndex++)
            {
                iEtestBin = WaferMapEtest.getWafMap()[iIndex].getBin();
                iStdfBin  = pFile->getWaferMapData().getWafMap()[iIndex].getBin();

                // Compute die location in matrix buffer
                iX = iIndex % pFile->getWaferMapData().SizeX;
                iY = iIndex / pFile->getWaferMapData().SizeX;

                // Compute die location including origin offset (to display in reports)
                iDieLocX = iX + pFile->getWaferMapData().iLowDieX;
                iDieLocY = iY + pFile->getWaferMapData().iLowDieY;
                switch(iEtestBin)
                {
                default:
                case '.':	// No die for this cell
                    break;

                case '1':		// Bin1 in E-Test wafer
                    // Keep track of good dies count in Etest map.
                    iGoodCount++;

                    // Keep track of total dies
                    iTotalDies++;

                    // Check if also a good die in the STDF
                    if(iStdfBin != 1)
                    {
                        // Keep track of total mismatchs
                        iTotalMismatch++;

                        // Report mismatch in LOG file.
                        strString.sprintf("Bin mismatch at (%3d,%3d) E-test Bin=1 ; STDF Bin=%d",iDieLocX,iDieLocY,iStdfBin);
                        cFields.m_hLogFile << strString << endl;

                        // HTML report mismatch info
                        strMismatch += strString + "<br>";

                        // Force to reject this bin (if this action is enabled in the recipe file)
                        if(lPatInfo->GetRecipeOptions().bMergeEtestStdf)
                            pFile->getWaferMapData().getWafMap()[iIndex].setBin(lPatInfo->GetRecipeOptions().iCompositeEtestStdf_SBin);
                    }
                    break;

                case 'X':	// E-test reject in E-test wafer
                    // Keep track of E-test reject count in Etest map.
                    iRejectCount++;

                    // Keep track of total dies
                    iTotalDies++;

                    // Check if also a bad die in the STDF
                    if(iStdfBin == 1)
                    {
                        // Keep track of total mismatchs
                        iTotalMismatch++;
                        strString.sprintf("Bin mismatch at (%3d,%3d) E-test Bin=X ; STDF Bin=1",iDieLocX,iDieLocY);
                        cFields.m_hLogFile << strString << endl;

                        // HTML report mismatch info
                        strMismatch += strString + "<br>";

                        // Force to reject this bin (if this action is enabled in the recipe file)
                        if(lPatInfo->GetRecipeOptions().bMergeEtestStdf)
                            pFile->getWaferMapData().getWafMap()[iIndex].setBin( lPatInfo->GetRecipeOptions().iCompositeEtestStdf_SBin);
                    }
                    break;

                case '@':	// Visual Reject in E-test wafer

                    // Keep track of Visual reject count in Etest map.
                    iVisualRejectCount++;

                    // Keep track of total dies
                    iTotalDies++;

                    // Force to reject this bin AFTER wafermap is printed (so we save the new bin in the retest fileds .iOrgBin! - if this action is enabled in the recipe file)
                    if(iStdfBin == 1 && lPatInfo->GetRecipeOptions().bMergeEtestStdf)
                        pFile->getWaferMapData().getWafMap()[iIndex].setOrgBin (lPatInfo->GetRecipeOptions().iCompositeEtestStdf_SBin);
                    break;
                }
            }

wafer_summary_status:
            // Keep track of maximum mismatch found over all wafers
            iMaxMismatch = gex_max(iMaxMismatch,iTotalMismatch);

            // Display statistics & Pass/Fail status.
            if(iTotalDies)
            {
                // Log file
                cFields.m_hLogFile << " E-test total dies , " << iTotalDies << endl;
                strString.sprintf(" E-test Good dies , %d (%.2lf %%)",iGoodCount,100.0*(double)iGoodCount/(double)iTotalDies);
                cFields.m_hLogFile << strString << endl;
                strString.sprintf(" E-test Rejects , %d (%.2lf %%)",iRejectCount,100.0*(double)iRejectCount/(double)iTotalDies);
                cFields.m_hLogFile << strString << endl;
                strString.sprintf(" E-test Visual Rejects , %d (%.2lf %%)",iVisualRejectCount,100.0*(double)iVisualRejectCount/(double)iTotalDies);
                cFields.m_hLogFile << strString << endl;
                strString.sprintf(" E-Test vs STDF dies discrepancies , %d (%.2lf %%)",iTotalMismatch,100.0*(double)iTotalMismatch/(double)iTotalDies);
                cFields.m_hLogFile << strString << endl;

                if(iTotalMismatch)
                    cFields.m_hLogFile << " Matching Status, *FAILED*" << endl;
                else
                    cFields.m_hLogFile << " Matching Status, Passed" << endl;
            }

            // Write Mismatch wafermap image
            // First, toggle current wafermap die info so to create a binary  colored wafer.
            for(iIndex = 0; iIndex < pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY; iIndex++)
            {
                // Get Die count at given location & save bin value in retest copy buffer (.iOrgBin)
                iStdfBin = pFile->getWaferMapData().getWafMap()[iIndex].getBin();
                iEtestBin = WaferMapEtest.getWafMap()[iIndex].getBin();

                // Check if bin marked as reject
                if(iStdfBin != GEX_WAFMAP_EMPTY_CELL)
                {
                    // Save Bin result so we can revert back once the wafermap image is created
                    pFile->getWaferMapData().getWafMap()[iIndex].setOrgBin (iStdfBin);

                    if(iStdfBin == lPatInfo->GetRecipeOptions().iCompositeEtestStdf_SBin)
                        pFile->getWaferMapData().getWafMap()[iIndex].setBin(2);	// Will appear as a Red die
                    else
                    {
                        // Not a mismatch. Then display PAss/Fali info as found in E-test file.
                        if(iEtestBin == 'X' || iEtestBin == '@' )
                            pFile->getWaferMapData().getWafMap()[iIndex].setBin(3);	// Will appear as a Yellow die: E-test failure
                        else
                            pFile->getWaferMapData().getWafMap()[iIndex].setBin(1);	// Will appear as a Green die
                    }
                }
            }

            // Write Discrepancies wafermap into Flat HTML report
            fprintf(cFields.m_hReportFile,"<p>E-Test vs STDF mismatching dies appear in <font color=\"#FF0000\"><b>Red</b></font><br>\n");
            fprintf(cFields.m_hReportFile,"E-Test failures appear in Yellow</p>\n");

            strImageName.sprintf("comp_mismatchwaf_%d.png",cWafer.iWaferID);
            strImageFile = CGexReport::outputLocation(&ReportOptions);
            strImageFile += "/images/";
            strImageFile += strImageName;
            //strImageFile=QDir::cleanPath(strImageFile); // We cant clean path because it can be a Samba path (\\SERVER\MACHINE)
            gexReport->CreateWaferMapImage(CGexReport::individualWafermap, pGroup, pFile, true, strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());

            // Set back wafer map to original binnings.
            for(iIndex = 0; iIndex < pFile->getWaferMapData().SizeX*pFile->getWaferMapData().SizeY; iIndex++)
                pFile->getWaferMapData().getWafMap()[iIndex].setBin(pFile->getWaferMapData().getWafMap()[iIndex].getOrgBin());

            // HTML report
            if(iTotalDies)
            {
                // List Lot details
                gexReport->WriteHtmlOpenTable(98,2);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)

                strString.sprintf("%d (%.2lf %%)",iGoodCount,100.0*(double)iGoodCount/(double)iTotalDies);
                gexReport->WriteInfoLine("E-test Good dies", strString.toLatin1().constData());

                strString.sprintf("%d (%.2lf %%)",iRejectCount,100.0*(double)iRejectCount/(double)iTotalDies);
                gexReport->WriteInfoLine("E-test Rejects", strString.toLatin1().constData());

                strString.sprintf("%d (%.2lf %%)",iVisualRejectCount,100.0*(double)iVisualRejectCount/(double)iTotalDies);
                gexReport->WriteInfoLine("E-test Visual Rejects", strString.toLatin1().constData());

                strString.sprintf("%d (%.2lf %%)",iTotalMismatch,100.0*(double)iTotalMismatch/(double)iTotalDies);
                gexReport->WriteInfoLine("E-Test vs STDF dies discrepancies", strString.toLatin1().constData());

                gexReport->WriteInfoLine("Dies discrepancies", strMismatch.toLatin1().constData());

                // Close table
                fprintf(cFields.m_hReportFile,"</table>\n");
            }

            // Write this page as a slide (image)
            gexReport->WritePageBreak();

            // If mirror was enabled to match the E-Test wafermap, then mirror-back
            if(cFields.bSwapDieX)
                pFile->SwapWaferMapDies(true,false);

            if(cFields.bSwapDieY)
                pFile->SwapWaferMapDies(false,true);
        }	// STDF vs E-Test file analysis performed.

        // Rotate STDF wafermap back to what it was prior to compare it to the e-test data.
        while(pFile->getWaferMapData().GetWaferNotch() != iStdfNotchLocation)
            pFile->getWaferMapData().RotateWafer();

        // Point to next Wafer definition (as defined in trigger file)
        ++it;
    };

    // Close Log file.
    CloseCompositePatLogFile(cFields);

create_zpat_map:
    // Re-Build 'Stacked wafermap' data array as some bins may have changed.
    int iWaferType = (lPatInfo->GetRecipeOptions().bZPAT_SoftBin) ? GEX_WAFMAP_SOFTBIN : GEX_WAFMAP_HARDBIN;
    pGroup->BuildStackedWaferMap(&ReportOptions,lPatInfo->GetRecipeOptions().pBadBinsZPAT_List->GetRangeList(),iWaferType);

    // Create Composite result file (to be used when processing each individual wafer)
    if(CreateCompositePatResultFile(cFields,strErrorMessage) == false)
        return strErrorMessage;

    // If no report to create, quit function now
    if(cFields.m_hReportFile == NULL)
        return strErrorMessage;

    // Display Exclusion zone in HTML report
    int		iTotalRejectedCells=0;
    double	lfPatternYieldLevel = lPatInfo->GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold;	// Yield level threshold to
    if(lPatInfo->GetRecipeOptions().GetExclusionZoneEnabled() && lPatInfo->GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold >= 0)
    {
        // Modify stacked wafer to be binary (0, or Max die count) to only have Blue and Red colors
        int		iDieCount;
        int	iDieCountThreshold = (int)(((double)(pGroup->cStackedWaferMapData.iHighestDieCount)*lfPatternYieldLevel)/100.0);
        for(iIndex = 0; iIndex < pGroup->cStackedWaferMapData.SizeX*pGroup->cStackedWaferMapData.SizeY; iIndex++)
        {
            // Get Die count at given location.
            iDieCount = pGroup->cStackedWaferMapData.cWafMap[iIndex].ldCount;
            if(iDieCount != GEX_WAFMAP_EMPTY_CELL)
            {
                // Check if count under threshold (if so, keep die, otherwise reject die).
                if(iDieCount < iDieCountThreshold)
                {
                    // Keep die
                    pGroup->cStackedWaferMapData.cWafMap[iIndex].ldCount = pGroup->cStackedWaferMapData.iHighestDieCount/2;
                }
                else
                {
                    // Fail this die on all wafers
                    pGroup->cStackedWaferMapData.cWafMap[iIndex].ldCount = pGroup->cStackedWaferMapData.iHighestDieCount;	// Reject die
                    // Keep track of total dies rejected
                    iTotalRejectedCells++;
                }
            }
        }

        // Write this page as a slide (image)
        gexReport->WritePageBreak();

        // Write Exclusion zones wafermap into Flat HTML report
        gexReport->WriteHtmlSectionTitle(cFields.m_hReportFile,"comp_exclusion_zone","Exclusion zone: Dies to reject on all wafers");
        strImageName = "comp_excluwaf.png";

        strImageFile = ReportOptions.strReportDirectory;
        strImageFile += "/images/";
        strImageFile += strImageName;
        fprintf(cFields.m_hReportFile,"<p>Dies to reject appear in <font color=\"#FF0000\"><b>Red</b></font></p>\n");
        pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        gexReport->CreateWaferMapImage(CGexReport::stackedWafermap, pGroup, pFile, true, strImageFile.toLatin1().constData(), strImageName.toLatin1().constData());
        fprintf(cFields.m_hReportFile,"<br>\n");

        // Info about exclusion zone
        gexReport->WriteHtmlOpenTable(98,2);	// HTML code to open table, 98%, cell spacing=1 (or 2 if creating PDF file)

        // Total wafers stacked
        strString.sprintf("%d",pGroup->cStackedWaferMapData.iTotalWafermaps);
        gexReport->WriteInfoLine("Total wafers stacked", strString.toLatin1().constData());

        // Total dies per wafer
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
        strString.sprintf("%d",pFile->getWaferMapData().iTotalPhysicalDies);
        gexReport->WriteInfoLine("Total dies per wafer", strString.toLatin1().constData());

        // Total rejected dies (exclusion zone)
        if(pFile->getWaferMapData().iTotalPhysicalDies>0)
            strString.sprintf("%d (%.2lf %%)",iTotalRejectedCells,100.0*(double)iTotalRejectedCells/(double)pFile->getWaferMapData().iTotalPhysicalDies);
        else
            strString = "n/a";
        gexReport->WriteInfoLine("Total dies rejected", strString.toLatin1().constData());

        // Exclusion zone: Yield threshold
        strString.sprintf("%.2lf %% (Reject Die location if composite yield is lower)",lfPatternYieldLevel);
        gexReport->WriteInfoLine("Low-Yield level set", strString.toLatin1().constData());

        // Exclusion zone alarm threshold
        if(ptOutlierTask)
            strString.sprintf("%ld Dies maximum.",ptOutlierTask->GetProperties()->lCompositeExclusionZoneAlarm);
        else
            strString = "n/a";
        gexReport->WriteInfoLine("Excl. zone alarm level", strString.toLatin1().constData());

        // Close table
        fprintf(cFields.m_hReportFile,"</table>\n");

        // Check if Exclusion zone includes too many rejects (if so, send Email notification).
        if(ptOutlierTask && (iTotalRejectedCells > ptOutlierTask->GetProperties()->lCompositeExclusionZoneAlarm))
            bSendEmailAlarm_ExclusionZone = true;
    }

    if(ptOutlierTask && (iMaxMismatch > ptOutlierTask->GetProperties()->lCompositeEtestAlarm))
        bSendEmailAlarm_DieMismatch = true;

    // Close HTML report
    CloseCompositePatReportFile(cFields);

    // Check if email notification needs to be sent
    if(bSendEmailAlarm_ExclusionZone || bSendEmailAlarm_DieMismatch)
    {
        // Get Email spooling folder.
        CGexMoTaskStatus *ptStatusTask = GetStatusTask();
        if(ptStatusTask == NULL)
        {
            strErrorMessage = "  > Reporting: failed to get Email spooling folder";
            return strErrorMessage;
        }

        GexMoBuildEmailString cEmailString;
        GexMoSendEmail Email;
        QString	strEmailBody;
        QString strAttachment;
        QString strFilePath;
        QString strFrom,strTo,strSubject;
        if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
            strFrom = GEX_EMAIL_PAT_MAN;
        else
            strFrom = GEX_EMAIL_YIELD_MAN;
        if ( ptOutlierTask->GetProperties()->strEmailFrom.isEmpty() == false )
            strFrom = ptOutlierTask->GetProperties()->strEmailFrom;
        strTo = ptOutlierTask->GetProperties()->strEmailNotify;
        strSubject = "Composite PAT Alarm: " + ptOutlierTask->GetProperties()->strTitle;
        pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        QString strHttpReportName;
        // Check if specify http path to report
        if (!ptStatusTask->GetProperties()->reportHttpURL().isEmpty())
            strHttpReportName = ptStatusTask->GetProperties()->reportHttpURL() + QDir::cleanPath("/" + gexReport->reportRelativeFilePath());

        // Build email!
        if(ptOutlierTask->GetProperties()->bHtmlEmail)
        {
            // HTML Header
            cEmailString.CreatePage(strSubject);

            // Table with Lot Info
            cEmailString.WriteHtmlOpenTable();

            // Highest die mismatch detected
            strString.sprintf("%d (Highest E-test vs STDF, mismatch count found)",iMaxMismatch);
            cEmailString.WriteInfoLine("High Die mismatch",strString,bSendEmailAlarm_DieMismatch);
            // High die alarm threshold
            strString.sprintf("%ld Dies maximum / wafer.",ptOutlierTask->GetProperties()->lCompositeEtestAlarm);
            cEmailString.WriteInfoLine("High Die alarm level",strString);
            // Total rejects generated on exclusion zone
            cEmailString.WriteInfoLine("Rejects from exclusion zone",QString::number(iTotalRejectedCells),bSendEmailAlarm_ExclusionZone);
            // Exclusion zone alarm threshold
            strString.sprintf("%ld Dies maximum.",ptOutlierTask->GetProperties()->lCompositeExclusionZoneAlarm);
            cEmailString.WriteInfoLine("Excl. zone alarm level",strString);
            if(!cFields.strProductName.isEmpty())
                cEmailString.WriteInfoLine("Product",cFields.strProductName);
            else
                cEmailString.WriteInfoLine("Product",pFile->getMirDatas().szPartType);
            cEmailString.WriteInfoLine("Lot",pFile->getMirDatas().szLot);
            cEmailString.WriteInfoLine();

            if(*pFile->getMirDatas().szJobName)
                cEmailString.WriteInfoLine("Program name",QString(pFile->getMirDatas().szJobName));

            if(*pFile->getMirDatas().szJobRev)
                cEmailString.WriteInfoLine("Program revision",QString(pFile->getMirDatas().szJobRev));

            // Table with Lot Info
            cEmailString.WriteHtmlCloseTable();

            // If Full Report to be created
            if(ptOutlierTask->GetProperties()->iEmailReportType)
            {
                // Check if report to be attached or remain on server and only the URL to send...
                if(ptOutlierTask->GetProperties()->iNotificationType == 0)
                {
                    cEmailString.AddHtmlString("<br><b>Full report attached to this message!</b><br>\n");
                    strAttachment= gexReport->reportAbsFilePath();
                }
                else
                {
                    // Leave report on server, email URL only.
                    cEmailString.AddHtmlString("<br><b>Full Report available on server at:<br>\n");

                    if (!strHttpReportName.isEmpty())
                    {
                        // Http hyperlink
                        cEmailString.AddHtmlString("<a href=\"");
                        cEmailString.AddHtmlString(strHttpReportName);
                        cEmailString.AddHtmlString("\">");
                        // Display File name
                        cEmailString.AddHtmlString(strHttpReportName);
                    }
                    else
                    {
                        // File hyperlink
                        cEmailString.AddHtmlString("<a href=\"file:///");
                        cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                        cEmailString.AddHtmlString("\">");
                        // Display File name
                        cEmailString.AddHtmlString(gexReport->reportAbsFilePath());
                    }

                    // Close URL hyperlink
                    cEmailString.AddHtmlString("</a>");

                    // Close message
                    cEmailString.AddHtmlString("</b><br><br>\n");
                }
            }
        }
        else
        {
            // Plain Text email.
            strEmailBody = "\n###############################################################\n";
            strString.sprintf("%d (Highest E-test vs STDF, mismatch count found)",iMaxMismatch);
            strEmailBody += QString("High Die mismatch           : ") + QString::number(iMaxMismatch) + QString("(Highest E-test vs STDF, mismatch count found)\n");
            strEmailBody += QString("High Die alarm level        : ") + QString::number(ptOutlierTask->GetProperties()->lCompositeEtestAlarm) + QString("\n");
            strEmailBody += QString("Rejects from exclusion zone : ") + QString::number(iTotalRejectedCells) + QString("\n");
            strEmailBody += QString("Excl. zone alarm level      : ") + QString::number(ptOutlierTask->GetProperties()->lCompositeExclusionZoneAlarm) + QString("\n");
            if(!cFields.strProductName.isEmpty())
                strEmailBody += QString("Product                     : ") + cFields.strProductName + QString("\n");
            else
                strEmailBody += QString("Product                     : ") + QString(pFile->getMirDatas().szPartType) + QString("\n");
            strEmailBody += QString("Lot                         : ") + QString(pFile->getMirDatas().szLot) + QString("\n");
            strEmailBody += QString("Job name                    : ") + QString(pFile->getMirDatas().szJobName) + QString("\n");
            strEmailBody += QString("Job revision                : ") + QString(pFile->getMirDatas().szJobRev) + QString("\n");
            strEmailBody += "\n";
            strEmailBody += "###############################################################\n\n";

            // If Full Report to be created
            if(ptOutlierTask->GetProperties()->iEmailReportType)
            {
                // Check if report to be attached or remain on server and only the URL to send...
                if(ptOutlierTask->GetProperties()->iNotificationType == 0)
                {
                    strEmailBody += "Full report attached to this message!\n";
                    strAttachment= gexReport->reportAbsFilePath();
                }
                else
                {
                    // Leave report on server, email URL only.
                    strEmailBody += "Full Report available on server at:\n  ";
                    if (!strHttpReportName.isEmpty())
                        strEmailBody += strHttpReportName;
                    else
                        strEmailBody += gexReport->reportAbsFilePath();

                    strEmailBody += "\n";
                }
                strEmailBody += "\n";
            }
        }

        // Close HTML email string if need be
        if(ptOutlierTask->GetProperties()->bHtmlEmail)
            strEmailBody = cEmailString.ClosePage();

        // We have a spooling folder: create email file in it!
        strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
        strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
        strFilePath += GEXMO_AUTOREPORT_EMAILS;

        // Send email with Yield Monitoring alarm message + report.
        Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptOutlierTask->GetProperties()->bHtmlEmail,strAttachment);
    }

    // Return error message (if any)
    return strErrorMessage;
}

///////////////////////////////////////////////////////////
// Constructor: structure holding all details about Composite PAT processing
///////////////////////////////////////////////////////////
CGexCompositePatProcessing::CGexCompositePatProcessing(QObject* parent):QObject(parent)
{
    // Delete any wafer info in memory
    cWaferMaps.clear();

    // No mirroring of the dies (default)
    bSwapDieX = false;
    bSwapDieY = false;
    bRetest_HighestBin = false;	// 'true' if retest policy is to promote highest bin instead of last bin

    // Handle to flat HTML report.
    m_hReportFile = NULL;
}

/*
CGexCompositePatProcessing::CGexCompositePatProcessing(const CGexCompositePatProcessing &lOther)
    :QObject(lOther.parent())
{
    // *this=lOther;
    m_hReportFile=0;
}
*/

bool CGexCompositePatProcessing::Set(const QString &lKey, const QString &lValue)
{
    if (lKey.startsWith("Recipe",Qt::CaseInsensitive))
    { strRecipeFile=lValue; return true;
    }
    if (lKey.startsWith("LogFile",Qt::CaseInsensitive))
    { strLogFilePath=lValue; return true;
    }
    if (lKey.startsWith("CompositeFile",Qt::CaseInsensitive))
    { strCompositeFile=lValue; return true;
    }
    if (lKey.startsWith("Product",Qt::CaseInsensitive))
    { strProductName=lValue; return true;
    }
    if (lKey.startsWith("CustomerName",Qt::CaseInsensitive))
    { strCustomerName=lValue; return true;
    }
    if (lKey.startsWith("SupplierName",Qt::CaseInsensitive))
    { strSupplierName=lValue; return true;
    }
    if (lKey.startsWith("StdfSwapDieX",Qt::CaseInsensitive))
    {
        if(lValue.startsWith("yes", Qt::CaseInsensitive))
            bSwapDieX = true;
        else
            bSwapDieX = false;
        return true;
    }
    if (lKey.startsWith("StdfSwapDieY",Qt::CaseInsensitive))
    {
        if(lValue.startsWith(QString("yes"),Qt::CaseInsensitive))
            bSwapDieY = true;
        else
            bSwapDieY = false;
        return true;
    }
    if (lKey.startsWith("RetestPolicy",Qt::CaseInsensitive))
    {
        if(lValue.startsWith(QString("highest_die"), Qt::CaseInsensitive))
            bRetest_HighestBin = true;
        else
            bRetest_HighestBin = false;
        return true;
    }
    if (lKey.startsWith("Shell",Qt::CaseInsensitive))
    {
        strPostProcessShell= lValue; return true;
    }

    if (lKey=="<wafer>")
    {
        GSLOG(SYSLOG_SEV_ERROR, "Code me");
        // Herv� ?
    }

    return false;
}

// Clear Wafer# info.
void CGexCompositePatWaferInfo::clear()
{
    iWaferID = 0;
    strSources.clear();			// Holds input test data files (if more than one, need to merge them)
    strOptionalSource = "";	// Optional input. E.g: KLA/INF
}

QString GS::Gex::SchedulerEngine::getOutputReportFormat(int iEmailReportType)
{
    switch(iEmailReportType)
    {
    default:
    case 0:	// HTML report to create & append
        return "html";

    case 1:	// WORD report to create & append
        return "word";

    case 2:	// CSV report to create & append
        return "excel";

    case 3:	// PowerPoint report to create & append
        return "ppt";

    case 4:	// PDF report to create & append
        return "pdf";
    }
}



#ifdef GCORE15334
// Exec the requested PAT pump task script if any and returns "ok" or "error:...". Will check PP Status.
QString ExecPumpScript(QString lScriptTaskKey, CGexMoTaskPatPump *lPATPumpTask, GS::Gex::PATProcessing& lPATProcessing)
{
    if (!lPATPumpTask)
        return "error: PAT Pump null";
    // Fix me: give access to the Pump task
    QVariant lContentVar = lPATPumpTask->GetProperties()->GetAttribute(lScriptTaskKey);
    if ( !lContentVar.isValid() || lContentVar.isNull() )
        return "ok"; // does not matter, silent return

    QString lScript=lContentVar.toString();
    if (lScript.isEmpty())
        return "ok"; // never mind

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Let's execute '%1'...").arg(lScriptTaskKey).toLatin1().data() );
    QScriptValue lSV=pGexScriptEngine->evaluate(lScript);
    GSLOG(SYSLOG_SEV_NOTICE, QString("%1 exec returned: '%2'").arg(lScriptTaskKey)
      .arg(lSV.toString().toLatin1().data()).toLatin1().constData());

    if(lSV.isError() || pGexScriptEngine->hasUncaughtException())
    {
        return "error: script execution exception: "+ pGexScriptEngine->uncaughtException().toString()
                      + " at line " + QString::number(pGexScriptEngine->uncaughtExceptionLineNumber());
    }

    // PAT-126
    // Check me: test if script prefers to continue or stop execution
    bool lOk=false;
    int lStatus=lPATProcessing.Get(GS::Gex::PATProcessing::sKeyStatus).toInt(&lOk);
    if ( (lStatus!=0) || (!lOk) )
    {
        return "error: " +QString("script asking to cancel PAT due to status:'%1':")
                .arg(lStatus)+lPATProcessing.Get(GS::Gex::PATProcessing::sKeyComment).toString();
    }
    return "ok";
}



QString GS::Gex::SchedulerEngine::ExecuteOutlierRemovalTask(CGexMoTaskOutlierRemoval* lOutlierRemovalTask,
                                                            CGexMoTaskPatPump *lPATPumpTask,
                                                            QtLib::DatakeysContent &dbKeysContent,
                                                            /* bool bTriggerFile, */
                                                            GS::Gex::PATProcessing& lPATProcessing, //cTriggerFields,
                                                            const QString& lDataFile)
{
    // If no matching task, just return
    if(lOutlierRemovalTask == NULL)
        return "";

    QString     Filter = "Product=" + dbKeysContent.Get("Product").toString();;
    QString     lLogMessage;
    QDateTime	cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString     strString;
    strString = "Execute OutlierRemoval Task :";
    strString += lOutlierRemovalTask->GetProperties()->strTitle;
    strString += ", ";
    strString += cCurrentDateTime.toString("hh:mm:ss");
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    onStartTask(lOutlierRemovalTask, Filter);

    // 1: Build report name & type;
    QString strErrorMessage, strProductID;

    // PAT-17 : this second PATProcessing seems to be unuseful
    //GS::Gex::PATProcessing cFields(this);	// To hold PAT details (file to process recipe file,....)

    // PAT-17 : let's register this one in the script engine asap in order to be used in pre and post task script ?
    if (!pGexScriptEngine)
        return "error: null ScriptEngine";
    QScriptValue lPPSV = pGexScriptEngine->newQObject( &lPATProcessing );
    if (lPPSV.isNull())
    {
       GSLOG(SYSLOG_SEV_ERROR, "Failed to new QObjectize PATProcessing object");
       // exit ?
       return "error: cannot create ScriptValue from QObject PAT Processing in the ScriptEngine";
    }
    pGexScriptEngine->globalObject().setProperty("CurrentGSPATProcessing", lPPSV);

    long lTotalBins=0;
    long lDistributionMismatch=0;

    CGexGroupOfFiles* pGroup = 0;
    CGexFileInGroup* pFile = 0;

    /*
    // In case some settings already loaded from a trigger file, retrieve them.
    if(bTriggerFile)
        cFields = cTriggerFields;
    */

    // Set alarm fields in case Yield alarm check skipped due to fatal error (eg: no recipe file, etc)
    lPATProcessing.mYieldThreshold = "100.0%"; //cFields.strYieldThreshold = "100.0%";
    lPATProcessing.mPatAlarm = true; //cFields.bPatAlarm = true;

    //cFields.strOutputReportFormat = getOutputReportFormat(iEmailReportType);
    lPATProcessing.strOutputReportFormat = getOutputReportFormat(lOutlierRemovalTask->GetProperties()->iEmailReportType);
    //cFields.strOutputReportMode = "alarm";	// Create report on alarm condition only.
    lPATProcessing.strOutputReportMode = "alarm";	// Create report on alarm condition only.
    // Set the Outlier Removal Task used
    lPATProcessing.setProperty(GS::PAT::Trigger::sPATOutlierRemovalTask, lOutlierRemovalTask->GetProperties()->strTitle);

    // If no custom recipe file given, then build path to production repository.
    BuildRecipeFileFullName(lPATProcessing, dbKeysContent); //BuildRecipeFileFullName(cFields, dbKeysContent);

    if (GS::Gex::PATEngine::GetInstance().GetContext() != NULL)
        GS::Gex::PATEngine::GetInstance().DeleteContext();
    GS::Gex::PATEngine::GetInstance().CreateContext();

    CPatInfo* lPatInfo = PATEngine::GetInstance().GetContext();

    // Process file (one at a time)
    GS::Gex::PATProcessWS lPATProcess(lPatInfo);

    // Processes STDF file, creates new STDF (eg: '<wat_file>_patman.std' file and report
    QFileInfo cFileInfo(dbKeysContent.Get("FileName").toString());
    if(lPATProcessing.strSources.isEmpty())
        lPATProcessing.strSources << cFileInfo.absoluteFilePath();	// Set Input STDF (unless already specified by a trigger file)
    if(lPATProcessing.mOutputFolderSTDF.isEmpty())
        // Output folder for new STDF (unless already specified by a trigger file)
        lPATProcessing.mOutputFolderSTDF = cFileInfo.absolutePath();

    // PAT-81: If any pre script, let's exec it now
    if (lPATPumpTask && lPATPumpTask->GetProperties())
    {
        QVariant lPumpPreScriptActivated=lPATPumpTask->GetProperties()->GetAttribute(
                    GexMoDataPumpTaskData::sPreScriptActivatedAttrName);
        if (!lPumpPreScriptActivated.isNull() && lPumpPreScriptActivated.toBool()==true )
        {
            QString lRet=ExecPumpScript(GexMoDataPumpTaskData::sPreScriptAttrName, lPATPumpTask, lPATProcessing);
            if (lRet.startsWith("err"))
            {
                strErrorMessage=lRet;
                goto end_pat;
            }
        }
    }

    // Call PAT processing function
    GSLOG(SYSLOG_SEV_NOTICE, QString("Exec outlier removal task for : %1")
          .arg(lPATProcessing.strSources.first()).toLatin1().data());


    if (lPATProcess.Execute(lPATProcessing, lDataFile) == false)
    {
        strErrorMessage = lPATProcess.GetErrorMessage();
        goto end_pat;
    }

    // Get pointer to first group & first file & parameter list
    // Pointer to file/structure processed
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
    {
        WriteDebugMessageFile("  pGroup == NULL...");
        goto end_pat;	// Error: probably missing input file from a merge command
    }
    pFile  = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
    {
        WriteDebugMessageFile("  pFile == NULL...");
        goto end_pat;	// Error: probably missing input file from a merge command
    }

    strProductID = pFile->getMirDatas().szPartType;
    if(!lPATProcessing.m_strProductName.isEmpty())
        strProductID = lPATProcessing.m_strProductName;

    // If an optional MAP file was defined, use it for computing the Yield loss
    if(lPATProcess.GetExternalMapDetails().mComputed == true)
        lTotalBins = lPATProcess.GetExternalMapDetails().mTotalDies;
    else
    {
        // No Optional input MAP selected, then get TotalDie count from STDF file
        if(pFile->getWaferMapData().bStripMap)
            lTotalBins = pFile->ldTotalPartsSampled;				// Final test data
        else
            lTotalBins = lPatInfo->m_AllSitesMap_Sbin.iTotalPhysicalDies;	// Wafersort data
    }

    // If gross die count defined in trigger file, use it!
    if(lPATProcessing.iGrossDiePerWafer > 0)
        lTotalBins = lPATProcessing.iGrossDiePerWafer;

    if(lTotalBins <= 0)
    {
        strErrorMessage = "";
        goto end_pat;
    }

    // If Custom Alarm defined in the recipre then use it
    // else use Custom alarm defined in the trigger file if any,
    // else use the global one (from Outlier Removal Task)
    if (lPatInfo->GetRecipeOptions().GetEnableYALimit())
    {
        lPATProcessing.mYieldAlarmLoss = lPatInfo->GetRecipeOptions().GetOveralPatYALimit();
        lPATProcessing.setProperty(GS::PAT::Trigger::sPATYieldLimitSource, GS::Gex::PATProcessing::PYLFromRecipe);
    }
    else if(lPATProcessing.mYieldAlarmLoss < 0)
    {
        lPATProcessing.mYieldAlarmLoss = lOutlierRemovalTask->GetProperties()->lfAlarmLevel;
        lPATProcessing.setProperty(GS::PAT::Trigger::sPATYieldLimitSource, GS::Gex::PATProcessing::PYLFromTask);
    }
    else
        lPATProcessing.setProperty(GS::PAT::Trigger::sPATYieldLimitSource, GS::Gex::PATProcessing::PYLFromTrigger);

    lPATProcessing.mAlarmType = lOutlierRemovalTask->GetProperties()->iAlarmType;
    // Tells if report attached to email or only URL sent
    lPATProcessing.iNotificationType = lOutlierRemovalTask->GetProperties()->iNotificationType;
    lPATProcessing.iExceptionLevel = lOutlierRemovalTask->GetProperties()->iExceptionLevel;
    // Email format: HTML or TXT
    lPATProcessing.bHtmlEmail = lOutlierRemovalTask->GetProperties()->bHtmlEmail;
    // Email title.
    lPATProcessing.strEmailTitle = lOutlierRemovalTask->GetProperties()->strTitle;
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        lPATProcessing.strEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        lPATProcessing.strEmailFrom = GEX_EMAIL_YIELD_MAN;
    if ( lOutlierRemovalTask->GetProperties()->strEmailFrom.isEmpty() == false)
        // Email 'From:'
        lPATProcessing.strEmailFrom = lOutlierRemovalTask->GetProperties()->strEmailFrom;
    // Email 'To:' addresses
    lPATProcessing.strEmailTo = lOutlierRemovalTask->GetProperties()->strEmailNotify;

    // Check if force email notification because of Distribution Shape shift detected
    // (and notificartion for such event is enabled)
    lDistributionMismatch=-1;
    if(lOutlierRemovalTask->GetProperties()->bNotifyShapeChange && lPatInfo->lPatShapeMismatch > 0)
        lDistributionMismatch = lPatInfo->lPatShapeMismatch;

    // Check if PAT Yield loss exceeds alarm threshold...
    //    nAlarmFlag = CheckPatYieldLoss(cFields, lfYieldLoss, lPatFailingParts,
    //                                   lTotalBins, lDistributionMismatch,
    //                                   dbKeysContent, strErrorMessage, lLogMessage);

    // PAT-82: If any post script, let's exec it now
    if (lPATPumpTask && lPATPumpTask->GetProperties())
    {
        QVariant lPumpPostScriptActivated=lPATPumpTask->GetProperties()->GetAttribute(
                    GexMoDataPumpTaskData::sPostScriptActivatedAttrName);
        if (!lPumpPostScriptActivated.isNull() && lPumpPostScriptActivated.toBool()==true )
        {
            QString lRet=ExecPumpScript(GexMoDataPumpTaskData::sPostScriptAttrName, lPATPumpTask, lPATProcessing);
            if (lRet.startsWith("err"))
            {
                strErrorMessage=lRet;
                //goto end_pat; ?
            }
        }
    }

    // Check whether we have to send a PAT mofification to the user.
    if (CheckForPATNotification(lPATProcessing, lTotalBins, lDistributionMismatch))
    {
        // Generate a PAT report if output report format is defined
        if (lPATProcessing.strOutputReportFormat.isEmpty() == false)
        {
            // Generate the PAT report
            if (GS::Gex::PATEngine::GetInstance().BuildPATReport(lPatInfo->GetOutputDataFilename(),
                                                                 lPATProcessing,
                                                                 lPatInfo->GetSiteList(),
                                                                 false) == false)
            {
                strErrorMessage = GS::Gex::PATEngine::GetInstance().GetErrorMessage();
                goto end_pat;
            }
        }

        if (SendPATNotification(lPATProcessing, lDistributionMismatch, lTotalBins,
                                dbKeysContent, strErrorMessage, lLogMessage) == false)
        {
            goto end_pat;
        }
    }

    // Successful completion, check if must erase source files...
    if(lPATProcessing.bDeleteDataSource)
    {
        // Delete list of files used as 'input data source'
        QString strFile;
        for( QStringList::Iterator it = lPATProcessing.strSources.begin();
             it != lPATProcessing.strSources.end(); ++it )
        {
            // Get file name to remove.
            strFile = *it;
            GS::Gex::Engine::RemoveFileFromDisk(strFile);	// Delete file if exists
        }
    }

end_pat:

    // If output report file is not STDF, (convert to other format if needed),
    // then erase the STDF temporary file created while computing PAT results
    switch(lPATProcessing.mOutputDatalogFormat)
    {
        case GEX_TPAT_DLG_OUTPUT_NONE:
            // Delete intermadiate STDF file.
            QFile::remove(lPatInfo->GetOutputDataFilename());
            break;

        case GEX_TPAT_DLG_OUTPUT_STDF:
            // Keep STDF file created
            break;

        case GEX_TPAT_DLG_OUTPUT_ATDF:
            // Convert STDF to ATDF
            CSTDFtoATDF lATDFConverter(false);
            QFileInfo   lSTDFFileInfo(lPatInfo->GetOutputDataFilename());
            QString     lATDFName   = lSTDFFileInfo.path() + "/" +
                                      lSTDFFileInfo.completeBaseName() + ".atd";

            lATDFConverter.SetProcessRecord(true);          // Convert ALL STDF records to ATDF
            lATDFConverter.SetWriteHeaderFooter(false);     // Disable DUMP comments in ATDF file.

            if (lATDFConverter.Convert(lPatInfo->GetOutputDataFilename(), lATDFName) == false)
            {
                lATDFConverter.GetLastError(strErrorMessage);
                GSLOG(SYSLOG_SEV_WARNING, "Failed to convert STDF output to ATDF. ");
                return strErrorMessage;
            }

            // Delete intermadiate STDF file.
            QFile::remove(lPatInfo->GetOutputDataFilename());

            // Set neew data file name generated
            lPatInfo->SetOutputDataFilename(lATDFName);
            break;
    }

    // GCORE-12352
    this->SetProperties(lPATProcessing, (strErrorMessage.isEmpty() ? lLogMessage : strErrorMessage),
                        lPATProcess.GetExternalMapDetails());

    // Create LogFile (if enabled)
    // ****************************** WARNING **********************************
    // PAT logs write in the errorMessage record either a true error message or
    // an informational message which must be located elsewhere in the log file.
    // ****************************** WARNING **********************************
    // 6653
    this->CreatePatLogFile(lPATProcessing, lPATProcess.GetExternalMapDetails());

    // In case caller is a trigger file, update the fields as some info (eg: log file name) can have been updated.
    //if(bTriggerFile)
      //  cTriggerFields = cFields;


    // If no PAT yield alarm set severity to -1
    int lSeverity = lPATProcessing.iExceptionLevel;
    if(!lPATProcessing.mPatAlarm)
    {
        // For PAT and YIELD the SeverityLevel is -1:PASS, 0:STANDARD, 1:CRITICAL
        lSeverity = -1;
    }

    LaunchAlarmShell(ShellPat, lSeverity, 1,
                     dbKeysContent.Get("Product").toString(), dbKeysContent.Get("Lot").toString(),
                     dbKeysContent.Get("SubLot").toString(), dbKeysContent.Get("Wafer").toString(),
                     dbKeysContent.Get("TesterName").toString(), dbKeysContent.Get("Operator").toString(),
                     dbKeysContent.Get("FileName").toString());

    onStopTask(lOutlierRemovalTask, strErrorMessage);
    return strErrorMessage;
}

#endif
