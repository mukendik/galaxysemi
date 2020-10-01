#ifndef ABSTRACT_QUERY_PROGRESS_H
#define ABSTRACT_QUERY_PROGRESS_H

#include <QTime>
#include <QColor>

namespace GS
{
namespace DbPluginBase
{

class AbstractQueryProgress
{
public:
    /// \brief Constructor
    AbstractQueryProgress();
    /// \brief Destructor
    virtual ~AbstractQueryProgress();
    /// \brief Show query start
    void Start(unsigned int totalFiles);
    /// \brief Show query end
    void Stop();
    /// \brief set files info
    void SetFileInfo(const QString & productName,
                     const QString & lotID,
                     const QString & sublotID,
                     const QString & waferID);
    /// \brief show new file creation by query
    void StartFileProgress(unsigned int runsInFile);
    /// \brief show retrieved runs
    void SetRetrievedRuns(unsigned int totalRuns,
                          bool forceUpdate = false);
    /// \brief show retrieved results
    void SetRetrievedResults(unsigned int runs,
                             unsigned int totalTestResults,
                             bool forceUpdate = false);
    /// \brief show end of file creation by query
    void EndFileProgress();
    /// \brief true if abort has been requested
    bool IsAbortRequested();
    /// \brief add new logs
    virtual void AddLog(const QString &);
    /// \brief clear logs
    virtual void ClearLogs();
    /// \brief set autoclose rule
    virtual void SetAutoClose(bool );
    /// \brief set log color
    virtual void SetLogsTextColor(const QColor&);

protected:
    Q_DISABLE_COPY(AbstractQueryProgress);
    /// \brief log start info
    virtual void LogStart() = 0;
    /// \brief log stop info
    virtual void LogStop() = 0;
    /// \brief log file info
    virtual void LogFileInfo(const QString &productName,
                             const QString &lotID,
                             const QString &sublotID,
                             const QString &waferID) = 0;
    /// \brief log file start
    virtual void LogStartFileProgress() = 0;
    /// \brief log retrieved runs
    virtual void LogRetrievedRuns(unsigned int totalRuns) = 0;
    /// \brief log retrieved tests
    virtual void LogRetrievedTests(unsigned int runs,
                                   unsigned int totalTestResults) = 0;
    /// \brief log end of file
    virtual void LogEndFileProgress() = 0;

    QTime           mQueryTime;             ///< holds time since the beginning of the query
    QTime           mTimeSinceLastUpdate;   ///< holds time since last update
    unsigned int    mTotalFiles;            ///< holds file to create
    unsigned int    mRetrievedFiles;        ///< holds created files
    unsigned int    mRunsInFile;            ///< holds runs in current file
    bool            mIsAbortRequested;      ///< holds if abort has been requested
};

} // END namespace DbPluginBase
} // END namespace GS

#endif // ABSTRACT_QUERY_PROGRESS_H
