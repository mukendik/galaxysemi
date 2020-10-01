///////////////////////////////////////////////////////////
// Examinator Monitoring: Send Email notification
///////////////////////////////////////////////////////////
#include <QDir>
#include <QDateTime>
#include <QTextStream>

#include <gqtl_log.h>

#include "mo_email.h"
#include "engine.h"

#ifndef GEX_TEMPORARY_EMAIL
#define GEX_TEMPORARY_EMAIL "_temp_gex_email"
#endif

static unsigned			iMessageID = 0;

///////////////////////////////////////////////////////////
// Class GexMoBuildEmailString: Used to build a QString with
// all the HTML code for a HTML message.
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Init. string that will end up as a HTML message
///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::CreatePage(QString strTitle)
{
    strEmailMessage  = "<html>\n";
    strEmailMessage += "<head>\n";
    strEmailMessage += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n";
    strEmailMessage += "<title>";
    strEmailMessage += strTitle;
    strEmailMessage += "</title>\n";
    strEmailMessage += "</head>\n";
    strEmailMessage += "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n";
    strEmailMessage += "<h1><font color=\"#000080\">" + strTitle;
    strEmailMessage += "</font></h1>\n";
}

///////////////////////////////////////////////////////////
// Close + Return full HTML string built.
///////////////////////////////////////////////////////////
QString	GexMoBuildEmailString::ClosePage(void)
{
    return strEmailMessage;
}

///////////////////////////////////////////////////////////
// HTML code to write a Title text
///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::WriteTitle(QString strTitle)
{
    strEmailMessage += "<p>&nbsp;</p>\n";
    strEmailMessage += "<h1 align=\"left\"><font color=\"#000080\">" + strTitle;
    strEmailMessage += "</font></h1>\n";
}

///////////////////////////////////////////////////////////
// HTML code to Open a table in HTML email message
///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::WriteHtmlOpenTable(int iWidth/*=98*/,int iCellSpacing/*=1*/,int iFontSize/*=12*/)
{
    QString strText;
    strText.sprintf("<table border=\"0\" cellspacing=\"%d\" width=\"%d%%\" style=\"font-size: %dpt; border-collapse: collapse\" bordercolor=\"#111111\" cellpadding=\"0\">\n",iCellSpacing,iWidth,iFontSize);
    strEmailMessage += strText;
}

///////////////////////////////////////////////////////////
// HTML code to Close a table in HTML email message
///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::WriteHtmlCloseTable(void)
{
    strEmailMessage += "</table>\n";
}

///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::WriteInfoLine(QString strLabel, QString strData, bool bError, bool bWarning, bool bSplitLines)
{

    strEmailMessage += "<tr>\n";
    if(strLabel.isEmpty())
    {
        // Display a separator line
        strEmailMessage += "<td width=\"160\" height=\"21\" bgcolor=\"#CCECFF\"><b><HR></b></td>";
        if(bSplitLines)
        {
            strEmailMessage += "\n";
        }
        strEmailMessage += "<td width=\"590\" height=\"21\" bgcolor=\"#F8F8F8\"><HR></td>\n";
    }
    else
    {
        // Display given parameters
        QString strText;
        strText.sprintf("<td width=\"160\" height=\"21\" bgcolor=\"#CCECFF\"><b>%s</b></td>",strLabel.toLatin1().constData());
        strEmailMessage += strText;
        if(bSplitLines)
        {
            strEmailMessage += "\n";
        }

        if(strData.isEmpty() == false)
        {
            strText = "<td width=\"590\" height=\"21\" bgcolor=\"#F8F8F8\">";
            if(bError)
                strText += "<span style=\"background-color: #FF0000\">";
            else if(bWarning)
                strText += "<span style=\"background-color: #FF8000\">";

            strText += strData;

            if(bError || bWarning)
                strText += "</span>";
            strText += "</td>\n";
        }
        else
            strText.sprintf("<td width=\"590\" height=\"21\" bgcolor=\"#F8F8F8\"><HR></td>\n");
        strEmailMessage += strText;
    }
    strEmailMessage += "</tr>\n";
}

void	GexMoBuildEmailString::WriteLabelLineList(QStringList strList)
{
    QStringList::Iterator it;
    strEmailMessage += "<tr>\n";
    for (it = strList.begin(); it != strList.end(); ++it)
    {
        strEmailMessage += "<td align=\"center\" bgColor=\"#CCECFF\"><b>" + (*it).trimmed();
        strEmailMessage += "</b></td>\n";
    }

    strEmailMessage += "</tr>\n";
}

// HTML code to write one line in the table
void	GexMoBuildEmailString::WriteInfoLineList(QStringList strList,QString strAlign)
{
    QStringList::Iterator it;

    strEmailMessage += "<tr>\n";

    for (it = strList.begin(); it != strList.end(); ++it)
    {
        strEmailMessage += "<td align=\"" + strAlign;
        strEmailMessage += "\" bgColor=\"#F8F8F8\"><b>" + (*it).trimmed();
        strEmailMessage += "</b></td>\n";
    }

    strEmailMessage += "</tr>\n";
}

void	GexMoBuildEmailString::WriteInfoLineList(QStringList strList,QList<bool> & listError, QString strAlign)
{
    WriteInfoLineList(strList, listError, QColor(Qt::red), strAlign);
}

void	GexMoBuildEmailString::WriteInfoLineList(QStringList strList,QList<bool> & listError, const QColor& clErrorBkgColor, QString strAlign)
{
    QStringList::Iterator	it;
    bool					bError;
    int					nIndex;
    QString					strText;

    strEmailMessage += "<tr>\n";

    for(nIndex = 0; nIndex < strList.size(); nIndex++)
    {
        strText = strList.at(nIndex);
        bError = false;
        if(nIndex < listError.size())
            bError = listError.at(nIndex);
        strEmailMessage += "<td align=\"" + strAlign;
        if(bError)
            strEmailMessage += "\" bgColor=\"" + clErrorBkgColor.name() + "\"><b>" + strText.trimmed();
        else
            strEmailMessage += "\" bgColor=\"#F8F8F8\"><b>" + strText.trimmed();
        strEmailMessage += "</b></td>\n";
    }

    strEmailMessage += "</tr>\n";
}

///////////////////////////////////////////////////////////
// Add HTML code to existing HTML string page
///////////////////////////////////////////////////////////
void	GexMoBuildEmailString::AddHtmlString(QString strString)
{
    strEmailMessage += strString;
}

bool GexMoSendEmail::Send(
        QString strEmailSpoolFolder,
        QString strFrom,QString strTo,
        QString strSubject,
        QString strBody,
        bool bHtmlFormat, QString strAttachments)
{
    if(strEmailSpoolFolder.isEmpty())
        // No spooling folder where to create the email file...then quietly return!
        return true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Send/generate email in %1").arg(strEmailSpoolFolder).toLatin1().data() );

    // If empty name: no email to send!
    if(strTo.isEmpty())
    {
        GSLOG(SYSLOG_SEV_WARNING, "Send email: no target email ('To' empty)");
        return true;
    }

    // Check email format
    QString strEmailFormat;
    if(bHtmlFormat)
        strEmailFormat = "HTML";
    else
        strEmailFormat = "TEXT";

    // Create email file in spooling folder.
    // Name: YYYYMMDD_HHMMSS_MS.mail
    QDir cDir;
    QDateTime cCurrentDateTime=GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString strEmailFile = strEmailSpoolFolder;
    cDir.mkdir(strEmailFile);	// <Intranet_WebPath>/examinator_monitoring/emails
    strEmailFile += cCurrentDateTime.toString("/yyyyMMdd_hhmmss_zzz");
    strEmailFile += QString::number(iMessageID,16);
    strEmailFile += ".mail";
    strEmailFile += GEX_TEMPORARY_EMAIL;
    QString strEmailBody = strEmailSpoolFolder;
    strEmailBody += cCurrentDateTime.toString("/yyyyMMdd_hhmmss_zzz");
    strEmailBody += QString::number(iMessageID,16);
    strEmailBody += ".body";

    // Update Message ID
    iMessageID++;

    // Create Email description file
    QFile file( strEmailFile );
    if (file.open(QIODevice::WriteOnly) == false)
        return false;	// Failed opening email file.

    QTextStream stream( &file );
    stream << "Type=MonitoringReport" << endl;			// Report type
    stream << "Format=" << strEmailFormat << endl;		// Email format "HTML" or "TEXT"
    stream << "To=" << strTo << endl;					// Destination: email list, or mailing list file
    stream << "From=" << strFrom << endl;				// Sender name.
    stream << "Subject=" << strSubject << endl;			// Subject
    stream << "Body=" <<  strEmailBody << endl;			// File holding the "Body"
    if(strAttachments.isEmpty() == false)
        stream << "Attachment=" <<  strAttachments << endl;	// Files to attach

    // Close Email description file.
    file.close();

    // Create Email body file
    file.setFileName(strEmailBody);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        cDir.remove(strEmailFile);	// Remove email description file just created
        return false;	// Failed opening body file.
    }

    // Write Body file, then close it.
    stream.setDevice(&file);

    stream << strBody << endl;

    // Email footer. Examinator Version.
    if(bHtmlFormat)
    {
        stream << "<br>"  << endl;
        stream << "<br>"  << endl;
        stream << "<hr>"  << endl;
        stream << "&nbsp;"  << endl;
        stream << "Report created with: "
               << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
        stream << "<br>&nbsp;"  << endl;
        stream << "&quot;Semiconductor Intelligence (tm)&quot;<br>"  << endl;
        stream << "<br>"  << endl;
        stream << "&nbsp; <a href=\"http://www.mentor.com\">www.mentor.com</a>&nbsp; "  << endl;
        stream << "<hr>"  << endl;
        stream << "</body>" << endl;
        stream << "</html>" << endl;
    }
    else
    {
        stream << endl << "============================================================" << endl;
        stream <<  "  Report created with: ";
        stream << GS::Gex::Engine::GetInstance().Get("AppFullName").toString() << endl;
        stream <<  "  \"Semiconductor Intelligence (tm)\"" << endl << endl;
        stream <<  "  www.mentor.com" << endl;
        stream <<  "============================================================" << endl;
    }
    file.close();

    // Rename Email description to correct extension...so spooling mailing daemons can capture the new email!
    QString strDest = strEmailFile;
    int iIndex = strDest.lastIndexOf(GEX_TEMPORARY_EMAIL);
    strDest = strDest.left(iIndex);

    // Erase destination
    cDir.remove(strDest);
    // Rename source to destination
    if(cDir.rename(strEmailFile,strDest) == true)
        return true;  // Success: remove entry from the
    else
        return false;
}

