#include <QDir>
#include "gs_qa_dump.h"
#include "gqtl_log.h"

namespace GS
{
namespace Gex
{

GsQaDump::GsQaDump(QObject *parent)
    : QObject(parent)
{

}

GsQaDump::~GsQaDump()
{

}

bool GsQaDump::Open(const QString &lShortFileName, QIODevice::OpenMode lMode)
{
    // If file opened, close it
    if(mFile.isOpen())
        mFile.close();

    // Check if QA env variables are set
    char* lQaOutputEnv=getenv("GS_QA_OUTPUT_FOLDER");
    if(!getenv("GS_QA") || !lQaOutputEnv)
        return false;

    // Create output directory if it doesn't exist
    QDir lDir;
    QString lQaOutputFolder = lDir.cleanPath(QString(lQaOutputEnv));
    lDir.setPath(lQaOutputFolder);
    if(!lDir.exists() && !lDir.mkpath(lDir.absolutePath()))
    {
        GSLOG(SYSLOG_SEV_WARNING, QString("Could not create folder %1").arg(lQaOutputFolder).toLatin1().constData());
        return false;
    }

    // Build full file name
    QString lFileName;
    lFileName=QString("%1/%2").arg(lQaOutputFolder).arg(lShortFileName);
    lFileName=lDir.cleanPath(lFileName);

    // Open file
    mFile.setFileName(lFileName);
    bool lStatus = mFile.open(lMode);
    if(!lStatus)
        GSLOG(SYSLOG_SEV_WARNING, QString("Could not open file %1").arg(lFileName).toLatin1().constData());
    return lStatus;
}

qint64 GsQaDump::WriteString(const QString & lLine, bool lAddCr)
{
    // Check if file opened
    if(!mFile.isOpen())
        return -1;

    // Write to file
    if(lAddCr)
        return mFile.write(QString("%1\n").arg(lLine).toLatin1().constData());

    return mFile.write(lLine.toLatin1().constData());
}

void GsQaDump::Close()
{
    if(mFile.isOpen())
        mFile.close();
}

void GsQaDump::RemoveAllFiles()
{
    // Check if QA env variables are set
    char* lQaOutputEnv=getenv("GS_QA_OUTPUT_FOLDER");
    if(!getenv("GS_QA") || !lQaOutputEnv)
        return;

    // Remove all files in QA dir
    QDir lDir;
    QString lQaOutputFolder = lDir.cleanPath(QString(lQaOutputEnv));
    lDir.setPath(lQaOutputFolder);
    QStringList lFiles = lDir.entryList(QStringList(QString("*")), QDir::Files);
    for(int i=0; i<lFiles.size(); ++i)
        lDir.remove(lFiles.at(i));
}

} // namespace GS
} // namespace Gex
