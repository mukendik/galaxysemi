/******************************************************************************!
 * \file message.cpp
 ******************************************************************************/

#include <QDateTime>
#ifndef GSDAEMON
#include <QMessageBox>
#endif
#include "message.h"
#include "gqtl_log.h"
#include "product_info.h"
#include "engine.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include "admin_engine.h"
#include "command_line_options.h"
#include "scheduler_engine.h"

#ifndef GSDAEMON
extern GexMainwindow* pGexMainWindow;
#endif

namespace GS
{
namespace Gex
{

/******************************************************************************!
 * \fn request
 ******************************************************************************/
void Message::request(const QString title, const QString message, bool& accepted)
{
    Message::send(MESSAGE_REQUEST, title, message, accepted);
}


/******************************************************************************!
 * \fn information
 ******************************************************************************/
void Message::information(const QString title, const QString message)
{
    bool accepted;
    Message::send(SYSLOG_SEV_INFORMATIONAL, title, message, accepted);
}

/******************************************************************************!
 * \fn warning
 ******************************************************************************/
void Message::warning(const QString title, const QString message)
{
    bool accepted;
    Message::send(SYSLOG_SEV_WARNING, title, message, accepted);
}

/******************************************************************************!
 * \fn critical
 ******************************************************************************/
void Message::critical(const QString title, const QString message)
{
    bool accepted;
    Message::send(SYSLOG_SEV_CRITICAL, title, message, accepted);
}

/******************************************************************************!
 * \fn send
 ******************************************************************************/
void Message::send(const int level,
                   const QString& title,
                   const QString& message,
                   bool &accepted)
{
    accepted = false;
    GSLOG(level, (message+title).toLatin1().data());

    // If monitoring, log error message
    if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() &&
        !GS::Gex::Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
    {
        // Request message are not logged in the admin log, as they have a default behavior
        if (level != MESSAGE_REQUEST)
        {
            if (GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            {
                QString status;  // FIXME: use const uint not string
                switch (level)
                {
                default:
                case SYSLOG_SEV_WARNING:       status = "WARNING";    break;
                case SYSLOG_SEV_INFORMATIONAL: status = "INFO";       break;
                case SYSLOG_SEV_CRITICAL:      status = "UNEXPECTED"; break;
                }
                GS::Gex::Engine::GetInstance().GetAdminEngine().
                    AddNewEvent("", "", status, message, "PopupMessage");
            }

            QDateTime cCurrentDateTime  = QDateTime::currentDateTime();
            QString   strHistoryMessage =
                cCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
            strHistoryMessage += message + "\n";
            GS::Gex::Engine::GetInstance().GetSchedulerEngine().
                AppendMoHistoryLog(strHistoryMessage);
        }

        GSLOG(SYSLOG_SEV_DEBUG, QString("ProductID=%1 : not poping message : %2").
               arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID()).
               arg(message).toLatin1().constData());
        return;
    }

    if (GS::Gex::Engine::GetInstance().GetCommandLineOptions().IsHidden())
    {
        GSLOG(level, message.toLatin1().data());
        return;
    }

#ifndef GSDAEMON
    QString theTitle;
    if (title == "")
    {
        theTitle = GS::Gex::Engine::GetInstance().Get("AppFullName").toString();
    }
    else
    {
        theTitle = title;
    }

    QMessageBox lMessageBox(QMessageBox::NoIcon, theTitle, message);
    switch (level)
    {
    default:
    case SYSLOG_SEV_WARNING:
        lMessageBox.setIcon(QMessageBox::Warning);
        lMessageBox.addButton(QMessageBox::Ok);
        break;
    case SYSLOG_SEV_INFORMATIONAL:
        lMessageBox.setIcon(QMessageBox::Information);
        lMessageBox.addButton(QMessageBox::Ok);
        break;
    case SYSLOG_SEV_CRITICAL:
        lMessageBox.setIcon(QMessageBox::Critical);
        lMessageBox.addButton(QMessageBox::Ok);
        break;
    case MESSAGE_REQUEST:
        lMessageBox.setIcon(QMessageBox::Question);
        lMessageBox.addButton(QMessageBox::Yes);
        lMessageBox.addButton(QMessageBox::No);
        break;
    }
    lMessageBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
    accepted = (lMessageBox.exec() == QMessageBox::Yes);
#else
    GSLOG(level, message.toLatin1().data());
#endif
}

}
}
