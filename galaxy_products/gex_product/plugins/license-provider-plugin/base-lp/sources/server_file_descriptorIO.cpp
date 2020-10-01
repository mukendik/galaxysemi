#include <QStringList>
#include <QFile>
#include <QList>
#include "gqtl_log.h"
#include "server_file_descriptorIO.h"

using namespace GS::LPPlugin;

ServerFileDescriptorIO::ServerFileDescriptorIO():   mLastProviderConfigured(""),
                                                    mVersion("1.0"),                                                                                          
                                                    mLastChoice(""),
                                                    mLastLicenseTypeUsed(undef),
                                                    mLoaded(false)
{

}

ServerFileDescriptorIO::~ServerFileDescriptorIO  ()
{}

//!--------------------------!//
//!      strToLicenseType    !//
//!--------------------------!//
T_LicenseType  ServerFileDescriptorIO::StrToLicenseType(const QString &licenseTypeStr) const
{
   if(!licenseTypeStr.compare("standalone", Qt::CaseInsensitive))
       return standalone;
   else if(!licenseTypeStr.compare("floating", Qt::CaseInsensitive))
       return floating;
   else if(!licenseTypeStr.compare("evaluation", Qt::CaseInsensitive))
       return evaluation;
   else
       return undef;
}

//!--------------------------!//
//!      licenseTypeToStr    !//
//!--------------------------!//
QString   ServerFileDescriptorIO::LicenseTypeToStr(T_LicenseType licenseType) const
{
    switch(licenseType) {
        case standalone : return "standalone";
        case evaluation : return "evaluation";
        case floating   : return "floating";
        default         : return "";
    }
}

//!--------------------------------------!//
//!      buildDefaultLicenseTypeLabel    !//
//!--------------------------------------!//
QString  ServerFileDescriptorIO::BuildDefaultLicenseTypeLabel(T_LicenseType licenseType) const
{
    switch(licenseType) {
        case standalone : return "Your full name:|Product key ID:";
        case evaluation : return "Evaluation (4 days)";
        case floating   : return "Server(s):Port(s)";
        default         : return "";
    }
}

//!--------------------------------------!//
//!      extractServersPortFromString    !//
//!--------------------------------------!//
bool ServerFileDescriptorIO::ExtractServersPortFromString(const QString &serversPortString, QVector<QString> &servers, QVector<int> &ports )
{
    QChar lSeparator(';');
    if(serversPortString.contains(","))
        lSeparator = ',';

    QStringList lServerPort = serversPortString.split(lSeparator);

    QStringList::iterator lIterBegin(lServerPort.begin()), lIterEnd(lServerPort.end());

    for(; lIterBegin != lIterEnd; ++lIterBegin)
    {
        QStringList lElements = (*lIterBegin).split(":");
        if(lElements.size() == 2) {
            servers.push_back(lElements[0]);
            ports.push_back(lElements[1].toInt());
        }
    }

    return (servers.size() > 0);
}


 bool  ServerFileDescriptorIO::ConvertAndSaveToJsonFormat(const QString &fileName/*, const QString& lastProvider*/)
 {
     ConvertToJson();
     QFile lFile(fileName);
     lFile.open(QIODevice::WriteOnly | QIODevice::Text);
     lFile.write(mJsonDocument.toJson());
     lFile.close();

     return true;
 }

//!------------------!//
//!      saveFile    !//
//!------------------!//
bool ServerFileDescriptorIO::SaveFile(const QString& fileName, const QString& providerType, const QString& licenseTypeStr, const QString &field1, const QString &field2)
{
    T_LicenseType lLicenseType = StrToLicenseType(licenseTypeStr);
    // -- extract server, port
    if(lLicenseType == floating) {
        QVector<QString>    lServers;
        QVector<int>        lPorts;
        if(ExtractServersPortFromString(field1, lServers, lPorts) == false) {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error while saving file %1. Server string does not have a correct format ").arg(fileName).toLatin1().constData());
            return false;
        }

        if(SaveFile(providerType, lLicenseType, lServers, lPorts) == false || mJsonDocument.isNull()) {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error while saving file %1").arg(fileName).toLatin1().constData());
            return false;
        }
    }
    else if (lLicenseType == standalone) {
        QVector<QString>    ldata;
        ldata.push_back(field1);
        ldata.push_back(field2);
        if(SaveFile(providerType, lLicenseType, ldata, QVector<int>()) == false || mJsonDocument.isNull()) {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error while saving file").arg(fileName).toLatin1().constData());
            return false;
        }
    }
    else {
        // -- evaluation
        if(SaveFile(providerType, lLicenseType, QVector<QString>(), QVector<int>()) == false || mJsonDocument.isNull()) {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error while saving file").arg(fileName).toLatin1().constData());
            return false;
        }
    }


    QFile lFile(fileName);
    lFile.open(QIODevice::WriteOnly | QIODevice::Text);
    lFile.write(mJsonDocument.toJson());
    lFile.close();

    return true;
}

//!--------------------------------!//
//!      updateConnectionStatus    !//
//!--------------------------------!//
void ServerFileDescriptorIO::UpdateConnectionStatus(const QString& fileName, bool connectionStatus)
{
    UpdateConnectionStatus(mJsonDocument, connectionStatus);

    QFile lFile(fileName);
    lFile.open(QIODevice::WriteOnly | QIODevice::Text);
    lFile.write(mJsonDocument.toJson());
    lFile.close();
}

//!--------------------------------!//
//!      getLastUsedLicenseType    !//
//!--------------------------------!//
T_LicenseType   ServerFileDescriptorIO::GetLastUsedLicenseType(QString ) const
{
    return mLastLicenseTypeUsed;
}

//!-----------------------------------!//
//!      getServerDescriptorString    !//
//!-----------------------------------!//
const QString&  ServerFileDescriptorIO::GetServerDescriptorString(const QString &provider, T_LicenseType licenseType)
{
    QPair<T_MultimapIter, T_MultimapIter> lPairIter = mServerDescriptorString.equal_range(provider);

    T_MultimapIter lIterBegin(lPairIter.first), lIterEnd(lPairIter.second);
    for(;lIterBegin !=  lIterEnd; ++lIterBegin)
    {
        if(lIterBegin.value().startsWith(LicenseTypeToStr(licenseType), Qt::CaseInsensitive))
        {
            return lIterBegin.value();
        }
    }

    return mEmptyString;

}
