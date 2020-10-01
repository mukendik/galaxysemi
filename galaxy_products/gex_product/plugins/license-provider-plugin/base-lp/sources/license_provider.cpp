#include "license_provider.h"
#include "license_provider_common.h"
#include "gqtl_log.h"

#include <QDir>
#include <QDomDocument>
#include <QTextStream>
#include <QThread>

namespace GS
{
namespace LPPlugin
{

class LicenseProviderPrivate
{
public :
  LicenseProviderPrivate(LicenseProvider::GexProducts product,
                         const QVariantMap &appConfigData);
  virtual ~LicenseProviderPrivate();

  LicenseProvider::GexProducts mProduct;
  QVariantMap mAppConfigData;
  QVariantMap mLPData;
  int mLastErrorCode;
  QString mLastError;
  QThread *mRunningThread;
  bool mExitAllreadyCalled;
};

LicenseProviderPrivate::LicenseProviderPrivate(LicenseProvider::GexProducts product,
                                               const QVariantMap &appConfigData)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Create ... ");
    mProduct = product;
    mAppConfigData = appConfigData;
    mLastErrorCode = LicenseProvider::eLPLibNoError;
    mLastError = "";
    mRunningThread = 0;
    mExitAllreadyCalled = false;
}

LicenseProviderPrivate::~LicenseProviderPrivate()
{
  GSLOG(SYSLOG_SEV_DEBUG, "Destroy ...");
  mAppConfigData.clear();
  mLPData.clear();
}

LicenseProvider::LicenseProvider(QObject* parent,
                                 LicenseProvider::GexProducts product,
                                 const QVariantMap &appConfigData)
    : QObject(parent)
{
  GSLOG(SYSLOG_SEV_DEBUG, "Create ... ");

  setProperty(LP_TYPE , QString("lp"));
  setProperty(LP_FRIENDLY_NAME , QString("Abstract License Provider"));
  setProperty(LP_USAGE_ORDER , 100);

  GEX_ASSERT(!property(LP_TYPE).isNull());
  GEX_ASSERT(!property(LP_FRIENDLY_NAME).isNull());
  GEX_ASSERT(!property(LP_USAGE_ORDER).isNull());
  mPrivate = new LicenseProviderPrivate(product, appConfigData);
  mServersDescription = 0;

}

LicenseProvider::~LicenseProvider()
{
  GSLOG(SYSLOG_SEV_DEBUG, "Destroy ...");
  if(mPrivate)
    delete mPrivate;
}

LicenseProvider::GexProducts LicenseProvider::getProduct()
{
    return mPrivate->mProduct;
}
void LicenseProvider::setProduct(LicenseProvider::GexProducts val)
{
    mPrivate->mProduct = val;
}


QVariantMap &LicenseProvider::getFullAppConfigData(){
   return mPrivate->mAppConfigData;
}

#ifndef QT_NO_DEBUG
QVariantMap &LicenseProvider::getFullLPData()
{
    return mPrivate->mLPData;
}
#endif

QVariant LicenseProvider::getLPData (const QString &key)
{
  GSLOG(SYSLOG_SEV_DEBUG, QString("Get %1").arg(key).toLatin1().constData());
  bool exist = mPrivate->mLPData.contains(key);
  //Q_ASSERT(exist);
  if(exist)
    return mPrivate->mLPData[key];

  return QVariant();
}

void LicenseProvider::setLPData (const QString &key, const QVariant &value)
{
  GSLOG(SYSLOG_SEV_DEBUG, QString("Set %1").arg(key).toLatin1().constData());

  if(mPrivate->mLPData.contains(key))
    mPrivate->mLPData[key] = value;
  else
    mPrivate->mLPData.insert(key, value);
}

QVariant LicenseProvider::getAppConfigData (const QString &key)
{
  GSLOG(SYSLOG_SEV_DEBUG, QString("Get %1").arg(key).toLatin1().constData());
  bool exist = mPrivate->mAppConfigData.contains(key);
  GEX_ASSERT(exist);
  if(exist)
      return mPrivate->mAppConfigData[key];

  return QVariant();
}

int LicenseProvider::getLastErrorCode()
{
  GSLOG(SYSLOG_SEV_DEBUG,"Error Code ");
  return mPrivate->mLastErrorCode;

}

QString LicenseProvider::getLastError()
{
  GSLOG(SYSLOG_SEV_DEBUG,"Error message ");
  return mPrivate->mLastError;
}

void LicenseProvider::setLastError(int errorCode, const QString &errorMessage)
{
  GSLOG(SYSLOG_SEV_DEBUG,"Set error ");
  mPrivate->mLastErrorCode = errorCode;
  mPrivate->mLastError = errorMessage;

}

void LicenseProvider::receiveGexMessage(const GS::LPPlugin::GEXMessage &gexMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG,QString("Process message sent by GEX App Type (%1) Data (%2)").arg((int)gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());
    if(gexMessage.getType() == GEXMessage::eLicenseRequest)
        return ;
    processGexMessage(gexMessage);

    if(gexMessage.getType() ==  GEXMessage::eExit)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "mLPThread->exit ...");

        if(thread() && thread()->isRunning())
        {
            if(!mPrivate->mExitAllreadyCalled)
            {
                GSLOG(SYSLOG_SEV_DEBUG, " thread()->exit(0) ...");
                if(thread())
                    thread()->exit(0);
                mPrivate->mExitAllreadyCalled = true;
            }
        }
        //qApp->processEvents();
    }
}

bool LicenseProvider::GetLastChoiceStatus()
{
    QString providerName = property(LP_TYPE).toString();
    QString fileDir = getAppConfigData("GalaxySemiFolder").toString()
            +QDir::separator()+ "xml" + QDir::separator() ;
    QString fileName = fileDir + XML_LP_SETTING_FILE;

    if(!QFile::exists(fileName))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("File not created %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QFile oFile(fileName);
    if(!oFile.open(QIODevice::ReadWrite))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Can not open %1").arg(fileName).toLatin1().constData());
        return false;
    }

    QDomDocument xmlDoc;
    QString errorMessage;
    int errorLine=-1, errorColumn=-1;
    if(!xmlDoc.setContent(&oFile, &errorMessage, &errorLine, &errorColumn))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(errorMessage).arg(errorLine).arg(errorColumn).toLatin1().constData());
        return false;
    }

    QDomNode oLPConf = xmlDoc.namedItem("lp_config");
    if(oLPConf.isNull())
        return false;

    QDomElement oCurrentLPConf = oLPConf.namedItem(providerName).toElement();
    if(oCurrentLPConf.isNull())
        return false;

    bool lStatus = ((oCurrentLPConf.attribute("connectionStatus",QString("0")).toInt() == 0) ? false : true);

    oFile.close();
    return lStatus;
}

QVariant LicenseProvider::getLastChoice()
{
    QString providerName = property(LP_TYPE).toString();
    QString fileDir = getAppConfigData("GalaxySemiFolder").toString()
            +QDir::separator()+ "xml" + QDir::separator() ;
    QString fileName = fileDir + XML_LP_SETTING_FILE;

    if(!QFile::exists(fileName))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("File not created %1").arg(fileName).toLatin1().constData());
        return QVariant();
    }

    QFile oFile(fileName);
    if(!oFile.open(QIODevice::ReadWrite))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Can not open %1").arg(fileName).toLatin1().constData());
        return QVariant();
    }

    QDomDocument xmlDoc;
    QString errorMessage;
    int errorLine=-1, errorColumn=-1;
    if(!xmlDoc.setContent(&oFile, &errorMessage, &errorLine, &errorColumn))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(errorMessage).arg(errorLine).arg(errorColumn).toLatin1().constData());
        return QVariant();
    }

    QDomNode oLPConf = xmlDoc.namedItem("lp_config");
    if(oLPConf.isNull())
        return QVariant();

    QDomElement oCurrentLPConf = oLPConf.namedItem(providerName).toElement();
    if(oCurrentLPConf.isNull())
        return QVariant();

    QString data = oCurrentLPConf.attribute("last_choice");

    oFile.close();
    return data;
}

void LicenseProvider::SaveLastChoiceStatus(bool status)
{

    QString jsonDir = getAppConfigData("GalaxySemiFolder").toString() + QDir::separator()+ "json" + QDir::separator() ;

    if(QFile::exists(jsonDir))
    {
        QString jsonFile = jsonDir + JSON_LP_SETTING_FILE;
        mServersDescription->UpdateConnectionStatus(jsonFile, status);
    }
}

void LicenseProvider::saveLastChoice(const QString &runningMode, const QString &editField1, const QString &editField2)
{

    QString jsonDir = getAppConfigData("GalaxySemiFolder").toString() + QDir::separator()+ "json" + QDir::separator() ;

    if(!QFile::exists(jsonDir))
    {
        QDir jsonDirCreate;
        jsonDirCreate.mkdir(jsonDir);
    }

    QString jsonFile = jsonDir + JSON_LP_SETTING_FILE;

    mServersDescription->SaveFile(jsonFile, property(LP_TYPE).toString(), runningMode.section("|",0,0), editField1.section("|",1), editField2.section("|",1));
}

bool LicenseProvider::createEvaluationLicense(ProductInfo *productInfo)
{
    Q_UNUSED(productInfo)
    GSLOG(SYSLOG_SEV_DEBUG, "By default, evaludation license not supported, unless overloaded by specific provider code.");
    return false;
}

void LicenseProvider::DirectRequest(const GEXMessage &gexMessage,LPMessage &lpMessage)
{
    Q_UNUSED(lpMessage)
    GSLOG(SYSLOG_SEV_WARNING, QString("DirectRequest GEXMessage(%1 , %2) not supported by license provider %3")
          .arg(gexMessage.getType())
          .arg(gexMessage.getData())
          .arg(getLPType()).toLatin1().constData());
}

void LicenseProvider::ConvertAndSaveToJsonFormat()
{

    QString jsonDir = getAppConfigData("GalaxySemiFolder").toString() + QDir::separator()+ "json" + QDir::separator() ;

    if(!QFile::exists(jsonDir))
    {
        QDir jsonDirCreate;
        jsonDirCreate.mkdir(jsonDir);
    }

    QString jsonFile = jsonDir + JSON_LP_SETTING_FILE;

    mServersDescription->ConvertAndSaveToJsonFormat(jsonFile);
}

}
}
