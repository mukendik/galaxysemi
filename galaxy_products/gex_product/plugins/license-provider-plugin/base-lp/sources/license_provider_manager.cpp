#include "license_provider_manager.h"
#include "license_provider_thread.h"
#include "license_provider.h"
#include "license_provider_common.h"
#include "license_provider_dialog.h"
#include "cpuchoices.h"
#include "evaluation_dialog.h"
#include "gqtl_log.h"

#include <QDir>
#include <QMessageBox>
#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QDesktopServices>

namespace GS
{
namespace LPPlugin
{

class LicenseProviderManagerPrivate {
public:
    LicenseProviderManagerPrivate();
    virtual ~LicenseProviderManagerPrivate();

  int mErrorCode;
  QString mError;
  QDir mPluginDir;
  QVariantMap mAppConfig;
  ProductInfo *mProductInfo;
  LicenseProvider::GexProducts mProduct;
  QMap <QString , QLibrary *> mAvailableLibrary;
  QMap <QString , class LicenseProvider *> mAvailableLP;

  class LicenseProviderThread *mLPThread;
  GS::LPPlugin::LicenseProvider *mCurrentLP;
  // Message box used to show a splash waiting screen during license download.
  QMessageBox *mWaitingMessage;
};

LicenseProviderManagerPrivate::LicenseProviderManagerPrivate()
{
      mWaitingMessage = NULL;
}

LicenseProviderManager *LicenseProviderManager::mInstance = 0;

LicenseProviderManager::LicenseProviderManager(GS::LPPlugin::LicenseProvider::GexProducts product,
                                               ProductInfo *productInfo,
                                               const QVariantMap &appConfig,
                                               QObject *parent) : QObject(parent)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Create ...");
    /*bool bChangeLP = false;
    QString lLastLP;
    if(appConfig.contains("isDAEMON"))
    {
        if(appConfig["isDAEMON"].toBool())
        {
            lLastLP = getLastLPUsed(appConfig["GalaxySemiFolder"].toString());
            if(!lLastLP.isEmpty())
                bChangeLP = true;
        }
    }*/
    mPrivate = new LicenseProviderManagerPrivate;
    mPrivate->mAppConfig = appConfig;
   // if(bChangeLP)
   //     mPrivate->mAppConfig["UseLP"] = lLastLP;
    mPrivate->mProductInfo = productInfo;
    mPrivate->mProduct = product;

    GEX_ASSERT(mPrivate->mAppConfig.contains("ApplicationDir"));
    mPrivate->mPluginDir = QDir(mPrivate->mAppConfig["ApplicationDir"].toString()+ QDir::separator()
            + LP_GEX_PLUGIN_DIR +  QDir::separator() + LP_SUB_PLUGIN_DIR);

    GEX_ASSERT(mPrivate->mAppConfig.contains("UserFolder"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("GalaxySemiFolder"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("AppVersion"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("AppFullName"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("WelcomeMode"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("HiddenMode"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("ClientApplicationId"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("IniClientSection"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("RunScript"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("isDAEMON"));
//    GEX_ASSERT(mPrivate->mAppConfig.contains("ReleaseDate"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("ExpirationDate"));
    GEX_ASSERT(mPrivate->mAppConfig.contains("TimeBeforeAutoClose"));

    mPrivate->mLPThread = new LicenseProviderThread(this, mPrivate->mProductInfo, 0);
    QObject::connect(mPrivate->mLPThread, SIGNAL(finished()), this, SLOT(requestingLicenseEnd()));

    if(initialize() != eNoError)
        return ;

}

LicenseProviderManager::~LicenseProviderManager()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Destroy ...");
    disconnect();
    cleanup();
    GSLOG(SYSLOG_SEV_DEBUG, "END Destroy ...");

}


int LicenseProviderManager::initialize()
{
    GSLOG(SYSLOG_SEV_DEBUG, "initialize ...");
    mPrivate->mCurrentLP = 0;
    setLastError(eNoError, "");
    if(!mPrivate->mPluginDir.exists())
    {
        setLastError(ePluginDirNotFound,
                     QString("License provider plugin directory (%1) not found").arg(mPrivate->mPluginDir.absolutePath()));
        return getLastErrorCode();
    }

    QString debugSuffix = "";
#ifdef QT_DEBUG
    debugSuffix = "d";
#endif

    // -- mAppConfig["UseLP"] is always set even if the option is not set
    QString forcePlugin = mPrivate->mAppConfig["UseLP"].toString();

    QString lPattern=QString("*%1%2.*").arg(LP_PLUGIN_SUFFIX).arg(debugSuffix);
    GSLOG(6, QString("Searching for LP plugins patterned '%1'...").arg(lPattern).toLatin1().data() );
    /*
    QFileInfoList pluginList = mPrivate->mPluginDir.entryInfoList(
                QStringList () << QString("*%1%2.*").arg(LP_PLUGIN_SUFFIX).arg(debugSuffix),  QDir::Files);
    */
    QFileInfoList pluginList = mPrivate->mPluginDir.entryInfoList(QStringList () << lPattern,  QDir::Files);

    if(pluginList.isEmpty())
    {
        GSLOG(4, QString("No LP plugin found in '%1'").arg(mPrivate->mPluginDir.absolutePath()).toLatin1().data() );
        setLastError(ePluginDirEmpty,
                     QString("No License provider plugin found in directory (%1)").arg(mPrivate->mPluginDir.dirName()));
        return getLastErrorCode();
    }

    foreach (QFileInfo entry, pluginList)
    {
        QString plugin = entry.filePath();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Loading plugin  '%1' ...").arg(plugin).toLatin1().constData());
        if(QLibrary::isLibrary(plugin))
        {
            QLibrary *library = new QLibrary(plugin);
            if(library->load())
            {
                //Check instanciate symbol resolution
                lp_initialize_function initializeFunction = (lp_initialize_function)library->resolve(LP_INSTANTIATE_FUNCTION) ;
                lp_destroy_function destroyFunction = (lp_destroy_function)library->resolve(LP_DESTROY_FUNCTION) ;
                if(initializeFunction && destroyFunction)
                {
                    int libErrorCode;
                    QString libError;
                    LicenseProvider *provider = initializeFunction(0,
                                                                   mPrivate->mProduct,
                                                                   mPrivate->mAppConfig,
                                                                   libErrorCode,
                                                                   libError);
                    if(!provider)
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                               QString("Plugin (%1) does not contains any license provider error (%2 : %3)")
                               .arg(library->fileName()).arg(libErrorCode).arg(libError).toLatin1().constData());
                        delete library;
                    }
                    else if( libErrorCode != LicenseProvider::eLPLibNoError)
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                               QString("Error when loading plugin (%1) with error (%2 : %3)")
                               .arg(library->fileName()).arg(libErrorCode).arg(libError).toLatin1().constData());
                        destroyFunction();
                        delete library;

                    }
                    else if(provider->property(LP_TYPE).isNull() || provider->property(LP_TYPE).toString().isEmpty() ||
                            provider->property(LP_FRIENDLY_NAME).isNull()
                            || provider->property(LP_FRIENDLY_NAME).toString().isEmpty() ||
                            provider->property(LP_USAGE_ORDER).isNull()
                            || provider->property(LP_USAGE_ORDER).toString().isEmpty())
                    {
                        GSLOG(SYSLOG_SEV_WARNING,
                               QString("can not identify license provider in plugin (%1)").arg(library->fileName())
                              .toLatin1().constData());
                        destroyFunction();
                        delete library;
                    }
                    else
                    {
                        if(forcePlugin.isEmpty())
                        {
                            provider->moveToThread(mPrivate->mLPThread);
                            mPrivate->mAvailableLibrary.insert(plugin, library);
                            mPrivate->mAvailableLP.insert(provider->property(LP_TYPE).toString(), provider);
                        }
                        else if(forcePlugin == provider->property(LP_TYPE).toString())
                        {
                            provider->moveToThread(mPrivate->mLPThread);
                            mPrivate->mAvailableLibrary.insert(plugin, library);
                            mPrivate->mAvailableLP.insert(provider->property(LP_TYPE).toString(), provider);
                        }
                        else
                        {
                            GSLOG(SYSLOG_SEV_WARNING, QString("Forcing plugin '%1'").arg(forcePlugin).toLatin1().data());
                            destroyFunction();
                            delete library;
                        }
                    }

                }
                else
                    GSLOG(SYSLOG_SEV_WARNING, QString("Cannot resolve init and destroy functions in '%1'")
                          .arg(plugin).toLatin1().data() );
            }
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Cannot load '%1'").arg(plugin).toLatin1().data());
        }
        else
            GSLOG(SYSLOG_SEV_WARNING, QString("'%1' is not a library").arg(plugin).toLatin1().data());
    }

    if(mPrivate->mAvailableLibrary.isEmpty())
    {
        setLastError(ePluginDirEmpty,
                     QString("No License provider plugin found in directory (%1)").arg(mPrivate->mPluginDir.dirName()));
        return getLastErrorCode();
    }

    if(mPrivate->mAvailableLP.isEmpty())
    {
        setLastError(eNoLPFound,
                     mPrivate->mError = QString("No License provider found in plugin directory (%1)").arg(mPrivate->mPluginDir.dirName()));
        return getLastErrorCode();
    }

    if(mPrivate->mAppConfig.contains("UseLP"))
    {
        if(!mPrivate->mAvailableLP.contains(mPrivate->mAppConfig["UseLP"].toString()))
        {
            setLastError(eNoLPFound,
                         mPrivate->mError = QString("The requested license provider found in plugin directory (%1)").arg(mPrivate->mPluginDir.dirName()));
            return getLastErrorCode();
        }
    }

    return getLastErrorCode();
}

QVariant LicenseProviderManager::GetAppConfig(const QString &key)
{
    if(mPrivate->mAppConfig.contains(key))
        return mPrivate->mAppConfig[key];

    return QVariant();
}

void LicenseProviderManager::SetAppConfig(const QString &key, const QVariant &val)
{
    if(mPrivate->mAppConfig.contains(key))
        mPrivate->mAppConfig[key] = val;
    else
        mPrivate->mAppConfig.insert(key, val);
}

QVariant LicenseProviderManager::getLPData (const QString &key)
{
  if(getCurrentProvider())
  {
      return getCurrentProvider()->getLPData(key);
  }
  return QVariant();


}

int LicenseProviderManager::cleanup()
{
    GSLOG(SYSLOG_SEV_DEBUG, "cleanup ...");
    if(mPrivate)
    {
        delete mPrivate;
        mPrivate = 0;
    }
    GSLOG(SYSLOG_SEV_DEBUG, "End cleanup ...");

    return 0;
}

int LicenseProviderManager::getLastErrorCode()
{
    GSLOG(SYSLOG_SEV_DEBUG, "getLastErrorCode ...");
    return mPrivate->mErrorCode;

}

QString LicenseProviderManager::getLastError()
{
    GSLOG(SYSLOG_SEV_DEBUG, "getLastError ...");
    return mPrivate->mError;
}

void LicenseProviderManager::setLastError(int code , const QString &message)
{
    if(mPrivate)
    {
        mPrivate->mErrorCode = code;
        mPrivate->mError = message;
    }
}

LicenseProviderManager *LicenseProviderManager::instanciate(GS::LPPlugin::LicenseProvider::GexProducts product,
                                                            ProductInfo *productInfo,
                                                            const QVariantMap &appConfig,
                                                            int &errorCode, QString &error,
                                                            QObject *parent)
{
  GSLOG(SYSLOG_SEV_DEBUG, "instanciate ...");

  qRegisterMetaType<GS::LPPlugin::LPMessage *>("GS::LPPlugin::LPMessage *");
  qRegisterMetaType<GS::LPPlugin::LPMessage>("GS::LPPlugin::LPMessage");
  qRegisterMetaType<GS::LPPlugin::GEXMessage *>("GS::LPPlugin::GEXMessage *");
  qRegisterMetaType<GS::LPPlugin::GEXMessage>("GS::LPPlugin::GEXMessage");

  if(!mInstance)
  {
    if(product == GS::LPPlugin::LicenseProvider::eNoProduct || !productInfo || appConfig.isEmpty())
    {
      errorCode = eMissingInitData;
      error = "Missing init Data";
      if(product == GS::LPPlugin::LicenseProvider::eNoProduct)
          error += "\n Can not determine product requested";
      if(!productInfo)
          error += "\n product info is null";
      if(!productInfo)
          error += "\n application config is missing";
      mInstance = 0;
    }
    else
    {
      mInstance = new LicenseProviderManager(product,
                                             productInfo,
                                             appConfig,
                                             parent);
      errorCode = mInstance->getLastErrorCode();
      error = mInstance->getLastError();

      if(mInstance->getLastErrorCode() != eNoError)
      {
        delete mInstance;
        mInstance = 0;
      }
    }
  }
  return mInstance;
}

void LicenseProviderManager::Destroy()
{
    GSLOG(SYSLOG_SEV_DEBUG, "destroy ...");
    disconnect(mInstance, 0, 0, 0);
    if(mInstance)
    {
        delete mInstance;
        mInstance = 0;
    }

}

LicenseProviderManager *LicenseProviderManager::getInstance()
{
    GSLOG(SYSLOG_SEV_DEBUG, "getInstance ...");
    GEX_ASSERT(mInstance);
    return mInstance;
}

QMap <QString , GS::LPPlugin::LicenseProvider *> &LicenseProviderManager::availableLP()
{
    GSLOG(SYSLOG_SEV_DEBUG, "availableLP ...");
    return mPrivate->mAvailableLP;
}

QMap <QString , QLibrary *> &LicenseProviderManager::availableLibrary()
{
    GSLOG(SYSLOG_SEV_DEBUG, "mAvailableLibrary ...");
    return mPrivate->mAvailableLibrary;

}

GS::LPPlugin::LicenseProvider *LicenseProviderManager::getCurrentProvider()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Get Current Provider ...");
    if(mInstance)
        return mInstance->mPrivate->mCurrentLP;
    return 0;

}

QString LicenseProviderManager::getLastLPUsed(const QString &path)
{
    QString lastLPUsed = "gs_lp";
    QString fileDir = path + QDir::separator()+ "xml" + QDir::separator() ;
    QString fileName = fileDir + XML_LP_SETTING_FILE;

    if(!QFile::exists(fileName))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("File not created %1").arg(fileName).toLatin1().constData());
        return lastLPUsed;
    }

    QFile oFile(fileName);
    if(!oFile.open(QIODevice::ReadWrite))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Can not open %1").arg(fileName).toLatin1().constData());
        return lastLPUsed;
    }

    QDomDocument xmlDoc;
    QString errorMessage;
    int errorLine=-1, errorColumn=-1;
    if(!xmlDoc.setContent(&oFile, &errorMessage, &errorLine, &errorColumn))
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Invalid XML file Error(%1) at Line(%2) and Column(%3)").arg(errorMessage).arg(errorLine).arg(errorColumn).toLatin1().constData());
        return lastLPUsed;
    }

    QDomNode oLPConf = xmlDoc.namedItem("lp_config");
    if(oLPConf.isNull())
        return lastLPUsed;

    QDomElement oCurrentLPConf = oLPConf.namedItem("last_lp_used").toElement();
    if(oCurrentLPConf.isNull())
        return lastLPUsed;

    lastLPUsed = oCurrentLPConf.attribute("use_lp");

    oFile.close();
    return lastLPUsed;
}

void LicenseProviderManager::setCurrentProvider(const QString &strLP)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Set Current Provider => -%1-").arg(strLP).toLatin1().constData());

    if(mPrivate->mCurrentLP)
    {
        QObject::disconnect(mPrivate->mCurrentLP, 0, this, 0);
        QObject::disconnect(this, 0,mPrivate->mCurrentLP , 0);
    }

    if(mPrivate->mAvailableLP.contains(strLP))
    {
        mPrivate->mCurrentLP = mPrivate->mAvailableLP[strLP];
        bool connOk = QObject::connect(this, SIGNAL(forwardGexMessage(const GS::LPPlugin::GEXMessage &)), mPrivate->mCurrentLP,
                                       SLOT(receiveGexMessage(const GS::LPPlugin::GEXMessage &)));
        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(sendLPMessage(const GS::LPPlugin::LPMessage &)),
                                   this, SIGNAL(forwardLPMessage(const GS::LPPlugin::LPMessage &)));

        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(userChoice(const QStringList *, int &)),
                         this, SLOT(userChoice(const QStringList *, int &))
                         ,Qt::BlockingQueuedConnection);
        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(ShowWaitingMessage(const QString &, int )),
                         this, SLOT(ShowWaitingMessage(const QString &, int ))
                         ,Qt::BlockingQueuedConnection);

        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(userAvailableChoices(const QString *, const QStringList *,bool, QString *)),
                         this, SLOT(userAvailableChoices(const QString *, const QStringList *,bool,QString *))
                         ,Qt::BlockingQueuedConnection);

        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(userEvalLicense(const QString *, QString *)),
                         this, SLOT(userEvalLicense(const QString *, QString *))
                         ,Qt::BlockingQueuedConnection);

        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(notifyUser(const QString *)),
                         this, SLOT(notifyUser(const QString *))
                         ,Qt::BlockingQueuedConnection);
        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP,
           SIGNAL(userInteraction(GS::LPPlugin::LicenseProvider *, QString *, QString *, int *, bool &, bool &, bool)),
                         this,
           SLOT(userInteraction(GS::LPPlugin::LicenseProvider *, QString *, QString *, int *, bool &, bool &, bool))
                         ,Qt::BlockingQueuedConnection);
        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(notifyDaemon()),
                         this, SLOT(notifyDaemon()), Qt::BlockingQueuedConnection);
        connOk &= (bool)QObject::connect(mPrivate->mCurrentLP, SIGNAL(openURL(const QString*)),
                         this, SLOT(openURL(const QString*)), Qt::BlockingQueuedConnection);
        GEX_ASSERT(connOk);
        if(!connOk)
        {
            mPrivate->mCurrentLP = 0;
        }

    }
    else
        mPrivate->mCurrentLP = 0;
}

void LicenseProviderManager::receiveGexMessage(const GS::LPPlugin::GEXMessage &gexMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("receiveGexMessage ... %1(%2)").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());
    if(gexMessage.getType() == GEXMessage::eLicenseRequest)
    {
        mPrivate->mLPThread->start();
    }
    else //Forward the message to LP
        emit forwardGexMessage(gexMessage);

    //qApp->processEvents();
}

void LicenseProviderManager::requestingLicenseEnd ()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("requestingLicenseEnd  ... %1").arg(mPrivate->mLPThread->getStatus()).toLatin1().constData());

    if(mPrivate->mLPThread->getStatus() == LicenseProviderThread::eNoProviderFound
        || mPrivate->mLPThread->getStatus() == LicenseProviderThread::eNoLicenseFound)
    {
        QString data = (mPrivate->mLPThread->getStatus() == LicenseProviderThread::eNoProviderFound) ? QString("No license Provider found") :
                                                                                                       mPrivate->mLPThread->getProviderMessage()/*QString("No license found")*/;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Fail requestingLicenseEnd  ... %1").arg(data).toLatin1().constData());
        emit forwardLPMessage(LPMessage(LPMessage::eReject,data));
        data.clear();
    }
    //qApp->processEvents();
}

void LicenseProviderManager::userEvalLicense(const QString *message, QString *orderId)
{
    if(!message || !orderId)
        return;
    EvaluationDialog lEval(*message,0);
    int lRet = lEval.exec();
    if(lRet == QDialog::Accepted)
    {
        orderId->clear();
        orderId->append(lEval.GetOrderId());
    }
    else
    {
        orderId->clear();
    }
}

void LicenseProviderManager::userAvailableChoices(const QString *message, const QStringList *products, bool activate,QString *returnedVal)
{
    if(!message || !products || !returnedVal)
    {
        GSLOG(SYSLOG_SEV_ERROR, "NULL pointer passed to the userAvailableChoices");
        return;
    }
    CPUChoices lChoice(*(message), *(products), activate, 0);
    int lRet = lChoice.exec();

    if(lRet == 0)
    {
        lChoice.SetChoice(2);
    }
    int lUserChoice = lChoice.GetChoice();
    QString lProduct = lChoice.GetProductToBeUsed();

    returnedVal->clear();
    returnedVal->append(QString("%1|%2").arg(lUserChoice).arg(lProduct));
}


void LicenseProviderManager::ShowWaitingMessage(const QString &message, int mode)
{

    //If not initialised then initialise it
    if(!mPrivate->mWaitingMessage)
    {
        mPrivate->mWaitingMessage = new QMessageBox(0);
    }

    if(mode > 0)//Show mode
    {
        //Configure the message box to show the message
        QWidget *lActiveWindow =  QApplication::activeWindow();
        mPrivate->mWaitingMessage->setWindowTitle(GetAppConfig("AppFullName").toString());
        mPrivate->mWaitingMessage->setText(message);
        mPrivate->mWaitingMessage->setStandardButtons(QMessageBox::NoButton);
        mPrivate->mWaitingMessage->setWindowFlags(Qt::SplashScreen);
        mPrivate->mWaitingMessage->setModal(true);
        mPrivate->mWaitingMessage->show();

        if(lActiveWindow)
        {
            const QPoint lGlobal = lActiveWindow->mapToGlobal(lActiveWindow->rect().center());
            mPrivate->mWaitingMessage->move(lGlobal.x() - mPrivate->mWaitingMessage->width() / 2,
                                                 lGlobal.y() - mPrivate->mWaitingMessage->height() / 2);
        }

    }
    else//hide mode
    {
        mPrivate->mWaitingMessage->hide();
    }

}

void LicenseProviderManager::userChoice(const QStringList *choices, int &choice)
{
    GSLOG(SYSLOG_SEV_DEBUG, "userChoice");

    if(choices->count() == 4)
        choice = QMessageBox::information(0,GetAppConfig("AppFullName").toString(),(*choices)[0],(*choices)[1],(*choices)[2],(*choices)[3]);
    else if(choices->count() == 3)
        choice = QMessageBox::information(0,GetAppConfig("AppFullName").toString(),(*choices)[0],(*choices)[1],(*choices)[2]);
    else if(choices->count() == 2)
        choice = QMessageBox::information(0,GetAppConfig("AppFullName").toString(),(*choices)[0],(*choices)[1]);

}

void LicenseProviderManager::notifyUser(const QString *)
{
    GSLOG(SYSLOG_SEV_DEBUG, "notifyUser");
    GEX_ASSERT(false);
}

void LicenseProviderManager::userInteraction(
        GS::LPPlugin::LicenseProvider *provider, QString *EditField1, QString *EditField2,
        int *selectionMode, bool &/*b*/, bool &result, bool legacy)
{
    GSLOG(SYSLOG_SEV_DEBUG, "userInteraction");
    GS::LPPlugin::LicenseProviderDialog activationKey(provider,0, true, Qt::Dialog);
    activationKey.hideLegacyButton(legacy);

    if(!activationKey.Init())
    {
        GSLOG(3, "failed to init ActivationKey dialog");
            result =  false;
            return ;
    }

    GSLOG(5, "License missing, popup login dialog...");
    // License file not correct or doesn't exist...
    if(activationKey.exec() != QDialog::Accepted)
    {
        GSLOG(4, "ActivationKey dialog exec cancelled");
        result =  false;	// User aborted the page requesting the Product Key ID
        return ;
    }

    bool legacyStatus = activationKey.getLegacyStatus();
    if(legacyStatus)
    {
        GSLOG(4, "ActivationKey dialog Go to legacy mode");
        result =  false;	// User aborted the page requesting the Product Key ID
        (*EditField1) = "UseLegacy";
        return ;
    }

    if (EditField1)
        (*EditField1) = activationKey.getEditField1();

    if (EditField2)
        (*EditField2) = activationKey.getEditField2();

    if (selectionMode)
        (*selectionMode) = activationKey.getSelectionMode();

    result = true;
}

void LicenseProviderManager::notifyDaemon()
{
    GSLOG(SYSLOG_SEV_DEBUG, "notifyDaemon");
    GEX_ASSERT(false);
}

void LicenseProviderManager::stopLicensingThread()
{
    if(mPrivate->mLPThread)
    {
        mPrivate->mLPThread->exit();
    }
}

void LicenseProviderManager::waitUntilThreadFinshed()
{
    if(mPrivate->mLPThread)
    {
        mPrivate->mLPThread->wait();
    }
}

void LicenseProviderManager::openURL(const QString *url)
{
    if (!QDesktopServices::openUrl(QUrl(*url)))
    {
        QString decoratedURL = QString("<a href='%1'>%1</a>").arg(*url);
        QMessageBox msgBox(0);

        msgBox.setWindowTitle(GetAppConfig("AppFullName").toString());
        msgBox.setWindowIcon(QPixmap(":/gex/icons/gex_application_48x48.png"));
        msgBox.setTextFormat(Qt::RichText);
        msgBox.setText(QString("Can not automatically open the URL %1, please copy and paste it into a browser window.").arg(decoratedURL));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

}

LicenseProviderManagerPrivate::~LicenseProviderManagerPrivate()
{
    GSLOG(SYSLOG_SEV_DEBUG, "~LicenseProviderManagerPrivate ...");

     GSLOG(SYSLOG_SEV_DEBUG, "mCurrentLP->disconnect ...");
    if(mCurrentLP)
        mCurrentLP->disconnect();

//    while(!mLPThread->isFinished())
//        qApp->processEvents();



    if(mLPThread)
    {
#ifdef Q_OS_MAC
        GSLOG(SYSLOG_SEV_DEBUG, "mLPThread->delete() on mac ...");
        mLPThread->exit(0);
        while (mLPThread->isRunning())
        {
            ;
        }
        //delete mLPThread;
        mLPThread = 0;
#else
        GSLOG(SYSLOG_SEV_DEBUG, "mLPThread->delete() on other platforms ...");
        //mLPThread->exit(0);
        delete mLPThread; // comment this line to check if the crash comes from this line
        mLPThread = 0;
#endif

    }

//    if(mCurrentLP)
//        delete mCurrentLP;

    mCurrentLP = 0;
    mProductInfo = 0;

//    GSLOG(SYSLOG_SEV_DEBUG, " destroyFunction(); ...");

/*
    QLibrary* lLibrary;
    QList<QLibrary*> lLibraryList = mAvailableLibrary.values();
    QList<QLibrary*>::const_iterator lLibraryIter;
    for(lLibraryIter = lLibraryList.begin();
        lLibraryIter != lLibraryList.end(); ++lLibraryIter) {
        lLibrary = *lLibraryIter;
        lp_destroy_function destroyFunction =
             (lp_destroy_function) lLibrary->resolve(LP_DESTROY_FUNCTION);
        destroyFunction();
        lLibrary->unload();
        delete lLibrary;
    }
*/
    GSLOG(SYSLOG_SEV_DEBUG, " clear ...");
    mAvailableLP.clear();
    mAppConfig.clear();
    mAvailableLibrary.clear();

    GSLOG(SYSLOG_SEV_DEBUG, "END ~LicenseProviderManagerPrivate ...");


}

}
}
