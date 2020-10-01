#include <QRegExp>
#include <QDateTime>
#include <QTextStream>
#include <QVariant>
#include <QStringList>
#include <QFileInfo>
#include <gqtl_datakeys_definition_loader.h>
#include <gqtl_datakeys.h>
#include <gqtl_log.h>

#include "gqtl_datakeys_content.h"

namespace GS
{
namespace QtLib
{

unsigned DatakeysContent::sNumOfInstances=0;

///////////////////////////////////////////////////////////
// Key structur constructor
///////////////////////////////////////////////////////////
DatakeysContent::DatakeysContent(QObject* parent, bool fromInputFile/*=false*/)
    : QObject(parent)
{
    sNumOfInstances++;
    setObjectName("DatabaseKeysContent");

    Clear(fromInputFile);
}

DatakeysContent::DatakeysContent(const DatakeysContent &other)
    : QObject(other.parent())
{
    sNumOfInstances++;
    *this = other;
}

DatakeysContent& DatakeysContent::operator=(const DatakeysContent& lOther)
{
    if (&lOther==this)
        return *this;

    for(int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        StdfRecordsCount[i]= lOther.StdfRecordsCount[i];

    mTestConditions=lOther.mTestConditions;
    mTestAttributes=lOther.mTestAttributes;
    mAttributes=lOther.mAttributes;
    mReadOnly=lOther.mReadOnly;

    return *this;
}

DatakeysContent::~DatakeysContent()
{
    sNumOfInstances--;
}

unsigned     DatakeysContent::GetNumberOfInstances()
{
    return sNumOfInstances;
}

QVariant DatakeysContent::Get(const QString &key) const
{
    QString lInternalKey = key.toLower();

    // if 'base' is added to the filename or the stdffilename
    // for FileName, StdfFileName, FullDestinationName, ConfigFileName, ...
    if(lInternalKey.startsWith("base",Qt::CaseInsensitive))
    {
        // Check if the end of the name exists
        if(mAttributes.contains(lInternalKey.mid(4)))
        {
            QString lValue = mAttributes[lInternalKey.mid(4)].toString();
            QFileInfo cFileInfo(lValue);
            if(cFileInfo.isFile())
                return QVariant(cFileInfo.fileName());
        }
    }

    if(mAttributes.contains(lInternalKey))
        return mAttributes[lInternalKey];

    QVariant lValue;
    if (!GetDbKeyContent(lInternalKey, lValue))
        return QVariant();

    return QVariant(lValue);
}

bool DatakeysContent::GetDbKeyContent(const QString &key, QString &value) const
{
    QVariant lValue;
    bool lResult = GetDbKeyContent(key,lValue);
    value = lValue.toString();
    // Trimmed space
    value = value.trimmed();
    return lResult;
}

bool DatakeysContent::GetDbKeyContent(const QString &key, QVariant &value) const
{
    QString lInternalKey = key.toLower();

    // Check which value is in field to overload
    QRegExp testConditionRegExp(REGEXP_ATTR_TEST_CONDITION, Qt::CaseInsensitive);
    QRegExp testRegExp(REGEXP_ATTR_TEST, Qt::CaseInsensitive);

    // Check dynamic keys first
    if (testConditionRegExp.exactMatch(lInternalKey.trimmed()))
    {
        QString conditionName = testConditionRegExp.capturedTexts().at(1);
        value = mTestConditions[conditionName];
    }
    else if (testRegExp.exactMatch(lInternalKey.trimmed()))
    {
        QString testAttribute = testRegExp.capturedTexts().at(1);
        value = mTestAttributes[testAttribute];
    }
    else if(mAttributes.contains(lInternalKey))
    {
        if (lInternalKey.compare("FileName", Qt::CaseInsensitive) == 0)
        {
            QFileInfo cFileInfo(mAttributes[lInternalKey].toString());
            value = cFileInfo.fileName();
        }
        else
            value = mAttributes[lInternalKey];
    }

    /*
    else if(strDbKey.startsWith("DataOrigin:", Qt::CaseInsensitive))
        value = strDataOrigin;
    else if(strDbKey.startsWith("Family:", Qt::CaseInsensitive))
        value = strFamilyID;
    else if(strDbKey.startsWith("Lot:", Qt::CaseInsensitive))
        value = strLot;
    else if(strDbKey.startsWith("TrackingLot:", Qt::CaseInsensitive))
        value = strTrackingLot;
    else if(strDbKey.startsWith("SubconLot:", Qt::CaseInsensitive))
        value = strSubconLot;
    else if(strDbKey.startsWith("Product:", Qt::CaseInsensitive))
        value = strProductID;
    else if(strDbKey.startsWith("SubLot:", Qt::CaseInsensitive))
        value = strSubLot;
    else if(strDbKey.startsWith("Facility:", Qt::CaseInsensitive))
        value = strFacilityID;
    else if(strDbKey.startsWith("Floor:", Qt::CaseInsensitive))
        value = strFloorID;
    else if(strDbKey.startsWith("FrequencyStep:", Qt::CaseInsensitive))
        value = strFrequencyStep;
    else if(strDbKey.startsWith("PackageType:", Qt::CaseInsensitive))
        value = strPackageType;
    else if(strDbKey.startsWith("Process:", Qt::CaseInsensitive))
        value = strProcessID;
    else if(strDbKey.startsWith("LoadBoard:", Qt::CaseInsensitive))
        value = strLoadBoardName;
    else if(strDbKey.startsWith("LoadBoardType:", Qt::CaseInsensitive))
        value = strLoadBoardType;
    else if(strDbKey.startsWith("ProberName:", Qt::CaseInsensitive))
        value = strProberName;
    else if(strDbKey.startsWith("ProberType:", Qt::CaseInsensitive))
        value = strProberType;
    else if(strDbKey.startsWith("ExtraName:", Qt::CaseInsensitive))
        value = strExtraName;
    else if(strDbKey.startsWith("ExtraType:", Qt::CaseInsensitive))
        value = strExtraType;
    else if(strDbKey.startsWith("ProgramName:", Qt::CaseInsensitive))
        value = strJobName;
    else if(strDbKey.startsWith("ProgramRevision:", Qt::CaseInsensitive))
        value = strJobRev;
    else if(strDbKey.startsWith("Temperature:", Qt::CaseInsensitive))
        value = strTemperature;
    else if(strDbKey.startsWith("TestingCode:", Qt::CaseInsensitive))
        value = strTestingCode;
    else if(strDbKey.startsWith("TesterName:", Qt::CaseInsensitive))
        value = strTesterName;
    else if(strDbKey.startsWith("TesterType:", Qt::CaseInsensitive))
        value = strTesterType;
    else if(strDbKey.startsWith("Wafer:", Qt::CaseInsensitive))
        value = strWaferID;
    else if(strDbKey.startsWith("WaferNb:", Qt::CaseInsensitive))
        value = strWaferNb;
    else if(strDbKey.startsWith("EtestSiteConfig:", Qt::CaseInsensitive))
        value = strEtestSiteConfig;
    else if(strDbKey.startsWith("WaferNotch:", Qt::CaseInsensitive))
        value = QString(cWaferNotch);
    else if(strDbKey.startsWith("BurninTime:", Qt::CaseInsensitive))
        value = strBurninTime;
    else if(strDbKey.startsWith("Operator:", Qt::CaseInsensitive))
        value = strOperator;
    else if(strDbKey.startsWith("DateCode:", Qt::CaseInsensitive))
        value = strDateCode;
    else if(strDbKey.startsWith("SpecName:", Qt::CaseInsensitive))
        value = strSpecificationName;
    else if(strDbKey.startsWith("DesignRevision:", Qt::CaseInsensitive))
        value = strDesignRevision;
    else if(strDbKey.startsWith("User1:", Qt::CaseInsensitive))
        value = strUser1;
    else if(strDbKey.startsWith("User2:", Qt::CaseInsensitive))
        value = strUser2;
    else if(strDbKey.startsWith("User3:", Qt::CaseInsensitive))
        value = strUser3;
    else if(strDbKey.startsWith("User4:", Qt::CaseInsensitive))
        value = strUser4;
    else if(strDbKey.startsWith("User5:", Qt::CaseInsensitive))
        value = strUser5;
    else if(strDbKey.startsWith("RetestIndex:", Qt::CaseInsensitive))
        value = QString::number(lRetestIndex);
    else if(strDbKey.startsWith("RetestBinList:", Qt::CaseInsensitive))
        value = strRetestBinList;
    else if(strDbKey.startsWith("Ft_YieldConsolidation_MissingPartsBin:", Qt::CaseInsensitive))
        value = QString::number(lRetestMissingPartsBin);
    else if(strDbKey.startsWith("Wt_YieldConsolidation_Rule:", Qt::CaseInsensitive))
        value = strRetestRule;
    else if(strDbKey.startsWith("DataType:", Qt::CaseInsensitive))
        value = strDataType;
    else if(strDbKey.startsWith("ProdData:", Qt::CaseInsensitive))
        value = bProdData ? "Y" : "N";
    else if(strDbKey.startsWith("Station:", Qt::CaseInsensitive))
        value = QString::number(iStation);
    else if(strDbKey.startsWith("SetupTime:", Qt::CaseInsensitive))
        value = QString::number(t_SetupTime);
    else if(strDbKey.startsWith("StartTime:", Qt::CaseInsensitive))
        value = QString::number(t_StartTime);
    else if(strDbKey.startsWith("FinishTime:", Qt::CaseInsensitive))
        value = QString::number(t_FinishTime);
    else if(strDbKey.startsWith("User1Label:", Qt::CaseInsensitive))
        value = strUser1Label;
    else if(strDbKey.startsWith("User2Label:", Qt::CaseInsensitive))
        value = strUser2Label;
    else if(strDbKey.startsWith("User3Label:", Qt::CaseInsensitive))
        value = strUser3Label;
    else if(strDbKey.startsWith("User4Label:", Qt::CaseInsensitive))
        value = strUser4Label;
    else if(strDbKey.startsWith("User5Label:", Qt::CaseInsensitive))
        value = strUser5Label;
    else if(strDbKey.startsWith("DibType:", Qt::CaseInsensitive))
        value = strDibType;
    else if(strDbKey.startsWith("DibName:", Qt::CaseInsensitive))
        value = strDibName;
    else if(strDbKey.startsWith("FtpServer:", Qt::CaseInsensitive))
        value = strFtpServer;
    else if(strDbKey.startsWith("FtpPort:", Qt::CaseInsensitive))
        value = QString::number(uiFtpPort);
    else if(strDbKey.startsWith("FtpUser:", Qt::CaseInsensitive))
        value = strFtpUser;
    else if(strDbKey.startsWith("FtpPassword:", Qt::CaseInsensitive))
        value = strFtpPassword;
    else if(strDbKey.startsWith("FtpPath:", Qt::CaseInsensitive))
        value = strFtpPath;
    else if(strDbKey.startsWith("MovePath:", Qt::CaseInsensitive))
        value = strMovePath;
    else if(strDbKey.startsWith("WeekNb:", Qt::CaseInsensitive))
        value = QString::number(uiWeekNb);
    else if(strDbKey.startsWith("Year:", Qt::CaseInsensitive))
        value = QString::number(uiYear);
    else if(strDbKey.startsWith("GrossDie:", Qt::CaseInsensitive))
        value = QString::number(uiGrossDie);
    else if(strDbKey.startsWith("IgnoreSummary:", Qt::CaseInsensitive))
        value = bIgnoreSummary ? "Y" : "N";
    else if(strDbKey.startsWith("UserText:", Qt::CaseInsensitive))
        value = strUserText;
    else if(strDbKey.startsWith("ForceTestingStage:", Qt::CaseInsensitive))
        value = strForceTestingStage;
    else if(strDbKey.startsWith("RejectIfNbPartsLessThan:", Qt::CaseInsensitive))
        value = QString::number(RejectIfNbPartsLessThan);
    else if(strDbKey.startsWith("RejectIfNbPartsPercentOfGDPWLessThan:", Qt::CaseInsensitive))
        value = QString::number(RejectIfNbPartsPercentOfGDPWLessThan);
    else if(strDbKey.startsWith("RejectIfPassHardBinNotInList:", Qt::CaseInsensitive))
        value = RejectIfPassHardBinNotInList;
    else if(strDbKey.startsWith("IgnoreResultsIfNbPartsMoreThan:", Qt::CaseInsensitive))
        value = QString::number(IgnoreResultsIfNbPartsMoreThan);
    // Special case for FileName used into config file, just return the fileName()
    else if(strDbKey.startsWith("FileName:", Qt::CaseInsensitive))
    {
        QFileInfo cFileInfo(mAttributes["filename"].toString());
        value = cFileInfo.fileName();
    }
    else if (mAttributes.contains(key.toLower()))
        value = mAttributes[key.toLower()].toString();
        */
    else
        return false;
    return true;
}


QStringList DatakeysContent::allowedStaticDbKeys() const
{
    return DataKeysDefinitionLoader::GetInstance().GetStaticKeys();
}

DataKeysDefinitionLoader &DatakeysContent::DataKeysDefinition()
{
    return DataKeysDefinitionLoader::GetInstance();
}

const QMap<QString, QString>& DatakeysContent::testConditions() const
{
    return mTestConditions;
}

const QMap<QString, QString>& DatakeysContent::testAttributes() const
{
    return mTestAttributes;
}

void DatakeysContent::Clear(bool fromInputFile/*=false*/)
{

    mReadOnly = false;

    for(int i=0; i<GQTL_STDF::Stdf_Record::Rec_COUNT; i++)
        StdfRecordsCount[i]= 0;

    ClearAttributes(fromInputFile);

    ClearDynamicDbKeys();
}

void DatakeysContent::ClearAttributes(bool fromInputFile/*=false*/)
{
    mAttributes.clear();
    mAttributesOverloaded.clear();

    QStringList lKeys = GS::QtLib::DataKeysDefinitionLoader::GetInstance().GetStaticKeys();
    for(int lIdx=0; lIdx<lKeys.count(); ++lIdx)
    {
        GS::QtLib::DataKeysData lData = GS::QtLib::DataKeysDefinitionLoader::GetInstance().GetDataKeysData(lKeys[lIdx]);

        // If static keys has a default value, set it now.
        if (!lData.GetDefaultValue().isNull())
        {
            SetDbKeyContent(lData.GetKeyName(), lData.GetDefaultValue(), fromInputFile);
        }
    }
}

void DatakeysContent::ClearDynamicDbKeys()
{
    ClearConditions();
    mTestAttributes.clear();
}

void DatakeysContent::ClearConditions()
{
    mTestConditions.clear();
}

bool DatakeysContent::SetInternal(const QString &key, QVariant value)
{
    GSLOG(7, QString("Set %1 to %2").arg(key).arg(value.toString()).toLatin1().data());
    mAttributes.insert(key.toLower(),value);
    // Initialize through Gex process
    // Internal properties must be ReadOnly for other process
    mReadOnly = true;
    return true;
}

bool DatakeysContent::Set(const QString &key, QVariant value, bool fromInputFile/*=false*/)
{
    GSLOG(7, QString("Set %1 to %2").arg(key.toLower()).arg(value.toString()).toLatin1().data());
    return SetDbKeyContent(key.toLower(), value, fromInputFile);
}

// Check if the Attibute was overloaded by the user
bool DatakeysContent::IsOverloaded(const QString &key)
{
    return mAttributesOverloaded.contains(key.toLower());
}


void DatakeysContent::CleanKeyData(const QString &key, const QStringList &chars )
{
    // only clean STRING keys
    if(DataKeysDefinition().GetDataKeysData(key).GetDataType() == "STRING")
    {
        QString lData = Get(key).toString();
        for(int lIdx=0; lIdx<chars.count(); ++lIdx)
        {
            lData.remove(chars[lIdx]);
        }
        // Add true to not update to TRUE the OverLoaded field
        Set(key,lData,true);
    }
}

bool DatakeysContent::SetDbKeyContent(const QString &entryKey,
                                      const QVariant &inValue,
                                      bool fromInputFile /*=false*/)
{
    bool bStatus = true;
    QString lInternalKey = entryKey.toLower();

    // To be compatible with RetestPhase and TestInsertion
    if(lInternalKey == "retestphase")
    {
        if(inValue.toString().isEmpty())
            return true;

        // Check if TestInsertion was already updated
        QString lValue;
        GetDbKeyContent(QString("TestInsertion"),lValue);
        if((lValue.isEmpty()) || (lValue=="default"))
            lInternalKey = "testinsertion";
        else
            return true;
    }

    QVariant value = inValue;
    QString lValidValue = inValue.toString();
    if(lValidValue.isNull() || lValidValue.isEmpty() || (lValidValue[0].toLatin1() == 0))
        value.clear();


    // Check which value is in field to overload
    QRegExp testConditionRegExp(REGEXP_ATTR_TEST_CONDITION, Qt::CaseInsensitive);
    QRegExp testRegExp(REGEXP_ATTR_TEST, Qt::CaseInsensitive);

    // Check dynamic key first.
    if (testConditionRegExp.exactMatch(lInternalKey.trimmed()))
    {
        QString conditionName = testConditionRegExp.capturedTexts().at(1);

        mTestConditions[conditionName] = value.toString();
        return true;
    }
    else if (testRegExp.exactMatch(lInternalKey.trimmed()))
    {
        QString testAttribute = testRegExp.capturedTexts().at(1);

        mTestAttributes[testAttribute] = value.toString();
        return true;
    }


    QString lDataType = "";
    // Check the type if it is specified
    if(allowedStaticDbKeys().contains(lInternalKey))
        lDataType = DataKeysDefinition().GetDataKeysData(lInternalKey).GetDataType().toUpper();

    if(lDataType == "NUMBER")
        value = value.toInt(&bStatus);
    else if(lDataType == "CHAR")
        bStatus = (value.toString().length()<=1);
    else if(lDataType.contains("|"))
    {
        bStatus = lDataType.split("|").contains(value.toString().toUpper());
        if(bStatus)
            value = value.toString().toUpper();
    }
    else if(lDataType == "TIMESTAMP")
    {
        unsigned int time = value.toLongLong();
        // From internal initialization
        if(fromInputFile &&
                ((time<=0) || (value<=0)))
        {
            // Allow timestamp 0 for initialization
            bStatus = true;
        }
        else
        {
            // Check if the timestamp is valid
            // Check if DateTime is valid
            QDateTime	clCurrentDateTime = QDateTime::currentDateTime().addDays(1);
            if((time <= 0) || (time > clCurrentDateTime.toTime_t()))
                bStatus = false;
        }
    }
    else if(lDataType == "BOOLEAN")
    {
        bStatus = QString("N|Y|NO|YES|FALSE|TRUE|0|1").split("|").contains(value.toString().toUpper());
        if(bStatus)
        {
            if(QString("Y|YES|TRUE|1").split("|").contains(value.toString().toUpper()))
                value = "TRUE";
            else
                value = "FALSE";
        }
    }
    else if(lDataType == "STRING")
    {
        if(!value.toString().isEmpty())
            value = value.toString().trimmed();
    }
    else if(lDataType == "DEPRECATED")
    {
        bStatus = false;
    }
    else if(!fromInputFile && mReadOnly && (lDataType == "READONLY"))
        bStatus = false;

    if(!bStatus)
    {
        QString lError = QString("Invalid value '%1' used for key '%2' - Value type is %3")
                .arg(value.toString()).arg(entryKey).arg(lDataType);
        QString lDataDesc = "";
        // Check the type if it is specified
        if(allowedStaticDbKeys().contains(lInternalKey))
            lDataDesc = " ("+DataKeysDefinition().GetDataKeysData(lInternalKey).GetDescription()+")";
        lError += lDataDesc;

        Set("Status",1);
        Set("Error", lError);
        GSLOG(SYSLOG_SEV_ERROR, lError.toLatin1().data());
        return false;
    }

    // HTH: Not needed anymore
//    bool lNewValue = false;
//    if(mAttributes.contains(lInternalKey) && (mAttributes[lInternalKey] != value))
//        lNewValue = true;

    mAttributes[lInternalKey] = value;

    // If not coming from input file, make sure we set the "...Overloaded" attributes.
    // Even if the key was overloaded with the default value
    if(!fromInputFile) // && lNewValue)
    {
        if(!mAttributesOverloaded.contains(lInternalKey))
            mAttributesOverloaded.append(lInternalKey);
    }

    return true;
}

QString DatakeysContent::defaultConfigFileName()
{
    return QString(DEFAULT_CONFIG_FILE_NAME);
}

bool DatakeysContent::isDefaultConfigFile(const QString& fileName)
{
    return fileName.trimmed() == defaultConfigFileName();
}

DatakeysContent::DbKeysType DatakeysContent::dbKeysType(const QString &dbKeyName)
{
    QString dbKey = dbKeyName.simplified();

    QRegExp dynamicTestKey(REGEXP_TEST_KEY, Qt::CaseInsensitive);
    QRegExp dynamicTestConditionKey(REGEXP_TEST_CONDITION_KEY, Qt::CaseInsensitive);

    if (dynamicTestKey.exactMatch(dbKey)
            || dynamicTestConditionKey.exactMatch(dbKey))
    {
        return DbKeyDynamic;
    }

    if (DataKeysDefinitionLoader::GetInstance().GetStaticKeys().contains(dbKey, Qt::CaseInsensitive))
        return DbKeyStatic;

    return DbKeyUnknown;
}

bool DatakeysContent::isValidDynamicKeys(const QString &lDynamicKey, QString& lMessage)
{
    QString lDbKey = lDynamicKey.simplified();
    QRegExp lTestCondition(REGEXP_TEST_CONDITION_KEY, Qt::CaseInsensitive);
    QRegExp lTest(REGEXP_TEST_KEY, Qt::CaseInsensitive);

    // Check dynamic key first.
    if (lTestCondition.exactMatch(lDbKey))
    {
        QRegExp lTestConditionAttr(REGEXP_ATTR_TEST_CONDITION, Qt::CaseInsensitive);

        if (!lTestConditionAttr.exactMatch(lDbKey) || lTestConditionAttr.capturedTexts().size()<1)
        {
            lMessage = QString("'%1' is not a valid attribute for testCondition dynamic key").arg(lDbKey);
            return false;
        }

        // GCORE-994: reject if space and ',' in test cond name
        // Let s capture test cond name
        QString lTestCondName=lTestConditionAttr.capturedTexts().at(1);
        GSLOG(SYSLOG_SEV_NOTICE, QString("Test condition name: '%1'").arg(lTestCondName).toLatin1().data() );
        if (lTestCondName.contains(' ') || lTestCondName.contains(','))
        {
            lMessage = QString("Not allowed to include a space or comma within a test condition name");
            return false;
        }
        return true;
    }
    else if (lTest.exactMatch(lDbKey))
    {

        QRegExp lTestAttr(REGEXP_ATTR_TEST, Qt::CaseInsensitive);

        if (lTestAttr.exactMatch(lDbKey))
            return true;

        lMessage = QString("%1 is not a valid attribute of test dynamic key (allowed values are 'name' or 'number')")
                .arg(lTest.capturedTexts().at(1));

        return false;
    }

    lMessage = QString("%1 is not a dynamic db key").arg(lDbKey);

    return false;
}

// gcore-1666: add bKeepEmptyParts
QMap<QString, QVariant> DatakeysContent::toMap(bool bKeepEmptyParts)
{
    QMap<QString, QVariant> mapSummary;
    mapSummary = mAttributes;

    // Remove empty keys?
    if(!bKeepEmptyParts)
    {

        foreach(const QString &Key, mapSummary.keys())
        {
            // Remove empty Key
            if(mapSummary[Key].isNull()
                    || mapSummary[Key].toString().simplified().remove(" ").isEmpty())
                mapSummary.remove(Key);
            else
            {
                // Check if default value
                if(allowedStaticDbKeys().contains(Key)
                        && (mapSummary[Key] == DataKeysDefinition().GetDataKeysData(Key).GetDefaultValue().toString()))
                    mapSummary.remove(Key);
            }
        }
    }
    else
    {
        QStringList lKeys = GS::QtLib::DataKeysDefinitionLoader::GetInstance().GetStaticKeys();
        for(int lIdx=0; lIdx<lKeys.count(); ++lIdx)
        {
            // If static keys is not set yet, add it with an empty value.
            if (!mapSummary.contains(lKeys[lIdx]))
                mapSummary.insert(lKeys[lIdx], "");
        }
    }

    return mapSummary;
}

QStringList DatakeysContent::toList()
{
    QStringList listSummary;
    QMap<QString, QVariant> mapSummary;
    mapSummary = toMap();

    foreach(const QString &Key, mapSummary.keys())
        listSummary << Key+"="+mapSummary[Key].toString();

    return listSummary;
}

} //END namespace QtLib
} //END namespace GS
