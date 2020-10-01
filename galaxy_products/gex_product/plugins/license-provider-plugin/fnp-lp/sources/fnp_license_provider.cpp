#include "fnp_license_provider.h"
#include "license_provider_common.h"
#include "product_info.h"
#include "gqtl_log.h"
#include "gex_constants.h"
#include "gex_errors.h"
#include "license_provider_dialog.h"
#include "download_license.h"
#include "gex_shared.h"
#include "lmclient.h"
#include "fnp_proxy.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMap>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QLibrary>
#include <QCoreApplication>
#include <QTimer>
#include <QProcess>
#include <QHostInfo>

namespace GS
{
namespace LPPlugin
{

#define FNP_LP_VERSION 0.0
#define FNP_LP_ORDER 98

#define LIB_GEX_FNP_PROXY "fnp"

#define LIBLP_RESOLVER_SYMBOL "resSym"
#define LIBLP_INIT_FCT "initLibrary"
#define LIBLP_CLEANUP_FCT "cleanupLibrary"
#define LIBLP_RESERVE_FCT "reserveLicense"
#define LIBLP_RELEASE_FCT "releaseLicense"
#define LIBLP_CLIENT_DAEMON_QA_FCT "ClientDaemonQA"
#define LIBLP_CLIENT_IDLE_FCT "idleStatus"
#define LIBLP_LAST_ERROR_FCT "getLastError"
#define LIBLP_LAST_ERROR_CODE_FCT "getLastErrorCode"
#define LIBLP_CONNECTION_STATUS_FCT "setConnectionStatusCallBack"
#define LIBLP_LIC_PATH_FCT "setLicPath"
#define LIBLP_HOSTID_FCT "getHostID"
#define LIBLP_VERSION_FCT "getLibVersion"

//Comment part with Protocol Com with DAEMON
#define GS_PING_CMD "PING:"
//YMADMINDB_WRITE_SETTING
#define GS_WRITE_YM_ADMINDB_CMD "YMWR:"
//YMADMINDB_READ_SETTING
#define GS_READ_YM_ADMINDB_CMD "YMRD:"
//Get Server Time
#define GS_SERVER_TIME "TIME:"
//Get Client global Timeout
#define GS_CLIENT_TIMEOUT "TIMEOUT:"

#define GS_START_CMD "START:"
#define GS_NEXT_CMD "NEXT:"
#define GS_CONTINUE_CMD "CONTINUE:"
#define GS_STOP_CMD "STOP:"
#define GS_END_CMD "END:"

#define GS_LC_VSEND_MESSAGE_LENGTH 100
//Comment part with Protocol Com with DAEMON

#ifdef Q_OS_WIN
#define DEFAULT_LICPATH "@localhost;license.dat;."
#else
#define DEFAULT_LICPATH "@localhost:license.dat:."
#endif

#define FNP_ACTIV_PASSIV_STATUS_INTERVAL 60000
#define GS_ACT_UTILITY "galaxy-la"

const char *sEvaluationActivation = "http://gxs-license.galaxysemi.com/gs_gen.htm";
const char *sLicenseDownloadUrl = "http://gxs-license.galaxysemi.com/download_license_app.php";

typedef void* (*resSym_function)(const char *routineName);
typedef void (*loger_call_back_function)(int sev, const char *messsage);
typedef bool (*initLibrary_function)(int &, char *, void * );
typedef void (*cleanupLibrary_function) ();
typedef bool (*reserveLibrary_function)(const char *szFeature, const char *szVersion,int nlic, char *internalInfo[]);
typedef void (*releaseLibrary_function)(const char *szFeature);
typedef const char *(*lastError_function) ();
typedef int (*lastErrorCode_function) ();
typedef char* (*ClientDaemonQA_function) (char *);
typedef void (*clientIdle_function) (int );
typedef void (*setLicPath_function)(char *);
typedef void (*setConnectionStatusCallBack_function)(void *, void *, void *);
typedef char* (*getHostID_function)();
typedef char* (*getLibVersion_function)();



static void logerCallBack(int sev, const char *messsage);
static void userExitCallBack(char *feature);//LM_A_USER_EXITCALL,




class FNPLicenseProviderPrivate
{
public:
    //Internal error handling
    QString mInternalErrorMessage;
    int mInternalError;
    QVariantMap mAppConfigData;
    QLibrary mLibraryLoader;
    QMap <QString, void *> mFNPFctPointer;
    QList<int> mGTLLibrary;
    static QStringList sSymbolToBeResolved ;
    QStringList mReservedIncrement;
    ProductInfo *mProductInfo;
    int mDefaultAllowedTimeShift;

    QMap<LicenseProvider::GexProducts, QString> mProductIncrement;
    QMap<LicenseProvider::GexOptions, QString> mOptionIncrement;
    QMap<LicenseProvider::GexProducts, QString> mPassivIncrement;
    QMap<LicenseProvider::GexProducts, QString> mMaintenanceIncrement;
    QMap<LicenseProvider::GexProducts, QString> mAdditionalIncrement;
    QStringList mBasicIncrement;
    QStringList mAlternativeIncrement;

    QString mEditField1/*, mEditField2*/;
    int miSelectionMode;
    QDateTime mLastUserActivity;
    QString mLicenseDenialStr;
    int mLicenseDenialCode;
    QStringList mDisconnectedFeature;
    QStringList mConnectReconnectItem;

    QTimer *mActiveStatusUpdate;
    bool mInPassivMode;

    int mLicenseMode;
    long mLicenseRunningMode;
    enum LicenseMode{
        eStandalone,
        eFloating
    };

    QMap<int, QString> mGalaxyLicenseMessages;

public:
    FNPLicenseProviderPrivate(const QVariantMap &appConfigData);
    virtual ~FNPLicenseProviderPrivate();
    bool addSymbol(const QString &strAddSymbol);
    bool getResSym();
    void initCodeMapping();
    QDate toDate(const QString &dates);
    void initErrorMessagesMap();
    QString userErrorMessage(int errorCode, const QString &fnpMessage);

};

QStringList FNPLicenseProviderPrivate::sSymbolToBeResolved = QStringList() <<   LIBLP_INIT_FCT
                                                                           <<   LIBLP_CLEANUP_FCT
                                                                           <<   LIBLP_RESERVE_FCT
                                                                           <<   LIBLP_RELEASE_FCT
                                                                           <<   LIBLP_CLIENT_DAEMON_QA_FCT
                                                                           <<   LIBLP_CLIENT_IDLE_FCT
                                                                           <<   LIBLP_LAST_ERROR_FCT
                                                                           <<   LIBLP_LAST_ERROR_CODE_FCT
                                                                           <<   LIBLP_CONNECTION_STATUS_FCT
                                                                           <<   LIBLP_LIC_PATH_FCT
                                                                           <<  LIBLP_HOSTID_FCT
                                                                           <<  LIBLP_VERSION_FCT;

FNPLicenseProvider* FNPLicenseProvider::mInstance = 0;
FNPLicenseProvider *FNPLicenseProvider::initializeLP(QObject* parent,
                                                     GexProducts product,
                                                     const QVariantMap &appConfig,
                                                     int &libErrorCode,
                                                     QString &libError)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");

    libErrorCode = eLPLibNoError;
    libError = "";

    if(!mInstance)
    {
        mInstance = new FNPLicenseProvider(parent, product, appConfig);
        if(mInstance->getLastErrorCode() != eLPLibNoError)
        {
            libErrorCode = mInstance->getLastErrorCode();
            libError = mInstance->getLastError();
            delete mInstance;
            mInstance = 0;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1, %2)").arg(libErrorCode).arg(libError).toLatin1().constData());
    return mInstance;
}

void FNPLicenseProvider::destroy()
{
    if(mInstance)
        delete mInstance;
    mInstance = 0;
}

FNPLicenseProvider::FNPLicenseProvider(QObject* parent,
                                       GexProducts product,
                                       const QVariantMap &appConfig)
    : LicenseProvider(parent, product, appConfig)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    setProperty(LP_TYPE , QString("fnp_lp"));
    setProperty(LP_FRIENDLY_NAME , QString("FlexLM License Provider"));
    setProperty(LP_USAGE_ORDER , FNP_LP_ORDER);
    setProperty(LP_VERSION , FNP_LP_VERSION);
    mPrivate = 0;
    int iRet = initialize();
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with initialize return code (%1)").arg(iRet).toLatin1().constData());
}

FNPLicenseProvider::~FNPLicenseProvider()
{
    /*int iRet =*/ cleanup();

}

FNPLicenseProvider *FNPLicenseProvider::getInstance()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    GSLOG(SYSLOG_SEV_DEBUG, "Finish");
    return mInstance;
}
bool FNPLicenseProvider::CheckAppAndFNPLibVersion()
{
    //set the FNP_PROXY_LIB_VERSION expected by the app
    QString lAppCompiledVersion = GS_FNP_PROXY_LIB_VERSION;
    // Set the library build-in  version
    QString lLibraryVersion = "UKNOWN";

    getLibVersion_function getLibVersion = 0;
    if(mPrivate->mFNPFctPointer.contains(LIBLP_VERSION_FCT) && mPrivate->mFNPFctPointer[LIBLP_VERSION_FCT])
        getLibVersion = (getLibVersion_function)mPrivate->mFNPFctPointer[LIBLP_VERSION_FCT];

    if(getLibVersion)
    {
        lLibraryVersion = getLibVersion();
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("lAppCompiledVersion (%1)  lLibraryVersion(%2)")
          .arg(lAppCompiledVersion).arg(lLibraryVersion).toLatin1().constData());
    if(lAppCompiledVersion != lLibraryVersion)
    {
        mPrivate->mInternalError = eLPLibMismatch;
        mPrivate->mInternalErrorMessage = QString("Version mismatch between application expected build \"%1\" and FNP library \"%2\"")
                                      .arg(lAppCompiledVersion).arg(lLibraryVersion);
        return false;
    }
    else
    {
        return true;
    }
}

void buildPortsServersListFromWelcomeString(const QString& aEditField, QVector<QString>& aServers, QVector<int>& aPorts)
{
    QChar lSeparator(';');
    if(aEditField.contains(","))
        lSeparator = ',';

    QStringList lServersPorts = aEditField.split(lSeparator, QString::SkipEmptyParts);
    QStringList::iterator lIterBegin(lServersPorts.begin()), lIterEnd(lServersPorts.end());
    for(;lIterBegin != lIterEnd; ++lIterBegin )
    {
        QStringList lElements = (*lIterBegin).split(':', QString::SkipEmptyParts);
        if(lElements.size() == 2)
        {
            aServers.append(lElements[0]);
            aPorts.append(lElements[1].toInt());
        }
    }
}

QString buildFormattedServersString(const QVector<QString>& aServers, const QVector<int>& aPorts)
{
    QString lFormattedString;
    int lMinSize = std::min(aServers.size(), aPorts.size());
    for (int i=0; i<lMinSize; ++i)
    {
        if(!lFormattedString.isEmpty())
        {
#if defined(_WIN32)
            lFormattedString += ";";
#elif defined unix || defined __MACH__
            lFormattedString += ":";
#endif
        }
        lFormattedString += QString("%1@%2").arg(aPorts[i]).arg(aServers[i]);
    }
    return lFormattedString;
}

QString buildFormattedServerString(unsigned int aIndex, const QVector<QString>& aServers, const QVector<int>& aPorts)
{
    QString lFormattedString;
    if((int)aIndex < std::min(aServers.size(), aPorts.size()))
    {
        lFormattedString = QString("%1@%2").arg(aPorts[aIndex]).arg(aServers[aIndex]);
    }
    return lFormattedString;
}

int FNPLicenseProvider::initialize()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    mPrivate = new FNPLicenseProviderPrivate(getFullAppConfigData());
    setLastError(mPrivate->mInternalError,  mPrivate->mInternalErrorMessage);

    if(mPrivate->mInternalError != eLPLibNoError)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with code (%1) message(%2)").arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage).toLatin1().constData());
        return mPrivate->mInternalError;
    }

    //Check if the application version and the lib version are aligned.
    if(!CheckAppAndFNPLibVersion())
    {
        setLastError(mPrivate->mInternalError,  mPrivate->mInternalErrorMessage);
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Error when loading the FNP library (%1): (%2)").arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage).toLatin1().constData());
        return mPrivate->mInternalError;
    }

    QString lxmlFileDir = getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "xml" + QDir::separator() ;
    QString ljsonFileDir = getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "json" + QDir::separator() ;

    QString lFileName = ljsonFileDir + JSON_LP_SETTING_FILE;
    QString /*lastChoice,*/ licPathVal;

    ServerFileDescriptorIOFactory* lFactory = ServerFileDescriptorIOFactory::GetInstance();

    if(!QFile::exists(lFileName))
    {
        lFileName = lxmlFileDir + XML_LP_SETTING_FILE;
    }

    mServersDescription = lFactory->GetServerDescriptor(lFileName);

    if(!QFile::exists(lFileName))
    {
        licPathVal = QString(DEFAULT_LICPATH);
        mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
        getFullAppConfigData().insert("WelcomeMode", true);
        mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_STANDALONE;

        // -- no file avalaible. The save will be done to the json format
        lFileName = ljsonFileDir + JSON_LP_SETTING_FILE;
        mServersDescription = lFactory->GetServerDescriptor(lFileName);
    }
    //    if(lastChoice.isNull() || lastChoice.isEmpty())
    //    {
    //       licPathVal = QString(DEFAULT_LICPATH);
    //       mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
    //       getFullAppConfigData().insert("WelcomeMode", true);
    //       mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_STANDALONE;
    //    }
    else
    {
        if(!mServersDescription || mServersDescription->LoadFile(lFileName, getAppConfigData("UseLP").toString()) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, QString("Error while loading file %1, bad format.").arg(lFileName).toLatin1().constData());
            return eLPInternalError;
        }
        /*lastChoice = getLastChoice().toString()*/;
        licPathVal = QString(DEFAULT_LICPATH);


        if(/*choiceItem[0]*/mServersDescription->GetLastUsedLicenseType() == standalone/*"Standalone"*/
           || /*choiceItem[0]*/mServersDescription->GetLastUsedLicenseType() == evaluation/*"Evaluation"*/)
        {
            mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
            licPathVal ="";
            mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_STANDALONE;
            if(mServersDescription->GetLastUsedLicenseType() == evaluation/*choiceItem[0] == "Evaluation"*/)
            {
                mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_EVALUATION;
                QString evalLicenseFiles = getLicenseFiles(getAppConfigData("UserFolder").toString());
                licPathVal = evalLicenseFiles;
            }
        }
        else
        {
            mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eFloating;
            mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_CLIENT;
            // Fix me: loop on the table of the IPs and port and add the ";" between servers
            QVector<QString> lServerList = mServersDescription->GetListServerIP();
            QVector<int> lPortList = mServersDescription->GetListSocketPorts();
            if (lServerList.size() != lPortList.size())
            {
                licPathVal = QString(DEFAULT_LICPATH);
                mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
                getFullAppConfigData().insert("WelcomeMode", true);
                mPrivate->mLicenseRunningMode = GEX_RUNNINGMODE_STANDALONE;
            }
            else
            {
                licPathVal = buildFormattedServersString(lServerList, lPortList);
                // Set internal key "ServerIP" to contain first server in the list
                setLPData("ServerIP", licPathVal);
                //setLPData("ServerName",lServerList[1]);
                //setLPData("ServerPort",lPortList[1]);
            }
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("initialize : setLicPath with (%1)").arg(licPathVal).toLatin1().constData());
    setLicPath_function licPath= (setLicPath_function)(mPrivate->mFNPFctPointer[LIBLP_LIC_PATH_FCT]);
    licPath(licPathVal.toLatin1().data());

    setConnectionStatusCallBack_function setConnectionStatusCallBack = (setConnectionStatusCallBack_function)(mPrivate->mFNPFctPointer[LIBLP_CONNECTION_STATUS_FCT]);
    setConnectionStatusCallBack((void *)userExitCallBack, 0/*(void *)userReconnectCallBack*/, 0/*(void *)userReconnectCallDoneCallBack*/);

    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with code (%1)").arg(eLPLibNoError).toLatin1().constData());
    return eLPLibNoError;
}

int FNPLicenseProvider::cleanup()
{
    //release(mPrivate->mReservedIncrement);
    if(mPrivate)
        delete mPrivate;
    mPrivate = 0;
    mServersDescription = 0;

    return 0;
}

void FNPLicenseProvider::processGexMessage(const GEXMessage &gexMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("process GexMessage : %1 : %2").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());
    if(gexMessage.getType() == GEXMessage::eLicenseRequest)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eLicenseRequest Ignore it is handled by LPManager");
        //Ignore it is handled by LPManager
    }
    else if(gexMessage.getType() == GEXMessage::eActive)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eActive Ignore it is handled by FNP");
    }
    else if(gexMessage.getType() == GEXMessage::eDisconnect)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eDisconnect clientIdle(1)");
        release(mPrivate->mReservedIncrement);
    }
    else if(gexMessage.getType() == GEXMessage::eReconnect)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eReconnect clientIdle(0)");
        QStringList increments = QStringList()<< mPrivate->mProductIncrement [mPrivate->mProductInfo->getProductID()];
        if(!reserve(mPrivate->mProductInfo, increments))
        {
            QString lReconnectErrorMessage = QString("Cannot retrieve the license again from %1.\n Error %2: %3")
                    .arg(GetFloatingLicensePath())
                    .arg(mPrivate->mLicenseDenialCode)
                    .arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr));
            GSLOG(SYSLOG_SEV_CRITICAL, lReconnectErrorMessage.toLatin1().constData());
            setInternalError(LPMessage::eReject,lReconnectErrorMessage);
            emit sendLPMessage(LPMessage(LPMessage::eReject, lReconnectErrorMessage));
        }
        else
        {

            if((mPrivate->mProductInfo->getProductID() == eGTM) && mPrivate->mGTLLibrary.count())
            {
                QList<int> lGTLLibrary = mPrivate->mGTLLibrary;
                mPrivate->mGTLLibrary.clear();
                for(int lIdx=0; lIdx<lGTLLibrary.count();++lIdx)
                {
                    QString lInfo ;
                    RequestGTLToken(lGTLLibrary[lIdx], lInfo);
                }
            }
            emit sendLPMessage(LPMessage(LPMessage::eAccept,"Reconnect succeed"));
        }
    }
    else if(gexMessage.getType() == GEXMessage::eGetYMAdminDBConfig)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "eGetYMAdminDBConfig ");
        QString yieldManDbSettingsConf;
        QString daemonError;
        bool ret = getYieldManDbSettings(yieldManDbSettingsConf, daemonError);
        GSLOG(SYSLOG_SEV_DEBUG, QString("eGetYMAdminDBConfig ret(%1)").arg(ret).toLatin1().constData());
        if(ret)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("eGetYMAdminDBConfig yieldManDbSettingsConf(%1)").arg(yieldManDbSettingsConf).toLatin1().constData());
            emit sendLPMessage(LPMessage(LPMessage::eSetYMAdminDBConfig,
                                         yieldManDbSettingsConf));
        }
        else
        {
            QString messageData = QString("Message::warning|%1|%2")
                    .arg("YM Admin DB")
                    .arg(QString("Error when retrieving YM Admin DB Config \n %1 \n%2").arg(daemonError).arg(yieldManDbSettingsConf));

            GSLOG(SYSLOG_SEV_WARNING, QString("eGetYMAdminDBConfig Message::warning(%1)").arg(messageData).toLatin1().constData());
//            emit sendLPMessage(LPMessage(LPMessage::eExtended,messageData));
            emit sendLPMessage(LPMessage(LPMessage::eSetYMAdminDBConfig,
                                         yieldManDbSettingsConf));
        }

    }
    else if(gexMessage.getType() == GEXMessage::eSetYMAdminDBConfig)
    {
        QString daemonError;
        bool ret = setYieldManDbSettings(gexMessage.getData(), daemonError);
        GSLOG(SYSLOG_SEV_DEBUG, QString("eSetYMAdminDBConfig message(%1) ret(%2) daemonError(%3)").arg(gexMessage.getData()).arg(ret).arg(daemonError).toLatin1().constData());
        if(!ret)
        {
            QString messageData = QString("Message::warning|%1|%2")
                    .arg("YM Admin DB")
                    .arg(QString("Error when setting YM Admin DB Config \n %1 \n%2").arg(daemonError).arg(""));

            GSLOG(SYSLOG_SEV_DEBUG, QString("eSetYMAdminDBConfig Message::warning(%1)").arg(messageData).toLatin1().constData());
        }
    }
    else if(gexMessage.getType() == GEXMessage::eExit)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEXMessage::eExit %1").arg(gexMessage.getData()).toLatin1().constData());
        release(mPrivate->mReservedIncrement);
        mPrivate->mReservedIncrement.clear();

    }
    else if(gexMessage.getType() == GEXMessage::eExtended)
    {
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("End processGexMessage : %1 : %2").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());
}


QString FNPLicenseProvider::clientDaemonQA(const QString &message)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(":: clientDaemonQA :: Start sending %1").arg(message).toLatin1().constData());

    GEX_ASSERT(message.count() <= GS_LC_VSEND_MESSAGE_LENGTH);

#ifndef GALAXY_DAEMON_CUSTOMIZATION
    ClientDaemonQA_function fnpClientDaemonQA = (ClientDaemonQA_function)(mPrivate->mFNPFctPointer[LIBLP_CLIENT_DAEMON_QA_FCT]);
#else
    ClientDaemonQA_function fnpClientDaemonQA = (ClientDaemonQA_function)GS_respond_to_lc_vsend;
#endif
    QString daemonAnswer = fnpClientDaemonQA(message.toLatin1().data());

    GSLOG(SYSLOG_SEV_DEBUG, QString(":: clientDaemonQA :: Finish with : %1").arg(daemonAnswer).toLatin1().constData());

    return daemonAnswer;
}


bool FNPLicenseProvider::getYieldManDbSettings(QString &getYMAdminDBConfig, QString &daemonError)
{
    getYMAdminDBConfig = "GEX_YieldManDbSettings|NotConfigured";

    GSLOG(SYSLOG_SEV_DEBUG, QString("getYieldManDbSettings : %1 : %2").arg(getYMAdminDBConfig).arg(daemonError).toLatin1().constData());

    QString answer = clientDaemonQA(QString("%1%2").arg(GS_READ_YM_ADMINDB_CMD).arg(GS_START_CMD));//send YMRD:START:
    GSLOG(SYSLOG_SEV_DEBUG, QString("Starting communication with daemon : %1 ").arg(answer).toLatin1().constData());


    //excpected error: //sprintf(vd_msg, "%s Can not find .gexlm_config.xml file", STOP_CMD);=>Returning error vd_msg = "STOP: ....."
    if(answer.isNull() || answer.isEmpty() || !answer.startsWith(GS_NEXT_CMD)  )
    {
        daemonError = answer;
        GSLOG(SYSLOG_SEV_DEBUG, QString("getYieldManDbSettings fail : %1 : %2").arg(getYMAdminDBConfig).arg(daemonError).toLatin1().constData());
        return false;
    }

    //expected OK  sprintf(vd_msg, "%s%ld", NEXT_CMD,fileSize);//Returning the file size vd_msg = "NEXT:XXXX...."

    QString daemonData;
    int daemonStringLength = answer.section(GS_NEXT_CMD,1,1).toInt() + QString(GS_END_CMD).size();// file size + last string appended with
    int partCount = (daemonStringLength / GS_LC_VSEND_MESSAGE_LENGTH) +
                    ((daemonStringLength % GS_LC_VSEND_MESSAGE_LENGTH) ? 1 : 0);

    GSLOG(SYSLOG_SEV_DEBUG, QString("daemonStringLength %1  , partCount %2").arg(daemonStringLength).arg(partCount).toLatin1().constData());


    for(int partIdx=0; partIdx < partCount; ++partIdx)
    {
        //sending "YMRD:NEXT:'..PART_NUMBER...'"
        answer =  clientDaemonQA(QString("%1%2%3").arg(GS_READ_YM_ADMINDB_CMD).arg(GS_NEXT_CMD).arg(QString::number(partIdx)));

        if(answer.startsWith(GS_STOP_CMD))//sprintf(GS_vd_msg, "%sCan not read find .gexlm_config.xml file", GS_STOP_CMD);
        {
            daemonError = answer;
            daemonData.clear();
            GSLOG(SYSLOG_SEV_DEBUG, QString("getYieldManDbSettings fail : %1 : %2").arg(getYMAdminDBConfig).arg(daemonError).toLatin1().constData());
            return false;
        }
        else if(answer.startsWith(GS_END_CMD))//sprintf(GS_vd_msg, "%s%s", GS_END_CMD,lastPart);//its the last string prpended with GS_END_CMD
        {
            daemonData+= answer.section(GS_END_CMD,1,1);
            break;
        }
        else
        {
            daemonData+= answer;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("getYieldManDbSettings : %1 : %2").arg(getYMAdminDBConfig).arg(daemonData).toLatin1().constData());

    if(daemonData.isEmpty())
    {
        getYMAdminDBConfig = "GEX_YieldManDbSettings|NotConfigured";
    }
    else
    {
        QXmlStreamReader stream(daemonData);

        QString	strKeyword;
        QString	strParameter;

        QString strHostName;
        QString strDriver;
        QString strDatabaseName;
        QString strUserName;
        QString strPassword;
        QString strSchemaName;
        QString strPort;


        while (!stream.atEnd())
        {
          stream.readNext();
          if(stream.isStartElement() && (stream.name().compare("YieldManDatabase",Qt::CaseInsensitive) == 0))
          {
            while (!stream.atEnd())
            {
              stream.readNext();
              if(stream.isStartElement() && (stream.name().compare("Connection",Qt::CaseInsensitive) == 0))
              {
                foreach(QXmlStreamAttribute attribute, stream.attributes() )
                {
                  strKeyword = attribute.name().toString();
                  strParameter = attribute.value().toString();
                  if(strKeyword.toLower() == "host")
                    strHostName = strParameter;
                  else
                  if(strKeyword.toLower() == "driver")
                    strDriver = strParameter;
                  else
                  if(strKeyword.toLower() == "databasename")
                    strDatabaseName = strParameter;
                  else
                  if(strKeyword.toLower() == "username")
                    strUserName = strParameter;
                  else
                  if(strKeyword.toLower() == "userpassword")
                    strPassword = strParameter;
                  else
                  if(strKeyword.toLower() == "schemaname")
                    strSchemaName = strParameter;
                  else
                  if(strKeyword.toLower() == "port")
                    strPort = strParameter;
                }
              }
            }
          }
        }


        if(strHostName.isEmpty())
        {
            getYMAdminDBConfig = "GEX_YieldManDbSettings|NotConfigured";	// Failed reading file.
        }
        else
        {
            getYMAdminDBConfig = strHostName + "|";
            getYMAdminDBConfig+= strDriver + "|";
            getYMAdminDBConfig+= strDatabaseName + "|";
            getYMAdminDBConfig+= strUserName + "|";
            getYMAdminDBConfig+= strPassword + "|";
            getYMAdminDBConfig+= strSchemaName + "|";
            getYMAdminDBConfig+= strPort;
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("getYieldManDbSettings : %1 : %2").arg(getYMAdminDBConfig).arg(daemonData).toLatin1().constData());

    return true;
}

bool FNPLicenseProvider::setYieldManDbSettings(const QString &setYMAdminDBConfig, QString &daemonError)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start with (%1)").arg(setYMAdminDBConfig).toLatin1().constData());

    QString connection =  setYMAdminDBConfig.section(";",1,1);

    QString hostName;
    QString driver;
    QString databaseName;
    QString userName;
    QString password;
    QString schemaName;
    QString port;

    if(connection.count("|") < 6)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("setYieldManDbSettings fail due to  connection.count(|) < 6 : %1 : %2").arg(setYMAdminDBConfig).arg(daemonError).toLatin1().constData());
        return false;
    }

    hostName        = connection.section("|",0,0);
    driver          = connection.section("|",1,1);
    databaseName    = connection.section("|",2,2);
    userName        = connection.section("|",3,3);
    password        = connection.section("|",4,4);
    schemaName      = connection.section("|",5,5);
    port            = connection.section("|",6,6);

    QString stringToDaemon;

    QXmlStreamWriter stream( &stringToDaemon);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    // Start definition marker
    stream.writeStartElement("YieldManDatabase");
    // Fill file with Database details...
    stream.writeEmptyElement("Connection");
    stream.writeAttribute("Host",hostName);
    stream.writeAttribute("Driver",driver);
    stream.writeAttribute("DatabaseName",databaseName);
    stream.writeAttribute("UserName",userName);
    stream.writeAttribute("UserPassword",password);
    stream.writeAttribute("SchemaName",schemaName);
    stream.writeAttribute("Port",port);
    // End definition marker
    stream.writeEndElement();
    stream.writeEndDocument();


    const int unitSize = GS_LC_VSEND_MESSAGE_LENGTH - (QString(GS_WRITE_YM_ADMINDB_CMD).size() +QString(GS_CONTINUE_CMD).size());
    int partCount = (stringToDaemon.count() / unitSize) +
                    ((stringToDaemon.count()% unitSize ) ? 1 : 0);

    //Init the communication send = "YMWR:START:......."
    QString answer = clientDaemonQA(QString("%1%2").arg(GS_WRITE_YM_ADMINDB_CMD).arg(GS_START_CMD));
    //Expected KO not started with GS_NEXT_CMD
    if(answer.isNull() || answer.isEmpty() || !answer.startsWith(GS_NEXT_CMD))
    {
        daemonError = answer;
        GSLOG(SYSLOG_SEV_DEBUG, QString("setYieldManDbSettings fail due to  answer.isNull() || answer.isEmpty() || answer!=NEXT : %1 : %2").arg(setYMAdminDBConfig).arg(daemonError).toLatin1().constData());
        return false;
    }

    //Expected OK start with GS_NEXT_CMD
    for(int partIdx=0; partIdx < partCount; ++partIdx)
    {
        QString part = stringToDaemon.mid(partIdx*unitSize,unitSize);
        //send = "YMWR:CONTINUE:...filePart..." <=>  send GS_WRITE_YM_ADMINDB_CMD+GS_CONTINUE_CMD+filePart
        answer = clientDaemonQA(QString("%1%2%3").arg(GS_WRITE_YM_ADMINDB_CMD).arg(GS_CONTINUE_CMD).arg(part));//Send this part
        if(answer.isNull() || answer.isEmpty() || !answer.startsWith(GS_NEXT_CMD))        //Result KO !GS_NEXT_CMD
        {
            daemonError = answer;
            GSLOG(SYSLOG_SEV_DEBUG, QString("setYieldManDbSettings fail due to  answer.isNull() || answer.isEmpty() || answer!=CONTINUE : %1 : %2").arg(setYMAdminDBConfig).arg(daemonError).toLatin1().constData());
            return false;
        }
        //Result OK GS_NEXT_CMD => NEXT
    }

    //FINISH Writing notify daemon with  "YMWR:END:......."
    answer = clientDaemonQA(QString("%1%2").arg(GS_WRITE_YM_ADMINDB_CMD).arg(GS_END_CMD));//Send this part

    if(answer.isNull() || answer.isEmpty() || !answer.startsWith(GS_END_CMD))        //Result KO !GS_END_CMD
    {
        daemonError = answer;
        GSLOG(SYSLOG_SEV_DEBUG, QString("setYieldManDbSettings fail due to  answer.isNull() || answer.isEmpty() || answer!=CONTINUE : %1 : %2").arg(setYMAdminDBConfig).arg(daemonError).toLatin1().constData());
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("setYieldManDbSettings OK: %1 : %2").arg(setYMAdminDBConfig).arg(daemonError).toLatin1().constData());
    return true;
}


QString FNPLicenseProvider::buildUserNotificationMessage()
{
    GEX_ASSERT(false);
    return QString();
}

void FNPLicenseProvider::launchActivation(const QString &AdditionalMessage, bool showDialog)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start launchActivation ").toLatin1().constData());
    int status = 0;
    if(showDialog)
    {
        QString lAdditionalInfo = "";
        if(!AdditionalMessage.isEmpty())
        {
            lAdditionalInfo = AdditionalMessage + QString("\n");
        }
        QStringList userChoiceList = QStringList()<< lAdditionalInfo + QString("Cannot find a Standalone license installed for the asked product. \n"
                                                                               "If you would like to activate your node-locked licenses press Activate to launch the activation utility\n")
                                                  << "Activate"
                                                  << "Exit";
        status = 1;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Start userChoice ").toLatin1().constData());
        emit userChoice(&userChoiceList, status);
        GSLOG(SYSLOG_SEV_DEBUG, QString("Start userChoice status (%1)").arg(status).toLatin1().constData());
    }
    if(status == 0)
    {
        //QProcess activationUtility;
        QString actAppDir = getAppConfigData("ApplicationDir").toString() + QDir::separator() ;//+ "actutil";
        QString utilityName = GS_ACT_UTILITY;
        QStringList utilityOptions = QStringList() << "-appact";

#ifdef QT_DEBUG
        utilityName += "d";
#endif
#ifdef Q_OS_WIN
        utilityName = utilityName + ".exe";
#endif

        utilityName = actAppDir + QDir::separator() + utilityName;

        GSLOG(SYSLOG_SEV_DEBUG, QString("startDetached actAppDir(%1) utilityName(%2) utilityOptions (%3)").arg(actAppDir).arg(utilityName).arg(utilityOptions.join(",")).toLatin1().constData());

//        userChoiceList = QStringList()<< "The activation utility will be launched to activate your license.\nThe application will now exit."
//                                      << "Ok";
//        emit userChoice(&userChoiceList, status);

        qint64 pid = 0;
        bool processStatus = QProcess::startDetached(utilityName, utilityOptions,actAppDir,&pid);
        GSLOG(SYSLOG_SEV_DEBUG, QString("Start processStatus status (%1) pid(%2)").arg(processStatus).arg(pid).toLatin1().constData());
    }


    GSLOG(SYSLOG_SEV_DEBUG, QString("End launchActivation ").toLatin1().constData());

}


void FNPLicenseProvider::DownloadLicenseFile(const QString &entitelemntId,const QString & hostName, const QString &hostIds, QString &licContent, int &error, QString &errorMessage)
{
    QString lLicFile = getAppConfigData("UserFolder").toString() + QDir::separator() + hostIds + ".lic";
    //Generate a unique licfile name
    lLicFile = GS::LPPlugin::DownloadLicense::GenerateFileName(lLicFile);

    //Build the download url
    QString lDownloadLicenseUrl = QString(sLicenseDownloadUrl)
                                + QString("?oid=%1&hn=%2&hid=%3").arg(entitelemntId).arg(hostName).arg(hostIds);

    //Show a waiting message during license download
    QString lUserMessage = "Application is downloading the license. Please Wait ...";
    int lShow = 1;
    emit ShowWaitingMessage(lUserMessage,lShow);

    //Instantiate the dwonload license class by providing the URL and the lic file path
    GS::LPPlugin::DownloadLicense lDownloadLic(lDownloadLicenseUrl,lLicFile);

    error = lDownloadLic.DownloadLicenseFile(licContent,errorMessage);

    GSLOG(SYSLOG_SEV_DEBUG, QString("DownloadLicenseFile returned (%1) lError (%2) lMessage (%3)")
          .arg(error).arg(error).arg(errorMessage).toLatin1().constData());
    lShow = 0;
    emit ShowWaitingMessage(lUserMessage,lShow);

}

void FNPLicenseProvider::LaunchEvaluationActivation(bool &exitGex)
{
    exitGex = false;
    getHostID_function getHostId = (getHostID_function)(mPrivate->mFNPFctPointer[LIBLP_HOSTID_FCT]);

    QString lMessage = QString("Your evaluation license cannot be found in the directory  \"%1\".\n\n"
                               "If you would like to activate your evaluation license enter your \"Order ID\" and press \"Activate\" button.\n")
                      .arg(getAppConfigData("UserFolder").toString());

    QString entitelemntId = "";
    emit userEvalLicense(&lMessage, &entitelemntId);

    if(!entitelemntId.isEmpty())
    {
        QString hostIds = getHostId();
        hostIds = hostIds.remove("\"");
        QString hostId = hostIds.split(" ").first();
        QString hostName = QHostInfo::localHostName();

        //Try to download the file directly.
        int lError = GS::LPPlugin::DownloadLicense::DOWNLOAD_NO_ERROR;
        QString lMessage = "No error";
        QString lLicContent;
        DownloadLicenseFile(entitelemntId, hostName, hostId, lLicContent, lError, lMessage);
        if(lError == GS::LPPlugin::DownloadLicense::DOWNLOAD_NO_ERROR)
        {
			// No error occured during the license donwload
            exitGex = false;
        }
        else
        {
			// an error occured during the license donwload so use the old way to generate a license
            QString activationUrl = sEvaluationActivation + QString("?oid=%1&hn=%2&hid=%3").arg(entitelemntId).arg(hostName).arg(hostId);

            QString urlData ="";
#ifdef QT_DEBUG
            urlData = QString("<a href='%1'>%1</a>").arg(activationUrl);
#endif

            emit openURL(&activationUrl);
            QStringList userChoiceList = QStringList()<< QString("The application is unable to retrieve your license file due to network issue.\n"
                                                                 "A web browser window will be oppened and you will be asked to generate and save your license file.\n"
                                                                 "After saving the .lic license file to your user directory \"%1\".\n"
                                                                 "Press the \"Next\" button to start the application or press \"Exit\" to exit the application")
                                                         .arg(getAppConfigData("UserFolder").toString())
                                                      << "Next"
                                                      << "Exit";
            int status = 1;
            emit userChoice(&userChoiceList, status);
            if(status == 0)
            {
                exitGex = false;
            }
            else
            {
                exitGex = true;
            }
        }

    }
    else
    {
        exitGex = true;
    }

}

// Fix me: respect the new flow
QString FNPLicenseProvider::GetFloatingLicensePath()
{
    QString lServerIP = getLPData("ServerIP").toString();
    if(lServerIP.isEmpty())
        return "";
    else
        return QString("license server(") + lServerIP + QString(")");
}

QString FNPLicenseProvider::getLicenseFiles(const QString &licensesDirPath)
{
    QDir licensesDir = QDir(licensesDirPath);
    QStringList files = licensesDir.entryList(QStringList(QString("*.lic")), QDir::Files);

    QString licSep = ":";
#ifdef Q_OS_WIN
    licSep = ";";
#endif

    if(files.isEmpty())
        return QString("");
    else
    {
        QStringList licFiles;
        foreach(QString lic, files)
        {
            licFiles.append(licensesDirPath + QDir::separator() + lic);
        }

        return licFiles.join(licSep);
    }
}

/*This function checks the license using the messages sent to lmadmin.
* Get the license mode from the gui
* Check the historical parameters saved in the file gs_lp.xml
* if ((welcome mode) or (not deamon) or (not hidden mode))
    show the welcome gui
    depends on the mode (standalone or floating), fill the gui fields
    call bool FNPLicenseProvider::reserve(ProductInfo *productInfo, const QStringList &increments)
        This function will reserve the license of the product with the appropriate increment
    if retrieve license, fill the needed field for gex and save the configuration.*/
bool FNPLicenseProvider::checkLicense(ProductInfo *productInfo)
{
    QString lErrorMsg;

    GSLOG(SYSLOG_SEV_DEBUG, QString("::checkLicense Start %1").arg(getProduct()).toLatin1().constData());

    if(!productInfo)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Internal error due to unspecified product" );
        setInternalError(LPMessage::eReject,"Internal error due to unspecified product");
        return false;
    }

    LicenseProvider::GexProducts lProduct = productInfo->getProductID();
    // Make sure we are not running in TER mode
    if((lProduct == eTerOEM) || (lProduct == eTerProPlus))
    {
        lErrorMsg = QString("You cannot use this license provider (fnp_lp) with a Teradyne edition.");
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    if(!mPrivate->mProductIncrement.contains(getProduct()))
    {
        lErrorMsg = QString("The requested product/edition (%1) is not supported by this license provider").arg((int)lProduct);
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    productInfo->setLicenseRunningMode(mPrivate->mLicenseRunningMode);

    if(!checkStartupArguments(productInfo))
    {
        GSLOG(SYSLOG_SEV_CRITICAL, QString("Finish with Internal Error startup argument not recognized").toLatin1().constData());
        //emit sendLPMessage(LPMessage(LPMessage::eReject,"Internal Error"));
        setInternalError(LPMessage::eReject,"Internal Error startup argument not recognized");
        return false;
    }

    // If LTXC running mode, accept the license
    if(lProduct == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        mPrivate->mProductInfo = productInfo;
        emit sendLPMessage(LPMessage(LPMessage::eAccept,"The license is granted..."));
        return true;
    }

    bool lForceWelcomeMode = false;

    QString lLastChoice = mServersDescription->GetLastChoice();

    if(lLastChoice.isEmpty())
    {
        lForceWelcomeMode = true;
    }
    else
    {
        bool lConnectionStatus = mServersDescription->GetConnectionStatus();
        //bool lConnectionStatus = GetLastChoiceStatus();
        GSLOG(SYSLOG_SEV_DEBUG, QString("lConnectionStatus (%1)").arg(lConnectionStatus ? "true" : "false").toLatin1().constData());
        if(!lConnectionStatus)
        {
            lForceWelcomeMode = true;
        }
    }

    // Fill the gui fields
    bool lIsDAEMON = getAppConfigData("isDAEMON").toBool();
    bool lHiddenMode = getAppConfigData("HiddenMode").toBool();
    GSLOG(SYSLOG_SEV_DEBUG, QString("lForceWelcomeMode (%1) WelcomeMode (%2) lIsDAEMON(%3) lHiddenMode(%4)").arg(lForceWelcomeMode ? "true" : "false")
          .arg(getAppConfigData("WelcomeMode").toBool() ? "true" : "false" ).arg(lIsDAEMON).arg(lHiddenMode).toLatin1().constData());
    if( (lForceWelcomeMode || getAppConfigData("WelcomeMode").toBool())
         && ( !getAppConfigData("isDAEMON").toBool() && !getAppConfigData("HiddenMode").toBool())
      )
    {
        bool notify = false;
        bool ret = false;
        emit userInteraction (this, &(mPrivate->mEditField1), NULL,
                              &(mPrivate->miSelectionMode), notify, ret, getLPData("EnableLegacyButton").toBool());
        GSLOG(SYSLOG_SEV_DEBUG, QString("userInteraction  ret(%1)").arg(ret ? "true" : "false").toLatin1().constData());
        if(!ret)
        {
            // License file not correct or doesn't exist...
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with license activation cancelled by user").toLatin1().constData());
            setLPData("Exit", QVariant(true));
            setInternalError(LPMessage::eReject,""/*"license activation cancelled by user"*/);
            return false;
        }
        else
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("miSelectionMode(%1) mEditField1(%2)"/* mEditField2(%3)"*/)
                  .arg(mPrivate->miSelectionMode)
                  .arg(mPrivate->mEditField1)
                  /*.arg(mPrivate->mEditField2)*/.toLatin1().constData());

            setLicPath_function licPath= (setLicPath_function)(mPrivate->mFNPFctPointer[LIBLP_LIC_PATH_FCT]);
            if(mPrivate->miSelectionMode == GEX_RUNNINGMODE_STANDALONE)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("checkLicense :setLicPath with (%1)").arg("").toLatin1().constData());
                licPath(QString("").toLatin1().data());

                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);
                mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
            }
            // Floating mode
            else if(mPrivate->miSelectionMode == GEX_RUNNINGMODE_CLIENT)
            {
                QVector<QString>    lServerList;
                QVector<int>        lPortList;
                buildPortsServersListFromWelcomeString(mPrivate->mEditField1, lServerList, lPortList);
                QString lAllServers = buildFormattedServersString(lServerList, lPortList);
                GSLOG(SYSLOG_SEV_DEBUG, QString("checkLicense: setLicPath with (%1)").arg(QString("%1"/*@%2"*/).arg(mPrivate->mEditField1).toLatin1().data()).toLatin1().constData());
                licPath(lAllServers.toLatin1().data());

                //licPath(QString("%1"/*@%2"*/).arg(mPrivate->mEditField1)/*.arg(mPrivate->mEditField2)*/.toLatin1().data());
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);
                setLPData("ServerIP", lAllServers);
               // setLPData("ServerName", mPrivate->mEditField1);
                //setLPData("SocketPort", mPrivate->mEditField2);
                mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eFloating;
            }
            else
            {
                QString evalLicenseFiles = getLicenseFiles(getAppConfigData("UserFolder").toString());
                GSLOG(SYSLOG_SEV_DEBUG, QString("evalLicenseFiles: (%1)").arg(evalLicenseFiles).toLatin1().constData());
                licPath(QString(evalLicenseFiles).toLatin1().data());
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
                GSLOG(SYSLOG_SEV_DEBUG, "createEvaluationLicense");
                mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
                createEvaluationLicense(productInfo);
            }
        }
    }
    else
    {
        //QString lastChoice = getLastChoice().toString();
        if(!lLastChoice.isNull() &&  !lLastChoice.isEmpty())
        {
            if(lLastChoice.startsWith("Evaluation", Qt::CaseInsensitive))
            {
                QString evalLicenseFiles = getLicenseFiles(getAppConfigData("UserFolder").toString());
                GSLOG(SYSLOG_SEV_DEBUG, QString("evalLicenseFiles: (%1)").arg(evalLicenseFiles).toLatin1().constData());
                setLicPath_function licPath= (setLicPath_function)(mPrivate->mFNPFctPointer[LIBLP_LIC_PATH_FCT]);
                licPath(QString(evalLicenseFiles).toLatin1().data());
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
                GSLOG(SYSLOG_SEV_DEBUG, "createEvaluationLicense");
                mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
                createEvaluationLicense(productInfo);
            }
        }
    }

    mPrivate->mProductInfo = productInfo;
    QStringList increments = QStringList()<< mPrivate->mProductIncrement[getProduct()];
    //increments += mPrivate->mOptionIncrement.values();

    GSLOG(SYSLOG_SEV_DEBUG, QString("Start reserving %1").arg(increments.join(" , ")).toLatin1().constData());

    setLPData("ExpirationDate",getAppConfigData("ExpirationDate"));
    setLPData("ReleaseDate",getAppConfigData("ReleaseDate"));
    setLPData("MaintenanceExpirationDate",getAppConfigData("MaintenanceExpirationDate"));
    setLPData("LicenseType", "");

    bool lReserveStatus = reserve(productInfo, increments);

    // check all the servers before going in the next steps
    if (!lReserveStatus
            && mPrivate->mLicenseDenialCode == LM_MAXUSERS
            && mPrivate->miSelectionMode == GEX_RUNNINGMODE_CLIENT)
    {
        QVector<QString>    lServers;
        QVector<int>        lPorts;
        buildPortsServersListFromWelcomeString(mPrivate->mEditField1, lServers, lPorts);
        setLicPath_function licPath= (setLicPath_function)(mPrivate->mFNPFctPointer[LIBLP_LIC_PATH_FCT]);
        for (int i=0; i<std::min(lServers.size(), lPorts.size()); ++i)
        {
            QString lServer = buildFormattedServerString(i, lServers, lPorts);
            licPath(lServer.toLatin1().data());
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);
            setLPData("ServerIP", lServer);
            mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eFloating;
            lReserveStatus = reserve(productInfo, increments);
            if(lReserveStatus)
                break;
        }
    }

    //if the checkout fails in standalone mode Check if other product modes are available
    bool lStandAloneExit = false;
    bool lStandloneActivate = false;
    bool lAlternativeProdUsed = false;
    if(!lReserveStatus && productInfo->getLicenseRunningMode() == GEX_RUNNINGMODE_EVALUATION)
    {
        if(getProduct() == LicenseProvider::eExaminatorEval)
        {
            bool exitGex = false;
            if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone
                    &&(mPrivate->mLicenseDenialCode == LM_NOCONFFILE/*-1*/
                       || mPrivate->mLicenseDenialCode == LM_NOFEATURE/*-5*/))
            {
                setInternalError(LPMessage::eReject,"");
                LaunchEvaluationActivation(exitGex);
                if(exitGex)
                {
                    lReserveStatus = false;
                }
                else
                {
                    QString evalLicenseFiles = getLicenseFiles(getAppConfigData("UserFolder").toString());
                    GSLOG(SYSLOG_SEV_DEBUG, QString("evalLicenseFiles: (%1)").arg(evalLicenseFiles).toLatin1().constData());
                    setLicPath_function licPath= (setLicPath_function)(mPrivate->mFNPFctPointer[LIBLP_LIC_PATH_FCT]);
                    licPath(QString(evalLicenseFiles).toLatin1().data());
                    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
                    GSLOG(SYSLOG_SEV_DEBUG, "createEvaluationLicense");
                    mPrivate->mLicenseMode = FNPLicenseProviderPrivate::eStandalone;
                    createEvaluationLicense(productInfo);
                    mPrivate->mProductInfo = productInfo;
                    QStringList increments = QStringList()<< mPrivate->mProductIncrement [getProduct()];
                    //increments += mPrivate->mOptionIncrement.values();

                    GSLOG(SYSLOG_SEV_DEBUG, QString("Start reserving %1").arg(increments.join(" , ")).toLatin1().constData());

                    setLPData("ExpirationDate",getAppConfigData("ExpirationDate"));
                    setLPData("ReleaseDate",getAppConfigData("ReleaseDate"));
                    setLPData("MaintenanceExpirationDate",getAppConfigData("MaintenanceExpirationDate"));
                    setLPData("LicenseType", "");

                    lReserveStatus = reserve(productInfo, increments);
                }
            }

            if(!exitGex && mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone
                    && mPrivate->mLicenseDenialCode == LM_LONGGONE /*-10*/)
            {
                setInternalError(LPMessage::eReject,"");
                QStringList lUserChoiceList = QStringList()
                        << QString("This evaluation License has expired. Please contact %1 to renew.").arg(GEX_EMAIL_SALES)
                        << "Ok";
                int lStatus;
                emit userChoice(&lUserChoiceList, lStatus);
            }
            else if(!exitGex)
            {
                setInternalError(LPMessage::eReject,QString("Can not find your Evaluation license under \"%1\" location or error due to : %2 \n")
                                 .arg(getAppConfigData("UserFolder").toString())
                                 .arg(QString("%1:%2").arg(mPrivate->mLicenseDenialCode).arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr))));
            }
        }
    }
    else if(!lReserveStatus && ((mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone)
                      || (mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eFloating
                            && mPrivate->mBasicIncrement.contains(mPrivate->mProductIncrement [getProduct()]))))
    {
        int lOriginalDenial = mPrivate->mLicenseDenialCode;
        QString lOriginalDenialStr = mPrivate->mLicenseDenialStr;

        QString lCurrentIncrement = mPrivate->mProductIncrement [getProduct()];

        if (mPrivate->mLicenseDenialCode == LM_NOCONFFILE /*-1*/
            || mPrivate->mLicenseDenialCode == LM_NOFEATURE/*-5*/
            || mPrivate->mLicenseDenialCode == LM_NOSERVSUPP /*-18*/)
        {
            QString lLicTypeMessage = "Cannot retrieve Standalone license.";
            if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eFloating)
                lLicTypeMessage = QString("Cannot retrieve floating license from %1.").arg(GetFloatingLicensePath());

            QString lUserMessage = QString("You have attempted to launch a Quantix Product using the shortcut for (%1). "
                                           "\nWe cannot find a license for this Examinator (%2). "
                                           "\nHowever we have found a license for another version of Examinator. "
                                           "\nYou can click \"Use\" below to use that license. "
                                           "\nIn the future you may want to use the correct shortcut for the version of Examinator that you are trying to launch.")
                    .arg(getAppConfigData("AppFullName").toString().section(" - ",0,0).simplified()).arg(getAppConfigData("AppFullName").toString());

            QString lMessageError = lLicTypeMessage + QString("\n %1 \n")
                    .arg(QString("Error %1: %2")
                         .arg(mPrivate->mLicenseDenialCode)
                         .arg(lUserMessage));
            mPrivate->mAlternativeIncrement.clear();
            for(int lIdx=0; lIdx<mPrivate->mBasicIncrement.count();++lIdx)
            {
                QString lAlternative = mPrivate->mBasicIncrement[lIdx];
                if( lAlternative != lCurrentIncrement)
                {
                    increments = QStringList() << lAlternative;
                    bool lAlternativeStatus = reserve(productInfo, increments);
                    if(lAlternativeStatus)
                    {
                        mPrivate->mAlternativeIncrement.append(lAlternative);
                    }
                }
            }

            if(!mPrivate->mAlternativeIncrement.isEmpty())
            {
                QString lCPURetValue;
                bool lActivateButton = (mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone);
                emit userAvailableChoices(&lMessageError, &(mPrivate->mAlternativeIncrement),lActivateButton,&lCPURetValue);
                QStringList lCPUValues = lCPURetValue.split("|",QString::SkipEmptyParts);
                int lCPUChoice = 1;
                QString lCPUProduct;
                if(lCPUValues.count()>0)
                {
                    lCPUChoice = lCPUValues[0].toInt();
                    if(lCPUValues.count()>1)
                    {
                        lCPUProduct = lCPUValues[1];
                    }
                }
                if(lCPUChoice == 1)
                {
                    mPrivate->mLicenseDenialCode = lOriginalDenial;
                    mPrivate->mLicenseDenialStr = lOriginalDenialStr;
                    lReserveStatus = false;
                    lStandloneActivate = true;
                    setInternalError(LPMessage::eReject,"");
                }
                else if(lCPUChoice == 2)
                {
                    setInternalError(LPMessage::eReject,"");
                    lReserveStatus = false;
                    lStandAloneExit = true;
                }
                else
                {
                    for(int lIdx=0; lIdx<mPrivate->mAlternativeIncrement.count();++lIdx)
                    {
                        QString lAlternative = mPrivate->mAlternativeIncrement[lIdx];
                        if( lAlternative != lCPUProduct)
                        {
                            releaseIncrement(lAlternative);
                        }
                    }
                    lAlternativeProdUsed = true;
                    lReserveStatus = true;
                    LicenseProvider::GexProducts lProductToBeUsed = mPrivate->mProductIncrement.key(lCPUProduct);
                    setProduct(lProductToBeUsed);
                    productInfo->setProductID(lProductToBeUsed);
                }
            }
        }
        else
        {
            mPrivate->mLicenseDenialCode = lOriginalDenial;
            mPrivate->mLicenseDenialStr = lOriginalDenialStr;
        }
    }


    if(!lReserveStatus)
    {
        SaveLastChoiceStatus(false);
        GSLOG(SYSLOG_SEV_DEBUG, QString("The license is not allowed due to  : \n %1 : %2").arg(mPrivate->mLicenseDenialCode).arg(mPrivate->mLicenseDenialStr).toLatin1().constData());
        GSLOG(SYSLOG_SEV_CRITICAL, QString("The license is not allowed due to  : \n %1 : %2").arg(mPrivate->mLicenseDenialCode).arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr)).toLatin1().constData());

        if(getProduct() != LicenseProvider::eExaminatorEval)
        {
            productInfo->setProductID(eNoProduct);
            if(!getAppConfigData("isDAEMON").toBool() && !getAppConfigData("HiddenMode").toBool())
            {
                QString addtionalData;
                if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone &&( mPrivate->mLicenseDenialCode == LM_NOCONFFILE /*-1*/ || mPrivate->mLicenseDenialCode == LM_NOFEATURE/*-5*/))
                {
                    addtionalData = "Please use the activation utility launched to activate your licenses and then restart the application";
                }
                setInternalError(LPMessage::eReject,"");

                if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eStandalone)
                {
                    if(lStandAloneExit)
                    {
                        setInternalError(LPMessage::eReject,"");
                    }
                    else if(mPrivate->mLicenseDenialCode == LM_NOCONFFILE /*-1*/ || mPrivate->mLicenseDenialCode == LM_NOFEATURE/*-5*/)
                    {
                        QString lMessageError = QString("Cannot retrieve Standalone license.\n %1 \n")
                                .arg(QString("Error %1: %2")
                                     .arg(mPrivate->mLicenseDenialCode)
                                     .arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr)));
                        bool lShowMessage = true;
                        if(lStandloneActivate)
                            lShowMessage = false;
                        else
                            lMessageError = "";
                        launchActivation(lMessageError, lShowMessage);
                    }
                    else
                    {
                        QStringList lUserChoiceList = QStringList()
                                << QString("Cannot retrieve Standalone license.\n %1 \n")
                                   .arg(QString("Error %1: %2").arg(mPrivate->mLicenseDenialCode).arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr)))
                                << "Exit";
                        int lStatus;
                        emit userChoice(&lUserChoiceList, lStatus);
                    }

                }

                if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eFloating)
                {
                    QStringList userChoiceList = QStringList()<< QString("Cannot retrieve floating license from %1. %2 \n"
                                                                         "\nPress Exit and check your license manager")
                                    .arg(GetFloatingLicensePath())
                                    .arg(QString("Error %1: %2").arg(mPrivate->mLicenseDenialCode)
                                    .arg(mPrivate->userErrorMessage(mPrivate->mLicenseDenialCode,mPrivate->mLicenseDenialStr)))
                                                              << "Exit";
                    int status = 1;
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Start userChoice ").toLatin1().constData());
                    emit userChoice(&userChoiceList, status);
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Start userChoice status (%1)").arg(status).toLatin1().constData());
                }
            }
        }
    }
    else
    {
        if(!lForceWelcomeMode && !getAppConfigData("WelcomeMode").toBool()) {
           ConvertAndSaveToJsonFormat();
        }

        if(lAlternativeProdUsed)
            SaveLastChoiceStatus(false);
        else
            SaveLastChoiceStatus(true);

        GSLOG(SYSLOG_SEV_DEBUG, QString("Succeeded to reserve %1").arg(getProduct()).toLatin1().constData())
                productInfo->setProductID(getProduct());
        mPrivate->mDisconnectedFeature.clear();
        if(getLPData("LicenseType").toString() == "nodelocked_uncounted")//"nodelocked", "floating"
        {
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);
            if(productInfo->isExaminatorEval())
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
        }
        else  if(getLPData("LicenseType").toString() == "nodelocked_counted")//"nodelocked", "floating"
        {
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);
        }
        else
        {
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);
            if(productInfo->isMonitoring())
                productInfo->setMonitorProducts(1);

            GSLOG(SYSLOG_SEV_DEBUG, QString("Test Daemon : %1").arg(clientDaemonQA(GS_PING_CMD)).toLatin1().constData());
        }

        productInfo->setEditionID(GEX_EDITION_ADV);


        QString strLOG = QString("\n\nLP Provider Details %1").arg(property(LP_FRIENDLY_NAME).toString());
        strLOG += QString("\n\nEditionID: %1").arg(productInfo->getEditionID());
        strLOG += QString("\nRunningMode: %1").arg(productInfo->getLicenseRunningMode());
        strLOG += QString("\nMonitorProducts: %1").arg(productInfo->getMonitorProducts());
        strLOG += QString("\nOptionalModules: %1").arg(productInfo->getOptionalModules());
        strLOG += QString("\nProductID: %1\n\n").arg(productInfo->getProductID());

        strLOG += QString("\n ExpirationDate: %1\n\n").arg(getLPData("ExpirationDate").toString());
        strLOG += QString("\n ReleaseDate: %1\n\n").arg(getLPData("ReleaseDate").toString());
        strLOG += QString("\n MaintenanceExpirationDate: %1\n\n").arg(getLPData("MaintenanceExpirationDate").toString());
        strLOG += QString("\n LicenseType: %1\n\n").arg(getLPData("LicenseType").toString());

        GSLOG(SYSLOG_SEV_DEBUG, QString("Result of checkout : %1").arg(strLOG).toLatin1().constData());

        QString acceptData = "";

        if(mPrivate->mInPassivMode)
        {
            acceptData = "License:PassiveMode";

            if(mPrivate->mActiveStatusUpdate)
            {
                mPrivate->mActiveStatusUpdate->deleteLater();
                mPrivate->mActiveStatusUpdate = 0;
            }
            mPrivate->mActiveStatusUpdate = new QTimer;
            mPrivate->mActiveStatusUpdate->moveToThread(thread());
            bool connectStatus = connect(mPrivate->mActiveStatusUpdate, SIGNAL(timeout()), this, SLOT(checkActiveStatusUpdate()) );
            GSLOG(SYSLOG_SEV_DEBUG, QString("connectStatus (%1)").arg(connectStatus).toLatin1().constData());

            mPrivate->mActiveStatusUpdate->start(FNP_ACTIV_PASSIV_STATUS_INTERVAL);
        }

        emit sendLPMessage(LPMessage(LPMessage::eAccept,acceptData));
    }

    if(lReserveStatus && getProduct() != LicenseProvider::eExaminatorEval)
        notifyUserOnExpiration(getLPData("ExpirationDate").toDate());

    GSLOG(SYSLOG_SEV_DEBUG, QString("::checkLicense Finish with return value(%1)").arg(lReserveStatus).toLatin1().constData());
    return lReserveStatus;
}

void FNPLicenseProvider::checkActiveStatusUpdate()
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start checkActiveStatusUpdate ").toLatin1().constData());

    QString activeIncrement = mPrivate->mProductIncrement[getProduct()];
    QString passivIncrement = mPrivate->mPassivIncrement[getProduct()];

    QString appVersion = "1.0";

    bool ret = reserveIncrement(activeIncrement.toLatin1().constData(), appVersion.toLatin1().constData() /*getAppConfigData("AppVersion").toString().toLatin1().constData()*/,1,0);
    if(!ret)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Fail to go to active mode with product (%1)").arg(activeIncrement).toLatin1().constData());
        mPrivate->mActiveStatusUpdate->start(FNP_ACTIV_PASSIV_STATUS_INTERVAL);
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Succed to go to active mode with product (%1)").arg(activeIncrement).toLatin1().constData());
        mPrivate->mReservedIncrement.append(activeIncrement);
        mPrivate->mReservedIncrement.removeAll(passivIncrement);
        releaseIncrement(passivIncrement);
        mPrivate->mActiveStatusUpdate->stop();
        mPrivate->mInPassivMode = false;
        QString messageData = "License:ActiveMode";
        emit sendLPMessage(LPMessage(LPMessage::eExtended, messageData));
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("End checkActiveStatusUpdate ").toLatin1().constData());
}

void FNPLicenseProvider::setInternalError(int i, const QString &m)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start ").toLatin1().constData());
    mPrivate->mInternalError = i;
    mPrivate->mInternalErrorMessage = m;
    GSLOG(SYSLOG_SEV_DEBUG, QString("FINISH ").toLatin1().constData());
}

int FNPLicenseProvider::getInternalError(QString &m)
{
    m = mPrivate->mInternalErrorMessage;
    return mPrivate->mInternalError;
}

bool FNPLicenseProvider::reserve(ProductInfo *productInfo, const QStringList &increments)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start ::reserve %1").arg(increments.join(" , ")).toLatin1().constData());

    QString compilationRawDate = QString(__DATE__).simplified();
    QDate compilationDate = mPrivate->toDate(QString("%1-%2-%3").arg(compilationRawDate.section(" ",1,1)).arg(compilationRawDate.section(" ",0,0)).arg(compilationRawDate.section(" ",2,2)));//"dd-MMM-yyyy "

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("__DATE__ %1 compilationRawDate %2 compilationDate %3 compilationDate.isValid %4")
          .arg(__DATE__)
          .arg(compilationRawDate).arg(compilationDate.toString("MMM d yyyy")).arg(compilationDate.isValid())
          .toLatin1().constData()
          );


    QString appVersion = "1.0";

    release(mPrivate->mReservedIncrement);
    mPrivate->mReservedIncrement.clear();

    if(mPrivate->mLicenseMode == FNPLicenseProviderPrivate::eFloating)
    {
        QString increment = "GEX-Init";
        reserveIncrement(increment.toLatin1().constData(), appVersion.toLatin1().constData(),1,0);
    }


    bool datesInitialized = false;
    char *licenseInternalInfo[5];
    for(int idx = 0; idx<5; idx++)
    {
        licenseInternalInfo[idx] = new char [10000];
        (licenseInternalInfo[idx])[0] = '\0';
    }
    foreach(QString increment, increments)
    {
        char **licenseInfo = 0;
        if(!datesInitialized)
            licenseInfo = (char **)licenseInternalInfo;

        bool ret = reserveIncrement(increment.toLatin1().constData(), appVersion.toLatin1().constData() /*getAppConfigData("AppVersion").toString().toLatin1().constData()*/,1,licenseInfo);
        //Check if we are under monitoring (ym/pm) try passiv mode
        if(!ret && (getProduct() == eYieldMan || getProduct() == eYieldManEnterprise
                    || getProduct() == ePATMan || getProduct() == ePATManEnterprise))
        {
            increment = mPrivate->mPassivIncrement[getProduct()];
            ret = reserveIncrement(increment.toLatin1().constData(), appVersion.toLatin1().constData()/* getAppConfigData("AppVersion").toString().toLatin1().constData()*/,1,licenseInfo);

        }

        if(!ret)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail due to increment (%1)").arg(increment).toLatin1().constData());

            mPrivate->mLicenseDenialCode = ((lastErrorCode_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT]))();
            mPrivate->mLicenseDenialStr = ((lastError_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_FCT]))();

            release(mPrivate->mReservedIncrement);
            mPrivate->mReservedIncrement.clear();
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail %1:%2").arg(mPrivate->mLicenseDenialCode ).arg(mPrivate->mLicenseDenialStr).toLatin1().constData());
            return false;
        }
        else
        {
            if(!datesInitialized)
            {
                if(licenseInfo)
                {
                    //0 expiration date
                    //1 release date
                    //2 maintenance date
                    //3 license type
                    //4 overdrat status
                    QString expirationDate = QString(licenseInfo[0]).toUpper();
                    int year = expirationDate.section("-",2,2).toInt();

                    if(year == 0)
                        expirationDate = "PERMANENT";

                    QString releaseDate = QString(licenseInfo[1]).toUpper();
                    QString maintenanceDate = QString(licenseInfo[2]).toUpper();
                    QString licenseType = QString(licenseInfo[3]);
                    QString overdraftStatus = QString(licenseInfo[4]);

                    if(expirationDate == "PERMANENT")
                    {
                        GSLOG(SYSLOG_SEV_DEBUG, QString("License (%1) is perpetual so try to check Maintenance date").arg(increment).toLatin1().constData());
                        QString maintenanceIncrement = mPrivate->mMaintenanceIncrement[getProduct()];
                        ret = reserveIncrement(maintenanceIncrement, appVersion.toLatin1().constData(),1,licenseInfo);


                        if(!ret)
                        {
                            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail due to increment (%1)").arg(maintenanceIncrement).toLatin1().constData());

                            mPrivate->mLicenseDenialCode = ((lastErrorCode_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT]))();
                            mPrivate->mLicenseDenialStr = ((lastError_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_FCT]))();

//                            release(mPrivate->mReservedIncrement);
//                            mPrivate->mReservedIncrement.clear();
                            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail %1:%2").arg(mPrivate->mLicenseDenialCode ).arg(mPrivate->mLicenseDenialStr).toLatin1().constData());
                            QDate lMaintenanceExprationDate;
                            bool lMaintenanceExist = getMaintenanceExpirationDate(licenseType, lMaintenanceExprationDate);
                            GSLOG(SYSLOG_SEV_DEBUG, QString(">>  getMaintenanceExpirationDate Return  %1").arg(lMaintenanceExist).toLatin1().constData());


                            if(!lMaintenanceExist)
                            {
                                //Reject it the maintanance expire license can not be used anymore.
                                mPrivate->mLicenseDenialStr = "Perpetual license rejected because no valid maintenance contract found";
                                mPrivate->mLicenseDenialCode= LM_LONGGONE /*-10*/;

                            release(mPrivate->mReservedIncrement);
                            mPrivate->mReservedIncrement.clear();

                                return false;
                            }

                            QDate maintenanceIncExpDate = lMaintenanceExprationDate;

                            GSLOG(SYSLOG_SEV_DEBUG, QString("maintenanceIncExpDate %1 compilationDate %2")
                                  .arg(maintenanceIncExpDate.toString("MMM d yyyy"))
                                  .arg(compilationDate.toString("MMM d yyyy")).toLatin1().constData());

                            if(compilationDate > maintenanceIncExpDate)
                            {
                                //Reject it the maintanance expire license can not be used anymore.
                                mPrivate->mLicenseDenialStr = "Perpetual license rejected because no valid maintenance contract";
                                mPrivate->mLicenseDenialStr += " (maintenance expired: " + maintenanceIncExpDate.toString("MMM dd yyyy") + " < ";
                                mPrivate->mLicenseDenialStr += compilationDate.toString("MMM dd yyyy") + ")";

                                mPrivate->mLicenseDenialCode= LM_LONGGONE /*-10*/;

                                release(mPrivate->mReservedIncrement);
                                mPrivate->mReservedIncrement.clear();

                                return false;
                            }
                            maintenanceDate = maintenanceIncExpDate.toString();
                        }
                        else
                        {
                            QString maintenanceIncExp = QString(licenseInfo[0]).toUpper();
                            QDate maintenanceIncExpDate = mPrivate->toDate(maintenanceIncExp);

                            GSLOG(SYSLOG_SEV_DEBUG, QString("maintenanceIncExpDate %1 compilationDate %2")
                                  .arg(maintenanceIncExpDate.toString("MMM d yyyy"))
                                  .arg(compilationDate.toString("MMM d yyyy")).toLatin1().constData());

                            if(compilationDate > maintenanceIncExpDate)
                            {
                                //Reject it the maintanance expire license can not be used anymore.
                                mPrivate->mLicenseDenialStr = "Perpetual license rejected because no valid maintenance contract";
                                mPrivate->mLicenseDenialStr += " (maintenance expired: " + maintenanceIncExpDate.toString("MMM dd yyyy") + " < ";
                                mPrivate->mLicenseDenialStr += compilationDate.toString("MMM dd yyyy") + ")";

                                mPrivate->mLicenseDenialCode= LM_LONGGONE /*-10*/;

                                release(mPrivate->mReservedIncrement);
                                mPrivate->mReservedIncrement.clear();

                                return false;
                            }
                            maintenanceDate = maintenanceIncExp;
                        }
                    }

                    GSLOG(SYSLOG_SEV_DEBUG, QString("getDates: expirationDate(%1) releaseDate(%2) maintenanceDate(%3) overdraftStatus(%4)")
                          .arg(expirationDate).arg(releaseDate).arg(maintenanceDate).arg(overdraftStatus).toLatin1().constData());

                    datesInitialized = !expirationDate.isEmpty() && !releaseDate.isEmpty() && !maintenanceDate.isEmpty() && !licenseType.isEmpty();
                    if(!expirationDate.isEmpty()) // dd-mmm-yyyy || permanent || XX-XXX-0
                    {
                        QDate expiration = mPrivate->toDate(expirationDate);
                        // we have to add 1 day to the expired day to finish at 23:59 instead of 00:00
                        setLPData("ExpirationDate",expiration);
                    }
                    if(!releaseDate.isEmpty())
                    {
                        QDate release;
                        release = mPrivate->toDate(releaseDate);
                        setLPData("ReleaseDate",release);
                    }
                    if(!maintenanceDate.isEmpty())
                    {
                        QDate maintenance;
                        maintenance = mPrivate->toDate(maintenanceDate);
                        setLPData("MaintenanceExpirationDate",maintenance);
                    }
                    else
                    {
                        setLPData("MaintenanceExpirationDate",getLPData("ExpirationDate"));
                    }
                    GSLOG(SYSLOG_SEV_DEBUG, QString("LicenseType: LicenseType(%1)")
                          .arg(licenseType).toLatin1().constData());
                    if(getProduct() == LicenseProvider::eExaminatorEval)
                        setLPData("LicenseType", "nodelocked_uncounted");//"nodelocked_uncounted"
                    else
                        setLPData("LicenseType", licenseType);//"nodelocked", "floating"

                    QString daemonAnswer = clientDaemonQA(GS_PING_CMD);
                    GSLOG(SYSLOG_SEV_DEBUG, QString("Test daemon >>App send : (%1) and receive (%2)")
                          .arg(GS_PING_CMD).arg(daemonAnswer).toLatin1().constData());

                    if(overdraftStatus == "overdraft_license")
                    {
                        setLPData("OverdraftStatus", true);
                    }

                }
            }

        }
        mPrivate->mReservedIncrement.append(increment);

        if(getProduct() == eYieldMan || getProduct() == eYieldManEnterprise
                || getProduct() == ePATMan || getProduct() == ePATManEnterprise)
        {
            if(mPrivate->mReservedIncrement.contains(mPrivate->mPassivIncrement[getProduct()]))
            {
                mPrivate->mInPassivMode = true;
            }
        }
    }
#ifdef QT_DEBUG
    if(getLPData("LicenseType").toString() ==  "floating")
    {
        QString lServerTimeAnswer = clientDaemonQA(GS_SERVER_TIME);
        GSLOG(SYSLOG_SEV_DEBUG, QString("lServerTimeAnswer : %1").arg(lServerTimeAnswer).toLatin1().constData());
    }
#endif
    if(!productInfo->isExaminatorEval())
    {
        if(getLPData("LicenseType").toString() ==  "floating")
        {
            //Check global timeout
            QString lClientGlobalTimeout = clientDaemonQA(GS_CLIENT_TIMEOUT);
            GSLOG(SYSLOG_SEV_DEBUG, QString("lClientGlobalTimeout : %1").arg(lClientGlobalTimeout).toLatin1().constData());
            if(!lClientGlobalTimeout.isEmpty())
            {
                QString lTimeoutString = lClientGlobalTimeout.section(GS_CLIENT_TIMEOUT,1,1);
                bool lOk = false;
                int lTimeoutValue = lTimeoutString.toInt(&lOk);
                if(!lTimeoutString.isEmpty() && lOk && lTimeoutValue>0)
                {
                    setLPData("GlobalTimeout", QVariant(lTimeoutValue));
                }
            }
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Client Global Timeout is not supported by the Galaxylm vendor daemon.").toLatin1().constData());

            //Check Timeshift between client and server.
            QString lServerTimeAnswer = clientDaemonQA(GS_SERVER_TIME);
            if(!lServerTimeAnswer.isEmpty())
            {
                QString lServerTimeTString = lServerTimeAnswer.section(GS_SERVER_TIME,1,1);
                int lYear   = lServerTimeTString.section(":",0,0).toInt()+1900;
                int lMonth  = lServerTimeTString.section(":",1,1).toInt()+1;
                int lDay    = lServerTimeTString.section(":",2,2).toInt();
                int lHour   = lServerTimeTString.section(":",3,3).toInt();
                int lMin    = lServerTimeTString.section(":",4,4).toInt();
                //int lIsdst  = lServerTimeTString.section(":",5,5).toInt();

                QDate lServerDate(lYear, lMonth, lDay);
                QTime lServerTime(lHour,lMin);

                QDateTime lServerDateTime(lServerDate,lServerTime);
                QDateTime lClientDateTime = QDateTime::currentDateTime();
                qint64 lTimeShift = qAbs(lServerDateTime.secsTo(lClientDateTime));
                lTimeShift = lTimeShift/60;

                GSLOG(SYSLOG_SEV_DEBUG, QString("Server Time(%1) and Client Time (%2) lTimeShift(%3 min)")
                      .arg(lServerDateTime.toString("dd MM yyyy hh:mm:ss"))
                      .arg(lClientDateTime.toString("dd MM yyyy hh:mm:ss"))
                      .arg(lTimeShift).toLatin1().constData());


                if(lTimeShift > mPrivate->mDefaultAllowedTimeShift)
                {
                    //Check if the license contains a WW Increment
                    // WW Increment Name = PRODUCT-WW
                    //(GEX-WW, GEX-Pro-WW, GEX-PAT-WW, Yield-Man-WW, PAT-Man-WW, GTM-WW )
                    QString lMainIncrement = increments.first() + "-WW";
                    bool lRet = reserveIncrement(lMainIncrement.toLatin1().constData(), appVersion.toLatin1().constData(),1,0);
                    if(!lRet)
                    {
                        mPrivate->mLicenseDenialStr = QString("You are trying to connect to a site license and are not co-located with the license server. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade.");
                        mPrivate->mLicenseDenialCode= LM_TZ_UNAUTHORIZED /*-188*/;
                        release(mPrivate->mReservedIncrement);
                        mPrivate->mReservedIncrement.clear();
                        return false;
                    }
                }
            }
            else
                GSLOG(SYSLOG_SEV_WARNING, QString("Time shift is not supported by the Galaxylm vendor daemon.").toLatin1().constData());
        }

        // GCORE-1674
        // GTM does not need the default options (SYA, TDR, GENE...)
        // I know this solution is not a long term solution but our current softwares are on the way to die so no time to loose.
        if (productInfo->isGTM())
            mPrivate->mOptionIncrement.clear();

        foreach(LicenseProvider::GexOptions option, mPrivate->mOptionIncrement.keys())
        {
            QString optionStr = mPrivate->mOptionIncrement[option];
            char **licenseInfo = 0;
            if(getLPData("LicenseType").toString() !=  "floating")
            {
                licenseInfo = (char **)licenseInternalInfo;
            }

            bool ret = reserveIncrement(optionStr.toLatin1().constData(), appVersion.toLatin1().constData()/*getAppConfigData("AppVersion").toString().toLatin1().constData()*/,1,licenseInfo);
            if(!ret)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Fail to reserve option increment (%1)").arg(optionStr).toLatin1().constData());
                int lDenialCode = ((lastErrorCode_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT]))();
                QString lDenialStr = ((lastError_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_FCT]))();
                GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail %1:%2").arg(lDenialCode ).arg(lDenialStr).toLatin1().constData());
            }
            else
            {
                mPrivate->mReservedIncrement.append(mPrivate->mOptionIncrement[option]);
                productInfo->setOptionalModules(option);
                //Update license type if needed.
                if(licenseInfo)
                {
                    QString licenseType = QString(licenseInfo[3]);
                    GSLOG(SYSLOG_SEV_DEBUG, QString("LicenseType from option: LicenseType(%1)")
                          .arg(licenseType).toLatin1().constData());
                    if(licenseType == "floating")
                    {
                        setLPData("LicenseType", licenseType);//"nodelocked", "floating"

                        QString daemonAnswer = clientDaemonQA(GS_PING_CMD);
                        GSLOG(SYSLOG_SEV_DEBUG, QString("Test daemon >>App send : (%1) and receive (%2)")
                              .arg(GS_PING_CMD).arg(daemonAnswer).toLatin1().constData());
                    }

                }
            }
        }

    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("qDeleteAll").toLatin1().constData());

    for(int idx = 0; idx<5; idx++)
    {
       delete [](licenseInternalInfo[idx]);
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with success by reserving %1").arg(mPrivate->mReservedIncrement.join(", ")).toLatin1().constData());
    return true;
}

void FNPLicenseProvider::release(const QStringList &increments)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    QStringList listOfIncrement;
    listOfIncrement.append("Empty List");
    if(!increments.isEmpty())
    {
        listOfIncrement.clear();
        foreach(QString increment, increments)
        {
            releaseIncrement(increment);
        }
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with releasing %1").arg(listOfIncrement.join(", ")).toLatin1().constData());
}

bool FNPLicenseProvider::createEvaluationLicense(ProductInfo *productInfo)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
    productInfo->setProductID(LicenseProvider::eExaminatorEval);
    setProduct(LicenseProvider::eExaminatorEval);
    GSLOG(SYSLOG_SEV_DEBUG, "Finish");
    return false;
}


/// This function get the last choose from the gs_lp.xml
/// Fill available modes in the returned list
QVariantList FNPLicenseProvider::getRunningModeAvailable()
{
    QVariantList availableMode ;

    GexProducts lProductType = getProduct();

    if( (lProductType != ePATMan) && (lProductType != ePATManEnterprise) &&
        (lProductType != eYieldMan) && (lProductType != eYieldManEnterprise) && (lProductType != eGTM))
    {
        const QString& lStandaloneString = mServersDescription->GetServerDescriptorString("fnp_lp", standalone);
        if(!lStandaloneString.isEmpty())
            availableMode.append(QString("Standalone|%1").arg(lStandaloneString));
        else
            availableMode.append(QString("Standalone|%1").arg("Standalone"));
    }

    const QString& lFloatingString = mServersDescription->GetServerDescriptorString("fnp_lp", floating);
    if(!lFloatingString.isEmpty())
        availableMode.append(QString("Connect to server (Client/Server)|%1").arg(lFloatingString));
    else
        availableMode.append(QString("Connect to server (Client/Server)|Floating|Server(s):Port(s)|localhost:27000"));

    /*const QString& lEvaluationString = mServersDescription->GetServerDescriptorString("fnp_lp", evaluation);
    if(!lEvaluationString.isEmpty())
        availableMode.append(QString("Evaluation (4 days)|%1").arg(lEvaluationString));
    else
        availableMode.append(QString("Evaluation (4 days)|Evaluation"));*/

    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ").toLatin1().constData());
    return availableMode;
}

#define GEX_HTMLPAGE_FNP_HLP_STARTUP	"_gexstd_startup_fnp.htm"

QString FNPLicenseProvider::getWelcomePage()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    QString page = GEX_HTMLPAGE_FNP_HLP_STARTUP;

    QString strSource = getAppConfigData("ApplicationDir").toString() + GEX_HELP_FOLDER;
    strSource += page;
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ").toLatin1().constData());
    return strSource;
}

bool FNPLicenseProvider::checkStartupArguments(ProductInfo *productInfo)
{
    if(!productInfo)
        return false;

    if(productInfo->getProductID() == GS::LPPlugin::LicenseProvider::eLtxcOEM)
    {
        // LTXC running mode is activated with a secret command line, it doesn't need a license file.
        productInfo->setEditionID(GEX_EDITION_STD);
        productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);

        QDate ed;
        // GCORE-2950: set a dynamic expiration date.
        // ed.setDate(2014,12,31);
        ed = QDate::currentDate().addYears(10);
        setLPData("ExpirationDate", ed);
        return true;
    }

    return true;
}

int FNPLicenseProvider::getExtendedError(QString &m)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start ").toLatin1().constData());
    //m = QString("%1 : %2").arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage);
    m = QString("%1").arg(mPrivate->mInternalErrorMessage);
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ").toLatin1().constData());
    return mPrivate->mInternalError;
}

bool FNPLicenseProvider::reserveIncrement(const QString &feature, const QString &version, int nlic, char *internalInfo[])
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start reserveIncrement reserving feature(%1) version(%2)").arg(feature).arg(version).toLatin1().constData());

    reserveLibrary_function reserveFct = (reserveLibrary_function)(mPrivate->mFNPFctPointer[LIBLP_RESERVE_FCT]);
    if(reserveFct)
    {
        bool ret = reserveFct(feature.toLatin1().constData(), version.toLatin1().constData(),nlic, internalInfo);
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish reserveIncrement reserve fct finish with ret (%1)").arg(ret).toLatin1().constData());
        return ret;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish reserveIncrement Can not found reserve function").toLatin1().constData());
        return false;
    }

}

bool FNPLicenseProvider::RequestGTLToken(int library, QString &info)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start RequestGTLToken ").toLatin1().constData());

    QString lGTLIncrement = mPrivate->mAdditionalIncrement[getProduct()];

    if(lGTLIncrement.isEmpty())
    {
        info  = QString("Internal error increment not avaialable for current product %1").arg(getProduct());
        return false;
    }

    QString appVersion = "1.0";
    int lLicNumber = mPrivate->mGTLLibrary.count() + 1;

    bool lRet = reserveIncrement(lGTLIncrement.toLatin1().constData(), appVersion.toLatin1().constData() /*getAppConfigData("AppVersion").toString().toLatin1().constData()*/,lLicNumber,0);
    if(!lRet)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Fail to reserve increment (%1)").arg(lGTLIncrement).toLatin1().constData());
        int lDenialCode = ((lastErrorCode_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT]))();
        QString lDenialStr = ((lastError_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_FCT]))();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail %1:%2").arg(lDenialCode ).arg(lDenialStr).toLatin1().constData());
        info  = QString("Can not retrieve GTL license. Error %1: %2").arg(mPrivate->userErrorMessage(lDenialCode,lDenialStr)).arg(lDenialCode);
        GSLOG(SYSLOG_SEV_DEBUG, QString("End RequestGTLToken ").toLatin1().constData());
        return false;
    }
    else
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Succed to retrieve GTL license for library (%1)").arg(library).toLatin1().constData());
        mPrivate->mGTLLibrary.append(library);
        info = "License Retrieved";
        GSLOG(SYSLOG_SEV_DEBUG, QString("End RequestGTLToken ").toLatin1().constData());
        return true;
    }
}

void FNPLicenseProvider::ReleaseGTLToken(int library)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start RequestGTLToken ").toLatin1().constData());

    if(!mPrivate->mGTLLibrary.contains(library))
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("End ReleaseGTLToken Library %1 not found ").arg(library).toLatin1().constData());
        return;
    }

    QString lGTLIncrement = mPrivate->mAdditionalIncrement[getProduct()];

    if(lGTLIncrement.isEmpty())
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("End ReleaseGTLToken Internal error increment not avaialable for current product"
                                        " %1 Library %2 not found ").arg(getProduct()).arg(library).toLatin1().constData());
        return;
    }

    releaseIncrement(lGTLIncrement);
    mPrivate->mGTLLibrary.removeOne(library);
    if(mPrivate->mGTLLibrary.count())
    {
        QString appVersion = "1.0";
        int lLicNumber = mPrivate->mGTLLibrary.count();
        bool lRet = reserveIncrement(lGTLIncrement.toLatin1().constData(), appVersion.toLatin1().constData() /*getAppConfigData("AppVersion").toString().toLatin1().constData()*/,lLicNumber,0);
        if(!lRet)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Fail to reserve increment (%1)").arg(lGTLIncrement).toLatin1().constData());
            int lDenialCode = ((lastErrorCode_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT]))();
            QString lDenialStr = ((lastError_function)(mPrivate->mFNPFctPointer[LIBLP_LAST_ERROR_FCT]))();
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish ::reserve with fail %1:%2").arg(lDenialCode ).arg(lDenialStr).toLatin1().constData());
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("End ReleaseGTLToken ").toLatin1().constData());
}

void FNPLicenseProvider::releaseIncrement(const QString &feature)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start releaseIncrement releasing  feature(%1)").arg(feature).toLatin1().constData());
    releaseLibrary_function releaseFct = (releaseLibrary_function)(mPrivate->mFNPFctPointer[LIBLP_RELEASE_FCT]);
    if(releaseFct)
    {
        releaseFct(feature.toLatin1().constData());
    }
    GSLOG(SYSLOG_SEV_DEBUG, QString("Finish releaseIncrement releasing  feature(%1)").arg(feature).toLatin1().constData());
}

void FNPLicenseProvider::disconnectFromLM(const QString &m, const QString &disconnectedFeature)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(">> Emit disconnectFromLM with message :<%1>").arg(m).toLatin1().constData());
    if(mPrivate->mDisconnectedFeature.isEmpty())
    {
        mPrivate->mDisconnectedFeature.append(disconnectedFeature);
        emit sendLPMessage(LPMessage(LPMessage::eDisconnected,m));
    }

    if(!mPrivate->mDisconnectedFeature.contains(disconnectedFeature))
        mPrivate->mDisconnectedFeature.append(disconnectedFeature);
}

void FNPLicenseProvider::reconnectToLM(const QString &m)
{
    mPrivate->mDisconnectedFeature.clear();
    emit sendLPMessage(LPMessage(LPMessage::eAccept,m));
}

void FNPLicenseProvider::notifyUserOnExpiration(const QDate &expirationDate)
{
    QString lMessage;

    QDate lCurrentDate = QDate::currentDate();
    int lDaysCount = lCurrentDate.daysTo(expirationDate);

    if(lDaysCount <= 10)
    {
        lMessage = QString("License expiration date is less then 10 days.\nTo avoid any service interruption, please"
                "contact us at %1, and we will help you with the renewal process.").arg(GEX_EMAIL_SALES);

    }
    else if(lDaysCount < 30)
    {
        lMessage = QString("License expiration date is less then 30 days.\nTo avoid any service interruption, please"
                "contact us at %1, and we will help you with the renewal process.").arg(GEX_EMAIL_SALES);
    }

    if(!lMessage.isEmpty())
    {
        bool lHiddenMode = getAppConfigData("HiddenMode").toBool();

        // GCORE-1289 HTH
        // Do not popup any dialog when hidden mode. Notify the customer about a close expiration date in the log
        if (lHiddenMode)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, lMessage.toLatin1().constData());
        }
        else
        {
            QStringList lUserChoiceList = QStringList()<< lMessage
                                                       << "Ok";
            int lStatus;
            emit userChoice(&lUserChoiceList, lStatus);
        }
    }
}

void FNPLicenseProvider::DirectRequest(const GEXMessage &gexMessage,LPMessage &lpMessage)
{
    GSLOG(SYSLOG_SEV_WARNING, QString("DirectRequest GEXMessage(%1 , %2)").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());
    if(gexMessage.getType() == GEXMessage::eExit)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEXMessage::eExit %1").arg(gexMessage.getData()).toLatin1().constData());
        release(mPrivate->mReservedIncrement);
        mPrivate->mReservedIncrement.clear();
    }
    else if(gexMessage.getType() == GEXMessage::eExtended)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEXMessage::eExtended => %1").arg(gexMessage.getData()).toLatin1().constData());

        QString lGexMessage = gexMessage.getData();
        if(lGexMessage.startsWith(GTLOperations))
        {
            lGexMessage = lGexMessage.section(GTLOperations,1,1);
            if(lGexMessage.startsWith(GTLRequestLicense))
            {
                //Message Data will look like this => "GTL_REQUEST:libraryID";
                QString lLibraryID = lGexMessage.section(GTLRequestLicense,1,1);
                QString info;
                bool lRes = RequestGTLToken(lLibraryID.toInt(),info);
                QString lResponse;
                if(lRes)
                {
                    lResponse = QString ("%1%2").arg(GTLOperations).arg(GTLResponseLicense);
                }
                else
                {
                    lResponse = QString ("%1%2").arg(GTLOperations).arg(GTLResponseNOLicense);
                }
                lpMessage = LPMessage(LPMessage::eExtended,lResponse+lLibraryID+":"+info);
            }
            else if(lGexMessage.startsWith(GTLReleaseLicense))
            {
                //Message Data will look like this => "GTL_RELEASE:libraryID";
                QString lLibraryID = lGexMessage.section(GTLReleaseLicense,1,1);
                ReleaseGTLToken(lLibraryID.toInt());
            }
        }
    }

    GSLOG(SYSLOG_SEV_WARNING, QString("DirectRequest LPMessage(%1 , %2)").arg(lpMessage.getType()).arg(lpMessage.getData()).toLatin1().constData());
}

FNPLicenseProviderPrivate::FNPLicenseProviderPrivate(const QVariantMap &appConfigData)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");

    initErrorMessagesMap();
    mLicenseMode = eStandalone;
    mDefaultAllowedTimeShift = 90;
    QCoreApplication::addLibraryPath(appConfigData["ApplicationDir"].toString()
            + QDir::separator() + LP_GEX_PLUGIN_DIR
            + QDir::separator() + LP_SUB_PLUGIN_DIR);

    mLibraryLoader.setFileName(LIB_GEX_FNP_PROXY);

    mLicenseDenialStr = "";
    mLicenseDenialCode = LM_NOCONFFILE/*-1*/;

    mInternalError = FNPLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";
    mProductInfo = 0;
    mAppConfigData = appConfigData;
    initCodeMapping();

    if(mLibraryLoader.load())
    {
        if(!getResSym())
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
            return ;
        }

        foreach(QString strSymbol, sSymbolToBeResolved)
        {
            if(!addSymbol(strSymbol))
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
                return ;
            }
        }

        //Check if all function are ok
        if((initLibrary_function)(mFNPFctPointer[LIBLP_INIT_FCT])
                && (cleanupLibrary_function)(mFNPFctPointer[LIBLP_CLEANUP_FCT])
                && (reserveLibrary_function)(mFNPFctPointer[LIBLP_RESERVE_FCT])
                && (releaseLibrary_function)(mFNPFctPointer[LIBLP_RELEASE_FCT])
                && (lastError_function)(mFNPFctPointer[LIBLP_LAST_ERROR_FCT])
                && (lastErrorCode_function)(mFNPFctPointer[LIBLP_LAST_ERROR_CODE_FCT])
                && (ClientDaemonQA_function)(mFNPFctPointer[LIBLP_CLIENT_DAEMON_QA_FCT])
                && (clientIdle_function)(mFNPFctPointer[LIBLP_CLIENT_IDLE_FCT])
                && (setLicPath_function)(mFNPFctPointer[LIBLP_LIC_PATH_FCT])
                && (setConnectionStatusCallBack_function)(mFNPFctPointer[LIBLP_CONNECTION_STATUS_FCT])
                && (getHostID_function)(mFNPFctPointer[LIBLP_HOSTID_FCT])
                && (getLibVersion_function)(mFNPFctPointer[LIBLP_VERSION_FCT]))
        {
            //Initialize the library
            int iInternaErrorLib = 0;
            char szInternalError[1000];
            initLibrary_function initLibraryFct =  (initLibrary_function)(mFNPFctPointer[LIBLP_INIT_FCT]);
            if(!initLibraryFct(iInternaErrorLib, szInternalError, (void *)logerCallBack))
            {
                mInternalError = FNPLicenseProvider::eLPInternalError;
                mInternalErrorMessage = QString("Internal Error when initializing the FNP library : Error(%1) Message(%2)").arg(iInternaErrorLib).arg(szInternalError);
                GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
            }
        }
        else
        {
            mInternalError = FNPLicenseProvider::eLPSymbolNotFound;
            mInternalErrorMessage = QString("Internal Error when resolving FNP library functions");
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
        }
    }
    else
    {
        mInternalError = FNPLicenseProvider::eLPLibNotFound;
        mInternalErrorMessage = mLibraryLoader.errorString();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
    }
    mActiveStatusUpdate = 0;
    mInPassivMode = false;
    mLicenseRunningMode = GEX_RUNNINGMODE_EVALUATION;
}

FNPLicenseProviderPrivate::~FNPLicenseProviderPrivate()
{
    mGalaxyLicenseMessages.clear();

    if(mFNPFctPointer.contains(LIBLP_CLEANUP_FCT))
    {
        cleanupLibrary_function cleanLib = (cleanupLibrary_function)(mFNPFctPointer[LIBLP_CLEANUP_FCT]);
        if(cleanLib)
            cleanLib();
    }

    sSymbolToBeResolved.clear();
    mFNPFctPointer.clear();
#   ifndef Q_OS_MAC  // GCORE-1314
    mLibraryLoader.unload();
#   endif
}

bool FNPLicenseProviderPrivate::getResSym()
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");

    if(mFNPFctPointer.contains(LIBLP_RESOLVER_SYMBOL) && mFNPFctPointer[LIBLP_RESOLVER_SYMBOL])
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with Success %1 already resolved").arg(LIBLP_RESOLVER_SYMBOL).toLatin1().constData());
        return true;
    }
    void *resSym = 0;
    resSym = (void*)mLibraryLoader.resolve(LIBLP_RESOLVER_SYMBOL);
    resSym_function resSymFct = (resSym_function)resSym;

    if(!resSym || !resSymFct)
    {
        mInternalError = FNPLicenseProvider::eLPSymbolNotFound;
        mInternalErrorMessage = QString("Symbol %1 not found due to : ").arg(LIBLP_RESOLVER_SYMBOL) + mLibraryLoader.errorString();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
        return false;
    }
    else
    {
        mFNPFctPointer.insert(LIBLP_RESOLVER_SYMBOL, resSym);
        GSLOG(SYSLOG_SEV_DEBUG, "Finish with Success");
        return true;
    }
}

bool FNPLicenseProviderPrivate::addSymbol(const QString &strAddSymbol)
{
    GSLOG(SYSLOG_SEV_DEBUG, "Start");
    resSym_function resSymFct = 0;
    if(!mFNPFctPointer.contains(LIBLP_RESOLVER_SYMBOL))
    {
        if(!getResSym())
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
            return false;
        }
    }

    resSymFct = (resSym_function)mFNPFctPointer[LIBLP_RESOLVER_SYMBOL];

    void *funcPointer = 0;
    funcPointer = resSymFct(strAddSymbol.toLatin1().constData());
    if(!funcPointer)
    {
        mInternalError = FNPLicenseProvider::eLPSymbolNotFound;
        mInternalErrorMessage = QString("Symbol %1 not found due to : ").arg(strAddSymbol) + mLibraryLoader.errorString();
        GSLOG(SYSLOG_SEV_DEBUG, QString("Finish with error(%1) : %2").arg(mInternalError).arg(mInternalErrorMessage).toLatin1().constData());
        return false;
    }
    else
    {
        mFNPFctPointer.insert(strAddSymbol, funcPointer);
        GSLOG(SYSLOG_SEV_DEBUG, "Finish with Success");
        return true;
    }

}

void FNPLicenseProviderPrivate::initCodeMapping()
{
    //Products
    mProductIncrement.insert(LicenseProvider::eLtxcOEM                  ,"GEX-OEM-LTXC") ;
    mProductIncrement.insert(LicenseProvider::eSzOEM                    ,"GEX-OEM-SZ") ;
    mProductIncrement.insert(LicenseProvider::eExaminatorEval           ,"GEX-Eval") ;
    mProductIncrement.insert(LicenseProvider::eExaminator               ,"GEX") ;
    mProductIncrement.insert(LicenseProvider::eExaminatorPro            ,"GEX-Pro") ;
    mProductIncrement.insert(LicenseProvider::eExaminatorPAT            ,"GEX-PAT") ;
    mProductIncrement.insert(LicenseProvider::eYieldMan                 ,"Yield-Man") ;
    mProductIncrement.insert(LicenseProvider::eYieldManEnterprise       ,"Yield-Man-ENT") ;
    mProductIncrement.insert(LicenseProvider::ePATMan                   ,"PAT-Man") ;
    mProductIncrement.insert(LicenseProvider::ePATManEnterprise         ,"PAT-Man-ENT") ;
    mProductIncrement.insert(LicenseProvider::eGTM                      ,"GTM") ;

    //Additional Options
    mOptionIncrement.insert(LicenseProvider::eSYA                       ,"SYA") ;
    mOptionIncrement.insert(LicenseProvider::eTDR                       ,"TDR") ;
    mOptionIncrement.insert(LicenseProvider::eGenealogy                 ,"GENE") ;

    //Active-Passive Mode
    mPassivIncrement.insert(LicenseProvider::eYieldMan                  ,"Yield-Man-Passive") ;
    mPassivIncrement.insert(LicenseProvider::eYieldManEnterprise        ,"Yield-Man-ENT-Passive") ;
    mPassivIncrement.insert(LicenseProvider::ePATMan                    ,"PAT-Man-Passive") ;
    mPassivIncrement.insert(LicenseProvider::ePATManEnterprise          ,"PAT-Man-ENT-Passive") ;

    //Maintenance Increment
    mMaintenanceIncrement.insert(LicenseProvider::eExaminator           ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::eExaminatorPro        ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::eExaminatorPAT        ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::eYieldMan             ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::eYieldManEnterprise   ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::ePATMan               ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::ePATManEnterprise     ,"Maintenance") ;
    mMaintenanceIncrement.insert(LicenseProvider::eGTM                  ,"Maintenance") ;

    mAdditionalIncrement.insert(LicenseProvider::eGTM                   ,"GTL") ;
    mAdditionalIncrement.insert(LicenseProvider::eExaminatorPAT         ,"GTL") ;

    //Basic Increment
    mBasicIncrement.append("GEX") ;
    mBasicIncrement.append("GEX-Pro") ;
    //mBasicIncrement.append("GEX-PAT") ;

}

QDate FNPLicenseProviderPrivate::toDate(const QString &dates)
{
    //"dd-MMM-yyyy "
    if(dates.isEmpty())
        return QDate();
    if(dates.toUpper() == "PERMANENT")
        return QDate(2050,1,1);

    int day = dates.section("-",0,0).toInt();
    int year = dates.section("-",2,2).toInt();
    if(year == 0)
        return QDate(2050,1,1);

    QString month = dates.section("-",1,1);
    int monthVal = 1;
    if(month.toLower() == "jan") monthVal = 1;
    if(month.toLower() == "feb") monthVal = 2;
    if(month.toLower() == "mar") monthVal = 3;
    if(month.toLower() == "apr") monthVal = 4;
    if(month.toLower() == "may") monthVal = 5;
    if(month.toLower() == "jun") monthVal = 6;
    if(month.toLower() == "jul") monthVal = 7;
    if(month.toLower() == "aug") monthVal = 8;
    if(month.toLower() == "sep") monthVal = 9;
    if(month.toLower() == "oct") monthVal = 10;
    if(month.toLower() == "nov") monthVal = 11;
    if(month.toLower() == "dec") monthVal = 12;
    return QDate(year,monthVal,day);
}

void logerCallBack(int sev, const char *messsage)
{
    GSLOG(sev, messsage);
}

//Check the reconnection
void userExitCallBack(char *feature)
{
    if(FNPLicenseProvider::getInstance()){
        GSLOG(SYSLOG_SEV_DEBUG, QString(">> userExitCallBack with message").toLatin1().constData());
        QString message =
            "Server closed the connection!\n"
            "Either :\n"
            "- the GalaxySemi license manager on the server was closed\n"
            "- the network is experiencing problems\n"
            "- the software was not replying during a too much long time.";
        message = QString("%1|%2").arg(GS::Error::SERVER_CONNECTION_CLOSE).arg(message);
        QString log = message + QString("\n Flexera send feature => %1").arg(feature);
        GSLOG(SYSLOG_SEV_DEBUG, QString(">> userExitCallBack log :<%1>").arg(log).toLatin1().constData());
        FNPLicenseProvider::getInstance()->disconnectFromLM(message, QString(feature));
    }
}

QString FNPLicenseProviderPrivate::userErrorMessage(int errorCode, const QString &fnpMessage)
{
    if(!mGalaxyLicenseMessages.contains(errorCode))
    {
        return fnpMessage;
    }
    else
    {
        QString lGSMessage = mGalaxyLicenseMessages[errorCode];
        if(lGSMessage.isEmpty())
        {
            return fnpMessage;
        }
        else
        {
            return lGSMessage;
        }
    }

}

void FNPLicenseProviderPrivate::initErrorMessagesMap()
{
    mGalaxyLicenseMessages.insert( 21 //	21	"lc_flexinit failed because there was insufficient rights to start the FlexNet Publisher Licensing Service. Resolve this by setting the service to start automatically."
                                   ,"Unable to start FlexNet Publisher Licensing Service. Please go to the service manager and set the service to start automatically.");
    mGalaxyLicenseMessages.insert( 20 //	20	FlexNet Publisher Licensing Service is not installed.
                                   ,"The FlexNet Publisher Licensing Service  is not installed. Please check that you are logged in as a user with admin rights, then relaunch Examinator as the admin (Windows OS) or sudo (Linux). For additional help please visit support.galaxysemi.com or contact Galaxy Semiconductor at "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( 13 //	13	Computed path to required file is too long for Mac OS X operating system.
                                   ,"The file name and path is too long, please choose another installation path/file name.");
    mGalaxyLicenseMessages.insert( 12 //	12	Invalid bundle ID on Mac OS X operating system.
                                   ,"");
    mGalaxyLicenseMessages.insert( 11 //	11	Framework specified by bundle ID was not loaded.
                                   ,"");
    mGalaxyLicenseMessages.insert( 10 //	10	Error creating path from URL.
                                   ,"");
    mGalaxyLicenseMessages.insert( 9 //	9	Error creating URL.
                                   ,"");
    mGalaxyLicenseMessages.insert( 8 //	8	Path string not specified in UTF-8 format.
                                   ,"");
    mGalaxyLicenseMessages.insert( 7 //	7	A call to lc_flexinit is not allowed after a call to lc_flexinit_cleanup.
                                   ,"");
    mGalaxyLicenseMessages.insert( 6 //	6	"The executable has not been prepped with the preptool. For more information about the preptool, see Programming Reference for Trusted Storage-Based Licensing for instructions."
                                   ,"");
    mGalaxyLicenseMessages.insert( 5 //	5	Unable to allocate resources.
                                   ,"");
    mGalaxyLicenseMessages.insert( 4 //	4	Initialization failed.
                                   ,"The FlexNet license utility is not properly installed. Please check that you are logged in as a user with admin rights, then relaunch Examinator as the admin (Windows OS) or sudo (Linux). For additional help please visit support.galaxysemi.com or contact Galaxy Semiconductor at "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( 3 //	3	Unsupported version of the operating system.
                                   ,"");
    mGalaxyLicenseMessages.insert( 2 //	2	Unable to load activation library.
                                   ,"The FlexNet library (fnp_libFNP) is not properly installed. Please check the dependancies for the PATH and LD_LIBRARY_PATH variables. For additional help please visit support.galaxysemi.com or contact Galaxy Semiconductor at "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( 1 //	1	Unable to find activation library.
                                   ,"The FlexNet library (fnp_libFNP) is not properly installed. Please check the dependancies for the PATH and LD_LIBRARY_PATH variables. For additional help please visit support.galaxysemi.com or contact Galaxy Semiconductor at "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_NOERROR //	0	 The was no error. @ref FLEX_ERROR
                                   ,"");
//    mGalaxyLicenseMessages.insert( LM_NOCONFFILE //	-1	 Can't find license file ""++++++++++++++++++++++++++++++++++++++
//                                   For Standalone (node-locked) :
//                                   ++++++++++++++++++++++++++++++++++++++
//                                   Cannot find a Standalone license installed for this Examinator product.
//                                   If you would like to activate your license, press Activate to launch the activation utility
//                                   -----------------------------------------------------------------------------------------------------
//                                   For Evaluation license :
//                                   ++++++++++++++++++++++++++++++++++++++
//                                   Your evaluation license cannot be found under \""%1\"" directory. If you would like to activate your evaluation license, press Activate.
//                                   A web browser window will open and you will be asked to generate and save your license file."");
    mGalaxyLicenseMessages.insert( LM_BADFILE //	-2	 License file corrupted
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_NOSERVER //	-3	 Cannot connect to a license server
                                   ,"No license server found.");
    mGalaxyLicenseMessages.insert( LM_MAXUSERS //	-4	 Maximum number of users reached
                                   ,"All licenses are currently in use by other users. Please try again later.");

//    mGalaxyLicenseMessages.insert( LM_NOFEATURE  //	-5	 No such feature exists ""++++++++++++++++++++++++++++++++++++++
//                                   For Standalone (node-locked) :
//                                   ++++++++++++++++++++++++++++++++++++++
//                                   Cannot find a Standalone license installed for this Examinator product.
//                                   If you would like to activate your license, press Activate to launch the activation utility
//                                   -----------------------------------------------------------------------------------------------------
//                                   For Evaluation license :
//                                   ++++++++++++++++++++++++++++++++++++++
//                                   Your evaluation license cannot be found under \""%1\"" directory. If you would like to activate your evaluation license, press Activate.
//                                   A web browser window will open and you will be asked to generate and save your license file."");
    mGalaxyLicenseMessages.insert( LM_NOSERVICE //	-6	 No TCP/IP service "FLEXlm"
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_NOSOCKET //	-7	 No socket to talk to server on
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_BADCODE //	-8	 Bad encryption code
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_NOTTHISHOST //	-9	 Hostid doesn't match license
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_LONGGONE //	-10	 Software Expired
                                   ,"This license has expired. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_BADDATE //	-11	 Bad date in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_BADCOMM //	-12	 Bad return from server
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_NO_SERVER_IN_FILE //	-13	 No servers specified in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_BADHOST //	-14	 Bad SERVER hostname in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_CANTCONNECT //	-15	 Cannot connect to server
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CANTREAD //	-16	 Cannot read from server
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CANTWRITE //	-17	 Cannot write to server
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_NOSERVSUPP //	-18	 Server does not support this feature
                                   ,"This license has either expired, is not for this feature, or is not for this software version. \nPlease contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_SELECTERR //	-19	 Error in select system call
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_SERVBUSY //	-20	 Application server "busy" (connecting)
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_OLDVER //	-21	 Config file doesn't support this version
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_CHECKINBAD //	-22	 Feature checkin failed at daemon end
                                   ,"Cannot connect to license server");
    mGalaxyLicenseMessages.insert( LM_BUSYNEWSERV //	-23	 Server busy/new server connecting
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_USERSQUEUED //	-24	 Users already in queue for this feature
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_SERVLONGGONE //	-25	 Version not supported at server end
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_TOOMANY //	-26	 Request for more licenses than supported
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_CANTREADKMEM //	-27	 Cannot read /dev/kmem
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CANTREADVMUNIX //	-28	 Cannot read /vmunix
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CANTFINDETHER  //	-29	 Cannot find ethernet device
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_NOREADLIC //	-30	 Cannot read license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_TOOEARLY //	-31	 Start date for feature not reached
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_NOSUCHATTR //	-32	 No such attr for lm_set_attr/ls_get_attr
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADHANDSHAKE //	-33	 Bad encryption handshake with server
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_CLOCKBAD //	-34	 Clock difference too large between client/server
                                   ,"You are trying to connect to a site license and are not co-located with the license server. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade.");
    mGalaxyLicenseMessages.insert( LM_FEATQUEUE //	-35	 We are in the queue for this feature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_FEATCORRUPT //	-36	 Feature database corrupted in daemon
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADFEATPARAM //	-37	 dup_select mismatch for this feature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_FEATEXCLUDE //	-38	 User/host on EXCLUDE list for feature
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_FEATNOTINCLUDE //	-39	 User/host not in INCLUDE list for feature
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_CANTMALLOC //	-40	 Cannot allocate dynamic memory
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NEVERCHECKOUT  //	-41	 Feature never checked out (lm_status())
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADPARAM //	-42	 Invalid parameter
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOKEYDATA //	-43	 No FLEXlm key data
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADKEYDATA //	-44	 Invalid FLEXlm key data
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_FUNCNOTAVAIL //	-45	 FLEXlm function not available
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_DEMOKIT //	-46	 FLEXlm software is demonstration version
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOCLOCKCHECK //	-47	 Clock check not available in daemon
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADPLATFORM //	-48	 FLEXlm platform not enabled
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_DATE_TOOBIG //	-49	 Date too late for binary format
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_EXPIREDKEYS //	-50	 FLEXlm key data has expired
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOFLEXLMINIT //	-51	 FLEXlm not initialized
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOSERVRESP //	-52	 Server did not respond to message
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CHECKOUTFILTERED  //	-53	 Request rejected by vendor-defined filter
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOFEATSET  //	-54	 No FEATURESET line present in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_BADFEATSET  //	-55	 Incorrect FEATURESET line in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_CANTCOMPUTEFEATSET  //	-56	 Cannot compute FEATURESET line
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_SOCKETFAIL //	-57	 socket() call failed
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_SETSOCKFAIL //	-58	 setsockopt() failed
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_BADCHECKSUM //	-59	 message checksum failure
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_SERVBADCHECKSUM  //	-60	 server message checksum failure
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_SERVNOREADLIC  //	-61	 Cannot read license file from server
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_NONETWORK //	-62	 Network software (tcp/ip) not available
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_NOTLICADMIN //	-63	 Not a license administrator
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_REMOVETOOSOON  //	-64	 lmremove request too soon
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADVENDORDATA  //	-65	 Bad VENDORCODE struct passed to lm_init()
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LIBRARYMISMATCH  //	-66	 FLEXlm include file/library mismatch
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NONETOBORROW //	-67	 No licenses to borrow
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_NOBORROWSUPP //	-68	 License BORROW support not enabled
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_NOTONSERVER  //	-69	 FLOAT_OK can't run standalone on SERVER
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROWLOCKED   //	-70	 Meter already being updated for another counter
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BAD_TZ //	-71	 Invalid TZ environment variable
                                   ,"An error has been detected with the system clock. Please contact "+QString(GEX_EMAIL_SUPPORT)+" for assistance.");
    mGalaxyLicenseMessages.insert( LM_OLDVENDORDATA  //	-72	"Old-style"" vendor keys (3-word)
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LOCALFILTER    //	-73	 Local checkout filter requested request
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ENDPATH //	-74	 Attempt to read beyond the end of LF path
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_VMS_SETIMR_FAILED //	-75	 VMS SYS$SETIMR call failed
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_INTERNAL_ERROR //	-76	 Internal FLEXlm error -- Please report
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_BAD_VERSION    //	-77	 Version number must be string of dec float
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_NOADMINAPI     //	-78	 FLEXadmin API functions not available
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_NOFILEOPS      //	-79	 FLEXlm internal error -79
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_NODATAFILE     //	-80	 FLEXlm internal error -80
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_NOFILEVSEND    //	-81	 FLEXlm internal error -81
                                   ,"Internal error. Please contact "+QString(GEX_EMAIL_SUPPORT)+".");
    mGalaxyLicenseMessages.insert( LM_BADPKG //	-82	 Invalid PACKAGE line in license file
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_SERVOLDVER //	-83	 Server FLEXlm version older than client's
                                   ,"The version of the FlexNet Publisher Licensing Service is too old. Please visit "+QString(GEX_EMAIL_SUPPORT)+" to upgrade.");
    mGalaxyLicenseMessages.insert( LM_USER_BASED //	-84	 Incorrect number of USERS/HOSTS INCLUDED in options file -- see server log
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOSERVCAP //	-85	 Server doesn't support this request
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_OBJECTUSED //	-86	 This license object already in use (Java only)
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_MAXLIMIT //	-87	 Checkout exceeds MAX specified in options file
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_BADSYSDATE //	-88	 System clock has been set back
                                   ,"An error has been detected with the system clock. Please contact "+QString(GEX_EMAIL_SUPPORT)+" for assistance.");
    mGalaxyLicenseMessages.insert( LM_PLATNOTLIC //	-89	 This platform not authorized by license
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_FUTURE_FILE //	-90	""Future license file format or misspelling in license file"
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_DEFAULT_SEEDS  //	-91	 ENCRYPTION_SEEDs are non-unique
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SERVER_REMOVED   //	-92	 Server removed during reread, or server hostid mismatch with license
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_POOL  //	-93	 This feature is available in a different license pool
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LGEN_VER  //	-94	 Attempt to generate license with incompatible attributes
                                   ,"This license has either expired, is not for this feature, or is not for this software version. Please contact "+QString(GEX_EMAIL_SALES)+" to renew.");
    mGalaxyLicenseMessages.insert( LM_NOT_THIS_HOST  //	-95	 Network connect to THIS_HOST failed
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_HOSTDOWN  //	-96	 Server node is down or not responding
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_VENDOR_DOWN  //	-97	 The desired vendor daemon is down
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_CANT_DECIMAL  //	-98	 The FEATURE line can't be converted to decimal format
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADDECFILE  //	-99	 The decimal format license is typed incorrectly
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_REMOVE_LINGER  //	-100	 Cannot remove a lingering license
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_RESVFOROTHERS  //	-101	 All licenses are reserved for others
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_BORROW_ERROR  //	-102	 A FLEXid borrow error occurred
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_TSOK_ERR  //	-103	 Terminal Server remote client not allowed
                                   ,"A Standalone license for Quantix products cannot be used with remote desktop. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade to a floating license.");
    mGalaxyLicenseMessages.insert( LM_BORROW_TOOLONG  //	-104	 Cannot borrow that long
                                   ,"The specified borrowing duration is longer than the 30 day maximum. Lease specify a shorter duration.");
    mGalaxyLicenseMessages.insert( LM_UNBORROWED_ALREADY  //	-105	 Feature already returned to server
                                   ,"Borrowed license has already been returned.");
    mGalaxyLicenseMessages.insert( LM_SERVER_MAXED_OUT  //	-106	 License server out of network connections
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_NOBORROWCOMP  //	-107	 Can't borrow a PACKAGE component
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_BORROW_METEREMPTY  //	-108	 Licenses all borrowed or meter empty
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_NOBORROWMETER  //	-109	 No Borrow Meter Found
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NODONGLE  //	-110	 Dongle not attached, or can't read dongle
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NORESLINK  //	-111	 lmgr.res, Windows Resource file, not linked
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NODONGLEDRIVER  //	-112	 Missing Dongle Driver
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_FLEXLOCK2CKOUT //	-113	 2 FLEXlock checkouts attempted
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SIGN_REQ  //	-114	SIGN= attribute required, but missing from license
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_PUBKEY_ERR  //	-115	 Error in Public Key package
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOTRLSUPPORT  //	-116	 TRL not supported for this platform
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOCROSUPPORT //	LM_NOTRLSUPPORT	 TRL not supported for this platform
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROW_LINGER_ERR  //	-117	 BORROW failed
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_BORROW_EXPIRED  //	-118	 BORROW period has expired
                                   ,"The borrowed license duration has expired. Please return your borrowed license to borrow it again.");
    mGalaxyLicenseMessages.insert( LM_MUST_BE_LOCAL  //	-119	 lmdown and lmreread must be run on license server node
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_BORROW_DOWN  //	-120	 Cannot lmdown the server when licenses are borrowed
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_FLOATOK_ONEHOSTID  //	-121	 FLOAT_OK license must have exactly one dongle hostid
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROW_DELETE_ERR  //	-122	 Unable to delete local borrow info
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROW_RETURN_EARLY_ERR //	-123	 Support for returning a borrowed license early is not enabled
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROW_RETURN_SERVER_ERR //	-124	 Error returning borrowed license on server
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_CANT_CHECKOUT_JUST_PACKAGE //	-125	 Error when trying to just checkout a PACKAGE(BUNDLE)
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_COMPOSITEID_INIT_ERR  //	-126	 Composite Hostid not initialized
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_COMPOSITEID_ITEM_ERR  //	-127	 An item needed for Composite Hostid missing or invalid
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BORROW_MATCH_ERR //	-128	 Error, borrowed license doesn't match any known server license.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NULLPOINTER //	-129	 A null pointer was detected. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADHANDLE //	-130	 A bad handle was used. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_EMPTYSTRING //	-131	 An emptstring was detected. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADMEMORYACCESS //	-132	 Tried to asscess memory that we shouldn't have. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOTSUPPORTED //	-133	 Operation is not supported yet. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NULLJOBHANDLE //	-134	 The job handle was NULL. @ref FLEX_ERROR
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_EVENTLOG_INIT_ERR //	-135	 Error enabling event log
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_EVENTLOG_DISABLED //	-136	 Event logging is disabled
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_EVENTLOG_WRITE_ERR //	-137	 Error writing to event log
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADINDEX //	-138	 Internal error. An invalid array index was used.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_TIMEOUT //	-139	 a timeout has occured
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCOMMAND //	-140	 A bad command was found in a message
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SOCKET_BROKEN_PIPE //	-141	 Error writing to socket.  Peer has closed socket
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_INVALID_SIGNATURE //	-142	 Attempting to generate version specific license tied to a single hostid, which is composite.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_UNCOUNTED_NOT_SUPPORTED //	-143	 Version specific signatures are not supported for uncounted licenses.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_REDUNDANT_SIGNATURES //	-144	 License template contains redundant signature specifiers.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V71_LK //	-145	 Bad V71_LK signature.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V71_SIGN //	-146	 Bad V71_SIGN signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V80_LK //	-147	 Bad V80_LK signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V80_SIGN //	-148	 Bad V80_SIGN signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V81_LK //	-149	 Bad V81_LK signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V81_SIGN //	-150	 Bad V81_SIGN signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V81_SIGN2 //	-151	 Bad V81_SIGN2 signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V84_LK //	-152	 Bad V84_LK signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V84_SIGN //	-153	 Bad V84_SIGN signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCODE_V84_SIGN2 //	-154	 Bad V84_SIGN2 signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LK_REQ //	-155	 License key required but missing from the license certificate.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADAUTH //	-156	 Bad AUTH={} signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_REPAIR_NEEDED //	-157	 TS record invalid
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_TS_OPEN //	-158	 Cannot open TS
                                   ,"Invalid license file.");
    mGalaxyLicenseMessages.insert( LM_BAD_FULFILLMENT //	-159	 Invalid Fulfillment record
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BAD_ACTREQ //	-160	 Invalid activation request received
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_TS_NO_FULFILL_MATCH //	-161	 Invalid activation request received
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BAD_ACT_RESP //	-162	 Invalid activation response received
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_CANTRETURN //	-163	 Can't return the fulfillment
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_RETURNEXCEEDSMAX //	-164	 Return would exceed max count(s)
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NO_REPAIRS_LEFT //	-165	 Return would exceed max count(s)
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_NOT_ALLOWED //	-166	 Specified operation is not allowed
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ENTLEXCLUDE //	-167	 User/host on EXCLUDE list for entitlement
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ENTLNOTINCLUDE //	-168	 User/host not in INCLUDE list for entitlement
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ACTIVATION //	-169	 Activation error
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_TS_BADDATE //	-170	 Invalid date format in trusted storage
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ENCRYPTION_FAILED //	-171	 Encryption failed
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_DECRYPTION_FAILED //	-172	 Encryption failed
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BADCONTEXT //	-173	 Bad filter context
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SUPERSEDE_CONFLICT //	-174	 SUPERSEDE and SUPERSEDE_SIGN can't be used at the same time
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_INVALID_SUPERSEDE_SIGN //	-175	 Invalid SUPERSEDE_SIGN syntax
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SUPERSEDE_SIGN_EMPTYSTRING //	-176	 SUPERSEDE_SIGN does not contain any license signature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ONE_TSOK_PLATFORM_ERR //	-177	 ONE_TS_OK is not supported in this Windows Platform.
                                   ,"A Standalone license for Quantix products cannot be used with remote desktop. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade to a floating license.");
    mGalaxyLicenseMessages.insert( LM_ONE_TSOK_MTX_ERR //	-178	 Failed to create or reopen the mutex. Terminal Server Remote Client checkout not allowed.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_ONE_TSOK_ERR //	-179	 Only One Terminal Server Remote Client checkout is allowed for this feature.
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_SSIDNULL //	-180	 Internal Error - 180. Please report to Flexera Software LLC.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SMTXNOTREL //	-181	 Internal Error - 181. Please report to Flexera Software LLC.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_MTXNOPERM //	-182	 Internal Error - 182. Please report to Flexera Software LLC.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_COMPOSITEID_ETHER_ERR //	-183	 More than one ethernet hostid not supported in composite hostid definition.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LIC_FILE_CHAR_EXCEED //	-184	 The number of characters in the license file paths exceeds the permissible limit.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_TZ_INVALID_SYNTAX //	-185	 Invalid TZ keyword syntax.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_TZ_INVALID_TZONE_SPEC //	-186	 Invalid time zone override specification in the client.
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_TZ_INVALID_TZONE_INFO //	-187	 The time zone information could not be obtained.
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_TZ_UNAUTHORIZED //	-188	 License client time zone not authorized for license rights.
                                   ,"You are trying to connect to a site license and are not co-located with the license server. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade.");
    mGalaxyLicenseMessages.insert( LM_INVALID_VM_PLATFORMS //	-189	 Invalid VM_PLATFORMS syntax
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_VM_PHYSICAL_ONLY //	-190	 Feature can be check-out from Physical machine only
                                   ,"You are trying to connect to a site license and are not co-located with the license server. Please contact "+QString(GEX_EMAIL_SALES)+" to upgrade.");
    mGalaxyLicenseMessages.insert( LM_VM_VIRTUAL_ONLY //	-191	 Feature can be check-out from Virtual machine only
                                   ,"Cannot connect to the license server.");
    mGalaxyLicenseMessages.insert( LM_VM_NOT_SUPPORT //	-192	 VM platform not authorized by license
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_VM_BAD_KEY //	-193	 FNP vendor keys do not support Virtualization feature
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_MAXLIMIT_EXCEED //	-194	Checkout request denied as it exceeds the MAX limit specified in the options file.
                                   ,"All licenses are currently in use by other users or are unavailable to borrow. Please try again later.");
    mGalaxyLicenseMessages.insert( LM_BA_API_INTERNAL_ERROR //	-195	 Binding agent API - Internal error
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_BA_COMM_ERROR //	-196	 Binding agent communication error
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_INVALID_BA_VERSION //	-197	 Invalid Binding agent version
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_UNSUPPORTED_FEATURE_HOSTID //	-198	 Unsupported hostid provided in feature line.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_SERVERQUERY_LOAD_REQUEST_FAILED //	-199	 Failed to load ServerQuery request.*/
                                   , "");
    mGalaxyLicenseMessages.insert( LM_SERVERQUERY_RESPONSE_FAILED //	-200	 Failed to generate ServerQuery response.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_INVALID_IPADDRESS //	-201	 Invalid IPaddress used while overriding.
                                   ,"");
    mGalaxyLicenseMessages.insert( LM_LAST_ERRNO //	-201	 The last valid error number
                                   ,"");

}

#define GS_ACTIVIATION_FNP_UTILS "fnp_utils"
#define GS_APPACTUTIL_ACTIVATION "appactutil"
#define GS_SERVERACTUTIL_ACTIVATION "serveractutil"
#define GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL "INCREMENT"
#define GS_ACTIVIATION_PRODUCT_START "Trust Flags:"
#define GS_ACTIVIATION_PRODUCT_FEATURES "Feature line(s):"
#define GS_ACTIVIATION_PRODUCT_PRODUCT_ID "Product ID:"
#define GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID "Entitlement ID:"
#define GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID "Fulfillment ID:"
#define GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE "Fulfillment Type:"
#define GS_ACTIVIATION_PRODUCT_STATUS "Status:"
#define GS_ACTIVIATION_PRODUCT_SUITE_ID "Suite ID:"
#define GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE "Expiration date:"
#define GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1 "INCREMENT"
#define GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2 "FEATURE"
#define GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN "Activation Server Chain:"
#define GS_ACTIVIATION_PRODUCT_DEDUCTION "Deduction Type:"

#define GS_ACTIVIATION_PRODUCT_CONCURENT    "Concurrent:"
#define GS_ACTIVIATION_PRODUCT_HYBRID       "Hybrid:"
#define GS_ACTIVIATION_PRODUCT_ACTIVATABLE  "Activatable:"
#define GS_ACTIVIATION_PRODUCT_REPAIR  "Repairs:"
#define GS_ACTIVIATION_PRODUCT_DSN "Destination System Name:"
#define GS_ACTIVIATION_PRODUCT_DEDUCTION_UKNOWN "Deduction Type: UNKNOWN"

#define GS_ACTIVIATION_PRODUCT_INCREMENT_SEPERATOR "!"


bool FNPLicenseProvider::getMaintenanceExpirationDate(const QString &licenseType, QDate &lMaintenanceExprationDate)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString(">>  getMaintenanceExpirationDate: %1 license type").arg(licenseType).toLatin1().constData());

    QString lFnpUtils = getAppConfigData("ApplicationDir").toString() + QDir::separator() + QString(GS_ACTIVIATION_FNP_UTILS)+ QDir::separator();
    QString lExtension = "";
#ifdef Q_OS_WIN
        lExtension = ".exe";
#endif

    QStringList lUtilityOptions;
    if(licenseType == "nodelocked_uncounted")
    {
        lFnpUtils += QString(GS_APPACTUTIL_ACTIVATION) + lExtension;
        lUtilityOptions = QStringList() << "-view" << "-long";
    }
    else if(licenseType == "floating")
    {
        lFnpUtils += QString(GS_APPACTUTIL_ACTIVATION) + lExtension;
        QString lServerIp = getLPData("ServerIP").toString();
        lUtilityOptions =  QStringList()
                          << "-serverview"
                          << "-commServer"
                          << lServerIp
                          << "-long";

    }

    QProcess lFNPUtility;
#ifndef Q_OS_WIN
    QProcessEnvironment lEnv = QProcessEnvironment::systemEnvironment();
    QString lLibraryPath = lEnv.value("LD_LIBRARY_PATH");
    lLibraryPath += QString(":%1").arg(getAppConfigData("ApplicationDir").toString() + QDir::separator() + QString(GS_ACTIVIATION_FNP_UTILS));
    lEnv.insert("LD_LIBRARY_PATH", lLibraryPath); // Add an environment variable
    lFNPUtility.setProcessEnvironment(lEnv);
#endif

    GSLOG(SYSLOG_SEV_DEBUG, QString(">>  getMaintenanceExpirationDate: starting \"%1\" with arguments \"%2\"")
          .arg(lFnpUtils).arg(lUtilityOptions.join(' ')).toLatin1().constData());

    lFNPUtility.start(lFnpUtils, lUtilityOptions);
    if (!lFNPUtility.waitForStarted(-1))
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString(">>  lFNPUtility Error  with message %1").arg(lFNPUtility.errorString()).toLatin1().constData());
        return false ;
    }

    if (!lFNPUtility.waitForFinished(-1))
    {
         GSLOG(SYSLOG_SEV_DEBUG, QString(">>  lFNPUtility Error  with message %1").arg(lFNPUtility.errorString()).toLatin1().constData());
         return false ;
    }

    QByteArray lResult = lFNPUtility.readAll();
    QTextStream lResultData(lResult);
    QString lLine("");
    QVariantMap lProductData;
    QStringList lFeatureData;
    QList<QDate> lMaintenanceExpirationDates;

    do
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Line read :%1").arg(lLine).toLatin1().constData());
        if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_START))//We found a new product process it
        {
            //This is a new product read until we reach GS_ACTIVIATION_PRODUCT_START or end of file
            do
            {
                if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_DEDUCTION))
                {
                    do
                    {
                        lLine = lResultData.readLine();
                        if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVATABLE)
                                || lLine.startsWith(GS_ACTIVIATION_PRODUCT_REPAIR)
                                || lLine.startsWith(GS_ACTIVIATION_PRODUCT_DSN)
                                || lLine.startsWith(GS_ACTIVIATION_PRODUCT_DEDUCTION_UKNOWN) )
                        {
                            if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_DSN))
                            {
                                lLine = lResultData.readLine();
                                if(!lLine.startsWith("Expiration Date:"))
                                    break;
                            }
                            else
                                break;
                        }
                    }
                    while(!lLine.isNull() && !lResultData.atEnd());

                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_FEATURES))
                {
                    //Retrive the feature item
                    do
                    {
                        lLine = lResultData.readLine();
                    }
                    while(lLine.isEmpty() && !lResultData.atEnd());

                    QString featureItem;
                    while(lLine.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2) || lLine.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1))
                    {
                        featureItem.clear();
                        while(!lResultData.atEnd())
                        {
                            featureItem += lLine + GS_ACTIVIATION_PRODUCT_INCREMENT_SEPERATOR;
                            lLine = lResultData.readLine();
                            if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL2) || lLine.startsWith(GS_ACTIVIATION_PRODUCT_INCREMENT_LABEL1))
                                break;
                            if(lLine.isNull())
                                break;
                            if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN))
                            {
                                lProductData.insert(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN, lLine.section(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN,1,1));
                                break;
                            }
                        }
                        lFeatureData.append(featureItem);
                    }
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_PRODUCT_ID))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_PRODUCT_ID, lLine.section(GS_ACTIVIATION_PRODUCT_PRODUCT_ID,1,1));

                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID, lLine.section(GS_ACTIVIATION_PRODUCT_ENTITLEMENT_ID,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID, lLine.section(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_ID,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE, lLine.section(GS_ACTIVIATION_PRODUCT_FULFILLEMENT_TYPE,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_STATUS))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_STATUS, lLine.section(GS_ACTIVIATION_PRODUCT_STATUS,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_SUITE_ID))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_SUITE_ID, lLine.section(GS_ACTIVIATION_PRODUCT_SUITE_ID,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE, lLine.section(GS_ACTIVIATION_PRODUCT_EXPIRATION_DATE,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN, lLine.section(GS_ACTIVIATION_PRODUCT_ACTIVIATION_SERVER_CHAIN,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_ACTIVATABLE))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_ACTIVATABLE, lLine.section(GS_ACTIVIATION_PRODUCT_ACTIVATABLE,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_HYBRID))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_HYBRID, lLine.section(GS_ACTIVIATION_PRODUCT_HYBRID,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_CONCURENT))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_CONCURENT, lLine.section(GS_ACTIVIATION_PRODUCT_CONCURENT,1,1));
                }
                else if(lLine.startsWith(GS_ACTIVIATION_PRODUCT_START))
                {
                    lProductData.insert(GS_ACTIVIATION_PRODUCT_START, lLine.section(GS_ACTIVIATION_PRODUCT_START,1,1));
                }
                lLine = lResultData.readLine();
            }
            while(!lLine.isNull() && !lLine.startsWith(GS_ACTIVIATION_PRODUCT_START) && !lResultData.atEnd());
            //We have all the data for current product show it in the GUI
            if(!lProductData.isEmpty() && !lFeatureData.isEmpty())
            {
                for(int lIdx=0; lIdx<lFeatureData.count(); ++lIdx)
                {
                    QString lCurrent = lFeatureData[lIdx];
                    QStringList itemsString = lCurrent.split(" ");
                    QString lFeature =  itemsString[1].trimmed();
                    QString lExpiration = itemsString[4].trimmed();
                    if(lFeature == "Maintenance")
                    {
                        QDate lExpDate = mPrivate->toDate(lExpiration);
                        lMaintenanceExpirationDates.append(lExpDate);
                    }
                }
            }

            lProductData.clear();
            lFeatureData.clear();
        }
        else
            lLine = lResultData.readLine();

    }while(!lLine.isNull() && !lResultData.atEnd());


    if(lMaintenanceExpirationDates.isEmpty())
    {
        return false;
    }
    else
    {
        qSort(lMaintenanceExpirationDates.begin(), lMaintenanceExpirationDates.end(), qGreater<QDate>());
        QDate maxValue = lMaintenanceExpirationDates[0];
        lMaintenanceExprationDate = maxValue;
        return true;
    }

}

}
}
