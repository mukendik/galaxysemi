#include "report_log_list.h"

namespace GS
{
namespace Gex
{

ReportLogList::ReportLogList() : QList <ReportLog> ()
{
}

ReportLogList::~ReportLogList()
{
    clear();
}

void ReportLogList::addReportLog(const QString &content, const ReportLog::ReportLogType &type)
{
    append(ReportLog(content, type));
}

QList<ReportLog> ReportLogList::filter(const ReportLog::ReportLogType &type)
{
    QList<ReportLog> filtredItems;
    for(int lIdx=0; lIdx < this->count(); ++lIdx)
    {
        if(this->at(lIdx).GetType() == type)
            filtredItems.append(this->at(lIdx));
    }
    return filtredItems;
}


}
}
