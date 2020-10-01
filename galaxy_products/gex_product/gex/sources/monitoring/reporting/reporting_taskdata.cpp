#include <QCheckBox>
#include <QFileDialog>

#include <gqtl_log.h>

#include "browser_dialog.h"
#include "engine.h"
#include "report_build.h"
#include "gex_report.h"
#include "reporting_taskdata.h"
#include "scheduler_engine.h"
#include "status/status_task.h"
#include "status/status_taskdata.h"
#include "mo_email.h"
#include "reporting/reporting_task.h"
#include "gexmo_constants.h"
#include "gex_shared.h"
#include "report_options.h"
#include "script_wizard.h"
#include "product_info.h"
#include "csl/csl_engine.h"
#include "message.h"

// main.cpp
extern void				WriteDebugMessageFile(const QString & strMessage);
extern GexMainwindow *	pGexMainWindow;

// in report_build.cpp
extern CGexReport*		gexReport;				// Handle to report class
extern CReportOptions	ReportOptions;			// Holds options

///////////////////////////////////////////////////////////
// Structure constructor
///////////////////////////////////////////////////////////
GexMoReportingTaskData::GexMoReportingTaskData(QObject* parent): TaskProperties(parent)
{
    iFrequency          = 0;
    iNotificationType   = 0;
    iDayOfWeek          = 0;
    bHtmlEmail          = false;
    bExecutionWindow    = false;

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strEmailFrom = GEX_EMAIL_PAT_MAN;
    else
        strEmailFrom = GEX_EMAIL_YIELD_MAN;

    mPriority = 1;
}

void GexMoReportingTaskData::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();
    // Insert new values
    SetPrivateAttribute("Title",strTitle);
    SetPrivateAttribute("ScriptPath",strScriptPath);
    SetPrivateAttribute("Frequency",QString::number(iFrequency));
    SetPrivateAttribute("DayOfWeek",QString::number(iDayOfWeek));
    SetPrivateAttribute("ExecWindow",(bExecutionWindow ? "YES" : "NO"));
    SetPrivateAttribute("StartTime",QString::number(cStartTime.hour()) + "," + QString::number(cStartTime.minute()));
    SetPrivateAttribute("StopTime",QString::number(cStopTime.hour()) + "," + QString::number(cStopTime.minute()));
    SetPrivateAttribute("NotificationType",QString::number(iNotificationType));
    SetPrivateAttribute("EmailFrom",strEmailFrom);
    SetPrivateAttribute("Emails",strEmailNotify);
    SetPrivateAttribute("EmailFormat",(bHtmlEmail ? "HTML" : "TEXT"));
    SetPrivateAttribute("Priority",QVariant(mPriority));

}

GexMoReportingTaskData &GexMoReportingTaskData::operator=(const GexMoReportingTaskData &copy)
{
    if(this != &copy)
    {
        strTitle                = copy.strTitle;
        strScriptPath           = copy.strScriptPath;
        iFrequency              = copy.iFrequency;
        iDayOfWeek              = copy.iDayOfWeek;
        bExecutionWindow        = copy.bExecutionWindow;
        cStartTime              = copy.cStartTime;
        cStopTime               = copy.cStopTime;
        iNotificationType       = copy.iNotificationType;
        strEmailFrom            = copy.strEmailFrom;
        strEmailNotify          = copy.strEmailNotify;
        bHtmlEmail              = copy.bHtmlEmail;
        mPriority               = copy.mPriority;

        TaskProperties::operator =(copy);
        UpdatePrivateAttributes();
    }
    return *this;
}
///////////////////////////////////////////////////////////
// Execute Reporting task...
///////////////////////////////////////////////////////////
QString GS::Gex::SchedulerEngine::ExecuteReportingTask(CGexMoTaskReporting *ptTask)
{
    QDateTime	cCurrentDateTime = GS::Gex::Engine::GetInstance().GetClientDateTime();

    QString strString;
    strString = "Execute Reporting Task ";
    strString += ptTask->GetProperties()->strTitle;
    strString += ", ";
    strString += cCurrentDateTime.toString("hh:mm:ss")+"...";
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().data() );

    QString strScriptFile;
    QString strErrorMessage;

    // Reset HTML sections to create flag: ALL pages to create.
    /*
    if(pGexMainWindow)
        pGexMainWindow->iHtmlSectionsToSkip = 0;
    */
    ReportOptions.iHtmlSectionsToSkip=0;

    // Get script name to execute.
    strScriptFile = ptTask->GetProperties()->strScriptPath;

    // Add task to task manager
    onStartTask(ptTask,"FileName="+strScriptFile);

    // Execute selected script.
    GS::Gex::CSLStatus lStatus = GS::Gex::CSLEngine::GetInstance().RunScript(strScriptFile);

    if(lStatus.IsFailed())
        strErrorMessage = "  > " + lStatus.GetErrorMessage();

    // Email notification.
    GexMoBuildEmailString cEmailString;
    bool	bHtmlEmail = ptTask->GetProperties()->bHtmlEmail;	// 'true' if email to be sent in HTML format
    GexMoSendEmail Email;
    QString strFilePath;
    QString strFrom,strTo,strSubject;
    QString strEmailBody;
    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
        strFrom = GEX_EMAIL_PAT_MAN;
    else
        strFrom = GEX_EMAIL_YIELD_MAN;
    if ( ptTask->GetProperties()->strEmailFrom.isEmpty() == false )
        strFrom = ptTask->GetProperties()->strEmailFrom;
    strTo = ptTask->GetProperties()->strEmailNotify;
    strSubject = ptTask->GetProperties()->strTitle;

    // Get Email spooling folder.
    CGexMoTaskStatus *ptStatusTask = GetStatusTask();
    if(ptStatusTask == NULL)
    {
        strErrorMessage = "  > Reporting: failed to get Email spooling folder";
        onStopTask(ptTask,strErrorMessage);
        return strErrorMessage;
    }

    // We have a spooling folder: create email file in it!
    strFilePath = ptStatusTask->GetProperties()->intranetPath() + "/";
    strFilePath += GEXMO_AUTOREPORT_FOLDER + QString("/");
    strFilePath += GEXMO_AUTOREPORT_EMAILS;

    QDateTime cTime = GS::Gex::Engine::GetInstance().GetClientDateTime();

    if(bHtmlEmail)
    {
        // HTML Header
        cEmailString.CreatePage(strSubject);

        // Table with Lot Info
        cEmailString.WriteHtmlOpenTable();

        // Write table
        cEmailString.WriteInfoLine("Report Date",cTime.toString("d MMMM yyyy h:mm:ss"));
        cEmailString.WriteInfoLine("Script title",ptTask->GetProperties()->strTitle);
        cEmailString.WriteInfoLine("Email to",ptTask->GetProperties()->strEmailNotify);
        cEmailString.WriteInfoLine("Script executed",strScriptFile);
        // Table with Lot Info
        cEmailString.WriteHtmlCloseTable();
    }
    else
    {

        strEmailBody = "\n###############################################################\n";
        strEmailBody += QString("Report Date     : ") + cTime.toString("d MMMM yyyy h:mm:ss") + QString("\n");
        strEmailBody +=       "\nScript title    : " + ptTask->GetProperties()->strTitle;
        strEmailBody +=       "\nEmail to        : " + ptTask->GetProperties()->strEmailNotify;
        strEmailBody +=       "\nScript executed : " + strScriptFile;
        strEmailBody += "\n###############################################################\n";
    }

    // Check if error on executing script
    if(strErrorMessage.isEmpty() == false)
    {
        // Error executing script.
        QString strError = "*** Error message: " + strErrorMessage;

        if(bHtmlEmail)
        {
            cEmailString.AddHtmlString("<br>");
            cEmailString.AddHtmlString(strError);
            strEmailBody = cEmailString.ClosePage();
        }
        else
        {
            strEmailBody += "\n" + strError;
            strEmailBody += "\n";
        }

        Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptTask->GetProperties()->bHtmlEmail);
    }
    else
    {
        // Script successful
        int iNotificationType = ptTask->GetProperties()->iNotificationType;
        bool bReportExists = QFile::exists(gexReport->reportAbsFilePath());
        switch(iNotificationType)
        {
        case 0:		// Email report as attachment
            if(bReportExists)
                cEmailString.AddHtmlString("<br>See report attached to this email!<br>");
            else
                strEmailBody += "\n\nEmail notification: Script executed.\n";
            Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptTask->GetProperties()->bHtmlEmail,gexReport->reportAbsFilePath());
            break;
        case 1:		// Report on intranet
            if(bReportExists)
            {
                QString strHttpReportName;
                // Check if specify http path to report
                if (!ptStatusTask->GetProperties()->reportHttpURL().isEmpty())
                    strHttpReportName = ptStatusTask->GetProperties()->reportHttpURL() + QDir::cleanPath("/" + gexReport->reportRelativeFilePath());

                if(bHtmlEmail)
                {
                    cEmailString.AddHtmlString("Report posted to:<br>");
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

                    strEmailBody = cEmailString.ClosePage();
                }
                else
                {
                    strEmailBody += "\n\nReport posted to:\n  ";
                    if (!strHttpReportName.isEmpty())
                        strEmailBody += strHttpReportName;
                    else
                        strEmailBody += gexReport->reportAbsFilePath();
                }
            }
            else
            {
                strEmailBody += "\n\nEmail notification: Script executed.\n  ";
                strEmailBody += "\n";
            }

            Email.Send(strFilePath,strFrom,strTo,strSubject,strEmailBody,ptTask->GetProperties()->bHtmlEmail);
            break;
        case 2:		// No notification (used for scripts that do sweeping stuff)
            break;
        }
    }

    // Remove task from task manager
    onStopTask(ptTask,strErrorMessage);
    return strErrorMessage;	// Return error message
}


