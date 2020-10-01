#ifndef TEST_OVERALL_PROMIS_INTERPRETER_CPP
#define TEST_OVERALL_PROMIS_INTERPRETER_CPP

#include "test_overall_promis_interpreter.h"
#include "promis_interpreter_factory.h"

#include <test_overall_promis_interpreter.h>

using namespace Qx::BinMapping;

Q_DECLARE_METATYPE( PromisIntepreterTypes )

void TestOverallPromisInterpreter::ResetTestCase()
{
    mPromisInputDirectory.clear();
    mPromisInputFilePath.clear();
    mPromisOutputDirectory.clear();
    mPromisOutputFilePath.clear();
    mPromisReferenceDirectory.clear();
    mPromisReferenceFilePath.clear();
    mPromisKey.clear();
    mConvertExternalFilePath = "/converter_exernal_file.xml";
}

void TestOverallPromisInterpreter::TestPromisInterpreter()
{
    if( mTestablePromisInterpreter.isNull() )
        QFAIL( "Unabled to instantiate the promis interpreter." );

    mTestablePromisInterpreter->SerializeToFile( mPromisOutputFilePath );
    CompareOutputAndReferenceFileContents();
}

void TestOverallPromisInterpreter::FailTest(PromisIntepreterTypes aPromisType) const
{
    QFAIL( QString( "Incorrect promis type specified. Either fix the `PromisInterpreterType` column "
                    "of your test data definition in `TestOverallPromisInterpreter::run_all_tests_data()` or "
                    "modify `TestOverallPromisInterpreter::InstantiatePromisInterpreter(const QString &aPromisType)` "
                    "in  order to create the correct mapping between %1 and a concrete promis interpreter of the "
                    "`qx_bin_mapping` library such as `Qx::BinMapping::LvmFtSubconDataPromisInterpreter` for instance" )
           .arg( aPromisType ).toStdString().c_str() );
}

void TestOverallPromisInterpreter::SetupPromisTestDirectoryPaths(const QString &aPromisType)
{
    mPromisInputDirectory =
        QString( "%1/%2" ).arg( PROMIS_INTERPRETER_TESTS_PROJECT_DIRECTORY ).arg( "input/%3" ).arg( aPromisType );

    mPromisReferenceDirectory =
        QString( "%1/%2" ).arg( PROMIS_INTERPRETER_TESTS_PROJECT_DIRECTORY ).arg( "reference/%3" ).arg( aPromisType );

    mPromisOutputDirectory =
        QString( "%1/%2" ).arg( PROMIS_INTERPRETER_TESTS_PROJECT_DIRECTORY ).arg( "output/%3" ).arg( aPromisType );

    if( ! QDir( mPromisOutputDirectory ).exists() )
        QDir().mkpath( mPromisOutputDirectory );
}

void TestOverallPromisInterpreter::SetupPromisTestFilePaths(const QString &aPromisInputFile )
{
    mPromisInputFilePath = QString( "%1/%2" ).arg( mPromisInputDirectory ).arg( aPromisInputFile );
    mPromisOutputFilePath = QString( "%1/%2" ).arg( mPromisOutputDirectory ).arg( aPromisInputFile );
    mPromisReferenceFilePath = QString( "%1/%2" ).arg( mPromisReferenceDirectory ).arg( aPromisInputFile );
}

void TestOverallPromisInterpreter::CompareOutputAndReferenceFileContents()
{
    std::ifstream lReferenceFileStream( mPromisReferenceFilePath.toStdString().c_str() );
    std::ifstream lOutputFileStream( mPromisOutputFilePath.toStdString().c_str() );

    if( ! lReferenceFileStream.is_open() || ! lOutputFileStream.is_open() )
        QFAIL( QString( "Could not open output or reference file for comparison purposes. "
                        "Reference file path : %1 ..... Output file path : %2" )
               .arg( mPromisReferenceFilePath ).arg( mPromisOutputFilePath ).toStdString().c_str() );

    std::string lReferenceLine, lOutputLine;

    while( std::getline( lReferenceFileStream, lReferenceLine ) && std::getline( lOutputFileStream, lOutputLine ) )
        if( lReferenceLine.compare( lOutputLine ) != 0 )
            QFAIL( "Reference file content differs from output file content." );
}

void TestOverallPromisInterpreter::initTestCase()
{
    ResetTestCase();
}

void TestOverallPromisInterpreter::cleanupTestCase()
{
    ResetTestCase();
}

void TestOverallPromisInterpreter::run_all_tests()
{
    QFETCH( PromisIntepreterTypes, PromisInterpreterType );
    QFETCH( QString, PromisInputDirectoryName );
    QFETCH( QString, PromisInputFileName );
    QFETCH( QString, PromisKey);

    SetupPromisTest( PromisInterpreterType, PromisInputDirectoryName, PromisInputFileName, PromisKey );
    TestPromisInterpreter();
}

void TestOverallPromisInterpreter::SetupPromisTest(PromisIntepreterTypes aPromisInterpreterType,
                                                   const QString &aPromisInputDirectoryName,
                                                   const QString &aPromisInputFileName,
                                                   const QString &aPromisKey)
{
    mPromisKey = aPromisKey;

    SetupPromisTestPaths(aPromisInputDirectoryName, aPromisInputFileName );
    SetupPromisInterpreter( aPromisInterpreterType );
}

void TestOverallPromisInterpreter::SetupPromisTestPaths(const QString &aPromisInputDirectoryName, const QString &aPromisInputFileName)
{
    SetupPromisTestDirectoryPaths( aPromisInputDirectoryName );
    SetupPromisTestFilePaths( aPromisInputFileName );
}

void TestOverallPromisInterpreter::SetupPromisInterpreter(PromisIntepreterTypes aPromisInterpreterType)
{
    try
    {
        InstantiatePromisInterpreter( aPromisInterpreterType );
    }
    catch( const std::exception &aException )
    {
        QFAIL( QString( "Exception thrown during the construction of the promis interpreter for %1 promis type : %2" )
               .arg( aPromisInterpreterType ).arg( aException.what() ).toStdString().c_str() );
    }
}

QTEST_APPLESS_MAIN(TestOverallPromisInterpreter)

#endif
