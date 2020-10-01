#include <gqtl_log.h>

#include "browser_dialog.h"
#include "scheduler_engine.h"
#include "mo_email.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_task.h"
#include "gexdb_plugin_base.h"
#include "gex_report.h"
#include "report_build.h"
#include "report_options.h"


extern CGexReport*      gexReport;          // Handle to report class if yield check failed
extern CReportOptions   ReportOptions;

QString GS::Gex::SchedulerEngine::SendAlarmEmail(
        GexMoYieldMonitoringTaskData* td,
        CGexMoTaskItem* /*ptTask*/, // unusefull : remmove me !
        GexDbPlugin_SYA_Item *pSyaItem,
        GexDbPlugin_SYA_Limitset &clSya_Limitset,
        QString logfile)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Send Alarm Email for product %1").arg( clSya_Limitset.m_strProduct_ID).toLatin1().constData());

    if ( (td==NULL)
         //|| (ptTask==NULL) // actualy unusefull
         || (pSyaItem==NULL) )
    {
        GSLOG(SYSLOG_SEV_ERROR, "SendAlarmEmail impossible because of some NULL params");
        return "error";
    }

    // Check if need to append report in email
    QString strReportType;
    switch(td->iEmailReportType)
    {
    case 0:	// Text: no need to create + append report
        break;
    case 1:	strReportType = "word"; // WORD report to create & append
        break;
    case 2:	strReportType = "excel"; // CSV report to create & append
        break;
    case 3:	strReportType = "ppt"; // PowerPoint report to create & append
        break;
    case 4:	strReportType = "pdf"; // PDF report to create & append
        break;
    default: strReportType ="";
    }

    // If report to append, create it now
    QString strAttachment;

    if(td->iEmailReportType)
    {
        GSLOG(SYSLOG_SEV_WARNING, "SYA Alarm email currently only support attached ASCII/HTML report, no DOC/CSV/PDF/PPT.");
        /*
        if(gexReport->buildReportArborescence(&ReportOptions,strReportType))
        {
            bool bNeedPostProcessing = false;
            gexReport->CreateTestReportPages(ExpirationDate,GS::Gex::Engine::GetInstance().GetReleaseDate(),&bNeedPostProcessing);
            // Does the report need post-processing?
            if(bNeedPostProcessing == true)
                gexReport->ReportPostProcessingConverter();
        }

        // If report to be sent as an attachment, then tell where the report is located
        if(td->iNotificationType == 0)
            strAttachment = gexReport->reportAbsFilePath();
        else
        {
            // Check if specify http path to report
            if (!ptTask->ptStatus->reportHttpURL().isEmpty())
                strHttpReportName = ptTask->ptStatus->reportHttpURL() + QDir::cleanPath("/" + gexReport->reportRelativeFilePath());
        }
        */
    } // EmailReportType

    // Build email + send it.
    QString					strEmailBody;
    int						iTotalAlarmCount=1;
    int						iAlarmSeverity=0;
    GexMoBuildEmailString	cEmailString;
    bool					bHtmlEmail = td->bHtmlEmail;	// 'true' if email to be sent in HTML format
    GexMoSendEmail			Email;
    QString					strFilePath, strText;
    QString					strFrom,strTo,strSubject,strTitle;
    QDate					clCurrentDate = QDate::currentDate();

    // Clear email message
    strEmailBody = "";

    // Init email fields
    strFrom = td->strEmailFrom;
    strTo = td->strEmailNotify;

    // Subject is: ** SYA ALARM (critical|standard) ** (Product=<product>, Lot=<lot>, Sublot=<sublot>, Wafer=<wafer>)
    if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical)
            || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
        strSubject = "** SYA ALARM (critical) **";
    else
        strSubject = "** SYA ALARM (standard) **";
    strSubject += " (Product=" + clSya_Limitset.m_strProduct_ID;
    strSubject += ", Lot=" + pSyaItem->m_strLotID;

    if(!pSyaItem->m_strWaferID.isEmpty())
        strSubject += ", Wafer=" + pSyaItem->m_strWaferID;
    strSubject += ")";

    // Build email!
    //time_t lStartTime = pKeyContent->t_StartTime;

    // Title
    if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical)
            || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
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
        //cEmailString.WriteInfoLine("Testing date",TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss"));
        cEmailString.WriteInfoLine("Product",clSya_Limitset.m_strProduct_ID);
        cEmailString.WriteInfoLine("Lot",pSyaItem->m_strLotID);
        //cEmailString.WriteInfoLine("SubLot", pKeyContent->strSubLot);
        cEmailString.WriteInfoLine("Wafer ID",pSyaItem->m_strWaferID);
        //cEmailString.WriteInfoLine("Tester",pKeyContent->strTesterName.isEmpty() ? "n/a" : pKeyContent->strTesterName);
        //cEmailString.WriteInfoLine("Operator",pKeyContent->strOperator.isEmpty() ? "n/a" : pKeyContent->strOperator);
        //cEmailString.WriteInfoLine("Program name",pKeyContent->strJobName.isEmpty() ? "n/a" : pKeyContent->strJobName);
        //cEmailString.WriteInfoLine("Program revision",pKeyContent->strJobRev.isEmpty() ? "n/a" : pKeyContent->strJobRev);
        cEmailString.WriteInfoLine("Total parts tested",
                                   QString::number(pSyaItem->m_uiPartsTested));
        if(pSyaItem->m_uiGrossDie > 0)
            cEmailString.WriteInfoLine("Gross Die",
                                       QString::number(pSyaItem->m_uiGrossDie));
        // Unlike other tasks, it's not the rule exception level that dictates the type of alarm, but the
        // type of SYA exception
        if((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical)
                || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
            cEmailString.WriteInfoLine("Alarm level", "Critical", true);
        else
            cEmailString.WriteInfoLine("Alarm level", "Standard", false, true);
        cEmailString.WriteHtmlCloseTable();

        // Table with SYA rule details
        cEmailString.AddHtmlString("<h2><font color=\"#000080\">SYA rule</font></h2>\n");
        cEmailString.WriteHtmlOpenTable();
        cEmailString.WriteInfoLine("Rule name", td->strTitle );
        if(td->GetAttribute("ExpirationDate").toDate() > clCurrentDate)
            cEmailString.WriteInfoLine("Rule expiration",
                                       td->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy"));
        else
            cEmailString.WriteInfoLine("Rule expiration",
                                       td->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy"), true);
        cEmailString.WriteInfoLine("Filtering algorithm", td->strSYA_Rule);
        cEmailString.WriteInfoLine("N1-Value", QString::number(td->fSYA_N1_value));
        cEmailString.WriteInfoLine("N2-Value", QString::number(td->fSYA_N2_value));
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
        if(td->iEmailReportType)
        {
            //cEmailString.WriteHtmlOpenTable();
            //cEmailString.WriteInfoLine("Warning", "SYA Alarm email currently only support attached ASCII/HTML report, no DOC/CSV/PDF/PPT.", false);
            //cEmailString.WriteHtmlCloseTable();

            /*
            // Check if report to be attached or remain on server and only the URL to send...
            if(td->iNotificationType == 0)
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
                    cEmailString.AddHtmlString("<a href=\"file://");
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
            */
        }
    } // html
    else  // raw text email (not html)
    {
        // Plain Text email.
        // Dataset details
        strEmailBody = "\n#### DATASET ##################################################\n";
        //strEmailBody += QString("Testing date       : ") + TimeStringUTC_F(lStartTime, "d MMMM yyyy h:mm:ss") + "\n";
        strEmailBody += QString("Product             : ") + clSya_Limitset.m_strProduct_ID + "\n";
        strEmailBody += QString("Lot                 : ") + pSyaItem->m_strLotID + "\n";
        //strEmailBody += QString("SubLot             : ") + pKeyContent->strSubLot + "\n";
        strEmailBody += QString("Wafer ID            : ") + pSyaItem->m_strWaferID + "\n";
        //strEmailBody += QString("Tester             : ") + (pKeyContent->strTesterName.isEmpty() ? QString("n/a\n") : QString(pKeyContent->strTesterName + "\n"));
        //strEmailBody += QString("Operator           : ") + (pKeyContent->strOperator.isEmpty() ? QString("n/a\n") : QString(pKeyContent->strOperator + "\n"));
        //strEmailBody += QString("Program name       : ") + (pKeyContent->strJobName.isEmpty() ? QString("n/a\n") : QString(pKeyContent->strJobName + "\n"));
        //strEmailBody += QString("Program revision   : ") + (pKeyContent->strJobRev.isEmpty() ? QString("n/a\n") : QString(pKeyContent->strJobRev + "\n"));
        strEmailBody += QString("Total parts tested  : ") + QString::number(pSyaItem->m_uiPartsTested) + "\n";
        if(pSyaItem->m_uiGrossDie > 0)
            strEmailBody += QString("Gross Die           : ") + QString::number(pSyaItem->m_uiGrossDie) + "\n";
        // Unlike other tasks, it's not the rule exception level that dictates the type of alarm, but the
        // type of SYA exception
        if ((pSyaItem->m_eFailStatus_SYL == eSyaFail_Critical) || (pSyaItem->m_eFailStatus_SBL == eSyaFail_Critical))
            strEmailBody += QString("Alarm level         : Critical\n");
        else
            strEmailBody += QString("Alarm level         : Standard\n");

        // SYA rule details
        strEmailBody += "\n#### SYA RULE #################################################\n";
        strEmailBody += QString("Rule title          : ")
                + td->strTitle + "\n";
        if(td->GetAttribute("ExpirationDate").toDate() > clCurrentDate)
            strEmailBody += QString("Rule expiration     : ")
                    + td->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy") + "\n";
        else
            strEmailBody += QString("Rule expiration     : ")
                    + td->GetAttribute("ExpirationDate").toDate().toString("dd MMM yyyy") + " (EXPIRED!)\n";
        strEmailBody += QString("Filtering algorithm : ") + td->strSYA_Rule + "\n";
        strEmailBody += QString("N1-Value            : ") + QString::number(td->fSYA_N1_value) + "\n";
        strEmailBody += QString("N2-Value            : ") + QString::number(td->fSYA_N2_value) + "\n";


        // SYA set details
        strEmailBody += "\n#### SYA SET DETAILS ##########################################\n";
        strEmailBody += QString("SYA ID              : ") + QString::number(clSya_Limitset.m_ulLimitset_ID) + "\n";
        strEmailBody += QString("User comment        : ") + clSya_Limitset.m_strUserComment + "\n";
        strText = clSya_Limitset.m_clCreationDate.toString("dd MMM yyyy hh:mm:ss");
        strEmailBody += QString("Creation date       : ") + strText + "\n";
        strText = clSya_Limitset.m_clExpirationDate.toString("dd MMM yyyy");
        if(clSya_Limitset.m_clExpirationDate > clCurrentDate)
            strEmailBody += QString("Expiration date     : ") + strText + "\n";
        else
            strEmailBody += QString("Expiration date     : ") + strText + "(EXPIRED!)\n";

        strEmailBody += "\n";
        strEmailBody += "###############################################################\n\n";

        // If Full Report to be created
        if(td->iEmailReportType)
        {
            GSLOG(SYSLOG_SEV_WARNING, "SYA Alarm Email only support attached ASCII or HTML report");
            /*
            // Check if report to be attached or remain on server and only the URL to send...
            if(td->iNotificationType == 0)
                strEmailBody += "Full report attached to this message!\n";
            else
            {
                // Leave report on server, email URL only.
                strEmailBody += "Full Report available on server at:\n  ";
                strEmailBody += strReportName;
                strEmailBody += "\n";
            }
            strEmailBody += "\n";
            */
        }
    }

    // Add Bin summary
    QString strSummary = GetBinSummaryString(pSyaItem, &clSya_Limitset,
                                                                              td->bHtmlEmail?HTML_FORMAT:TSV_FORMAT );
    if(bHtmlEmail)
        cEmailString.AddHtmlString(strSummary);
    else
        strEmailBody += strSummary;

    // Close HTML email string if need be
    if(bHtmlEmail)
    {
        if(td->iEmailReportType)
        {
            //cEmailString.WriteHtmlOpenTable();
            //cEmailString.WriteInfoLine("Warning", "SYA Alarm email currently only support attached ASCII/HTML report, no DOC/CSV/PDF/PPT.", false);
            //cEmailString.WriteHtmlCloseTable();
        }
        strEmailBody = cEmailString.ClosePage();
    }

    CGexMoTaskStatus* st=GetStatusTask();
    if (!st)
    {
        GSLOG(SYSLOG_SEV_WARNING, "cant retrieve StatusTask through GexMoScheduler !");
        return "error : can't retrieve StatusTask !";
    }
    // We have a spooling folder: create email file in it!
    strFilePath = st->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;
    GSLOG(SYSLOG_SEV_DEBUG, QString("output path = %1").arg( strFilePath).toLatin1().constData());

    // Send email with Yield Monitoring alarm message + report.
    if (!Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,td->bHtmlEmail,strAttachment))
    {
        GSLOG(SYSLOG_SEV_WARNING, " failed to send email !");
        //return "error : Send() failed !";
    }

    // Keep track of highest severity level encountered.
    iAlarmSeverity = gex_max(td->iExceptionLevel,iAlarmSeverity);

    LaunchAlarmShell(
                ShellYield, iAlarmSeverity, iTotalAlarmCount,
                clSya_Limitset.m_strProduct_ID, pSyaItem->m_strLotID, "?",
                pSyaItem->m_strWaferID=="All"?"?":pSyaItem->m_strWaferID,
                "?", "?", "?", logfile
                );

    return "ok";
}


