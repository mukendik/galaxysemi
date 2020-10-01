#include "query_progress_core.h"
#include <gqtl_log.h>

namespace GS
{
namespace DbPluginBase
{

QueryProgressCore::QueryProgressCore(QObject *parent):
    QObject( parent )
{
}

QueryProgressCore::~QueryProgressCore()
{
}

void QueryProgressCore::LogStart()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Start Query");
}

void QueryProgressCore::LogStop()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Query finished");
}

void QueryProgressCore::LogFileInfo(const QString &productName,
                                    const QString &lotID,
                                    const QString &sublotID,
                                    const QString &waferID)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Retrieve Product: %1, LotId: %2, SublotId: %3, WaferId: %4").
           arg(productName).
           arg(lotID).
           arg(sublotID).
           arg(waferID).
           toLatin1().data());
}

void QueryProgressCore::LogStartFileProgress()
{
    int lProgressPercent = 0;
    if (mTotalFiles > 0)
        lProgressPercent = (mRetrievedFiles*100)/mTotalFiles;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Progress: %1%").
           arg(QString::number(lProgressPercent))
           .toLatin1().data());
}

void QueryProgressCore::LogRetrievedRuns(unsigned int totalRuns)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Runs retrieved: %1").
           arg(QString::number(totalRuns)).toLatin1().data());
}

void QueryProgressCore::LogRetrievedTests(unsigned int /*nbRuns*/, unsigned int totalTestResults)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Tests retrieved: %1").
           arg(QString::number(totalTestResults)).toLatin1().data());
}

void QueryProgressCore::LogEndFileProgress()
{
}

void QueryProgressCore::AddLog(const QString &log)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,log.toLatin1().data());
}

} //END namespace DbPluginBase
} //END namespace GS

