#include <QDomDocument>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include "gqtl_log.h"
#include "json_server_file_descriptorIO.h"
#include "xml_server_file_descriptorIO.h"

using namespace GS::LPPlugin;

XMLServerFileDescriptorIO::XMLServerFileDescriptorIO():ServerFileDescriptorIO(),mConvertionDone(false)
{
}

XMLServerFileDescriptorIO::~XMLServerFileDescriptorIO()
{
    foreach(XMLServerDescriptor* lServerDescriptor, mServerLicenseDescripors) {
        delete  lServerDescriptor;
    }
    mServerLicenseDescripors.clear();
}

//!------------------------!//
//!         loadFile       !//
//!------------------------!//
bool XMLServerFileDescriptorIO::LoadFile(const QString &fileName, QString forceProvider)
{
    if(mLoaded)
        return mLoaded;

    if(!QFile::exists(fileName)) {
        GSLOG(SYSLOG_SEV_DEBUG, QString("File not created %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QFile lOFile(fileName);
    if(!lOFile.open(QIODevice::ReadWrite)) {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Can not open %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QDomDocument   lXmlDoc;
    QString        lErrorMessage;
    int lErrorLine=-1, lErrorColumn=-1;
    if(!lXmlDoc.setContent(&lOFile, &lErrorMessage, &lErrorLine, &lErrorColumn)) {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(lErrorMessage).arg(lErrorLine).arg(lErrorColumn).toLatin1().constData());
        return false;
    }

    QDomNode lOLPConf = lXmlDoc.namedItem("lp_config");
    if(lOLPConf.isNull()) {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Invalid XML license file format, missing <lp_config> section.").toLatin1().constData());
        return false;
    }

    QDomNodeList lChilds = lOLPConf.childNodes();
    int lSizeList = lChilds.size();
    for(int i = 0; i < lSizeList; ++i) {

        QDomElement lNodeChild = lChilds.at(i).toElement();
        QString lProvider = lNodeChild.tagName();
        if(!lProvider.compare("last_lp_used"))
        {
           mLastProviderConfigured = lNodeChild.attribute("use_lp");
        }
        else {
            XMLServerDescriptor* lServerDescriptor = new XMLServerDescriptor();
            // -- connection status
            if(!lNodeChild.attributes().namedItem("connectionStatus").isNull())
                lServerDescriptor->mConnectionStatus = (lNodeChild.attribute("connectionStatus").toInt() == 0)?false:true;
             else
                lServerDescriptor->mConnectionStatus = false;

            // -- retrieve last_choice
            QString lLastChoiceData         = lNodeChild.attribute("last_choice");
            QStringList lChoiceItems        = lLastChoiceData.split("|");

            lServerDescriptor->mLicenseType  = StrToLicenseType(lChoiceItems[0]);
            //lServerDescriptor->mLabel        = lChoiceItems[1];

            if(lServerDescriptor->mLicenseType == floating) {
                lServerDescriptor->mServerName   = lChoiceItems[2];

                lServerDescriptor->mServerIP.push_back(lChoiceItems[2]);
                lServerDescriptor->mSocketPort.push_back(lChoiceItems[4].toInt());
            }
            else if(lServerDescriptor->mLicenseType == standalone) {
                 if(lChoiceItems.count() >= 3)
                    lServerDescriptor->mUserName   = lChoiceItems[2];
                 if(lChoiceItems.count() >= 5)
                    lServerDescriptor->mProductKey = lChoiceItems[4];
            }

            BuildServerLicenseDescriptorString(lProvider, lChoiceItems, lServerDescriptor, lServerDescriptor->mLicenseType);
            mServerLicenseDescripors.insert(lProvider, lServerDescriptor);
        }
    }

    if(!forceProvider.isEmpty())
        mLastProviderConfigured = forceProvider;

    if(!mLastProviderConfigured.isEmpty()) {
        if(mServerLicenseDescripors.contains(mLastProviderConfigured) ) {
            mLastLicenseTypeUsed    = mServerLicenseDescripors[mLastProviderConfigured]->mLicenseType;
            mLastChoice             = *(mServerDescriptorString.equal_range(mLastProviderConfigured).first);
        }
    }

    mLoaded                 = true;

    return true;
}

//!--------------------------------------------!//
//!         buildServerLicenseDescriptor       !//
//!--------------------------------------------!//
void XMLServerFileDescriptorIO::BuildServerLicenseDescriptorString(const QString provider, const QStringList& lastChoiceItems, XMLServerDescriptor* lServerDescriptor, T_LicenseType licenseType)
{
    QString lServerDescriptorString;
    lServerDescriptorString.append(LicenseTypeToStr(licenseType));
    lServerDescriptorString.append("|");
    lServerDescriptorString.append(BuildDefaultLicenseTypeLabel(licenseType));
    lServerDescriptorString.append("|");

     if(licenseType == floating) {
         lServerDescriptorString.append(lastChoiceItems[2]);
         lServerDescriptorString.append(":");
         lServerDescriptorString.append(lastChoiceItems[4]);
     }
     else if(licenseType == standalone && provider == "gs_lp") {
         lServerDescriptorString.append(lServerDescriptor->mUserName);
         lServerDescriptorString.append("|");
         lServerDescriptorString.append(lServerDescriptor->mProductKey);
     }

    mServerDescriptorString.insert(provider, lServerDescriptorString);
}

//!----------------------------------!//
//!         getListSocketPorts       !//
//!----------------------------------!//
const QVector<int>&  XMLServerFileDescriptorIO::GetListSocketPorts  (QString providerType, T_LicenseType) const {
    if(providerType.isEmpty() )
        providerType = mLastProviderConfigured;

    if(mServerLicenseDescripors.contains(providerType)) {
        return mServerLicenseDescripors[providerType]->mSocketPort;
    }

    return GetEmptyServerPorts();
}

//!-------------------------------!//
//!         getListServerIP       !//
//!-------------------------------!//
const QVector<QString>& XMLServerFileDescriptorIO::GetListServerIP(QString providerType , T_LicenseType) const
{
    if(providerType.isEmpty() )
        providerType = mLastProviderConfigured;

    if(mServerLicenseDescripors.contains(providerType)) {
        return mServerLicenseDescripors[providerType]->mServerIP;
    }

    return GetEmptyServerIPs();
}

//!-----------------------------------!//
//!         getConnectionStatus       !//
//!-----------------------------------!//
bool  XMLServerFileDescriptorIO::GetConnectionStatus  (QString providerType, T_LicenseType) const
{
    if(providerType.isEmpty() )
        providerType = mLastProviderConfigured;

    if(mServerLicenseDescripors.contains(providerType)) {
        return mServerLicenseDescripors[providerType]->mConnectionStatus;
    }

    return false;
}

//!---------------------------!//
//!         getUserName       !//
//!---------------------------!//
const QString&  XMLServerFileDescriptorIO::GetUserName  (QString providerType, T_LicenseType) const
{
    if(providerType.isEmpty() )
        providerType = mLastProviderConfigured;

    if(mServerLicenseDescripors.contains(providerType)) {
        return mServerLicenseDescripors[providerType]->mUserName;
    }

    return GetEmptyString();
}

//!---------------------------!//
//!         getProductKey     !//
//!---------------------------!//
const QString&  XMLServerFileDescriptorIO::GetProductKey  (QString providerType, T_LicenseType) const
{
    if(providerType.isEmpty() )
        providerType = mLastProviderConfigured;

    if(mServerLicenseDescripors.contains(providerType)) {
        return mServerLicenseDescripors[providerType]->mProductKey;
    }

    return GetEmptyString();
}

//!------------------------!//
//!         saveFile       !//
//!------------------------!//
bool XMLServerFileDescriptorIO::SaveFile(const QString& providerType, T_LicenseType licenseType, const QVector<QString> &datas1, const QVector<int> &datas2)
{
    // -- init from current xml if needed
    XmlToJsonDocument(mJsonDocument) ;

    mLastProviderConfigured = providerType;
    // -- update the JSonDocument
    JSONServerFileDescriptorIO lJSONServerFileDescriptorIO;
    lJSONServerFileDescriptorIO.UpdateJsondocument(mJsonDocument, providerType, licenseType, datas1, datas2);

    return true;
}

//!--------------------------------!//
//!      updateConnectionStatus    !//
//!--------------------------------!//
void XMLServerFileDescriptorIO::UpdateConnectionStatus(QJsonDocument& lJsonDocument, bool connectionStatus)
{
    if(!mConvertionDone)
        ConvertToJson();

    JSONServerFileDescriptorIO lJSONServerFileDescriptorIO;
    lJSONServerFileDescriptorIO.mLastProviderConfigured = mLastProviderConfigured;
    lJSONServerFileDescriptorIO.UpdateConnectionStatus(lJsonDocument, connectionStatus);
}

//!------------------------------!//
//!         xmlToJsonDocument    !//
//!------------------------------!//
bool XMLServerFileDescriptorIO::ConvertToJson() {
    // -- init from current xml
    mConvertionDone = XmlToJsonDocument(mJsonDocument) ;
  //  mLastProviderConfigured = lastProvider;

    if(!mConvertionDone || mJsonDocument.isNull()) {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Conversion from xml to JsonDocument failed").toLatin1().constData());
        return false;
    }

    return true;
}

//!------------------------------!//
//!         xmlToJsonDocument    !//
//!------------------------------!//
bool XMLServerFileDescriptorIO::XmlToJsonDocument(QJsonDocument & jsonDocument ) const {

    if(mServerLicenseDescripors.isEmpty())
        return false;

    QJsonObject lObjectRoot;
    lObjectRoot.insert("version", QString("1.0"));
  //  lObjectRoot.insert("last_lp_used", mLastProviderConfigured);
   // lObjectRoot.insert("last_license_type_used", LicenseTypeToStr(mLastLicenseTypeUsed));

    QJsonObject lProvider;
    QMap<QString, XMLServerDescriptor*>::const_iterator lIterBegin(mServerLicenseDescripors.begin()), lIterEnd(mServerLicenseDescripors.end());
    for(; lIterBegin != lIterEnd; ++lIterBegin) {

        QJsonObject lElement;
        if(lIterBegin.value()->mLicenseType == floating) {
            lElement.insert("port", *lIterBegin.value()->mSocketPort.begin() );
            lElement.insert("host", *lIterBegin.value()->mServerIP.begin() );
        }
        else if(lIterBegin.value()->mLicenseType == standalone && !lIterBegin.key().compare("gs_lp")) {
             lElement.insert("user_name", lIterBegin.value()->mUserName);
             lElement.insert("product_key", lIterBegin.value()->mProductKey);
        }

        QJsonObject lLicenseType;
        lLicenseType.insert(LicenseTypeToStr(lIterBegin.value()->mLicenseType), lElement);
        lLicenseType.insert("last_license_type_used", LicenseTypeToStr(mLastLicenseTypeUsed));
        lProvider.insert(lIterBegin.key(), lLicenseType);
    }

    lObjectRoot.insert("lp_config", lProvider);

    jsonDocument.setObject(lObjectRoot);

    return true;
}
