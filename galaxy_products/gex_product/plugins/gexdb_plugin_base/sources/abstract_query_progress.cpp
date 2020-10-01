#include "abstract_query_progress.h"
#include <gqtl_log.h>

namespace GS
{
namespace DbPluginBase
{

AbstractQueryProgress::AbstractQueryProgress()
{
}

AbstractQueryProgress::~AbstractQueryProgress()
{
}

void AbstractQueryProgress::Start(unsigned int totalFiles)
{
    mQueryTime.start();
    mTimeSinceLastUpdate.start();
    mRetrievedFiles = 0;
    mIsAbortRequested = false;
    mTotalFiles = totalFiles;
    LogStart();
}

void AbstractQueryProgress::Stop()
{
    LogStop();
}

void AbstractQueryProgress::SetFileInfo(
        const QString & productName,
        const QString & lotID,
        const QString & sublotID,
        const QString & waferID)
{
    // re-init timer
    mTimeSinceLastUpdate.start();
    LogFileInfo(productName, lotID, sublotID, waferID);
}

void AbstractQueryProgress::StartFileProgress(unsigned int runsInFile)
{
    mRunsInFile = runsInFile;
    LogStartFileProgress();
}

void AbstractQueryProgress::SetRetrievedRuns(unsigned int totalRuns,
                                             bool forceUpdate/*=false*/)
{
    if(forceUpdate || (mTimeSinceLastUpdate.elapsed() > 200))
    {
        mTimeSinceLastUpdate.start();
        LogRetrievedRuns(totalRuns);
    }
}

void AbstractQueryProgress::SetRetrievedResults(unsigned int runs,
                                                unsigned int totalTestResults,
                                                bool forceUpdate/*=false*/)
{
    if(forceUpdate || (mTimeSinceLastUpdate.elapsed() > 200))
    {
        // re-init timer
        mTimeSinceLastUpdate.start();
        LogRetrievedTests(runs, totalTestResults);
    }
}

void AbstractQueryProgress::EndFileProgress()
{
    // update file count
    ++mRetrievedFiles;
    LogEndFileProgress();
}

bool AbstractQueryProgress::IsAbortRequested()
{
    return mIsAbortRequested;
}

void AbstractQueryProgress::SetLogsTextColor(const QColor &)
{
}

void AbstractQueryProgress::SetAutoClose(bool )
{
}

void AbstractQueryProgress::ClearLogs()
{
}

void AbstractQueryProgress::AddLog(const QString &)
{
}

} // END namespace DbPluginBase
} // END namespace GS

