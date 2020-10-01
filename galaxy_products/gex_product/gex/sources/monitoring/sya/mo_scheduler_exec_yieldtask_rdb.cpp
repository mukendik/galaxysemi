#include <qfiledialog.h>
#include <QDate>
#include <QDateTime>
#include <QDesktopWidget>

// Galaxy modules includes
#include <gqtl_sysutils.h>
#include <gqtl_log.h>

#include "engine.h"
#include "gex_shared.h"
#include "sya/sya_taskdata.h"
#include "browser_dialog.h"
#include "report_build.h"
#include "report_options.h"
#include "browser_dialog.h"
#include "pickproduct_id_dialog.h"
#include "scheduler_engine.h"
#include "status/status_taskdata.h"
#include "mo_email.h"
#include "autoadmin/autoadmin_taskdata.h"
#include "sya/syardb_task.h"
#include "status/status_task.h"
#include "gqtl_datakeys.h"
#include "db_transactions.h"
#include "gex_database_entry.h"
#include "tb_toolbox.h"
#include "mo_rdb_sylsbl_editor_dialog.h"
#include "gex_report.h"
#include "product_info.h"


extern CGexReport*		gexReport;			// Handle to report class
// report_build.cpp
extern CReportOptions	ReportOptions;		// Holds options (report_build.h)


QString GS::Gex::SchedulerEngine::ExecuteYieldTask_Rdb(GexDatabaseEntry *pDatabaseEntry,
        CGexMoTaskYieldMonitorRDB *ptTask,
        QtLib::DatakeysContent &dbKeysContent)
{
    QString Filter;
    Filter = "Product="+dbKeysContent.Get("Product").toString();
    Filter+= "|Database="+dbKeysContent.Get("DatabaseName").toString();
    Filter+= "|TestingStage="+dbKeysContent.Get("TestingStage").toString();
    onStartTask(ptTask,Filter);

    QString strString = "Execute Yield Task (Rdb) '";
    strString += ptTask->GetProperties()->strTitle;
    strString += "' ";
    GSLOG(SYSLOG_SEV_NOTICE, strString.toLatin1().data());

    // Make sure we are in RDB mode with insertion supported
    if(!pDatabaseEntry->IsExternal() || !pDatabaseEntry->m_pExternalDatabase
            || !pDatabaseEntry->m_pExternalDatabase->IsInsertionSupported())
    {
        onStopTask(ptTask,"Insertion not supported");
        return "";
    }

    // Check SYA yield task for this product
    QString strMessage = ExecuteYieldTask_Rdb_SYA(pDatabaseEntry, ptTask, dbKeysContent);
    onStopTask(ptTask,strMessage);
    return strMessage;
}

QString GS::Gex::SchedulerEngine::ExecuteYieldTask_Rdb_StdYield(GexDatabaseEntry* /*pDatabaseEntry*/,
        CGexMoTaskYieldMonitorRDB* /*ptTask*/,
        QtLib::DatakeysContent & /*dbKeysContent*/)
{
    return "";
}

QString GS::Gex::SchedulerEngine::ExecuteYieldTask_Rdb_SYA(GexDatabaseEntry *pDatabaseEntry,
        CGexMoTaskYieldMonitorRDB *ptTask,
        QtLib::DatakeysContent &dbKeysContent)
{
    QString		strErrorMessage, strError;
    QDateTime	clDateTime;

    // Check SYA
    GexDbPlugin_SYA_Limitset	clSya_Limitset;
    GexDbPlugin_SYA_ItemList	listSyaItems;

    // Update the RuleName to check
    clSya_Limitset.m_strRuleName = ptTask->GetProperties()->strTitle;
    // Search for bintype to check
    QString bt=ptTask->GetAttribute("BinType").toString();
    if (bt=="soft")
        clSya_Limitset.m_eBinType = GexDbPlugin_SYA_Limitset::BINTYPE_SOFTWARE;
    else if (bt=="hard")
        clSya_Limitset.m_eBinType = GexDbPlugin_SYA_Limitset::BINTYPE_HARDWARE;
    else
        GSLOG(SYSLOG_SEV_WARNING, "Execute SYA task : Unknown bin type to check. Will use default.");

    // Search for bintype to check
    QString lExclusionBins = ptTask->GetAttribute(YIELDMO_TASK_ATTR_SYA_BIN_EXCLUSION).toString();

    if(pDatabaseEntry->m_pExternalDatabase->SYA_CheckSplitlot(clSya_Limitset, listSyaItems, lExclusionBins) == false)
    {
        clDateTime.setTimeSpec(Qt::UTC);
        clDateTime.setTime_t(dbKeysContent.Get("StartTime").toUInt());
        pDatabaseEntry->m_pExternalDatabase->GetLastError(strError) ;
        strErrorMessage =  "  > *ERROR* Couldn't check SYA alarms on this splitlot\n";
        strErrorMessage += "  > File name      = " + dbKeysContent.Get("FileName").toString() + "\n";
        strErrorMessage += "  > Product        = " + dbKeysContent.Get("Product").toString() + "\n";
        strErrorMessage += "  > Lot            = " + dbKeysContent.Get("Lot").toString() + "\n";
        strErrorMessage += "  > Wafer          = " + dbKeysContent.Get("Wafer").toString() + "\n";
        strErrorMessage += "  > Test date/time = " + clDateTime.toString(Qt::ISODate) + "\n";
        strErrorMessage += "  > Error          = " + strError;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMessage.toLatin1().data() );
        return strErrorMessage;
    }

    // If no SYA failures, return
    if(listSyaItems.count() == 0)
    {
        return "";
    }

    // Get SYA rule for product
    GexMoYieldMonitoringTaskData *ptYield=NULL;

    // Check if right Product
    if(ptTask->GetProperties()->strProductID.toLower() == clSya_Limitset.m_strProduct_ID.toLower())
        ptYield = ptTask->GetProperties();

    if(ptYield == NULL)
    {
        return "";
    }

    // Check if rule disabled
    if(!ptTask->IsUsable())
    {
        return "";
    }

    GexMoSendEmail			Email;
    QString					strFrom,strTo,strSubject,strTitle,strFilePath,strEmailBody;

    // Clear email message
    strEmailBody = "";

    // Init email fields
    strFrom = ptYield->strEmailFrom;
    strTo = ptYield->strEmailNotify;

    // Get Email spooling folder.
    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        strErrorMessage = "  > Reporting: failed to get Email spooling folder";
        return strErrorMessage;
    }

    // We have a spooling folder: create email file in it!
    strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;


    if(!(GS::LPPlugin::ProductInfo::getInstance()->isSYLSBLAllowed()))
    {
        strSubject = "** SYA ALARM DISABLED **";
        strEmailBody = "An SYA alarm has been detected, but SYA is now an optional module, and the SYA option is not enabled in your current license file.\n";
        strEmailBody += "\n";
        strEmailBody += "Please contact our sales team at "+QString(GEX_EMAIL_SALES)+" for more information.\n";
        strEmailBody += "\n";
        strEmailBody += "*******************************************************************\n";
        strEmailBody += "* Quantix Semiconductor Technical Support\n";
        strEmailBody += "*\n";
        strEmailBody += "* Software Solutions to Reduce Cost of Test\n";
        strEmailBody += "*\n";
        strEmailBody += "* Web site: www.mentor.com\n";
        strEmailBody += "* Quantix support: "+QString(GEX_EMAIL_SUPPORT)+"\n";
        strEmailBody += "*******************************************************************\n";
        // Send warning email
        Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody);
        return "";
    }


    // Rule enabled, and we have SYA failures: trigger alarm

    // Get SYA item
    GexDbPlugin_SYA_Item *pSyaItem = listSyaItems.first();

    // Insert alarm into DB
    // Unlike other tasks, it's not the rule exception level that dictates the type of alarm, but the
    // type of SYA exception
    QString	strItemName = "SYA rule: " + ptYield->strTitle;
    GexDbPlugin_Base::AlarmLevels lLevels=GexDbPlugin_Base::eAlarmLevel_Standard ;
    GexDbPlugin_Base::AlarmCategories lCat=GexDbPlugin_Base::eAlarmCat_SYA ;

    // Alarm Levels
    if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
        lLevels = GexDbPlugin_Base::eAlarmLevel_Critical;

    // Category
    if(pSyaItem->m_eFailStatus_SYL != eSyaFail_None)
        lCat = GexDbPlugin_Base::eAlarmCat_SYA_SYL;
    if(pSyaItem->m_eFailStatus_SBL != eSyaFail_None)
        lCat = GexDbPlugin_Base::eAlarmCat_SYA_SBL;

    pDatabaseEntry->m_pExternalDatabase->InsertAlarm(
                lCat, lLevels, clSya_Limitset.m_ulLimitset_ID , strItemName, 0, 0.0F, 0.0F, 0.0F, "");

    // Check if need to append report
    QString strReportType;
    switch(ptYield->iEmailReportType)
    {
    case 0:	// Text: no need to create + append report
        break;
    case 1:	// WORD report to create & append
        strReportType = "word";
        break;
    case 2:	// CSV report to create & append
        strReportType = "excel";
        break;
    case 3:	// PowerPoint report to create & append
        strReportType = "ppt";
        break;
    case 4:	// PDF report to create & append
        strReportType = "pdf";
        break;
    }

    // If report to append, create it now
    QString	strReportName;
    QString strHttpReportName = "";
    QString strAttachment;
    if(ptYield->iEmailReportType)
    {
        if(gexReport->buildReportArborescence(&ReportOptions,strReportType))
        {
            bool bNeedPostProcessing = false;

            gexReport->CreateTestReportPages(GS::Gex::Engine::GetInstance().GetExpirationDate(),
                                             &bNeedPostProcessing);

            // Does the report need post-processing?
            if(bNeedPostProcessing == true)
                gexReport->ReportPostProcessingConverter();
        }

        // If report to be sent as an attachment, then tell where the report is located
        if(ptYield->iNotificationType == 0)
            strAttachment = gexReport->reportAbsFilePath();
        else
        {
            // Check if specify http path to report
            if (!ptStatusTask->GetProperties()->reportHttpURL().isEmpty())
                strHttpReportName = ptStatusTask->GetProperties()->reportHttpURL() + QDir::cleanPath("/" + gexReport->reportRelativeFilePath());

        }
    }

    // Build email + send it.
    int						iTotalAlarmCount=1;
    int						lAlarmSeverity=0;
    GexMoBuildEmailString	cEmailString;
    bool					bHtmlEmail = ptYield->bHtmlEmail;	// 'true' if email to be sent in HTML format
    QString					strText, strText2;
    QString					strYield;
    QDate					clCurrentDate = QDate::currentDate();

    // Subject is: ** SYA ALARM (critical|standard) ** (Product=<product>, Lot=<lot>, Sublot=<sublot>, Wafer=<wafer>)
    if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
        strSubject = "** SYA ALARM (critical) **";
    else
        strSubject = "** SYA ALARM (standard) **";
    strSubject += " (Product=" + clSya_Limitset.m_strProduct_ID;
    strSubject += ", Lot=" + pSyaItem->m_strLotID;
    if(!dbKeysContent.Get("SubLot").toString().isEmpty())
        strSubject += ", Sublot=" + dbKeysContent.Get("SubLot").toString();
    if(!pSyaItem->m_strWaferID.isEmpty())
        strSubject += ", Wafer=" + pSyaItem->m_strWaferID;
    strSubject += ")";

    // Build email!
    time_t lStartTime = dbKeysContent.Get("StartTime").toUInt();

    // Title
    if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
        strTitle = "** SYA ALARM (critical) **";
    else
        strTitle = "** SYA ALARM (standard) **";
    if(bHtmlEmail)
    {
        // HTML Header
        cEmailString.CreatePage(strTitle);

        // Table with dataset details
        cEmailString.AddHtmlString("<h2><font color=\"#000080\">Dataset</font></h2>\n");
        cEmailString.WriteHtmlOpenTable();
        cEmailString.WriteInfoLine("Testing date",TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
        cEmailString.WriteInfoLine("Product",clSya_Limitset.m_strProduct_ID);
        cEmailString.WriteInfoLine("Lot",pSyaItem->m_strLotID);
        cEmailString.WriteInfoLine("SubLot",dbKeysContent.Get("SubLot").toString());
        cEmailString.WriteInfoLine("Wafer ID",pSyaItem->m_strWaferID);
        cEmailString.WriteInfoLine("Tester",dbKeysContent.Get("TesterName").toString().isEmpty() ? "n/a" : dbKeysContent.Get("TesterName").toString());
        cEmailString.WriteInfoLine("Operator",dbKeysContent.Get("Operator").toString().isEmpty() ? "n/a" : dbKeysContent.Get("Operator").toString());
        cEmailString.WriteInfoLine("Program name",dbKeysContent.Get("ProgramName").toString().isEmpty() ? "n/a" : dbKeysContent.Get("ProgramName").toString());
        cEmailString.WriteInfoLine("Program revision",dbKeysContent.Get("ProgramRevision").toString().isEmpty() ? "n/a" : dbKeysContent.Get("ProgramRevision").toString());
        cEmailString.WriteInfoLine("Total parts tested",QString::number(pSyaItem->m_uiPartsTested));
        if(pSyaItem->m_uiGrossDie > 0)
            cEmailString.WriteInfoLine("Gross Die",QString::number(pSyaItem->m_uiGrossDie));
        // Unlike other tasks, it's not the rule exception level that dictates the type of alarm, but the
        // type of SYA exception
        if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
            cEmailString.WriteInfoLine("Alarm level", "Critical", true);
        else
            cEmailString.WriteInfoLine("Alarm level", "Standard", false, true);
        cEmailString.WriteHtmlCloseTable();

        // Table with SYA rule details
        cEmailString.AddHtmlString("<h2><font color=\"#000080\">SYA rule</font></h2>\n");
        cEmailString.WriteHtmlOpenTable();
        cEmailString.WriteInfoLine("Rule name", ptYield->strTitle );
        if(ptYield->GetAttribute("ExpirationDate").toDate() > clCurrentDate)
            cEmailString.WriteInfoLine("Rule expiration", ptYield->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy"));
        else
            cEmailString.WriteInfoLine("Rule expiration", ptYield->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy"), true);
        cEmailString.WriteInfoLine("Filtering rule", ptYield->strSYA_Rule);
        cEmailString.WriteInfoLine("N1-Value", QString::number(ptYield->fSYA_N1_value));
        cEmailString.WriteInfoLine("N2-Value", QString::number(ptYield->fSYA_N2_value));
        cEmailString.WriteHtmlCloseTable();

        // Table with SYA set details
        cEmailString.AddHtmlString("<h2><font color=\"#000080\">SYA set</font></h2>\n");
        cEmailString.WriteHtmlOpenTable();
        cEmailString.WriteInfoLine("SYA ID", QString::number(clSya_Limitset.m_ulLimitset_ID));
        cEmailString.WriteInfoLine("User comment", clSya_Limitset.m_strUserComment);
        strText = clSya_Limitset.m_clCreationDate.toString("dd MMM yyyy hh:mm:ss");
        cEmailString.WriteInfoLine("Creation date", strText);
        strText = clSya_Limitset.m_clExpirationDate.toString("dd MMM yyyy");
        if(clSya_Limitset.m_clExpirationDate > clCurrentDate)
            cEmailString.WriteInfoLine("Expiration date", strText);
        else
            cEmailString.WriteInfoLine("Expiration date", strText, true);
        cEmailString.WriteHtmlCloseTable();

        // If Full Report to be created
        if(ptYield->iEmailReportType)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(ptYield->iNotificationType == 0)
                cEmailString.AddHtmlString("<br><b>Full report attached to this message!</b><br>\n");
            else
            {
                // Leave report on server, email URL only.
                cEmailString.AddHtmlString("<br><b>Full Report available on server at:<br>\n");

                // Create Hyperlilnk
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
                    cEmailString.AddHtmlString(strReportName);
                    cEmailString.AddHtmlString("\">");
                    // Display File name
                    cEmailString.AddHtmlString(strReportName);
                }

                // Close URL hyperlink
                cEmailString.AddHtmlString("</a>");

                // Close message
                cEmailString.AddHtmlString("</b><br><br>\n");
            }
        }
    }
    else  // raw text email (not html)
    {
        // Plain Text email.
        // Dataset details
        strEmailBody = "\n#### DATASET ##################################################\n";
        strEmailBody += QString("Testing date       : ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + "\n";
        strEmailBody += QString("Product            : ") + clSya_Limitset.m_strProduct_ID + "\n";
        strEmailBody += QString("Lot                : ") + pSyaItem->m_strLotID + "\n";
        strEmailBody += QString("SubLot             : ") + dbKeysContent.Get("SubLot").toString() + "\n";
        strEmailBody += QString("Wafer ID           : ") + pSyaItem->m_strWaferID + "\n";
        strEmailBody += QString("Tester             : ") + (dbKeysContent.Get("TesterName").toString().isEmpty() ? QString("n/a\n") : QString(dbKeysContent.Get("TesterName").toString() + "\n"));
        strEmailBody += QString("Operator           : ") + (dbKeysContent.Get("Operator").toString().isEmpty() ? QString("n/a\n") : QString(dbKeysContent.Get("Operator").toString() + "\n"));
        strEmailBody += QString("Program name       : ") + (dbKeysContent.Get("ProgramName").toString().isEmpty() ? QString("n/a\n") : QString(dbKeysContent.Get("ProgramName").toString() + "\n"));
        strEmailBody += QString("Program revision   : ") + (dbKeysContent.Get("ProgramRevision").toString().isEmpty() ? QString("n/a\n") : QString(dbKeysContent.Get("ProgramRevision").toString() + "\n"));
        strEmailBody += QString("Total parts tested : ") + QString::number(pSyaItem->m_uiPartsTested) + "\n";
        if(pSyaItem->m_uiGrossDie > 0)
            strEmailBody += QString("Gross Die          : ") + QString::number(pSyaItem->m_uiGrossDie) + "\n";
        // Unlike other tasks, it's not the rule exception level that dictates the type of alarm, but the
        // type of SYA exception
        if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
            strEmailBody += QString("Alarm level        : Critical\n");
        else
            strEmailBody += QString("Alarm level        : Standard\n");

        // SYA rule details
        strEmailBody += "\n#### SYA RULE #################################################\n";
        strEmailBody += QString("Rule title         : ") + ptYield->strTitle + "\n";
        if(ptYield->GetAttribute("ExpirationDate").toDate() > clCurrentDate)
            strEmailBody += QString("Rule expiration    : ") + ptYield->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy") + "\n";
        else
            strEmailBody += QString("Rule expiration    : ")
                    + ptYield->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy") + " (EXPIRED!)\n";
        strEmailBody += QString("Filtering rule     : ") + ptYield->strSYA_Rule + "\n";
        strEmailBody += QString("N1-Value           : ") + QString::number(ptYield->fSYA_N1_value) + "\n";
        strEmailBody += QString("N2-Value           : ") + QString::number(ptYield->fSYA_N2_value) + "\n";

        // SYA set details
        strEmailBody += "\n#### SYA SET DETAILS ##########################################\n";
        strEmailBody += QString("SYA ID             : ") + QString::number(clSya_Limitset.m_ulLimitset_ID) + "\n";
        strEmailBody += QString("User comment       : ") + clSya_Limitset.m_strUserComment + "\n";
        strText = clSya_Limitset.m_clCreationDate.toString("dd MMM yyyy hh:mm:ss");
        strEmailBody += QString("Creation date      : ") + strText + "\n";
        strText = clSya_Limitset.m_clExpirationDate.toString("dd MMM yyyy");
        if(clSya_Limitset.m_clExpirationDate > clCurrentDate)
            strEmailBody += QString("Expiration date    : ") + strText + "\n";
        else
            strEmailBody += QString("Expiration date    : ") + strText + "(EXPIRED!)\n";

        strEmailBody += "\n";
        strEmailBody += "###############################################################\n\n";

        // If Full Report to be created
        if(ptYield->iEmailReportType)
        {
            // Check if report to be attached or remain on server and only the URL to send...
            if(ptYield->iNotificationType == 0)
                strEmailBody += "Full report attached to this message!\n";
            else
            {
                // Leave report on server, email URL only.
                strEmailBody += "Full Report available on server at:\n  ";
                strEmailBody += strReportName;

                strEmailBody += "\n";
            }
            strEmailBody += "\n";
        }
    }

    // Add Bin summary
    QString strSummary = GetBinSummaryString(pSyaItem, &clSya_Limitset, ptYield->bHtmlEmail?HTML_FORMAT:TSV_FORMAT);
    if(bHtmlEmail)
        cEmailString.AddHtmlString(strSummary);
    else
        strEmailBody += strSummary;

    // Close HTML email string if need be
    if(bHtmlEmail)
        strEmailBody = cEmailString.ClosePage();

    // We have a spooling folder: create email file in it!
    strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;

    // Send email with Yield Monitoring alarm message + report.
    Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptYield->bHtmlEmail,strAttachment);

    // Keep track of highest severity level encountered.
    lAlarmSeverity = gex_max(ptYield->iExceptionLevel,lAlarmSeverity);

    // If one or more yield alarms detected for this ProductID, launch the shell
    if(iTotalAlarmCount == 0)
    {
        lAlarmSeverity = -1;
    }
    LaunchAlarmShell(ShellYield, lAlarmSeverity, iTotalAlarmCount,
                     dbKeysContent.Get("Product").toString(), dbKeysContent.Get("Lot").toString(),
                     dbKeysContent.Get("SubLot").toString(), dbKeysContent.Get("Wafer").toString(),
                     dbKeysContent.Get("TesterName").toString(), dbKeysContent.Get("Operator").toString(),
                     dbKeysContent.Get("FileName").toString(), "?" );

    return "";	// Return error message
}
