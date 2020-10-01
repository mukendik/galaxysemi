#include "test_overall_promis_interpreter.h"
#include "details/test_overall_promis_interpreter.cpp"
#include "testable_promis_interpreter.h"
#include "promis_interpreter_factory.h"

void TestOverallPromisInterpreter::run_all_tests_data()
{
    QTest::addColumn< PromisIntepreterTypes >( "PromisInterpreterType" );
    QTest::addColumn< QString >( "PromisInputDirectoryName" );
    QTest::addColumn< QString >( "PromisInputFileName" );
    QTest::addColumn< QString >( "PromisKey" );

    QTest::newRow("lvm_ft_subcon_data")
        << promis_lvm_ft_subcon_data
        << "lvm_ft_subcon_data"
        << "subcon_promis_data.txt"
        << "H21D166.1";

    QTest::newRow("lvm_ft_lotlist")
        << promis_lvm_ft_lotlist
        << "lvm_ft_lotlist"
        << "Vishay_ASE_lotlist.csv"
        << "H21D166.10";

    QTest::newRow("lvm_wt")
        << promis_lvm_wt
        << "lvm_wt"
        << "promis_data.txt"
        << "A35260.11";

    QTest::newRow("hvm_wt")
        << promis_hvm_wt
        << "hvm_wt"
        << "HVM_PROMIS_DATA.TXT"
        << "A04K041.17";

    QTest::newRow("lvm_ft - one product")
        << promis_lvm_ft
        << "lvm_ft"
        << "PROMIS_DATA_FT_1.TXT"
        << "A35250.11";

    QTest::newRow("lvm_ft - two products")
        << promis_lvm_ft
        << "lvm_ft"
        << "PROMIS_DATA_FT_2.TXT"
        << "F18A014.85";

    QTest::newRow("lvm_ft - three products")
        << promis_lvm_ft
        << "lvm_ft"
        << "PROMIS_DATA_FT_3.TXT"
        << "F40D456.16";

    QTest::newRow("hvm_ft")
        << promis_hvm_ft
        << "hvm_ft"
        << "HVM_PROMIS_DATA_FT.TXT"
        << "K09Y178.1";
}

void TestOverallPromisInterpreter::InstantiatePromisInterpreter(Qx::BinMapping::PromisIntepreterTypes aPromisType)
{
    switch( aPromisType )
    {
    case promis_lvm_ft_subcon_data :
    {
        typedef TestablePromisInterpreter< LvmFtSubconDataPromisInterpreter > PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    case promis_lvm_ft_lotlist:
    {
        typedef TestablePromisInterpreter< LvmFtLotlistPromisInterpreter > PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    case promis_lvm_wt:
    {
        typedef TestablePromisInterpreter< LvmWtPromisInterpreter> PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    case promis_hvm_wt:
    {
        typedef TestablePromisInterpreter< HvmWtPromisInterpreter> PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    case promis_lvm_ft:
    {
        typedef TestablePromisInterpreter< LvmFtPromisInterpreter> PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    case promis_hvm_ft:
    {
        typedef TestablePromisInterpreter< HvmFtPromisInterpreter> PromisInterpreter;
        mTestablePromisInterpreter.reset( new PromisInterpreter( mPromisKey.toStdString(),
                                                                 mPromisInputFilePath,
                                                                 mConvertExternalFilePath ) );
        break;
    }

    default :
        FailTest( aPromisType );
    }
}
