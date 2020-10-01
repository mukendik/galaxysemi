#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include "gqtl_log.h"
#include "server_file_descriptorIO_factory.h"
#include "json_server_file_descriptorIO.h"

using namespace GS::LPPlugin;


JSONServerFileDescriptorIO::JSONServerFileDescriptorIO():ServerFileDescriptorIO() {}
JSONServerFileDescriptorIO::~JSONServerFileDescriptorIO()
{
    foreach( const ProviderDescriptor* lProviderDescriptor, mProviders) {
        delete lProviderDescriptor;
    }
    mProviders.clear();
}


//!--------------------------------!//
//!             loadFile           !//
//!--------------------------------!//
bool  JSONServerFileDescriptorIO::LoadFile (const QString &fileName, QString forceProvider)
{
    if(mLoaded)
        return mLoaded;

    if(!QFile::exists(fileName)) {
        GSLOG(SYSLOG_SEV_ERROR, QString("File not created %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QFile lFile(fileName);

    if(!lFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        GSLOG(SYSLOG_SEV_ERROR, QString("Can not open %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QString lSettings = lFile.readAll();
    lFile.close();

    mJsonDocument = QJsonDocument::fromJson(lSettings.toUtf8());

    if(mJsonDocument.isNull()) {
        GSLOG(SYSLOG_SEV_ERROR, QString("Error while loading json file %1. Wrong format").arg(fileName).toLatin1().constData());
        return false;
    }

    if(!forceProvider.isEmpty())
        mLastProviderConfigured = forceProvider;

    QJsonObject vObjectRoot = mJsonDocument.object();

    QStringList lListKeys = vObjectRoot.keys();

   // mLastProviderConfigured = vObjectRoot.value(c_lastLpUsed).toString();
   // mLastLicenseTypeUsed    = strToLicenseType(vObjectRoot.value("c_lastLtUsed").toString());

    foreach(QString lKey, lListKeys) {
        if(!lKey.compare(c_version, Qt::CaseInsensitive) ) {
           mVersion = vObjectRoot.value(lKey).toString();
        }
        /*else if(!lKey.compare(c_lastLpUsed, Qt::CaseInsensitive) ) {
           mLastProviderConfigured = vObjectRoot.value(lKey).toString();
        }*/
        else if (!lKey.compare(c_lpConfig, Qt::CaseInsensitive) ) {
            QJsonObject lLpConfig = vObjectRoot.value(lKey).toObject();
            if( LoadProviderDescriptor(lLpConfig) == false ) {
              GSLOG(SYSLOG_SEV_ERROR, QString("Error while loading json file %1. Wrong format").arg(fileName).toLatin1().constData());
              return false;
            }
        }
       /* else if(!lKey.compare("c_lastLtUsed", Qt::CaseInsensitive) ) {
            mLastLicenseTypeUsed = strToLicenseType(vObjectRoot.value(lKey).toString());
        }*/
        else {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Unexpected key found %1. Loading json file $2 failed.").arg(lKey).arg(fileName).toLatin1().constData());
            return false;
        }
    }

    mLoaded = true;

    return true;

}

//!--------------------------------------!//
//!         loadProviderDescriptor       !//
//!--------------------------------------!//
bool JSONServerFileDescriptorIO::LoadProviderDescriptor(QJsonObject &jsonObject) {
    QStringList lListKeys   = jsonObject.keys();

    foreach(QString lKey, lListKeys) {
        if(!lKey.compare("fnp_lp", Qt::CaseInsensitive) || !lKey.compare("gs_lp", Qt::CaseInsensitive)) {

            QJsonObject lProviderTypeObject  = jsonObject.value(lKey).toObject();
            QStringList lLicenseTypeList    = lProviderTypeObject.keys();

            QScopedPointer<ProviderDescriptor> lProviderDescriptor(new ProviderDescriptor());

            foreach(QString lLicenseTypeElement, lLicenseTypeList) {

                if(!lLicenseTypeElement.compare("floating", Qt::CaseInsensitive) || (/* !lKey.compare("gs_lp", Qt::CaseInsensitive) &&*/ !lLicenseTypeElement.compare("standalone", Qt::CaseInsensitive) || !lLicenseTypeElement.compare("evaluation", Qt::CaseInsensitive)) ) {
                   QScopedPointer<LicenseTypeDescriptor> lLicenseTypeDescriptor(new LicenseTypeDescriptor(StrToLicenseType(lLicenseTypeElement)));
                   QJsonObject lServersDescriptorObject =  lProviderTypeObject.value(lLicenseTypeElement).toObject();
                   if(LoadServerDescriptor(lServersDescriptorObject, lLicenseTypeDescriptor.data()) == false) {
                       return false;
                   }
                   lProviderDescriptor->AddLicenseTypeDescriptor(StrToLicenseType(lLicenseTypeElement), lLicenseTypeDescriptor.take());
                }
                else if(!lLicenseTypeElement.compare(c_lastLtUsed, Qt::CaseInsensitive) ) {
                    if(mLastProviderConfigured == lKey) {
                        mLastLicenseTypeUsed = StrToLicenseType(lProviderTypeObject.value(lLicenseTypeElement).toString());
                    }
                    //QString llAstLp = lProviderTypeObject.value(lLicenseTypeElement).toString();
                    lProviderDescriptor->RetLastLp(StrToLicenseType(lProviderTypeObject.value(lLicenseTypeElement).toString()));
                 }
                else {
                    GSLOG(SYSLOG_SEV_ERROR, QString("Unexpected json key found : <%1>.").arg(lLicenseTypeElement).toLatin1().constData());
                    return false;
                }
            }
            mProviders.insert(lKey, lProviderDescriptor.take());
        }
        else {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Unexpected json key found : <%1>.").arg(lKey).toLatin1().constData());
            return false;
        }
    }

    BuildServerDescriptorString();
    return true;
}

//!------------------------------------!//
//!         loadServerDescriptor       !//
//!------------------------------------!//
bool JSONServerFileDescriptorIO::LoadServerDescriptor(QJsonObject &jsonObject, LicenseTypeDescriptor* licenseTypeDescriptor)
{
    QStringList lListKeys   = jsonObject.keys();

    ServerDescriptor lServerDescriptor;
    lServerDescriptor.mOrder    = 1;
    bool lToInsert              = true;
    foreach(QString lKey, lListKeys) {

        QJsonValue lJsonValue = jsonObject.value(lKey);
        if(!lKey.compare(c_connectionStatus, Qt::CaseInsensitive)) {
            licenseTypeDescriptor->SetConnectionStatus(jsonObject.value(lKey).toInt());
            jsonObject.insert(c_connectionStatus, 0);
        }
        else if(!lKey.compare("label", Qt::CaseInsensitive)) {
            licenseTypeDescriptor->SetLabel(jsonObject.value(lKey).toString());
        }
        else if(!lKey.compare(c_servers, Qt::CaseInsensitive)) {
            QJsonArray lArray = lJsonValue.toArray();
            foreach(QJsonValue lIter, lArray) {
                QJsonObject lServerDescriptorObject = lIter.toObject();
                LoadServerDescriptor(lServerDescriptorObject, licenseTypeDescriptor);
            }
            lToInsert = false;
        }
        else if(!lKey.compare(c_port, Qt::CaseInsensitive)) {
            lServerDescriptor.mSocketPort   = lJsonValue.toInt();
        }
        else if(!lKey.compare(c_host, Qt::CaseInsensitive)) {
            lServerDescriptor.mServerIP     = lJsonValue.toString();
            lServerDescriptor.mServerName   = lJsonValue.toString();
        }
        else if(!lKey.compare("order", Qt::CaseInsensitive)) {
            lServerDescriptor.mOrder        = lJsonValue.toInt();
        }
        else if(!lKey.compare(c_userName, Qt::CaseInsensitive)) {
            lServerDescriptor.mUserName     = lJsonValue.toString();
        }
        else if(!lKey.compare(c_productKey, Qt::CaseInsensitive)) {
            lServerDescriptor.mProductKey   = lJsonValue.toString();
        }
        else {
            GSLOG(SYSLOG_SEV_ERROR, QString("Unexpected json key found : <%1>.").arg(lKey).toLatin1().constData());
            return false;
        }
    }

    if(lToInsert) {
        licenseTypeDescriptor->AddServerDescriptor(lServerDescriptor);
        licenseTypeDescriptor->BuidLabelGUI(lServerDescriptor);

    }

    return true;
}

//!----------------------------------!//
//!     checkValidityOfRequestedData !//
//!----------------------------------!//
bool JSONServerFileDescriptorIO::CheckValidityOfRequestedData(QString &providerType, T_LicenseType &licenseType) const
{
    if(providerType.isEmpty() ) {
        providerType   = mLastProviderConfigured;
        licenseType    = mLastLicenseTypeUsed;
    }

    if(licenseType == undef)
        licenseType = mLastLicenseTypeUsed;

    if(mProviders.contains(providerType))
        return mProviders[providerType]->GetLicenseTypeDescriptors().contains(licenseType);

    return false;
}


//!------------------------------------!//
//!     buildServerDescriptorString    !//
//!------------------------------------!//
void JSONServerFileDescriptorIO::BuildServerDescriptorString()
{
    QMap<QString, const ProviderDescriptor* >::iterator lIterBegin(mProviders.begin()), lIterEnd(mProviders.end()) ;

    for(;lIterBegin != lIterEnd; ++lIterBegin) {

        QString lProvider = lIterBegin.key();

        const QMap<T_LicenseType, const LicenseTypeDescriptor*>& lLicenseTypeDescriptor = lIterBegin.value()->GetLicenseTypeDescriptors() ;

        QMap<T_LicenseType, const LicenseTypeDescriptor*>::const_iterator lIterBeginLicense(lLicenseTypeDescriptor.begin()), lIterEndLicense(lLicenseTypeDescriptor.end()) ;

        for(;lIterBeginLicense != lIterEndLicense; ++lIterBeginLicense){
           QString lServerDescriptorString;
           lServerDescriptorString.append(LicenseTypeToStr(lIterBeginLicense.key()));
           lServerDescriptorString.append("|");
           lServerDescriptorString.append(BuildDefaultLicenseTypeLabel(lIterBeginLicense.key()));
           lServerDescriptorString.append("|");
           lServerDescriptorString.append(lIterBeginLicense.value()->GetLabelGUI());
           mServerDescriptorString.insert(lIterBegin.key(), lServerDescriptorString);

           if(lProvider == mLastProviderConfigured && lIterBeginLicense.key() == mLastLicenseTypeUsed) {
               mLastChoice = lServerDescriptorString;
           }
        }
   }
}

//!----------------------------------!//
//!         getListSocketPorts       !//
//!----------------------------------!//
const QVector<int>&   JSONServerFileDescriptorIO::GetListSocketPorts  (QString providerType, T_LicenseType licenseType) const
{
    if(CheckValidityOfRequestedData(providerType, licenseType))
        return mProviders[providerType]->GetLicenseTypeDescriptors()[licenseType]->GetListSocketPorts();
    else
        return GetEmptyServerPorts();
}

//!-------------------------------!//
//!         getListServerIP       !//
//!-------------------------------!//
const QVector<QString>&   JSONServerFileDescriptorIO::GetListServerIP( QString providerType, T_LicenseType licenseType) const
{
    if(CheckValidityOfRequestedData(providerType, licenseType))
        return mProviders[providerType]->GetLicenseTypeDescriptors()[licenseType]->GetListServerIP();
    else
      return GetEmptyServerIPs();
}

//!-----------------------------------!//
//!         getConnectionStatus       !//
//!-----------------------------------!//
bool  JSONServerFileDescriptorIO::GetConnectionStatus( QString providerType, T_LicenseType licenseType) const
{
    if(CheckValidityOfRequestedData(providerType, licenseType))
        return mProviders[providerType]->GetLicenseTypeDescriptors()[licenseType]->GetConnectionStatus();
    else
        return false;
}

//!-----------------------------------!//
//!         getConnectionStatus       !//
//!-----------------------------------!//
const QString&   JSONServerFileDescriptorIO::GetUserName(QString providerType, T_LicenseType licenseType) const
{
    if(CheckValidityOfRequestedData(providerType, licenseType) && licenseType == standalone)
        return mProviders[providerType]->GetLicenseTypeDescriptors()[licenseType]->mServerDescriptors.begin()->mUserName;
    else
        return GetEmptyString();
}
//!-----------------------------------!//
//!         getConnectionStatus       !//
//!-----------------------------------!//
const QString&    JSONServerFileDescriptorIO::GetProductKey(QString providerType, T_LicenseType licenseType) const
{
    if(CheckValidityOfRequestedData(providerType, licenseType) && licenseType == standalone)
        return mProviders[providerType]->GetLicenseTypeDescriptors()[licenseType]->mServerDescriptors.begin()->mProductKey;
    else
        return GetEmptyString();
}


//!------------------------!//
//!         getLabel       !//
//!------------------------!//
/*const QString&   JSONServerFileDescriptorIO::getLabel(QString providerType, T_LicenseType licenseType) const
{
    if(checkValidityOfRequestedData(providerType, licenseType))
        return mProviders[providerType]->getLicenseTypeDescriptors()[licenseType]->getLabel();
    else
        return getEmptyLabel();
}*/

//!--------------------------------------!//
//!         getLastUsedLicenseType       !//
//!--------------------------------------!//
T_LicenseType   JSONServerFileDescriptorIO::GetLastUsedLicenseType(QString providerType) const {

    if(providerType.isEmpty() ) {
        providerType   = mLastProviderConfigured;
    }

    if(mProviders.contains(providerType))
        return mProviders[providerType]->GetLastLp();

    return undef;
}


//!------------------------!//
//!         saveFile       !//
//!------------------------!//
bool JSONServerFileDescriptorIO::SaveFile (const QString& providerType, T_LicenseType licenseType, const QVector<QString> &datas1, const QVector<int> &datas2)
{
    UpdateJsondocument(mJsonDocument, providerType, licenseType, datas1, datas2);

    return true;
}

//!----------------------------------!//
//!         updateConnectionStatus   !//
//!----------------------------------!//
/*void   JSONServerFileDescriptorIO::updateConnectionStatus  (bool connectionStatus)
{
    updateConnectionStatus(mJsonDocument, connectionStatus);
}*/

//!----------------------------------!//
//!         updateConnectionStatus   !//
//!----------------------------------!//
void JSONServerFileDescriptorIO::UpdateConnectionStatus(QJsonDocument& lJsonDocument, bool connectionStatus)
{
    QJsonObject vObjectRoot      = lJsonDocument.object();
    QString     lLastProvider    = mLastProviderConfigured;

    QJsonObject lObjectLPConfig = vObjectRoot.value(c_lpConfig).toObject();

    QStringList lListProviderKeys   = lObjectLPConfig.keys();
    foreach(QString lProviderStr, lListProviderKeys) {
        QJsonObject lProviderTypeObject = lObjectLPConfig.value(lProviderStr).toObject();
        QString     lLastLicense        = lProviderTypeObject.value(c_lastLtUsed).toString();

        QStringList lListLicenseKeys   = lProviderTypeObject.keys();
        foreach(QString lLicenseTypeStr, lListLicenseKeys) {
            if(StrToLicenseType(lLicenseTypeStr) == floating || StrToLicenseType(lLicenseTypeStr) == standalone || StrToLicenseType(lLicenseTypeStr) == evaluation ) {
                QJsonObject lLicenseTypeObject = lProviderTypeObject.value(lLicenseTypeStr).toObject();
                lLicenseTypeObject.remove(c_connectionStatus);
                if(lProviderStr == lLastProvider && lLicenseTypeStr == lLastLicense) {
                    lLicenseTypeObject.insert(c_connectionStatus, (connectionStatus==true)?1:0 );
                }

                lProviderTypeObject.insert(lLicenseTypeStr, lLicenseTypeObject);
            }
        }
        lObjectLPConfig.insert(lProviderStr, lProviderTypeObject);
    }
    vObjectRoot.insert(c_lpConfig, lObjectLPConfig);
    lJsonDocument.setObject(vObjectRoot);
}

//!-------------------------------!//
//!         updateJsondocument    !//
//!-------------------------------!//
void JSONServerFileDescriptorIO::UpdateJsondocument(QJsonDocument& lJsonDocument, const QString &providerType, T_LicenseType licenseType,const QVector<QString> &datas1, const QVector<int> &datas2)
{

    QJsonObject vObjectRoot = lJsonDocument.object();

    QString lLicenseTypeStr = LicenseTypeToStr(licenseType);
    if(mLastProviderConfigured.isEmpty())
        mLastProviderConfigured = providerType;

    QJsonObject lObjectLPConfig = vObjectRoot.value(c_lpConfig).toObject();

    QJsonObject lProviderRoot = lObjectLPConfig.value(providerType).toObject();

    lProviderRoot.remove(lLicenseTypeStr);
    lProviderRoot.insert(c_lastLtUsed, lLicenseTypeStr);
    // -- License Type
    QJsonObject lLicenseType;

    if(StrToLicenseType(lLicenseTypeStr) == floating) {
        // -- Server Array
        if(datas1.size() > 1) {
            QJsonArray lServerArray;
            //int lOrder = 1;
            int lIter  = 0;
            foreach(QString lServerIP, datas1) {
                QJsonObject lServer;
               // lServer.insert("order", lOrder++);
                lServer.insert(c_host, lServerIP);

                if(lIter >= datas2.size())
                    lServer.insert(c_port, datas2.last());
                else
                    lServer.insert(c_port, datas2[lIter]);

                lServerArray.insert(lIter, lServer);
                ++lIter;
            }
            lLicenseType.insert(c_servers, lServerArray);
        }
        else {
            lLicenseType.insert(c_port, datas2[0]);
            lLicenseType.insert(c_host, datas1[0]);
        }
    }
    else if(StrToLicenseType(lLicenseTypeStr) == standalone && !providerType.compare("gs_lp",Qt::CaseInsensitive)) {
        lLicenseType.insert(c_userName, datas1[0]);
        lLicenseType.insert(c_productKey, datas1[1]);
    }

    // -- Provider
    lProviderRoot.insert(lLicenseTypeStr, lLicenseType);

    lObjectLPConfig.insert(providerType, lProviderRoot);

    vObjectRoot.insert(c_lpConfig, lObjectLPConfig);
    lJsonDocument.setObject(vObjectRoot);
}




LicenseTypeDescriptor::LicenseTypeDescriptor(T_LicenseType licenseType) :mConnectionStatus(0), mLicenseType(licenseType)
{}
LicenseTypeDescriptor::~LicenseTypeDescriptor()
{}

//!-----------------------------------!//
//!         addServerDescriptor       !//
//!-----------------------------------!//
void  LicenseTypeDescriptor::AddServerDescriptor (const ServerDescriptor& serverDescriptor)  {
    mServerDescriptors.append(serverDescriptor);
    mServerIPs.append(serverDescriptor.mServerIP);
    mSocketPorts.append(serverDescriptor.mSocketPort);
}

//!-----------------------------------!//
//!              buidLabelGUI         !//
//!-----------------------------------!//
void LicenseTypeDescriptor::BuidLabelGUI(const ServerDescriptor &serverDescriptor)
{
    if(!mLabelGUI.isEmpty())
        mLabelGUI.append(";");
    if(mLicenseType == floating) {
        mLabelGUI.append(serverDescriptor.mServerIP);
        mLabelGUI.append(":");
        mLabelGUI.append(QString::number(serverDescriptor.mSocketPort));
    }
    else if (mLicenseType == standalone) {
        mLabelGUI.append(serverDescriptor.mUserName);
        mLabelGUI.append("|");
        mLabelGUI.append(serverDescriptor.mProductKey);
    }
}

ProviderDescriptor::ProviderDescriptor() {}
ProviderDescriptor::~ProviderDescriptor() {
    foreach (const LicenseTypeDescriptor* lLicenseTypeDescriptor, mLicenseTypeDescriptors) {
        delete lLicenseTypeDescriptor;
    }
    mLicenseTypeDescriptors.clear();
}

