/******************************************************************************!
 * \file main.cpp
 ******************************************************************************/
#include <QDebug>
#include <QMap>
#include <QSet>
#include <QFileInfo>
#include <QStringList>
#include "gqtl_datakeys.h"

/******************************************************************************!
 * \fn keysAreConsistent
 ******************************************************************************/
bool keysAreConsistent(GS::QtLib::DataKeysDefinitionLoader& dkdl)
{

    QStringList lAllowedStaticDbKeys =
        QStringList() <<
        "validation" << "dataorigin" << "family" << "lot" <<
        "trackinglot" << "subconlot" << "product" << "sublot" << "facility" <<
        "floor" << "frequencystep" << "packagetype" << "process" <<
        "loadboard" << "loadboardtype" << "probername" <<
        "probertype" << "extraname" << "extratype" << "programname" <<
        "programrevision" << "temperature" << "testingcode" <<
        "testername" << "testertype" << "wafer" << "wafernb" <<
        "etestsiteconfig" << "wafernotch" << "burnintime" <<
        "operator" << "datecode" << "specname" << "designrevision" <<
        "user1" << "user2" << "user3" << "user4" << "user5" <<
        "retestindex" << "retestbinlist" <<
        "wt_yieldconsolidation_rule" << "datatype" << "proddata" <<
        "filename" << "station" << "setuptime" << "starttime" <<
        "finishtime" << "sourcearchive" << "stdffilename" <<
        "user1label" << "user2label" << "user3label" << "user4label" <<
        "user5label" << "dibtype" << "dibname" << "ftpserver" <<
        "ftpport" << "ftpuser" << "ftppassword" << "ftppath" <<
        "movepath" << "datapumpname" << "weeknb" << "year" <<
        "grossdie" << "ignoresummary" << "usertext" << "forcetestingstage" <<
        "rejectifnbpartslessthan" << "rejectifnbpartspercentofgdpwlessthan" <<
        "rejectifpasshardbinnotinlist" << "ignoreresultsifnbpartsmorethan" <<

        // new values
        "exectype" << "execversion" << "retestcode" << "protectioncode" <<
        "commandmodecode" << "auxiliaryfile" << "specversion" << "flow" <<
        "setup" << "engineering" << "romcode" << "serialnumber" <<
        "supervisorname" << "headnumber" << "cardtype" << "card" <<
        "cabletype" << "cable" << "contactortype" << "contactor" <<
        "lasertype" << "laser" << "sitenumbers" << "allowbinprrmismatch" <<
        "testingstage" << "timeinsertion" << "splitlotid" << "filesize" <<
        "timeconvertion" << "timeuncompress" << "fulldestinationname" <<
        "stdffilesize" << "insertionstatus" << "configfilename" <<
        "databasename" << "retestphase" << "replaceexistingdata" <<
        "consolidationprocess" << "consolidationprocessstatus" <<
        "uploadfile" << "movefile" << "ignoreresults" <<
        "testremovesequencername" << "testremovepinname" << "testmergeby" <<
        "consolidationalgo" << "testinsertion" << "testflow" << "testignoredtrtestconditions";

    QStringList lDataKeys = dkdl.GetStaticKeys();

    QSet<QString> lResult;
    if (lDataKeys.count() > lAllowedStaticDbKeys.count())
    {
        lResult = lDataKeys.toSet().subtract(lAllowedStaticDbKeys.toSet());
    }
    else
    {
        lResult = lAllowedStaticDbKeys.toSet().subtract(lDataKeys.toSet());
    }

    if (! lResult.isEmpty())
    {
        qDebug() << "error: subtract return non empty set";
        QSet<QString>::const_iterator iter;
        for (iter = lResult.begin(); iter != lResult.end(); ++iter)
        {
            qDebug() << *iter;
        }
        return false;
    }

    if (dkdl.GetDataKeysData(QString("BurninTime")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("RetestIndex")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("ProdData")).
        GetDataType() != QString("BOOLEAN"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("Station")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("SetupTime")).
        GetDataType() != QString("TIMESTAMP"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("StartTime")).
        GetDataType() != QString("TIMESTAMP"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("FinishTime")).
        GetDataType() != QString("TIMESTAMP"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("FtpPort")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("WeekNb")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("Year")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("GrossDie")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("IgnoreSummary")).
        GetDataType() != QString("BOOLEAN"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("RejectIfNbPartsLessThan")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("RejectIfNbPartsPercentOfGDPWLessThan")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("IgnoreResultsIfNbPartsMoreThan")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("CommandModeCode")).
        GetDataType() != QString("CHAR"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("WaferNb")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("HeadNumber")).
        GetDataType() != QString("NUMBER"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("ProtectionCode")).
        GetDataType() != QString("CHAR"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("RetestCode")).
        GetDataType() != QString("CHAR"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("AllowBinPRRMismatch")).
        GetDataType() != QString("ERROR|WARNING|IGNORE"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("FileName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("FileSize")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("StdfFileName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("StdfFileSize")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("SourceArchive")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("FullDestinationName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("ConfigFileName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("DataPumpName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("DatabaseName")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("SplitlotId")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("TestingStage")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("TimeUncompress")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("TimeConvertion")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("TimeInsertion")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if (dkdl.GetDataKeysData(QString("InsertionStatus")).
        GetDataType() != QString("READONLY"))
    {
        return false;
    }
    if(dkdl.GetDataKeysData(QString("TestIgnoreDtrTestConditions")).
        GetDataType() != QString("BOOLEAN"))
    {
        return false;
    }

    return true;

}

/******************************************************************************!
 * \fn keysLoadingIsOk
 ******************************************************************************/
bool keysLoadingIsOk(const QString& fileName)
{
    QString lDatakeysLoaderError;
    GS::QtLib::DatakeysContent lDatakeysContent;

    bool lResult =
        GS::QtLib::DatakeysLoader::Load(fileName,
                                        lDatakeysContent,
                                        lDatakeysLoaderError);
    if (! lResult)
    {
        qDebug() <<
            QString("DatakeysLoader::Load return false on file") << fileName;
        qDebug() <<
            QString("error: ") << lDatakeysLoaderError;
        return false;
    }

    QMap<QString, QVariant> lKeysContent = lDatakeysContent.toMap();
    QStringList lKeys = lKeysContent.keys();
    for (int lIdx = 0; lIdx < lKeys.count(); ++lIdx)
    {
        qDebug() << lKeys[lIdx] << ";" <<
            lDatakeysContent.Get(lKeys[lIdx]).toString();

    }

    lResult = (lDatakeysContent.Get("BurninTime").toInt() == 65535);
    lResult &= (lDatakeysContent.Get("DataType").toString() == "E");
    lResult &= (lDatakeysContent.Get("DibType").toInt() == 0);
    lResult &= (lDatakeysContent.Get("ProgramName").toString() == "mobile-05");
    lResult &= (lDatakeysContent.Get("ProgramRevision").toInt() == 16);
    lResult &= (lDatakeysContent.Get("Lot").toString() == "GAL-LOT");
    lResult &= (lDatakeysContent.Get("Operator").toString() == "ews");
    lResult &= (lDatakeysContent.Get("ProberType").toString() == "electrogl");
    lResult &= (lDatakeysContent.Get("Product").toString() == "GOLD8BAR");
    lResult &= (lDatakeysContent.Get("SubLot").toInt() == 03);
    lResult &= (lDatakeysContent.Get("SubconLot").toString() == "GAL-LOT");
    lResult &= (lDatakeysContent.Get("TesterName").toString() == "tester-1");
    lResult &= (lDatakeysContent.Get("TesterType").toString() == "A530");
    lResult &= (lDatakeysContent.Get("TestingCode").toString() == "E38");
    lResult &= (lDatakeysContent.Get("TrackingLot").toString() == "GAL-LOT");
    lResult &= (lDatakeysContent.Get("Wafer").toString() == "GAL-LOT-03");
    lResult &= (lDatakeysContent.Get("FinishTime").toULongLong() == 991795688);
    lResult &= (lDatakeysContent.Get("SetupTime").toULongLong() == 991732686);
    lResult &= (lDatakeysContent.Get("StartTime").toULongLong() == 991790025);
    lResult &= (lDatakeysContent.Get("CommandModeCode").toString() == "a");
    lResult &= (lDatakeysContent.Get("ExecType").toString() ==
                "IMAGE V6.3.y2k D8 052200");
    lResult &= (lDatakeysContent.Get("WaferNotch").toChar() == 'D');
    lResult &= (lDatakeysContent.Get("Station").toInt() == 1);
    lResult &= (lDatakeysContent.Get("RetestIndex").toInt() == 0);

/*
    lResult &= (lDatakeysContent.Get("FtpPort").toInt() == 21);
    lResult &=
        (lDatakeysContent.Get("fulldestinationname").toString() == fileName);
    lResult &= (lDatakeysContent.Get("sourcearchive").toString() == fileName);
    lResult &= (lDatakeysContent.Get("headnumber").toInt() == 1);
    lResult &= (lDatakeysContent.Get("GrossDie").toInt() == 0);
    lResult &= (lDatakeysContent.Get("WeekNb").toInt() == 0);
    lResult &= (lDatakeysContent.Get("Year").toInt() == 0);
    lResult &= (lDatakeysContent.Get("FinishTimeOverloaded").toBool() == false);
    lResult &= (lDatakeysContent.Get("IgnoreSummary").toBool() == false);
    lResult &= (lDatakeysContent.Get("InsertTestResults").toBool() == true);
    lResult &= (lDatakeysContent.Get("MoveFile").toBool() == false);
    lResult &= (lDatakeysContent.Get("ProdData").toBool() == false);
    lResult &= (lDatakeysContent.Get("StartTimeOverloaded").toBool() == false);
    lResult &= (lDatakeysContent.Get("UploadFile").toBool() == false);
    lResult &= (lDatakeysContent.Get("WaferIDOverloaded").toBool() == false);
    lResult &= (lDatakeysContent.Get("WaferNotchOverloaded").toBool() == false);
    lResult &= (lDatakeysContent.Get("Ft_YieldConsolidation_MissingPartsBin").
                toInt() == -1);
    lResult &=
        (lDatakeysContent.Get("IgnoreResultsIfNbPartsMoreThan").toInt() == -1);
    lResult &= (lDatakeysContent.Get("RejectIfNbPartsLessThan").toInt() == -1);
    lResult &= (lDatakeysContent.Get("RejectIfNbPartsPercentOfGDPWLessThan").
                toInt() == -1);
*/
    return lResult;
}

/******************************************************************************!
 * \fn main
 ******************************************************************************/
int main(int  /*argc*/, char**  /*argv[]*/)
{

    QString lDefinitionLoaderMessage;
    if (! GS::QtLib::DataKeysDefinitionLoader::GetInstance().
        LoadingPass(lDefinitionLoaderMessage))
    {
        qDebug() << "Return 1 EXIT_FAILURE: " << EXIT_FAILURE;
        return EXIT_FAILURE;
    }

    bool lCheck =
        keysAreConsistent(GS::QtLib::DataKeysDefinitionLoader::GetInstance());
    if (! lCheck)
    {
        qDebug() << "Return 2 EXIT_FAILURE: " << EXIT_FAILURE;
        return EXIT_FAILURE;
    }

    QString lFileName = "./data_samples.std";
    lCheck = keysLoadingIsOk(QFileInfo(lFileName).absoluteFilePath());
    if (! lCheck)
    {
        qDebug() << "Return 3 EXIT_FAILURE: " << EXIT_FAILURE;
        return EXIT_FAILURE;
    }

    GS::QtLib::DataKeysDefinitionLoader::DestroyInstance();
    qDebug() << "Return 4 EXIT_SUCCESS : " << EXIT_SUCCESS;
    return EXIT_SUCCESS;
}
