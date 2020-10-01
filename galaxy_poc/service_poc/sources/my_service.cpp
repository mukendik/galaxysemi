#include "my_service.h"
#include "my_event.h"

#include <gqtl_log.h>

#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QStringList>
#include <QWebView>

MyService::MyService(int argc, char **argv, const QString &name)
    : QtService<QCoreApplication>(argc, argv, name)
{
    QtServiceBase::setServiceDescription("description");
    QtServiceBase::setServiceFlags(QtServiceBase::NeedsStopOnShutdown);
    QtServiceBase::setStartupType(QtServiceController::AutoStartup);
}

MyService::~MyService()
{
    GSLOG(SYSLOG_SEV_NOTICE, "MyService::~MyService");
}

void MyService::start()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Receive Start instruction from Service Manager");
    GSLOG(SYSLOG_SEV_NOTICE, "Testing qApp->arguments(): known to crash in debug.");
    // Test : strangely qApp->arguments() is crashing in debug but not in release...
    //GSLOG(SYSLOG_SEV_NOTICE, qApp->arguments().join(' ').toLatin1().data() );
    //QWebView lWV; // impossible: QWidget: Cannot create a QWidget without QApplication
}

void MyService::stop()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Receive Stop instruction from Service Manager");

    while(mCoreTask.IsRunning())
    {
        application()->processEvents(QEventLoop::AllEvents);
    }

    GSLOG(SYSLOG_SEV_NOTICE, "All tasks have been cleanly stopped.");
}

void MyService::resume()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Receive Resume instruction from Service Manager");
}

void MyService::processCommand(int code)
{
    GSLOG(SYSLOG_SEV_NOTICE,
          QString("Receive Command instruction from Service Manager to start task# %1")
          .arg(code).toLatin1().constData());

    int lType       = MyEvent::UserSyncTask;
    int lDuration   = 0;

    if (GetTaskInfo(code, lType, lDuration))
        application()->postEvent(&mCoreTask, new MyEvent((MyEvent::Type)lType, lDuration));
    else
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Unable to find task# %1.")
              .arg(code).toLatin1().constData());
}

bool MyService::GetTaskInfo(int lTaskID, int &lTaskType, int &lDuration) const
{
    bool                        lSucceed    = false;
    QString                     xmlFileName = QCoreApplication::applicationDirPath() + "/service_poc.xml";
    QDomDocument				xmlDoc;
    QFile						xmlFile(xmlFileName);

    if (xmlFile.open(QIODevice::ReadOnly))
    {
        if (xmlDoc.setContent(&xmlFile))
        {
            if (xmlDoc.documentElement().tagName() == "service_poc")
            {
                QDomNodeList    lNodeList   = xmlDoc.documentElement().childNodes();

                for (int lNodeIdx = 0; lNodeIdx < lNodeList.count(); ++lNodeIdx)
                {
                    QDomElement	docElement	= lNodeList.item(lNodeIdx).toElement();

                    if (docElement.attribute("id", "-1").toInt() == lTaskID)
                    {
                        if (docElement.attribute("type", "synchronous") == "asynchronous")
                            lTaskType = MyEvent::UserAsyncTask;
                        else
                            lTaskType = MyEvent::UserSyncTask;

                        lDuration = docElement.attribute("duration", "0").toInt();
                        lSucceed  = true;
                    }
                }

                if (lSucceed == false)
                    GSLOG(SYSLOG_SEV_ERROR,
                          QString("Task# %1 not found.").arg(lTaskID).toLatin1().constData());
            }
            else
                GSLOG(SYSLOG_SEV_ERROR, "Root node <service_poc> not found.");
        }
        else
            GSLOG(SYSLOG_SEV_ERROR, "service_poc.xml is not XML compliant.");

        xmlFile.close();
    }
    else
        GSLOG(SYSLOG_SEV_ERROR, "Unable to open file service_poc.xml.");

    return lSucceed;
}

void MyService::pause()
{
    GSLOG(SYSLOG_SEV_NOTICE, "Receive Pause instruction from Service Manager");
}

