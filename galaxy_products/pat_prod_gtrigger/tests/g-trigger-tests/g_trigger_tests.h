#ifndef G_TRIGGER_TEST_H
#define G_TRIGGER_TEST_H

#include "g-trigger_engine.h"

#include <QObject>
#include <QScopedPointer>
#include <QStringList>
#include <QByteArray>

class QDir;
class QFileInfo;

class GTriggerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();

    void GTriggerTestsSuite_data();
    void GTriggerTestsSuite();

private :
    // TODO - const correctness, single role purpose
    void RunTest();
    void CopyInputFileFromDirectoryToOutputDirectory( const QString& aInputDirName );
    void CopyDirectoryContentToOutputDirectory( QDir &aInputDirectory );
    void WipeOutputDirectory();
    bool AreOutputFileAndReferenceFileIdentical();
    void RemoveOutputDirectoryEntry(const QFileInfo &aFileInfo );
    QString MoveReferenceFileToOutputDirectory();
    QString GetOutputFilePath();
    bool AreFilesIdentical(const QString &aFile1Path, const QString &aFile2Path);
    QByteArray GetFileHash( const QString &aFilePath );
    void PostProcessOutputFiles();
    void PostProcessLogFiles();
    void PostProcessStdfFiles();
    void PostProcessWaferLogFiles();
    void PostProcessSummaryLogFiles();
    QStringList GetLogFileContentWithoutDate( const QFileInfo &aFileInfo );
    void CreateConcatenatedLogFileWithContent( const QString &aFileName, const QStringList &aFileContent );
    bool ReadNonDateLineFromStream(QTextStream &lStream , QString &lLineRead);
    QStringList GetLogFilesContentFromDirectoryAndNamePattern( const QString &aDirectoryPath,
                                                               const QString &aFileNamePattern );
    QStringList GetOrderedStdfFilesPathInDirectory( const QString &aDirectoryPath );
    QStringList GetOrderedAtdfFilesPathFromStdfFilesPath( const QStringList &aOrderedStdfFilesPath );
    void ConvertFilesFromStdfToAtdf(const QStringList &aStdfFileList , const QStringList &aAtdfFileList);
    void ConcatenateAllAtdfFiles( const QStringList &aFileList );
    void AppendFileContentToStream( const QString &aFilePath, QTextStream &aFileStream );
    void ConcatenateLogsAndAtdfFiles();

private :
    QScopedPointer< GS::GTrigger::GTriggerEngine > mEngine;
    QStringList mInputFiles;

};

#endif // G_TRIGGER_TEST_H
