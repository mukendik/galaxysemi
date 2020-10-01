#include "gqtl_datakeys_definition_loader.h"
#include "gqtl_log.h"

#include <QDomDocument>
#include <QStringList>
#include <QFile>
#include <QMap>
#include <QSet>

namespace GS
{
namespace QtLib
{

const QString cDataKeysXMLFile = ":/gqtl_datakeys/xml/gexdbkeys_definition.xml";

class DataKeysDefinitionLoaderPrivate
{
public:
    DataKeysDefinitionLoaderPrivate();
    ~DataKeysDefinitionLoaderPrivate();
    QString mErrorMessage;
    bool mLoadingStatus;
    QMap<QString, DataKeysData> mDataKeys;

};


DataKeysDefinitionLoader *DataKeysDefinitionLoader::mInstance=0;

DataKeysDefinitionLoader &DataKeysDefinitionLoader::GetInstance()
{
    if(!mInstance)
    {
        mInstance = new DataKeysDefinitionLoader;
    }

    return *mInstance;

}

void DataKeysDefinitionLoader::DestroyInstance()
{
    if(mInstance)
    {
        delete mInstance;
        mInstance = 0;
    }
}

bool DataKeysDefinitionLoader::LoadingPass(QString &errorMessage)
{
    errorMessage = mPrivate->mErrorMessage;
    return mPrivate->mLoadingStatus;

}

QStringList DataKeysDefinitionLoader::GetStaticKeys()
{
    return mPrivate->mDataKeys.keys();
}

DataKeysData DataKeysDefinitionLoader::GetDataKeysData(const QString &key)
{
    QString lInternalKey = key.toLower();
    if(mPrivate->mDataKeys.contains(lInternalKey))
        return mPrivate->mDataKeys[lInternalKey];

    return DataKeysData();

}

void DataKeysDefinitionLoader::initialize()
{
    QDomDocument lDoc;
    QFile lFile(cDataKeysXMLFile);
    if (!lFile.open(QIODevice::ReadOnly))
    {
        mPrivate->mErrorMessage = QString("Unable to open %1 source file").arg(cDataKeysXMLFile);
        mPrivate->mLoadingStatus = false;
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mErrorMessage.toLatin1().constData());
    }
    if (!lDoc.setContent(&lFile))
    {
        mPrivate->mErrorMessage = QString("Unable to load %1 source file").arg(cDataKeysXMLFile);
        mPrivate->mLoadingStatus = false;
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mErrorMessage.toLatin1().constData());
        lFile.close();
    }

    QDomElement docElt = lDoc.documentElement();

    QDomNode lNode = docElt.firstChildElement("key");

    while (!lNode.isNull())
    {
        QDomElement lKeyElt = lNode.toElement(); // try to convert the node to an element.
        // Convert the key name to lower case to be sure that it is stocked in lower case
        QString     lKeyName = lKeyElt.attribute("name", "").toLower();
        QString     lDesc,lStdfField,lType,lDataType;
        QVariant    lDefault;
        // Default data type is STRING
        lDataType = "STRING";
        if (!lKeyName.isEmpty() && !mPrivate->mDataKeys.contains(lKeyName))
        {
            QDomElement lKeyContent = lKeyElt.firstChildElement("description");
            if(!lKeyContent.isNull())
                lDesc = lKeyContent.text();

            lKeyContent = lKeyElt.firstChildElement("stdf_field");
            if(!lKeyContent.isNull())
                lStdfField = lKeyContent.text();

            lKeyContent = lKeyElt.firstChildElement("type");
            if(!lKeyContent.isNull())
                lType = lKeyContent.text();

            lKeyContent = lKeyElt.firstChildElement("datatype");
            if(!lKeyContent.isNull())
                lDataType = lKeyContent.text();

            lKeyContent = lKeyElt.firstChildElement("default");
            if(!lKeyContent.isNull())
                lDefault = lKeyContent.text();

            mPrivate->mDataKeys.insert(lKeyName, DataKeysData(lKeyName, lDesc, lStdfField, lType, lDataType,lDefault));
        }

        lNode = lNode.nextSibling();
    }
    lFile.close();
}

DataKeysDefinitionLoader::DataKeysDefinitionLoader()
{
    mPrivate = new DataKeysDefinitionLoaderPrivate;
    initialize();

}

void DataKeysDefinitionLoader::clear()
{

}

DataKeysDefinitionLoader::~DataKeysDefinitionLoader()
{
    clear();
    delete mPrivate;

}

DataKeysDefinitionLoaderPrivate::DataKeysDefinitionLoaderPrivate()
{
    mLoadingStatus = true;
}
DataKeysDefinitionLoaderPrivate::~DataKeysDefinitionLoaderPrivate()
{
    mDataKeys.clear();
}

}
}
