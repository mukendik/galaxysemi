#include "gqtl_log.h"
#include "engine.h"
#include "gex_report.h"
#include "report_log_unit.h"
#include "report_log.h"

#include <QTextStream>
#include <QFile>

namespace GS
{
namespace Gex
{

const char * const cReportMessageUnitTitle = "Message Log";
const char * const cErrorTitle = "Error count";
const char * const cWarningTitle = "Warning count";
const char * const cInformationTitle = "Information count";
const char * const cMessageType = "Message Type";
const char * const cMessageContent = "Message Content";
const char * const cHtmlLineToUpdate = "Error (0)<br>Warning (0)<br>Information (0)";

ReportLogUnit::ReportLogUnit(CGexReport *report, const QString& cslkey) : ReportUnit(report, cslkey)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("new ReportMessageUnit : %1").arg(cslkey).toLatin1().constData());
}

ReportLogUnit::~ReportLogUnit()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "destructor...");
}

QString ReportLogUnit::CreatePages()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start").toLatin1().constData());
    if(mGexReport->GetReportLogList().count() == 0)
        return "ok";


    QString lFormat=mGexReport->GetOption("output", "format").toString();

    int lWarningCount = mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportWarning).count();
    int lErrorCount = mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportError).count();
    int lInfoCount =  mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportInformation).count();
    GS::Gex::ReportLogList &lReportLogList = mGexReport->GetReportLogList();


    if(lFormat=="CSV")
    {
        // Generating .CSV report file.
        fprintf(mGexReport->getReportFile(),"\n\n----%s----\n", cReportMessageUnitTitle);
        if(lErrorCount > 0)
            fprintf(mGexReport->getReportFile(),"%s:%d\n", cErrorTitle, lErrorCount);
        if(lWarningCount > 0)
            fprintf(mGexReport->getReportFile(),"%s:%d\n", cWarningTitle, lWarningCount);
        if(lInfoCount > 0)
            fprintf(mGexReport->getReportFile(),"%s:%d\n", cInformationTitle, lInfoCount);
        fprintf(mGexReport->getReportFile(),"\n");



    }
    else if(mGexReport->getReportOptions()->isReportOutputHtmlBased())
    {
        if(OpenFile(mGexReport->getReportOptions()->strReportDirectory + "/pages/log.htm").startsWith("error") )
            return "error : cant open file";

        mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000080");	// Default: Text is Dark Blue
        // Title + bookmark
        mGexReport->WriteHtmlSectionTitle(mGexReport->getReportFile(),"report_log", cReportMessageUnitTitle);
        fprintf(mGexReport->getReportFile(),"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
        if(lErrorCount > 0)
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), cErrorTitle, lErrorCount);
        if(lWarningCount > 0)
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), cWarningTitle, lWarningCount);
        if(lInfoCount > 0 )
            mGexReport->WriteInfoLine(mGexReport->getReportFile(), cInformationTitle, lInfoCount);

        // Close header table
        fprintf(mGexReport->getReportFile(),"</table>\n<br>\n");

        // List Message
        fprintf(mGexReport->getReportFile(),"<table border=\"0\" width=\"750\" cellspacing=\"1\">\n");
        fprintf(mGexReport->getReportFile(),"<tr><td width=\"160\" bgcolor=%s valign=\"top\" align=\"left\"><b>%s</b></td>\n",szFieldColor,cMessageType);
        fprintf(mGexReport->getReportFile(),"<td width=\"590\" bgcolor=%s>%s</td></tr>\n",szDataColor,cMessageContent);
    }


    for(int lIdx=0; lIdx < lReportLogList.count(); ++lIdx)
    {
        GS::Gex::ReportLog::ReportLogType lMessageType = lReportLogList[lIdx].GetType();
        QString lMessageTypeString =  "Information";
        QString lColor = szFieldColor;
        if(lMessageType == GS::Gex::ReportLog::ReportError)
        {
            lMessageTypeString = "Error";
            lColor = szAlarmColor;
        }
        else if(lMessageType == GS::Gex::ReportLog::ReportWarning)
        {
            lMessageTypeString = "Warning";
            lColor = szWarningColor;
        }


        QString lMessageContent = lReportLogList[lIdx].GetContent();

        if(lFormat=="CSV")
        {
            fprintf(mGexReport->getReportFile(),"%s,%s\n", lMessageTypeString.toLatin1().constData(), lMessageContent.toLatin1().constData());
        }
        else
        {
            fprintf(mGexReport->getReportFile(),"<tr><td width=\"160\" bgcolor=%s><b><a>%s</a></b></td>",
                    lColor.toLatin1().constData(),lMessageTypeString.toLatin1().constData());
            fprintf(mGexReport->getReportFile(),"<td bgcolor=%s>%s</td></tr>\n",szDataColor,lMessageContent.toLatin1().constData());
        }
    }

    if(mGexReport->getReportOptions()->isReportOutputHtmlBased())
    {
        fprintf(mGexReport->getReportFile(),"</table>\n<br>\n");
    }

    this->CloseSection();

    GSLOG(SYSLOG_SEV_DEBUG, QString("End").toLatin1().constData());
    return "ok";
}

QString ReportLogUnit::PrepareSection(bool /*bValidSection*/)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start").toLatin1().constData());


    QString lFormat=mGexReport->GetOption("output", "format").toString();

    if(mGexReport->GetReportLogList().count() == 0)
    {
        if(lFormat=="CSV")
        {
            // Generating .CSV report file.
            fprintf(mGexReport->getReportFile(),"\n\n----%s----\n", cReportMessageUnitTitle);
            fprintf(mGexReport->getReportFile(),"No log message to report\n");
        }
        else if(mGexReport->getReportOptions()->isReportOutputHtmlBased())
        {
            if(OpenFile(mGexReport->getReportOptions()->strReportDirectory + "/pages/log.htm").startsWith("error") )
                return "error : cant open file";

            mGexReport->WriteHeaderHTML(mGexReport->getReportFile(),"#000080");	// Default: Text is Dark Blue
            // Title + bookmark
            mGexReport->WriteHtmlSectionTitle(mGexReport->getReportFile(),"report_log", cReportMessageUnitTitle);
            fprintf(mGexReport->getReportFile(),"<p align=\"left\">&nbsp;</p>\n");
            fprintf(mGexReport->getReportFile(),"<p align=\"left\"><font color=\"#000000\" size=\"%d\">No log message to report!<br>\n",mGexReport->iHthmNormalFontSize);
            this->CloseSection();
        }
        return "ok";
    }

    int lWarningCount = mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportWarning).count();
    int lErrorCount = mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportError).count();
    int lInfoCount =  mGexReport->GetReportLogList().filter(GS::Gex::ReportLog::ReportInformation).count();

    QString lLineToReplace = QString("Error(%1) <br>Warning(%2) <br>Information(%3)")
            .arg((lErrorCount>0) ? QString::number(lErrorCount): QString("None"))
            .arg((lWarningCount>0) ? QString::number(lWarningCount): QString("None"))
            .arg((lInfoCount>0) ? QString::number(lInfoCount): QString("None"));


    if(lFormat == "HTML")
    {
        //Update index.htm if needed
        QString lPageIn = mGexReport->getReportOptions()->strReportDirectory + QDir::separator() + "index.htm";
        QString lPageOut = mGexReport->getReportOptions()->strReportDirectory + QDir::separator() + "index.tmp.htm";

        QFile lFileIn(lPageIn);
        QFile lFileOut(lPageOut);
        if(lFileIn.open(QFile::ReadOnly))
        {
            QTextStream lStreamIn(&lFileIn);
            if(lFileOut.open(QFile::WriteOnly|QFile::Truncate))
            {
                QTextStream lStreamOut(&lFileOut);
                QString lLine;
                while (!lStreamIn.atEnd())
                {
                    lLine = lStreamIn.readLine();
                    if(lLine.contains(cHtmlLineToUpdate))
                    {
                        lLine = lLineToReplace;
                    }
                    lStreamOut << lLine << endl;

                }
                lFileOut.close();
                lFileIn.close();
                QFile::remove(lPageIn);
                QFile::copy(lPageOut,lPageIn);
                QFile::remove(lPageOut);
            }

        }

    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("End").toLatin1().constData());

    return "ok";
}

QString ReportLogUnit::CloseSection()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start").toLatin1().constData());
    QString lFormat=mGexReport->GetOption("output", "format").toString();

    if(lFormat == "HTML")
    {
        fprintf(mGexReport->getReportFile(), C_HTML_FOOTER,
                GS::Gex::Engine::GetInstance().Get("AppFullName").toString().toLatin1().data() );
        fprintf(mGexReport->getReportFile(),"</body>\n");
        fprintf(mGexReport->getReportFile(),"</html>\n");
        CloseReportFile();	// Close report file
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("End").toLatin1().constData());
    return "ok";
}
}
}
