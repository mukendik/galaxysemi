#ifndef REPORT_LOG_LIST_H
#define REPORT_LOG_LIST_H

#include <QList>

#include "report_log.h"

namespace GS
{
namespace Gex
{
/*! \class ReportLogList
 * \brief data class used to save a list of ReportLog in the order they are appended
 *
 */
class ReportLogList : public QList<ReportLog>
{
public:
    /// \brief Constructor
    ReportLogList();
    /// \brief Destructor
    virtual ~ReportLogList();
    /// \brief add a ReportLog to the end of ReportLogList
    void addReportLog(const QString &content, const ReportLog::ReportLogType &type);
    QList<ReportLog> filter(const ReportLog::ReportLogType &type);

};

}
}

#endif // REPORT_LOG_STACK_H
