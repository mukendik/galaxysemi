#include <iostream>
#include <QMap>
#include <QDateTime>
#include <QStringList>


#include "gexdatabasekeyscontent_ut.h"
#include "gqtl_datakeys.h"
#include "gqtl_datakeys_file.h"

using namespace std;

GexDatabaseKeysContent_UT::GexDatabaseKeysContent_UT()
{
}

bool GexDatabaseKeysContent_UT::RunTests()
{
    return true;
}

bool GexDatabaseKeysContent_UT::ut_InvalidStaticKeyName()
{
    cout << "Try to set an invalid static key: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = dbKeys.SetDbKeyContent("dummy","");
    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_InvalidStaticKeyValue()
{
    cout << "Try to set an invalid value to a static key: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = !dbKeys.SetDbKeyContent("RetestIndex", "dummy");
    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_SetReadOnlyStaticKey()
{
    cout << "Try to set an READONLY value to a static key: ";

    GS::QtLib::DatakeysContent dbKeys;

    // Set an read only keys
    bool bStatus = dbKeys.SetDbKeyContent("SplitlotId", "11111");

    if (bStatus)
    {
        // Set a key internally forced to read only mode
        dbKeys.SetInternal("SplitlotId", "111222");

        bStatus = !dbKeys.SetDbKeyContent("SplitlotId","111333");
    }

    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_SetStaticKey()
{
    cout << "Check all Static keys: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = true;
    QString lType;
    QString lValue;
    QString lKey;
    QStringList lKeys = dbKeys.allowedStaticDbKeys();

    for(int lIdx = 0; lIdx < lKeys.count(); ++lIdx)
    {
        lKey    = lKeys.at(lIdx);
        lType   = "STRING";
        lValue  = "dummy";

        if(dbKeys.allowedStaticDbKeys().contains(lKey.toLower()))
            lType = dbKeys.DataKeysDefinition().GetDataKeysData(lKey.toLower()).GetDataType();

        if(lType == "READONLY" || lType == "DEPRECATED")
            continue;

        if(lType == "NUMBER")
            lValue = "999";
        else if (lType.contains("|"))
            lValue = lType.section("|",0,0);
        else if (lType == "BOOLEAN")
            lValue = "Y";
        else if (lType == "TIMESTAMP")
            lValue = QString::number(QDateTime::currentDateTime().toTime_t());
        else if (lType == "CHAR")
            lValue = "A";

        bStatus = dbKeys.SetDbKeyContent(lKey,lValue);

        if(!bStatus)
        {
            cout << "Error=" << dbKeys.Get("Error").toString().toLatin1().constData() <<  endl;
            break;
        }
    }

    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_SetGetCustomKey()
{
    cout << "Try to set and read an CUSTOM key: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = dbKeys.Set("MyCustomVar","dummy");
    if(bStatus)
        bStatus = (dbKeys.Get("MyCustomVar").toString() == "dummy");

    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_TestNumberKey()
{
    cout << "Try to set a numeric key with different type of data: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = dbKeys.Set("Station","10");

    // Try with a string
    if(bStatus)
        bStatus = !dbKeys.Set("Station","dummy");

    // Try with a char
    if(bStatus)
        bStatus = !dbKeys.Set("Station", "Y");

    cout << bStatus << endl;

    if(!bStatus)
        cout << "Error=" << dbKeys.Get("Error").toString().toLatin1().constData() <<  endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_TestBooleanKey()
{
    cout << "Try to set a boolean key with different type of data: ";

    GS::QtLib::DatakeysContent dbKeys;
    QVariant                   lVarRes;

    // Try with 'Y'
    bool bStatus = dbKeys.Set("Validation","Y");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == true);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['Y']");
        }
    }

    // Try with 'N'
    if(bStatus)
        bStatus = dbKeys.Set("Validation","N");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == false);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['N']");
        }
    }

    // Try with 0
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "0");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == false);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation [0]");
        }
    }

    // Try with 1
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "1");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == true);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation [1]");
        }
    }

    // Try with "Yes"
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "Yes");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == true);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['Yes']");
        }
    }

    // Try with "No"
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "No");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == false);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['No']");
        }
    }

    // Try with "True"
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "True");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == true);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['True']");
        }
    }

    // Try with "False"
    if(bStatus)
        bStatus = dbKeys.Set("Validation", "False");

    if (bStatus)
    {
        lVarRes = dbKeys.Get("Validation");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toBool() == false);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for Validation ['False']");
        }
    }

    // Try with string
    if(bStatus)
        bStatus = !dbKeys.Set("Validation", "Dummy");

    // Try with number different from 0 and 1
    if(bStatus)
        bStatus = !dbKeys.Set("Validation", "2");

    cout << bStatus << endl;

    if(!bStatus)
        cout << "Error=" << dbKeys.Get("Error").toString().toLatin1().constData() <<  endl;


    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_TestCharKey()
{
    cout << "Try to set a char key with different type of data: ";

    GS::QtLib::DatakeysContent dbKeys;
    bool bStatus = dbKeys.Set("ProtectionCode", "A");

    // Try with a string
    if(bStatus)
        bStatus = !dbKeys.Set("ProtectionCode", "dummy");

    cout << bStatus << endl;

    if(!bStatus)
        cout << "Error=" << dbKeys.Get("Error").toString().toLatin1().constData() <<  endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_TestDateTimeKey()
{
    cout << "Try to set a timestamp key with different type of data: ";

    GS::QtLib::DatakeysContent dbKeys;
    QVariant    lVarRes;
    bool bStatus = dbKeys.Set("FinishTime", QVariant(991790035));

    // Check finishtime value
    if (bStatus)
    {
        lVarRes = dbKeys.Get("FinishTime");
        if (lVarRes.isValid())
            bStatus = (lVarRes.toLongLong() == 991790035);
        else
        {
            bStatus = false;
            dbKeys.Set("Error", "Wrong value found for FinishTime [991790035]");
        }
    }

    // Try with a string
    if(bStatus)
        bStatus = !dbKeys.Set("FinishTime", "Dummy");

    cout << bStatus << endl;

    if(!bStatus)
        cout << "Error=" << dbKeys.Get("Error").toString().toLatin1().constData() <<  endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_LoadInvalidDbKeysFile(const QString& configFileName)
{
    QList<GS::QtLib::DatakeysError> lErrors;
    GS::QtLib::DatakeysFile dbKeysFile = GS::QtLib::DatakeysFile(configFileName);

    cout << "Loading invalid db keys " << configFileName.toLatin1().constData() << " : ";

    bool bStatus = !dbKeysFile.Read(lErrors);

    cout << bStatus << endl;

    return bStatus;
}

bool GexDatabaseKeysContent_UT::ut_LoadValidDbKeysFile(const QString& configFileName)
{
    QList<GS::QtLib::DatakeysError> lErrors;
    GS::QtLib::DatakeysFile dbKeysFile = GS::QtLib::DatakeysFile(configFileName);

    cout << "Loading valid db keys " << configFileName.toLatin1().constData() << " : ";

    bool bStatus = dbKeysFile.Read(lErrors);

    cout << bStatus << endl;

    if (!bStatus)
    {
        if (lErrors.isEmpty() == false)
            cout << "Error at line " << lErrors.at(0).mLine << " with message: " <<
                    lErrors.at(0).mMessage.toLatin1().constData() << endl;
    }

    return bStatus;
}


