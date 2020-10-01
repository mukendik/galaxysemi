#include "test_overall_bin_mapping.h"
#include "bin_map_store_factory.h"
#include "details/test_overall_bin_mapping.cpp"

#include <QString>

void TestOverallBinMapping::run_all_tests_data()
{
    QTest::addColumn< BinMapStoreTypes >( "BinMapStoreType" );
    QTest::addColumn< QString >( "BinMappingInputDirectoryName" );
    QTest::addColumn< QString >( "BinMappingInputFileName" );

    QTest::newRow( "lvm_ws" ) << lvm_ws << "lvm_ws_fettest" << "Vishay_Test_Binning_Mapping.csv";
    QTest::newRow( "hvm_ft" ) << hvm_ft << "hvm_ft" << "HVM_FT_Bin_Mapping.csv";

    QTest::newRow( "hvm_ws_fet_test" ) << hvm_ws_fet_test << "hvm_ws_fettest" << "HVM_Sort_Bin_Mapping_kcmod.csv";
    QTest::newRow( "hvm_ws - specktra" ) << hvm_ws_spektra << "hvm_ws_spektra" << "HVM_Sort_Bin_Mapping_kcmod.csv";

    QTest::newRow( "lvm_ft_ft" ) << lvm_ft_ft << "lvm_ft_ft" << "CSVFinalTestsFile20180402.csv";
    QTest::newRow( "lvm_ft_se" ) << lvm_ft_se << "lvm_ft_se" << "CSVSortEntriesFile.csv";

    QTest::newRow( "lvm_ft_se_new" ) << lvm_ft_se_new << "lvm_ft_se_new" << "CSVSortEntriesFile.csv";
}

void TestOverallBinMapping::InstantiateBinMapStore(Qx::BinMapping::BinMapStoreTypes aBinMapType)
{
    const QString lDummyConverterExternalFilePath( "dummy/converter_external_file.xml" );
    switch( aBinMapType )
    {
    case lvm_ws :
        mTestableBinMapStore.reset( new TestableBinMapStore< LvmWsBinMapStore >( mBinMapInputFilePath,
                                                                                 lDummyConverterExternalFilePath ) );
        break;
    case hvm_ft :
        mTestableBinMapStore.reset( new TestableBinMapStore< HvmFtBinMapStore >( mBinMapInputFilePath,
                                                                                 lDummyConverterExternalFilePath ) );
        break;
    case hvm_ws_spektra:
        mTestableBinMapStore.reset( new TestableBinMapStore< HvmWsBinMapStoreSpecktra >( mBinMapInputFilePath,
                                                                                         lDummyConverterExternalFilePath ) );
        break;
    case hvm_ws_fet_test:
        mTestableBinMapStore.reset( new TestableBinMapStore< HvmWsBinMapStoreFetTest >( mBinMapInputFilePath,
                                                                                        lDummyConverterExternalFilePath ) );
        break;
    case lvm_ft_ft:
        mTestableBinMapStore.reset( new TestableBinMapStore< LvmFtFinalTestBinMapStore> (mBinMapInputFilePath,
                                                                                         lDummyConverterExternalFilePath) );
        break;
    case lvm_ft_se:
        mTestableBinMapStore.reset( new TestableBinMapStore< LvmFtSortEntriesBinMapStore> (mBinMapInputFilePath,
                                                                                           lDummyConverterExternalFilePath) );
        break;
    case lvm_ft_se_new:
        mTestableBinMapStore.reset( new TestableBinMapStore< LvmFtSortEntriesNewBinMapStore> (mBinMapInputFilePath,
                                                                                              lDummyConverterExternalFilePath) );
        break;
    default :
        FailTest( aBinMapType );
    }
}
