#ifndef _TEST_OVERALL_BIN_MAPPING_H_
#define _TEST_OVERALL_BIN_MAPPING_H_

#include <QtTest>
#include <QScopedPointer>

#include "testable_bin_map_store.h"
#include "bin_map_store_factory.h"

class TestOverallBinMapping : public QObject
{
    Q_OBJECT

private :
    void SetupBinMapTest(Qx::BinMapping::BinMapStoreTypes aBinMapStoreType,
                         const QString &aBinmapInputDirectoryName,
                         const QString &aBinMapInputFileName );
    void SetupBinMapTestPaths( const QString &aBinMapInputDirectoryName,
                               const QString &aBinMapInputFile );
    void SetupBinMapTestDirectoryPaths( const QString &aBinMapType );
    void SetupBinMapTestFilePaths(const QString &aBinMapInputFile );
    void SetupBinMapStore(Qx::BinMapping::BinMapStoreTypes aBinMapType);
    void InstantiateBinMapStore(Qx::BinMapping::BinMapStoreTypes aBinMapType );
    void TestBinMapStore();
    void CompareOutputAndReferenceFileContents();
    void ResetTestCase();
    void FailTest(Qx::BinMapping::BinMapStoreTypes aBinMapType ) const;
    void CheckReferenceAndOutputFileStreams( std::ifstream &aReferenceFileStream,
                                             std::ifstream &aOutputFileStream ) const;
    void CompareReferenceAndOutputFileStreamLines( std::ifstream &aReferenceFileStream,
                                             std::ifstream &aOutputFileStream ) const;

private slots:
    void initTestCase();
    void cleanupTestCase();

    void run_all_tests_data();
    void run_all_tests();

private :
    QScopedPointer< SerializableToFile > mTestableBinMapStore;
    QString mBinMapInputDirectory;
    QString mBinMapOutputDirectory;
    QString mBinMapReferenceDirectory;
    QString mBinMapInputFilePath;
    QString mBinMapOutputFilePath;
    QString mBinMapReferenceFilePath;
};

#endif //_TEST_OVERALL_BIN_MAPPING_H_