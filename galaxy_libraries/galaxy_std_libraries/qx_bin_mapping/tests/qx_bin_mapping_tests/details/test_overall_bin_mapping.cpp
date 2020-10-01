#ifndef _TEST_OVERALL_BIN_MAPPING_CPP_
#define _TEST_OVERALL_BIN_MAPPING_CPP_

#include "test_overall_bin_mapping.h"
#include "bin_map_store_factory.h"

#include <QDir>

#include <exception>
#include <fstream>

using namespace Qx::BinMapping;

Q_DECLARE_METATYPE( BinMapStoreTypes )

void TestOverallBinMapping::SetupBinMapTest(BinMapStoreTypes aBinMapStoreType,
                                            const QString &aBinmapInputDirectoryName,
                                            const QString &aBinMapInputFileName)
{
    SetupBinMapTestPaths(aBinmapInputDirectoryName, aBinMapInputFileName );
    SetupBinMapStore( aBinMapStoreType );
}

void TestOverallBinMapping::SetupBinMapStore( BinMapStoreTypes aBinMapType)
{
    try
    {
        InstantiateBinMapStore( aBinMapType );
    }
    catch( const std::exception &aException )
    {
        QFAIL( QString( "Exception thrown during the construction of the bin map store for %1 bin mapping type : %2" )
               .arg( aBinMapType ).arg( aException.what() ).toStdString().c_str() );
    }
}

void TestOverallBinMapping::SetupBinMapTestPaths(const QString &aBinMapInputDirectoryName,
                                                 const QString &aBinMapInputFile )
{
    SetupBinMapTestDirectoryPaths( aBinMapInputDirectoryName );
    SetupBinMapTestFilePaths( aBinMapInputFile );
}

void TestOverallBinMapping::SetupBinMapTestDirectoryPaths(const QString &aBinMapType)
{
    mBinMapInputDirectory =
        QString( "%1/%2" ).arg( QX_BIN_MAPPING_TESTS_PROJECT_DIRECTORY ).arg( "input/%3" ).arg( aBinMapType );

    mBinMapReferenceDirectory =
        QString( "%1/%2" ).arg( QX_BIN_MAPPING_TESTS_PROJECT_DIRECTORY ).arg( "reference/%3" ).arg( aBinMapType );

    mBinMapOutputDirectory =
        QString( "%1/%2" ).arg( QX_BIN_MAPPING_TESTS_PROJECT_DIRECTORY ).arg( "output/%3" ).arg( aBinMapType );

    if( ! QDir( mBinMapOutputDirectory ).exists() )
        QDir().mkpath( mBinMapOutputDirectory );
}

void TestOverallBinMapping::SetupBinMapTestFilePaths(const QString &aBinMapInputFile)
{
    mBinMapInputFilePath = QString( "%1/%2" ).arg( mBinMapInputDirectory ).arg( aBinMapInputFile );
    mBinMapOutputFilePath = QString( "%1/%2" ).arg( mBinMapOutputDirectory ).arg( aBinMapInputFile );
    mBinMapReferenceFilePath = QString( "%1/%2" ).arg( mBinMapReferenceDirectory ).arg( aBinMapInputFile );
}

void TestOverallBinMapping::TestBinMapStore()
{
    if( mTestableBinMapStore.isNull() )
        QFAIL( "Unabled to instantiate the bin map store." );

    mTestableBinMapStore->SerializeToFile( mBinMapOutputFilePath );
    CompareOutputAndReferenceFileContents();
}

void TestOverallBinMapping::CompareOutputAndReferenceFileContents()
{
    std::ifstream lReferenceFileStream( mBinMapReferenceFilePath.toStdString().c_str() );
    std::ifstream lOutputFileStream( mBinMapOutputFilePath.toStdString().c_str() );

    CheckReferenceAndOutputFileStreams( lReferenceFileStream, lOutputFileStream );
    CompareReferenceAndOutputFileStreamLines( lReferenceFileStream, lOutputFileStream );
}

void TestOverallBinMapping::ResetTestCase()
{
    mTestableBinMapStore.reset( Q_NULLPTR );
    mBinMapInputDirectory.clear();
    mBinMapOutputDirectory.clear();
    mBinMapReferenceDirectory.clear();
    mBinMapInputFilePath.clear();
    mBinMapOutputFilePath.clear();
    mBinMapReferenceFilePath.clear();
}

void TestOverallBinMapping::FailTest(BinMapStoreTypes aBinMapType) const
{
    QFAIL( QString( "Incorrect bin mapping type specified. Either fix the `BinMapStoreType` column "
                    "of your test data definition in `TestOverallBinMapping::run_all_tests_data()` or "
                    "modify `TestOverallBinMapping::InstantiateBinmapStore(const QString &aBinMapType)` in "
                    "order to create the correct mapping between %1 and a concrete bin map store of the "
                    "`qx_bin_mapping` library such as `Qx::BinMapping::LvmWsBinMapStore` for instance" )
           .arg( aBinMapType ).toStdString().c_str() );
}

void TestOverallBinMapping::CheckReferenceAndOutputFileStreams( std::ifstream &aReferenceFileStream,
                                                                std::ifstream &aOutputFileStream ) const
{
    if( ! aReferenceFileStream.is_open() || ! aOutputFileStream.is_open() )
        QFAIL( QString( "Could not open output or reference file for comparison purposes. "
                        "Reference file path : %1 ..... Output file path : %2" )
               .arg( mBinMapReferenceFilePath ).arg( mBinMapOutputFilePath ).toStdString().c_str() );

    if( aReferenceFileStream.peek() == std::ifstream::traits_type::eof() )
        QWARN( "Reference file is empty, make sure that it is intended." );
}

void TestOverallBinMapping::CompareReferenceAndOutputFileStreamLines( std::ifstream &aReferenceFileStream,
                                                                      std::ifstream &aOutputFileStream ) const
{
    std::string lReferenceLine, lOutputLine;
    bool lReferenceLineRead, lOutputLineRead;

    do
    {
        lReferenceLineRead = std::getline( aReferenceFileStream, lReferenceLine );
        lOutputLineRead = std::getline( aOutputFileStream, lOutputLine );

        if( lReferenceLine.compare( lOutputLine ) != 0 )
            QFAIL( "Reference file content differs from output file content." );
    }
    while( lReferenceLineRead && lOutputLineRead );
}

void TestOverallBinMapping::initTestCase()
{
    ResetTestCase();
}

void TestOverallBinMapping::cleanupTestCase()
{
    ResetTestCase();
}

void TestOverallBinMapping::run_all_tests()
{
    QFETCH( BinMapStoreTypes, BinMapStoreType );
    QFETCH( QString, BinMappingInputDirectoryName );
    QFETCH( QString, BinMappingInputFileName );

    SetupBinMapTest( BinMapStoreType, BinMappingInputDirectoryName, BinMappingInputFileName );
    TestBinMapStore();
}

QTEST_APPLESS_MAIN(TestOverallBinMapping)

#endif // _TEST_OVERALL_BIN_MAPPING_CPP_