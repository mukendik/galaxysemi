#ifndef QUERY_PROGRESS_CORE_H
#define QUERY_PROGRESS_CORE_H

#include <QObject>
#include "abstract_query_progress.h"

namespace GS
{
namespace DbPluginBase
{

class QueryProgressCore: public QObject, public AbstractQueryProgress
{
    Q_OBJECT
public:
    /// \brief Constructor
    QueryProgressCore(QObject *parent = 0);
    /// \brief Destructor
    ~QueryProgressCore();
    /// \brief add new logs
    void AddLog(const QString &log);

private:
    Q_DISABLE_COPY(QueryProgressCore);
    /// \brief log start info
    void LogStart();
    /// \brief log stop info
    void LogStop();
    /// \brief log file info
    void LogFileInfo(const QString &productName,
                     const QString &lotID,
                     const QString &sublotID,
                     const QString &waferID);
    /// \brief log file start
    void LogStartFileProgress();
    /// \brief log retrieved runs
    void LogRetrievedRuns(unsigned int totalRuns);
    /// \brief log retrieved tests
    void LogRetrievedTests(unsigned int ,
                           unsigned int totalTestResults);
    /// \brief log end of file
    void LogEndFileProgress();
};

} //END namespace DbPluginBase
} //END namespace GS


#endif // QUERY_PROGRESS_CORE_H
