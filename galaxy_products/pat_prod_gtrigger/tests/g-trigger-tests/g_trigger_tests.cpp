#include "details/g_trigger_tests.cpp"

#include <QTest>
#include <QString>

void GTriggerTests::GTriggerTestsSuite_data()
{
    QTest::addColumn< QString >( "InputFilesDirectoryName" );
    QTest::addColumn< QString >( "ReferenceFileName" );

    QTest::newRow( "FetTest LVM_WS" ) << "lvm_ws" << "result.txt";
}