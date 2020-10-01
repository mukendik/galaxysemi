#include <QtCore/QCoreApplication>
#include "dbkeysunittest.h"

#include <QDir>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString     unitTestFolder  = a.applicationDirPath() + QDir::separator() + "data";
    QDir        dataFolder(unitTestFolder);
    QStringList unitTestFiles   = dataFolder.entryList(QDir::nameFiltersFromString("unit_test_*.xml"), QDir::Files);
    int         failedUT        = 0;

    qDebug(QString("UnitTestFolder: %1").arg(unitTestFolder).toLatin1().constData());

    foreach(QString unitFile, unitTestFiles)
    {
        qDebug(QString("UnitTestFile: %1").arg(unitFile).toLatin1().constData());

        DbKeysUnitTest unitTest(unitTestFolder + QDir::separator() + unitFile);

        if (unitTest.execute() == false)
            ++failedUT;
    }

    return failedUT;
}
