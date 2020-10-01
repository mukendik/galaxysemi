#ifndef _G_TRIGGER_TESTS_CPP_
#define _G_TRIGGER_TESTS_CPP_

#include "g_trigger_tests.h"
#include "test_doubles/testable_g_trigger_engine.h"
#include "testable_g_trigger_engine_worker.h"
#include "simple_stdf_to_atdf_converter.h"

#include <QtTest>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>

void GTriggerTests::GTriggerTestsSuite()
{
    try
    {
        RunTest();
    }
    catch( const char *&aMessage )
    {
        QFAIL( QString{ "Exceptional test failure : %1" }.arg( aMessage ).toStdString().c_str() );
    }
}

void GTriggerTests::RunTest()
{
    TestableGTriggerEngineWorker{ QCoreApplication::applicationName(),
                QCoreApplication::applicationDirPath() }.Await();

    PostProcessOutputFiles();

    if( ! AreOutputFileAndReferenceFileIdentical() )
        QFAIL( "Reference file differs from output file" );
}

void GTriggerTests::CopyInputFileFromDirectoryToOutputDirectory(const QString &aInputDirName)
{
    QDir lInputDir{ QString( "%1/%2/%3" ).arg( TEST_PROGRAM_DIRECTORY ).arg( "../input" ).arg( aInputDirName ) };
    if( ! lInputDir.exists() )
        QFAIL( "Input file directory does not exist." );

    CopyDirectoryContentToOutputDirectory( lInputDir );
}

void GTriggerTests::CopyDirectoryContentToOutputDirectory(QDir &aInputDirectory)
{
    mInputFiles = aInputDirectory.entryList( QDir::NoDotAndDotDot | QDir::Files );

    for( int lIndex = 0; lIndex < mInputFiles.count(); lIndex++ )
    {
        QFile lFile{ QString{ "%1/%2" }.arg( aInputDirectory.absolutePath() ).arg( mInputFiles[ lIndex ] ) };
        if( ! lFile.copy( QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( mInputFiles[ lIndex ] ) ) )
            QFAIL( "could not copy input file to output directory" );
    }
}

void GTriggerTests::WipeOutputDirectory()
{
    QFileInfoList lDirectoryEntries =
        QDir{ TEST_PROGRAM_DIRECTORY }.entryInfoList( QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs );

    for( int lIndex = 0; lIndex < lDirectoryEntries.count(); lIndex++ )
        RemoveOutputDirectoryEntry( lDirectoryEntries[ lIndex ] );
}

bool GTriggerTests::AreOutputFileAndReferenceFileIdentical()
{
    const QString & lReferenceFilePath = MoveReferenceFileToOutputDirectory();
    const QString & lOutputFilePath = GetOutputFilePath();
    return AreFilesIdentical( lReferenceFilePath, lOutputFilePath );
}

void GTriggerTests::RemoveOutputDirectoryEntry(const QFileInfo &aFileInfo )
{
    if( aFileInfo.isDir() )
        QDir{ aFileInfo.filePath() }.removeRecursively();
    if( aFileInfo.isFile() && aFileInfo.baseName() != TEST_PROGRAM_NAME )
        QDir{}.remove( aFileInfo.filePath() );
}

QString GTriggerTests::MoveReferenceFileToOutputDirectory()
{
    QFETCH( QString, ReferenceFileName );
    QFETCH( QString, InputFilesDirectoryName );

    QString lReferenceOriginalFilePath{ QString{ "%1/%2/%3/%4" }
                                        .arg( TEST_PROGRAM_DIRECTORY )
                                        .arg( "../reference" )
                                        .arg( InputFilesDirectoryName )
                                        .arg( ReferenceFileName ) };

    QString lReferenceNewFilePath{ QString{ "%1/%2.ref" }
                                   .arg( TEST_PROGRAM_DIRECTORY )
                                   .arg( ReferenceFileName ) };

    if( ! QFile{ lReferenceOriginalFilePath }.copy( lReferenceNewFilePath ) )
        throw "Could not copy the reference file to the output directory";

    return lReferenceNewFilePath;
}

QString GTriggerTests::GetOutputFilePath()
{
    return { QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "result.txt.out" ) };
}

bool GTriggerTests::AreFilesIdentical(const QString &aFile1Path , const QString &aFile2Path)
{
    const QByteArray &lFile1Sha1 = GetFileHash( aFile1Path );
    const QByteArray &lFile2Sha1 = GetFileHash( aFile2Path );

    return lFile1Sha1 == lFile2Sha1;
}

QByteArray GTriggerTests::GetFileHash(const QString &aFilePath)
{
    QFile lFile{ aFilePath };

    if( ! lFile.open( QFile::ReadOnly ) )
        throw "Cannot open file to get its sha1";

    QCryptographicHash lFileSha1{ QCryptographicHash::Sha1 };
    lFileSha1.addData( lFile.readAll() );

    return lFileSha1.result();
}

void GTriggerTests::PostProcessOutputFiles()
{
    PostProcessLogFiles();
    PostProcessStdfFiles();
    ConcatenateLogsAndAtdfFiles();
}

void GTriggerTests::PostProcessLogFiles()
{
    PostProcessWaferLogFiles();
    PostProcessSummaryLogFiles();
}

void GTriggerTests::PostProcessStdfFiles()
{
    const QString lStdfFilesPath{ QString{ "%1/%2" }
                                  .arg( TEST_PROGRAM_DIRECTORY )
                                  .arg( "pat_stdf" ) };

    const QStringList &lOrderedStdfFilesPath = GetOrderedStdfFilesPathInDirectory( lStdfFilesPath );
    const QStringList &lOrderedAtdfFilesPath = GetOrderedAtdfFilesPathFromStdfFilesPath( lOrderedStdfFilesPath );

    ConvertFilesFromStdfToAtdf( lOrderedStdfFilesPath, lOrderedAtdfFilesPath );
    ConcatenateAllAtdfFiles( lOrderedAtdfFilesPath );
}

void GTriggerTests::PostProcessWaferLogFiles()
{
    const QString lLogFilesPath{ QString{ "%1/%2" }
                                 .arg( TEST_PROGRAM_DIRECTORY )
                                 .arg( "pat_log" ) };

    QStringList lAllLogFilesContent =
        GetLogFilesContentFromDirectoryAndNamePattern( lLogFilesPath, "*.log" );

    CreateConcatenatedLogFileWithContent( "wafers.log.out", lAllLogFilesContent );
}

void GTriggerTests::PostProcessSummaryLogFiles()
{
    const QString lLogFilesPath{ TEST_PROGRAM_DIRECTORY };

    QStringList lAllLogFilesContent =
        GetLogFilesContentFromDirectoryAndNamePattern( lLogFilesPath, "*-summary.log" );

    CreateConcatenatedLogFileWithContent( "summaries.log.out", lAllLogFilesContent );
}

QStringList GTriggerTests::GetLogFileContentWithoutDate(const QFileInfo &aFileInfo)
{
    QFile lFile{ aFileInfo.filePath() };

    if( ! lFile.open( QFile::ReadOnly | QFile::Text ) )
        throw "Cannot open wafer log file in read only mode";

    QTextStream lStream{ &lFile };

    QStringList lFileContent;

    for( QString lLineRead; ReadNonDateLineFromStream( lStream, lLineRead ); )
        lFileContent << lLineRead;

    return lFileContent;
}

void GTriggerTests::CreateConcatenatedLogFileWithContent(const QString &aFileName, const QStringList &aFileContent)
{
    QString lFilePath{ QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( aFileName ) };
    QFile lFile{ lFilePath };

    if( ! lFile.open( QFile::WriteOnly | QFile::Text ) )
        throw "Cannot create concatenated log file";

    QTextStream lStream{ &lFile };

    for( int lIndex = 0; lIndex < aFileContent.count(); lIndex++ )
        lStream << aFileContent[ lIndex ] << '\n';
}

bool GTriggerTests::ReadNonDateLineFromStream(QTextStream &lStream, QString &lLineRead)
{
    if( lStream.atEnd() )
        return false;

    do
        lLineRead = lStream.readLine();
    while( ( ! lStream.atEnd() ) && lLineRead.startsWith( "date", Qt::CaseInsensitive ) );

    return true;
}

QStringList GTriggerTests::GetLogFilesContentFromDirectoryAndNamePattern(const QString &aDirectoryPath,
                                                                         const QString &aFileNamePattern)
{
    QDir lLogFilesDirectory{ aDirectoryPath };
    auto lFileInfoList = lLogFilesDirectory.entryInfoList( QStringList{} << aFileNamePattern,
                                                           QDir::NoDotAndDotDot | QDir::Files,
                                                           QDir::Name );

    QStringList lAllLogFilesContent;

    for( int lIndex = 0; lIndex < lFileInfoList.count(); lIndex++ )
        lAllLogFilesContent << GetLogFileContentWithoutDate( lFileInfoList[ lIndex ] );

    return lAllLogFilesContent;
}

QStringList GTriggerTests::GetOrderedStdfFilesPathInDirectory(const QString &aDirectoryPath)
{
    QDirIterator lPatStdfDirectoryIterator( aDirectoryPath,
                                            QDir::NoDotAndDotDot | QDir::Files,
                                            QDirIterator::Subdirectories  );

    QStringList lOrderedStdfFilePath;

    while( lPatStdfDirectoryIterator.hasNext() )
    {
        lOrderedStdfFilePath << lPatStdfDirectoryIterator.next();
    }

    std::sort( lOrderedStdfFilePath.begin(), lOrderedStdfFilePath.end() );

    return lOrderedStdfFilePath;
}

QStringList GTriggerTests::GetOrderedAtdfFilesPathFromStdfFilesPath(const QStringList &aOrderedStdfFilesPath)
{
    QStringList lAtdfFilesList;
    for( int lIndex = 0; lIndex < aOrderedStdfFilesPath.count(); lIndex++ )
        lAtdfFilesList << QString{ "%1.%2" }.arg( aOrderedStdfFilesPath[ lIndex ] ).arg( "atd" );

    return lAtdfFilesList;
}

void GTriggerTests::ConvertFilesFromStdfToAtdf(const QStringList &aStdfFileList, const QStringList &aAtdfFileList)
{
    for( int lindex = 0; lindex < aStdfFileList.count(); lindex++ )
        SimpleStdfToAtdfConverter{ aStdfFileList[ lindex ], aAtdfFileList[ lindex ] }.RunConversion();
}

void GTriggerTests::ConcatenateAllAtdfFiles(const QStringList &aFileList)
{
    QString lResultAtdfFilePath{ QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "result.atd.out" ) };

    QFile lResultAtdfFile{ lResultAtdfFilePath };

    if( ! lResultAtdfFile.open( QFile::WriteOnly | QFile::Text ) )
        throw "Could not create the atdf result file";

    QTextStream lResultAtdfFileStream{ &lResultAtdfFile };

    for( int lIndex = 0; lIndex < aFileList.count(); lIndex++ )
        AppendFileContentToStream( aFileList[ lIndex ], lResultAtdfFileStream );
}

void GTriggerTests::AppendFileContentToStream(const QString &aFilePath, QTextStream &aFileStream)
{
    QFile aAtdfFile{ aFilePath };

    if( ! aAtdfFile.open( QFile::ReadOnly | QFile::Text ) )
        throw "Cannot open atdf file to create final result atdf file";

    aFileStream << aAtdfFile.readAll();
}

void GTriggerTests::ConcatenateLogsAndAtdfFiles()
{
    QStringList aFileList = QStringList{}
        << QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "result.atd.out" )
        << QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "summaries.log.out" )
        << QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "wafers.log.out" );

    QFile lResultTxtFile{ QString{ "%1/%2" }.arg( TEST_PROGRAM_DIRECTORY ).arg( "result.txt.out" ) };

    if( ! lResultTxtFile.open( QFile::WriteOnly | QFile::Text ) )
        throw "Could not open final result.txt file for writing";

    QTextStream lResultFileStream{ &lResultTxtFile };

    for( int lIndex = 0; lIndex < aFileList.count(); lIndex++ )
        AppendFileContentToStream( aFileList[ lIndex ], lResultFileStream );
}

void GTriggerTests::initTestCase()
{
    WipeOutputDirectory();
    mEngine.reset(Q_NULLPTR);
}

void GTriggerTests::cleanupTestCase()
{
    mEngine.reset(Q_NULLPTR);
}

void GTriggerTests::init()
{
    QFETCH( QString, InputFilesDirectoryName );

    CopyInputFileFromDirectoryToOutputDirectory( InputFilesDirectoryName );
}

QTEST_GUILESS_MAIN(GTriggerTests)

#endif // _G_TRIGGER_TESTS_CPP_