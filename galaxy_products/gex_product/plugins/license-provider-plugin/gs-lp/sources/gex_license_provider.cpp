#include <QDir>
#include <QFile>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QThread>

#include <time.h>
#include <stdio.h>

#include "license_provider_profile.h"
#include "cryptofile.h"
#include "gqtl_log.h"
#include "gex_shared.h"
#include "gex_version.h"
#include "product_info.h"
#include "gqtl_sysutils.h"
#include "gex_constants.h"
#include "gex_license_provider.h"
#include "gex_errors.h"
#include "license_provider_common.h"
#include "license_provider_dialog.h"
#include "read_system_info.h"
#include "gstdl_type.h"
#include <QMessageBox>
#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <tlhelp32.h>
    #undef CopyFile
#endif

#if defined(unix) || __MACH__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

// Used for encrypting data.
#define GEX_CRYPTO_KEY		"(gex@galaxysemi.com)"
#define GEX_KEY_BYTES		12

// GEX Optional modules (bit)
#define	GEX_OPTIONAL_MODULE_PLUGIN		0x1		// Bit0 = 1 to allow Plugin support
#define	GEX_OPTIONAL_MODULE_PATMAN		0x2		// Bit1 = 1 to allow Outlier-Removal functions.
#define	GEX_OPTIONAL_MODULE_Y123WEB		0x4		// Bit2 = 1 to allow integration with Y123-Web process
#define	GEX_OPTIONAL_ALLTIMEZONES		0x8		// Bit3 = 1 to allow Gex clients of any time zone sharing a LicMan.
#define	GEX_OPTIONAL_MODULE_SYL_SBL		0x10	// Bit4 = 1 to allow SYL/SBL
#define	GEX_OPTIONAL_MODULE_GENEALOGY	0x20	// Bit5 = 1 to allow Genealogy
#define	GEX_OPTIONAL_MODULE_TDR         0x40	// Bit6 = 1 to allow TDR
#define	GEX_OPTIONAL_MODULE_GTM         0x80	// Bit7 = 1 to allow GTM (Galaxy Tester Monitoring)

// Examinator Running mode allowed to process
#define	GEX_DATATYPE_ALLOWED_ANY				0		// Examinator: Allow any type of test data.
#define	GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE	1		// Examinator-OEM for Credence SAPPHIRE data only
#define	GEX_DATATYPE_ALLOWED_SZ					2		// Examinator-OEM for Credence SZ data only
#define	GEX_DATATYPE_ALLOWED_TERADYNE			3		// Examinator: Allow Teradyne data only
#define	GEX_DATATYPE_ALLOWED_DATABASE			4		// Examinator Pro: Allow any files from database.
#define	GEX_DATATYPE_ALLOWED_DATABASEWEB		5		// ExaminatorWeb: Allow any files from database.
#define	GEX_DATATYPE_GEX_MONITORING				6		// Yield-Man
#define	GEX_DATATYPE_GEX_TOOLBOX				7		// Examinator-ToolBox: STDF converter, Dump, etc...
#define	GEX_DATATYPE_ALLOWED_LTXC				8		// Examinator-OEM for LTXC
#define	GEX_DATATYPE_ALLOWED_CREDENCE_ASL		9		// Examinator-OEM for Credence-ASL
#define	GEX_DATATYPE_GEX_YIELD123				10		// Yield123
#define	GEX_DATATYPE_GEX_PATMAN					11		// PAT-Man (superset of Yield-Man)
#define	GEX_DATATYPE_GTM                        12		// GTM (Galaxy Tester Monitoring)


namespace GS
{
namespace LPPlugin
{

#define GEX_LP_VERSION "0.0"
#define GEX_LP_ORDER 99
#define GEX_LP_SOCKET_STATUS_INTERVAL 4000

class GexLicenseProviderPrivate
{
public:
    GexLicenseProviderPrivate(LicenseProvider::GexProducts Product, QVariantMap &appConfigData);
    virtual ~GexLicenseProviderPrivate();
    bool    SetLegacyStuff(LicenseProvider::GexProducts Product);
    bool	CheckDisabledSupportPerpetualLicense(QString strCurrentLicense);
    void	WriteCryptedFile(const char *szString);	// Encrypt+write string to file
    void	WriteCryptedFile(long lData);		// Encrypt+write (long)  to file
    bool	ReadFileLine(char *szLine,int iMaxChar,FILE *hFile);
    void	ReadCryptedFile(char *szString);
    void	ReadCryptedFile(long *lData);
    QString GetLocalConfigFilePath();


    //Legacy function
    bool ProductEnumToLegacy(GS::LPPlugin::LicenseProvider::GexProducts eProduct, int &legacyProduct, int &legacyOptions);
    bool LegacyProductToEnum(int legacyProduct, int legacyOptions, GS::LPPlugin::LicenseProvider::GexProducts &, GS::LPPlugin::LicenseProvider::GexOptions &);
    bool LegacyOptionsToEnum(int legacyOptions, unsigned int &Options);


public:
    //Internal error handling
    QString mInternalErrorMessage;
    int mInternalError;
    QVariantMap &mAppConfigData;

    //Legacy
    int mLegacyProduct,mLegacyOptions; //Attribute to contain legacy procut type

    //GEX_License handling
    FILE *hCryptedFile;		// Used to create license request file.
    QString strConfigFilePath;	// Used to store the complete path to the config file path+name.
    QString strLicenseFile;		// $HOME/gex_request.txt
    QString strLicenseeName;
    QString strProductKey;
    QString strRunningMode;		// Used to store running mode so next GEX launch defaults to this same running mode!
    unsigned long uChecksum;	// Used to check License file checksum.

    //User interaction
    QString mEditField1, mEditField2;
    int miSelectionMode;
    ProductInfo *mProductInfo;
    ReadSystemInfo	mSystemInfo;
    LicenseProvider::GexProducts mProduct;
    QDateTime mLastUserActivity;
public:
    ReadSystemInfo& GetSystemInfo()
    {
        return mSystemInfo;
    }
    //GEXLM connections
    QTcpSocket* m_pGexLmSocket;					// Used if running in Client/Server mode.
    QTimer *mSocketStatusTimer;
    int mLastSocketError;
    bool mInitError;

    quint16 m_uiGexLmServerPort;			// Port of the GexLm server this node is connected to
    QString m_strGexLmServerName;			// Name of the GexLm server this node is connected to
    QString m_strGexLmServerIP;				// IP address of the GexLm server this node is connected to
    QDateTime dtSocketEvent;					// Date time of given event started...used to detect timeout.
    int m_iSocketStatus; // Keeps track of Client/server pending events.
    QString mYMAdminDBConfig;
};


GexLicenseProvider* GexLicenseProvider::mInstance = 0;
GexLicenseProvider *GexLicenseProvider::initializeLP(QObject* parent,
                                                    GexProducts product,
                                                    const QMap<QString, QVariant> &appConfig,
                                                    int &libErrorCode,
                                                    QString &libError)
{
  libErrorCode = eLPLibNoError;
  libError = "";

  if(!mInstance)
  {
      mInstance = new GexLicenseProvider(parent, product, appConfig);
      if(mInstance->getLastErrorCode() != eLPLibNoError)
      {
          libErrorCode = mInstance->getLastErrorCode();
          libError = mInstance->getLastError();
          delete mInstance;
          mInstance = 0;
      }
  }
  return mInstance;
}

void GexLicenseProvider::destroy()
{
  if(mInstance)
    delete mInstance;
  mInstance = 0;
}

GexLicenseProvider::GexLicenseProvider(QObject* parent,
                                       GexProducts product,
                                       const QVariantMap &appConfig)
  : LicenseProvider(parent, product, appConfig)
{
  GSLOG(SYSLOG_SEV_DEBUG, "Start");
  setProperty(LP_TYPE , QString("gs_lp"));
  setProperty(LP_FRIENDLY_NAME , QString("Gex License Provider"));
  setProperty(LP_USAGE_ORDER , GEX_LP_ORDER);
  setProperty(LP_VERSION , GEX_LP_VERSION);
  mPrivate = 0;
  initialize();
}

GexLicenseProvider::~GexLicenseProvider()
{
  cleanup();
}


void GexLicenseProvider::InitServerDescription()
{
    QString lFile = getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "json" + QDir::separator() + JSON_LP_SETTING_FILE;

    if(!QFile::exists(lFile)) {
       lFile = getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "xml" + QDir::separator() + XML_LP_SETTING_FILE;
    }

    ServerFileDescriptorIOFactory* lFactory = ServerFileDescriptorIOFactory::GetInstance();
    mServersDescription = lFactory->GetServerDescriptor(lFile);

    mServersDescription->LoadFile(lFile, getAppConfigData("UseLP").toString()) ;
}

int GexLicenseProvider::initialize()
{
    // Alloc private object
    mPrivate = new GexLicenseProviderPrivate(getProduct(), getFullAppConfigData());
    if(!mPrivate)
    {
        setLastError(eLPAlloc, "Could not allocate private object.");
        return eLPAlloc;
    }

    // Propagate error if any, exit in error
    setLastError(mPrivate->mInternalError,  mPrivate->mInternalErrorMessage);
    if(mPrivate->mInternalError != GexLicenseProvider::eLPLibNoError)
        return mPrivate->mInternalError;

    // Load last configuration for this LP
    InitServerDescription();

    return eLPLibNoError;
}

int GexLicenseProvider::cleanup()
{
    if(mPrivate)
        delete mPrivate;
    mPrivate = 0;
    return 0;
}

void GexLicenseProvider::setInternalError(int i, const QString &m)
{
    mPrivate->mInternalError = i;
    mPrivate->mInternalErrorMessage = m;
}

int GexLicenseProvider::getInternalError(QString &m)
{
    m = mPrivate->mInternalErrorMessage;
    return mPrivate->mInternalError;
}

int GexLicenseProvider::getExtendedError(QString &m)
{
    //m = QString("%1 : %2").arg(mPrivate->mInternalError).arg(mPrivate->mInternalErrorMessage);
    m = QString("%1").arg(mPrivate->mInternalErrorMessage);
    return mPrivate->mInternalError;
}

QStringList GexLicenseProvider::getUserChoice()
{
    QStringList userChoice = QStringList()<< "It seems Examinator may use a floating license...\nbut you also have a valid local Standalone license!\n\nWhich running mode do you want?"
                                          << "Floating license\n(Always)"
                                          << "Floating license\n(This time only)"
                                          << "Standalone / Evaluation\n(use local license file)";

    return userChoice;
}



QString GexLicenseProvider::getWelcomePage()
{
    QString ptPage;

    switch(getProduct())
    {
    case ePATMan:		// Galaxy PAT-Man
        ptPage = GEX_HTMLPAGE_HLP_STARTUP_PATMAN;
        break;

    case eYieldMan:	// ExaminatorMonitoring
        ptPage = GEX_HTMLPAGE_HLP_STARTUP_MONITORING;
        break;
    case eGTM:	// Galaxy Tester Monitor
        ptPage = GEX_HTMLPAGE_HLP_STARTUP_GTM;
        break;
    case eExaminatorPAT:	// ExaminatorDB
        ptPage = GEX_HTMLPAGE_HLP_STARTUP_DBPAT;	// Examinator-PAT = Examinaotr-DB + PAT support.
        break;
    case eExaminatorPro:
        ptPage = GEX_HTMLPAGE_HLP_STARTUP_DB;	// Examinator-DB
        break;
    default:
        ptPage = GEX_HTMLPAGE_HLP_STARTUP;
        break;
    }

    QString strSource = getAppConfigData("ApplicationDir").toString() + GEX_HELP_FOLDER;
    strSource += ptPage;

    return strSource;
}

// Fix me
QString GexLicenseProvider::getFields(RunningMode mode)
{
    QStringList fields;
    char	szString[GEX_MAX_PATH+1];

    switch (mode)
    {
    case eStandalone:
    {

        bool bCriticalError;

        const QString& lStandaloneString = mServersDescription->GetServerDescriptorString("gs_lp", standalone);
        if(!lStandaloneString.isEmpty() )
        {
            fields.clear();
            fields.append(lStandaloneString);
        }
        else if (ReadLicenseFile(mPrivate->mProductInfo, true, &bCriticalError) == true)
        {
            fields.clear();
            fields.append("Standalone");
            fields.append("Your full name:");
            fields.append("Product key ID:");
            fields.append(getLicenseeName());
            fields.append(getProductKey());

        }
        else
        {
            fields.clear();
            fields.append("Standalone");
            fields.append("Your full name:");
            fields.append("Product key ID:");
            get_private_profile_string("Standalone","Username","",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());
            fields.append(szString);
            get_private_profile_string("Standalone","KeyID","",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());
            fields.append(szString);

        }
    }
        break;
    case eClientServer:
    {

        const QString& lFloatingString = mServersDescription->GetServerDescriptorString("gs_lp", floating);
        if(!lFloatingString.isEmpty() )
        {
            fields.clear();
            fields.append(lFloatingString);
        }
        else
        {
            fields.append("Floating");

            fields.append("Server(s):Port(s):");
            int ret = get_private_profile_string(getIniClientSection().toLatin1().constData(),"Server","localhost",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());

            QString lServerPort;
            if(ret)
                lServerPort.append(szString);
            else
                lServerPort.append("localhost");

            lServerPort.append(":");
            //fields.append("Socket Port:");
            ret = get_private_profile_string(getIniClientSection().toLatin1().constData(),"SocketPort","4242",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());

            if(ret)
                lServerPort.append(szString);
            else
                lServerPort.append("4242");

            fields.append(lServerPort);
            /*fields.append("Floating");

            fields.append("Server name / IP:");
            int ret = get_private_profile_string(getIniClientSection().toLatin1().constData(),"Server","localhost",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());
            if(ret)
                fields.append(szString);
            else
                fields.append("localhost");

            fields.append("Socket Port:");
            ret = get_private_profile_string(getIniClientSection().toLatin1().constData(),"SocketPort","4242",szString,GEX_MAX_PATH,
                                       getConfigFilePath().toLatin1().constData());
            if(ret)
                fields.append(szString);
            else
                fields.append("4242");*/
        }
    }
        break;
    case eEvaluation:
    {
        fields.append("Evaluation");
    }
        break;
    default :
        break;
    }

    return fields.join("|");
}

QVariantList GexLicenseProvider::getRunningModeAvailable()
{
    QVariantList availableMode ;
    availableMode.append(QString("Standalone|%1").arg(getFields(eStandalone)));
    availableMode.append(QString("Connect to server (Client/Server)|%1").arg(getFields(eClientServer)));
    //availableMode.append(QString("Evaluation (4 days)|%1").arg(getFields(eEvaluation)));
    switch(getProduct())
    {
    case ePATMan:		// Galaxy PAT-Man
        availableMode.clear();
        availableMode.append(QString("Standalone / Evaluation|%1").arg(getFields(eStandalone)));
        availableMode.append(QString("Connect to server (Client/Server)|%1").arg(getFields(eClientServer)));
        break;

    case eYieldMan:	// ExaminatorMonitoring
        availableMode.clear();
        availableMode.append(QString("Standalone / Evaluation|%1").arg(getFields(eStandalone)));
        availableMode.append(QString("Connect to server (Client/Server)|%1").arg(getFields(eClientServer)));
        break;
    case eGTM:	// Galaxy Tester Monitor
        availableMode.clear();
        availableMode.append(QString("Standalone / Evaluation|%1").arg(getFields(eStandalone)));
        availableMode.append(QString("Connect to server (Client/Server)|%1").arg(getFields(eClientServer)));
        break;
    default:
        break;
    }
    return availableMode;
}


QString GexLicenseProvider::buildUserNotificationMessage()
{
    GEX_ASSERT(false);
    return QString();
}

void GexLicenseProvider::processGexMessage(const GEXMessage &gexMessage)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("processGexMessage GEXMessage(%1 , %2)").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData())
    if(gexMessage.getType() == GEXMessage::eLicenseRequest)
    {
        //Should never happen handled by LPManager
        GSLOG(SYSLOG_SEV_DEBUG, QString("Should never happen handled by LPManager GEXMessage::eLicenseRequest(%1 , %2)").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData())
        GEX_ASSERT(false);
    }
    else if(gexMessage.getType() == GEXMessage::eActive)
    {
        mPrivate->mLastUserActivity = QDateTime::currentDateTime();
    }
    else if(gexMessage.getType() == GEXMessage::eDisconnect)
    {
        CloseGexLmSocket();
        setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
    }
    else if(gexMessage.getType() == GEXMessage::eReconnect)
    {
        CloseGexLmSocket();
        setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
        OnConnectToServer();
    }
    else if(gexMessage.getType() == GEXMessage::eGetYMAdminDBConfig)
    {
        emit sendLPMessage(LPMessage(LPMessage::eSetYMAdminDBConfig, mPrivate->mYMAdminDBConfig));

    }
    else if(gexMessage.getType() == GEXMessage::eSetYMAdminDBConfig)
    {
        QString ymConf = gexMessage.getData();
        WriteLineToSocket(ymConf);

    }
    else if(gexMessage.getType() == GEXMessage::eExit)
    {
        CloseGexLmSocket();
        setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
    }
    else if(gexMessage.getType() == GEXMessage::eExtended)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("GEXMessage::eExtended(%1 , %2)").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData())

    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("End processGexMessage : %1 : %2").arg(gexMessage.getType()).arg(gexMessage.getData()).toLatin1().constData());

}

bool GexLicenseProvider::checkLicense(ProductInfo *productInfo)
{
    QString lErrorMsg;

    GSLOG(SYSLOG_SEV_DEBUG
          , QString("GexLicenseProvider::checkLicense ... Product(%1)").arg(getProduct()).toLatin1().constData());

    if(!productInfo)
    {
        GSLOG(SYSLOG_SEV_DEBUG, "Internal error due to unspecified product" );
        setInternalError(LPMessage::eReject,"Internal error due to unspecified product");
        return false;
    }

    LicenseProvider::GexProducts lProduct = getProduct();
    mPrivate->mProductInfo = productInfo;
    productInfo->setProductID(lProduct);

    // Make sure we are not running in TER mode
    if((lProduct == eTerOEM) || (lProduct == eTerProPlus))
    {
        lErrorMsg = QString("You cannot use this license provider (gs_lp) with a Teradyne edition.");
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    // Make sure we have a match with gs_lp legacy product codes
    if(!mPrivate->SetLegacyStuff(lProduct))
    {
        lErrorMsg = mPrivate->mInternalErrorMessage;
        GSLOG(SYSLOG_SEV_CRITICAL, lErrorMsg.toLatin1().constData());
        setInternalError(LPMessage::eReject, lErrorMsg);
        return false;
    }

    if(!getAppConfigData("isDAEMON").toBool())
    {
        GexLicenseProvider::MissingLicenseChoice eStatus  = GexLicenseProvider::eUndefinedChoice;
        QString additionalMessage;
        if(!IsCorrectLicenseID(eStatus, productInfo, additionalMessage))
        {
            if(eStatus == GexLicenseProvider::eValidLicenseFound)
            {
                int choice = 0;
                QStringList userChoiceList = getUserChoice();
                emit userChoice(&userChoiceList, choice);
                if(choice == 0)
                    eStatus = GexLicenseProvider::eUseFloatingAlways;
                else if(choice == 1)
                    eStatus = GexLicenseProvider::eUseFloatingOnce;
                else
                    eStatus = GexLicenseProvider::eUseLocal;
                productInfo->setProductID(getProduct());
                bool ret =  IsCorrectLicenseID(eStatus, productInfo, additionalMessage);
                if(!ret)
                {
                    // License file not correct or doesn't exist...
                    //!!! emit sendLPMessage(LPMessage(LPMessage::eReject,"LPMan :License file not correct or doesn't exist ..."));
                    setInternalError(LPMessage::eReject,"License file not correct or doesn't exist ...");
                    return false;
                }
                else
                {
                    emit sendLPMessage(LPMessage(LPMessage::eAccept,"LPMan"));
                    return true;
                }

            }
            else
            {
                if(mServersDescription->IsLoaded() && !mServersDescription->GetConnectionStatus())
                    getFullAppConfigData()["WelcomeMode"] = true;

                if( getAppConfigData("WelcomeMode").toBool() &&
                    (!getAppConfigData("isDAEMON").toBool() || !getAppConfigData("HiddenMode").toBool())
                  )
                {
                    bool notify = false;
                bool ret = false;
                emit userInteraction(this,&(mPrivate->mEditField1), &(mPrivate->mEditField2),
                                     &(mPrivate->miSelectionMode), notify,ret, false);
                productInfo->setProductID(getProduct());

                SaveLastChoiceStatus(ret);

                if(!ret)
                {
                    // License file not correct or doesn't exist...
                    //!!!! emit sendLPMessage(LPMessage(LPMessage::eReject,"license activation cancelled by user"));
                    setInternalError(LPMessage::eReject,""/*"license activation cancelled by user"*/);
                    setLPData("Exit", QVariant(true));
                    return false;
                }
                ret = StartRunning(productInfo);
                return ret;
            }
                else
                    return false;
        }
        }
        else
        {
            if(mPrivate->mProductInfo->getLicenseRunningMode() != GEX_RUNNINGMODE_CLIENT)
                emit sendLPMessage(LPMessage(LPMessage::eAccept,"The license is granted..."));

            if(getAppConfigData("WelcomeMode").toBool() == false) {

                if(!mServersDescription->IsLoaded())
                    InitServerDescription();
                if(!mServersDescription->IsLoaded())
                    ConvertAndSaveToJsonFormat();
                //saveLastChoice(getLPData("LicenseType").toString(), getLPData("ServerIP").toString(),"");
            }
            SaveLastChoiceStatus(true);

            return true;
        }
    }
    else
    {
        GexLicenseProvider::MissingLicenseChoice eStatus = GexLicenseProvider::eUseFloatingAlways;
        GSLOG(SYSLOG_SEV_DEBUG, QString("Calling IsCorrectLicenseID : (%1) (%2)").arg(eStatus).arg(productInfo->getProductID()).toLatin1().constData());
        QString additionalMessage;
        if(!IsCorrectLicenseID(eStatus, productInfo, additionalMessage))
        {
            // License file not correct or doesn't exist...
            //GSLOG(SYSLOG_SEV_CRITICAL, "You are in deamon mode and the license is missing...");
            //!!! emit sendLPMessage(LPMessage(LPMessage::eReject,"You are in deamon mode and the license is missing..."));
            setInternalError(LPMessage::eReject,QString("You are in daemon mode and the license is missing  %1")
                             .arg(additionalMessage.isEmpty() ? QString("") : QString("due to (%1)").arg(additionalMessage)).toLatin1().constData());
            GSLOG(SYSLOG_SEV_CRITICAL,
                  QString("You are in daemon mode and the license is missing %1")
                  .arg(additionalMessage.isEmpty() ? QString("") : QString("due to (%1)").arg(additionalMessage)).toLatin1().constData());
            return false;
        }
        else
        {
            GSLOG(SYSLOG_SEV_DEBUG, "You are in daemon mode and the license is granted...");
            //emit sendLPMessage(LPMessage(LPMessage::eAccept,"You are in daemon mode and the license is granted..."));
            return true;

        }
    }

    return false;
}


///////////////////////////////////////////////////////////
// Check application arguments...
///////////////////////////////////////////////////////////
bool GexLicenseProvider::checkStartupArguments(ProductInfo *productInfo)
{
    if(!productInfo)
        return false;

    switch(mPrivate->mLegacyProduct/*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct()*/)
    {
        case GEX_DATATYPE_ALLOWED_LTXC:
            {
                // LTXC running mode is activated with a secret command line, it doesn't need a license file.
                productInfo->setProductID(GS::LPPlugin::LicenseProvider::eLtxcOEM);
                productInfo->setEditionID(GEX_EDITION_STD);
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);

                QDate ed;
                // GCORE-2950: set a dynamic expiration date.
                // ed.setDate(2014,12,31);
                ed = QDate::currentDate().addYears(10);
                setLPData("ExpirationDate", ed);
                return true;
            }
            break;

/*
        case GEX_DATATYPE_ALLOWED_SZ:
            {
                // SZ running mode is activated with a secret command line, it doesn't need a license file.
                productInfo->setProductID(GS::LPPlugin::LicenseProvider::eSzOEM);
                productInfo->setEditionID(GEX_EDITION_ADV);
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);

                QDate ed;
                // GCORE-2950: set a dynamic expiration date.
                // ed.setDate(2014,12,31);
                ed = QDate::currentDate().addYears(10);
                setLPData("ExpirationDate", ed);
                return true;
            }
            break;
*/

        case GEX_DATATYPE_GEX_MONITORING:
            productInfo->setProductID(GS::LPPlugin::LicenseProvider::eYieldMan);
            break;

        case GEX_DATATYPE_GEX_PATMAN:
            productInfo->setProductID(GS::LPPlugin::LicenseProvider::ePATMan);
            break;

        case GEX_DATATYPE_GTM:
            productInfo->setProductID(GS::LPPlugin::LicenseProvider::eGTM);
            break;

        default:
            break;
    }

    return (getAppConfigData("HiddenMode").toBool()) || (getAppConfigData("WelcomeMode").toBool() == false );
}

///////////////////////////////////////////////////////////
// Check if LicenseID proposed is correct...
///////////////////////////////////////////////////////////
bool GexLicenseProvider::IsCorrectLicenseID(MissingLicenseChoice &eStatus, ProductInfo *productInfo, QString &additionalError)
{
    GSLOG(SYSLOG_SEV_DEBUG, QString("IsCorrectLicenseID eStatus(%1) getProductID(%2)")
          .arg(eStatus).arg(productInfo->getProductID()).toLatin1().constData());

    bool bCriticalError=false;
    // Check if user FORCES to display the welcome page!

    if(checkStartupArguments(productInfo) == false || (mServersDescription->IsLoaded() && !mServersDescription->GetConnectionStatus()))
    {
        GSLOG(4, QString("checkStartupArguments failed").toLatin1().data() );
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("checkStartupArguments  OK").toLatin1().constData());

    bool	bServerApplication=false;	// Set to 'true' if running as a server app (eg: Yield-Man, PAT-Server)
    switch(mPrivate->mLegacyProduct/*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct()*/)
    {
        // OEM versions do not need a license file...
        case GEX_DATATYPE_ALLOWED_LTXC:
        case GEX_DATATYPE_ALLOWED_SZ:
            return true;

        case GEX_DATATYPE_GEX_MONITORING:
        case GEX_DATATYPE_GEX_PATMAN:
        case GEX_DATATYPE_GTM:
            bServerApplication = true;
            break;

        default:
            break;
    }

    // If last Launch was in 'Client/Server' mode and was successful...connect to server
    bool	bForceServer=false;
    GSLOG(SYSLOG_SEV_DEBUG, QString(" If last Launch was in 'Client/Server' get_private_profile_int => getIniClientSection(%1) getConfigFilePath (%2)")
          .arg(getIniClientSection()).arg(getConfigFilePath()).toLatin1().constData());

    int lR=get_private_profile_int(getIniClientSection().toLatin1().constData(),"ServerFound",0,
                                   getConfigFilePath().toLatin1().constData()); /*Ret false in deamon mode. WT: Are you sure ?*/

    if(lR)
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("Start server get_: private_profile_int => getIniClientSection(%1) getConfigFilePath (%2) bForceServer(%3)")
              .arg(getIniClientSection()).arg(getConfigFilePath()).arg(bForceServer).toLatin1().constData());

        // Start server...unless we also have a valid license file. in such case, we ask
        if(get_private_profile_int(getIniClientSection().toLatin1().constData(),"AlwaysConnectToServer",0, getConfigFilePath().toLatin1().constData()) == 1)
            bForceServer = true;	// User previously said to ALWAYS connect to server.

        // what we do have to run
        GSLOG(SYSLOG_SEV_DEBUG, QString("Try ReadLicenseFile").toLatin1().constData());
        if(ReadLicenseFile(productInfo, false, &bCriticalError) == true)
        {
            GSLOG(SYSLOG_SEV_DEBUG, QString("Try ReadLicenseFile == true").toLatin1().constData());
            // If Examinator launched with a script in argument...we must not popup any dialogbox!
            if((bForceServer == false) && (bServerApplication == false) &&
               getAppConfigData("RunScript").toString().isEmpty()
                    /*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetRunScript().isEmpty()*/)
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Condition 1 eStatus(%1)").arg(eStatus).toLatin1().constData());

                if(eStatus == GexLicenseProvider::eUndefinedChoice)
                {
                    GSLOG(4, "Status is 'UndefinedChoice'");
                    eStatus = GexLicenseProvider::eValidLicenseFound;
                    return false;
                }
                // User want to ALWAYS connect to server in the future...
                if(eStatus == GexLicenseProvider::eUseFloatingAlways)
                    write_private_profile_string(getIniClientSection().toLatin1().constData(),"AlwaysConnectToServer","1", mPrivate->strConfigFilePath.toLatin1().constData());
            }
            else
            {
                GSLOG(SYSLOG_SEV_DEBUG, "Condition 2");
                eStatus = GexLicenseProvider::eUseFloatingAlways;	// Connect to server quietly and execute script.

            }

            if (eStatus != GexLicenseProvider::eUseLocal)
            {
                //productInfo->setProductID(GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct());
                GS::LPPlugin::LicenseProvider::GexProducts productID;
                GS::LPPlugin::LicenseProvider::GexOptions productOptions;
                mPrivate->LegacyProductToEnum(mPrivate->mLegacyProduct, mPrivate->mLegacyOptions, productID, productOptions);

                GSLOG(SYSLOG_SEV_DEBUG, QString("eStatus != GexLicenseProvider::eUseLocal eStatus(%1) productID(%2) "
                                                "productOptions(%3) mPrivate->mLegacyProduct(%4) mPrivate->mLegacyOptions(%5)")
                      .arg(eStatus).arg(productID).arg(productOptions).arg(mPrivate->mLegacyProduct).arg(mPrivate->mLegacyOptions).toLatin1().constData());


                productInfo->setProductID(productID);
                char szString[GEX_MAX_PATH+1];

                QString fieldLogs =  QString("StartRunningServer : ");
                get_private_profile_string(getIniClientSection().toLatin1().constData(),"Server","localhost",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());
                setEditField1(szString);
                fieldLogs += QString("EditField1 (%1)").arg(szString);

                get_private_profile_string(getIniClientSection().toLatin1().constData(),"SocketPort","4242",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());
                setEditField2(szString);
                fieldLogs += QString("EditField2 (%1)").arg(szString);

                GSLOG(SYSLOG_SEV_DEBUG, fieldLogs.toLatin1().constData());

                return StartRunningServer(productInfo);	// Run in Server mode...be so.
            }
            else
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Clears the auto-launch config file: next run won't launch GEX in Server mode").toLatin1().constData());
                // We have a local VALID license...so allow GEX to start!
                // Clears the auto-launch config file: next run won't launch GEX in Server mode
                write_private_profile_string(getIniClientSection().toLatin1().constData(),"ServerFound","0", mPrivate->strConfigFilePath.toLatin1().constData());
            }
        }
        else
        {
            // -- if missing json file , loading gex_localconfig.conf file
            if(!mServersDescription->IsLoaded())
            {
                GSLOG(SYSLOG_SEV_DEBUG, QString("Try ReadLicenseFile == false").toLatin1().constData());

                GSLOG(SYSLOG_SEV_DEBUG, QString(" If last Launch was in 'Client/Server' get_private_profile_int => getIniClientSection(%1) getConfigFilePath (%2)")
                      .arg(getIniClientSection()).arg(getConfigFilePath()).toLatin1().constData());

                char szString[GEX_MAX_PATH+1]="";

                QString fieldLogs =  QString("StartRunningServer : ");
                get_private_profile_string(getIniClientSection().toLatin1().constData(),"Server","localhost",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());

                QString lHostPort = szString;
                lHostPort.append(":");

                //setEditField1(szString);
                fieldLogs += QString("EditField1 (%1)").arg(szString);

                get_private_profile_string(getIniClientSection().toLatin1().constData(),"SocketPort","4242",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());
                lHostPort.append(szString);
                setEditField2(lHostPort);
                fieldLogs += QString("EditField2 (%1)").arg(szString);

                GSLOG(SYSLOG_SEV_DEBUG, fieldLogs.toLatin1().constData());

                SaveToJson("gs_lp", "floating", lHostPort);
            }
            else {
                QString lLastChoice = mServersDescription->GetLastChoice();
                setEditField2(lLastChoice.section("|", 2, 2));

            }
            return StartRunningServer(productInfo);	// No valid local license...so no conflict!
        }
    }
    else
    {

        if(mServersDescription->IsLoaded())
        {
            QString lLastChoice = mServersDescription->GetLastChoice();
            setEditField2(lLastChoice.section("|", 2, 2));

            if(mServersDescription->GetLastUsedLicenseType() == floating)
                return StartRunningServer(productInfo);	// No
            else if(mServersDescription->GetLastUsedLicenseType() == standalone) {
                productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);
                return false;
            }
        }
        else {
            GSLOG(5, QString("get_private_profile_int returned %1").arg(lR).toLatin1().data() );
            additionalError = QString("can not read \"%1\" ").arg(getConfigFilePath()).toLatin1().constData();
        }
    }

    // Read license file (if any), return validity flag..or may abort GEX if file not valid
    GSLOG(SYSLOG_SEV_DEBUG, QString("Read license file (if any), return validity flag..or may abort GEX if file not valid").toLatin1().constData());
    bool bReadLicense = ReadLicenseFile(productInfo, true, &bCriticalError);
    GSLOG(5, QString("ReadLicenseFile returned %1, CriticalError=%2")
          .arg(bReadLicense?"true":"false").arg(bCriticalError?"true":"false").toLatin1().data() );

    GSLOG(SYSLOG_SEV_DEBUG, QString("productInfo->getLicenseRunningMode() = (%1)")
          .arg(productInfo->getLicenseRunningMode()).toLatin1().constData());

    if(productInfo->getLicenseRunningMode() == GEX_RUNNINGMODE_STANDALONE )
    {
        GSLOG(SYSLOG_SEV_DEBUG, QString("productInfo->getLicenseRunningMode() == GEX_RUNNINGMODE_STANDALONE").toLatin1().constData());

        char stroredKey[GEX_MAX_PATH+1]="";
        get_private_profile_string("Standalone","KeyID","",stroredKey,GEX_MAX_PATH, getConfigFilePath().toLatin1().constData());
        bReadLicense = bReadLicense && (getProductKey() == stroredKey);
        if(getProductKey() != stroredKey)
        {
            GSLOG(SYSLOG_SEV_WARNING, "License key and key stored does not match \n");
        }
    }

    GSLOG(SYSLOG_SEV_DEBUG, QString("bReadLicense(%1) bCriticalError(%2)").arg(bReadLicense).arg(bCriticalError).toLatin1().constData());
    if(bReadLicense == false || bCriticalError)
        return bReadLicense;

    // Check if Product Key is of a Perpetual license Without active support!
    bool bAcceptLicense = mPrivate->CheckDisabledSupportPerpetualLicense(mPrivate->strProductKey);
    if((bAcceptLicense == false) && (bForceServer == false) && (bServerApplication == false) &&
       getAppConfigData("RunScript").toString().isEmpty()
            /*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetRunScript().isEmpty()*/)
    {
        GSLOG(SYSLOG_SEV_WARNING,
               "Your maintenance contract has expired. You can't run this recent release.\nContact the Quantix sales team to upgrade your contract.");
    }

    return bAcceptLicense;
}

///////////////////////////////////////////////////////////
// Save parameter connection in th json file. Only called when the welvome dialog is not displayed and no xml, no json...
///////////////////////////////////////////////////////////
void GexLicenseProvider::SaveToJson(const QString& provider, const QString& licenseType, const QString &hostPort)
{
    QString lFile = getAppConfigData("GalaxySemiFolder").toString() +QDir::separator()+ "json" + QDir::separator() + JSON_LP_SETTING_FILE;

    ServerFileDescriptorIOFactory* lFactory = ServerFileDescriptorIOFactory::GetInstance();
    mServersDescription = lFactory->GetServerDescriptor(lFile);
    //QString lHostPort = hostPort;
    //std::replace(lHostPort.begin(), lHostPort.end(),':', ';');
    mServersDescription->SaveFile(lFile,provider, licenseType, hostPort, "");
}


///////////////////////////////////////////////////////////
// Read the license file if any found localy...
///////////////////////////////////////////////////////////
bool GexLicenseProvider::ReadLicenseFile(ProductInfo *productInfo, bool bValidityCheckOnly, bool *pbCriticalError)
{
    *pbCriticalError = false;

    // Check license file and see if valid!
    QString	strLicenseFile;
    char	szString[2048]="";
    QString	strLicenseSoftwareVersion;	// Hold Examinator version name used to create the license file.
    QString strLicenseRevision;
    int		nLicenseMajorVersion = -1;
    int		nLicenseMinorVersion = -1;
    int		iFileVersion = 0;
    long	lData=0;

    // Get GEX application path: $GEX_PATH/gex_license.txt
    //strLicenseFile = GS::Gex::GexLicenseProvider::GetInstance().Get("ApplicationDir").toString() + "/gex_license.txt";
    strLicenseFile = getAppConfigData("ApplicationDir").toString() + "/gex_license.txt";

    GSLOG(6, QString("Opening '%1'...").arg(strLicenseFile).toLatin1().data() );

    setCryptedFile(fopen(strLicenseFile.toLatin1().constData(),"rb"));
    if(getCryptedFile() == NULL)
    {
        // Second chance: try to open gex_license.bin file...for backward compatibility
        //strLicenseFile = GS::Gex::GexLicenseProvider::GetInstance().Get("ApplicationDir").toString() + "/gex_license.bin";
        strLicenseFile = getAppConfigData("ApplicationDir").toString() + "/gex_license.bin";
        GSLOG(6, QString("Opening '%1'...").arg(strLicenseFile).toLatin1().data() );
        setCryptedFile (fopen(strLicenseFile.toLatin1().constData(),"rb"));
        if(getCryptedFile() == NULL)
        {
            // Try finding file from User folder
            //strLicenseFile= GS::Gex::GexLicenseProvider::GetInstance().Get("UserFolder").toString();    //GetUserFolder(strLicenseFile);
            strLicenseFile =  getAppConfigData("UserFolder").toString();
            strLicenseFile += "/gex_license.txt";
            GSLOG(6, QString("Opening '%1'...").arg(strLicenseFile).toLatin1().data() );
            setCryptedFile (fopen(strLicenseFile.toLatin1().constData(),"rb"));

            if(getCryptedFile() == NULL)
            {
                // Try finding file from User folder
                //strLicenseFile=GS::Gex::GexLicenseProvider::GetInstance().Get("GalaxySemiFolder").toString()+QDir::separator();
                strLicenseFile = getAppConfigData("GalaxySemiFolder").toString() + QDir::separator();
                strLicenseFile += "gex_license.txt";
                GSLOG(6, QString("Opening '%1'...").arg(strLicenseFile).toLatin1().data() );
                setCryptedFile (fopen(strLicenseFile.toLatin1().constData(),"rb"));
            }
        }
    }

    if(getCryptedFile() == NULL)
    {

        if(getProduct() != LicenseProvider::eGTM)
            // License file doesn't exist...so abort, and popup registration window!
            GSLOG(4, QString("Cannot find any license file.").toLatin1().data() );

        setInternalError(LPMessage::eReject,"License file doesn't exist...so abort, and popup registration window!");
        return false;
    }

    // Read lines from crypted file...
    setChecksum( 0);	// Clear checksum
    mPrivate->ReadCryptedFile(szString);					// 'Galaxy Examinator - VX.Y.Z'
    strLicenseSoftwareVersion = szString;

    // Extract Version# of this client application...
    strLicenseRevision		= strLicenseSoftwareVersion.section('-',1);		// eg: " V6.1 B147 (Beta)"
    strLicenseRevision		= strLicenseRevision.section('V',1);				// eg: "6.1 B147 (Beta)"
    nLicenseMajorVersion	= strLicenseRevision.section('.',0,0).toInt();

    strLicenseRevision		= strLicenseRevision.section('.',1);		// eg: "1 B147 (Beta)"
    strLicenseRevision		= strLicenseRevision.section(' ',0,0);		// eg: "1"
    nLicenseMinorVersion	= strLicenseRevision.section('.',0,0).toInt();

    mPrivate->ReadCryptedFile(szString);					// License file version.
    iFileVersion=0;
    int r=sscanf(szString,"%d",&iFileVersion);
    if (r==EOF)
        GSLOG(4, QString("failed to interpret File Version : '%1'").arg(szString).toLatin1().constData());
    mPrivate->ReadCryptedFile(szString);					// Time and date license file was created (sec. elapsed since Jan 1 1970)
    QDateTime dtLicenseTime;
    time_t lLicenseTime=time(NULL);
    r=sscanf(szString,"%ld",(long *)&lLicenseTime);
    if (r==EOF)
        GSLOG(4, "failed to interpret a Licence string");
    dtLicenseTime.setTime_t(lLicenseTime);
    dtLicenseTime = dtLicenseTime.addDays(-7);	// Allow a clock error of up to one week from normal time!
    //setReleaseDate (dtLicenseTime.date());			// This ensures STDF older than License file generation date - 7 days will be REFUSED!!!!
    setLPData("ReleaseDate", dtLicenseTime.date());

    mPrivate->ReadCryptedFile(szString);					// License expiration date (YYYY MM DD)
    int	yy=0,mm=0,dd=0;
    r=sscanf(szString,"%d %d %d",&yy,&mm,&dd);
    if (r==EOF)
        GSLOG(4, "failed to interpret a Licence string");
    QDate	CurrentData = QDate::currentDate();
    sprintf(szString,"%d %d %d",yy,mm,dd);
    QDate ed;
    if(ed.setDate(yy,mm,dd) == false)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile(NULL);

        if(bValidityCheckOnly == false)
        {
            GSLOG(4,
                   QString("Internal ERROR: Failed checking license expiration date.\nPlease contact Quantix support at "
                   +QString(GEX_EMAIL_SUPPORT)).toLatin1().constData());

            *pbCriticalError = true;
            setInternalError(LPMessage::eReject, QString("Internal ERROR: Failed checking license expiration date.\nPlease contact Quantix support at "
                                                         +QString(GEX_EMAIL_SUPPORT)).toLatin1().constData());
            return false;
        }
        else
            return false;	// Failed processing license file...behave as if first installation: license request process
    }
    else
    {
        setLPData("ExpirationDate", ed);
    }

    if((CurrentData > getLPData("ExpirationDate").toDate()) || (CurrentData < getLPData("ReleaseDate").toDate()))
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        if(bValidityCheckOnly == false)
        {
            GSLOG(4,
                QString("%1%2").arg("Your License has expired, you need to renew it...\nPlease contact ").arg(GEX_EMAIL_SALES).toLatin1().constData());
            // erase outdated license file
            unlink(strLicenseFile.toLatin1().constData());
            *pbCriticalError = true;
            setInternalError(LPMessage::eReject,QString("%1%2").arg("Your License has expired, you need to renew it...\nPlease contact ").arg(GEX_EMAIL_SALES).toLatin1().constData());

            return false;
        }
        else
        {
            setInternalError(LPMessage::eReject,"Failed processing license file...behave as if first installation: license request process");
            return false;
        }
    }

    mPrivate->ReadCryptedFile(szString);					// User name....(used to populate activation dialog if Welcome/Switch mode)
    setLicenseeName(szString);
    mPrivate->ReadCryptedFile(szString);					// Platform : Windows, Solaris, HP-UX

    int		iLicenseType=-1;
    int		iLicensesOrdered=0;
    long	lProductID=0;
    int		iEditionType=0;	// Standard Edition
    int		iMonitorProducts=0;	// Total Products allowed in Monitoring
    int		iOptionalModules=0;	// Flags: Bit0=plugin support (default), Bit1=PAT Outier removal AEC-Q001 specs support
    int		iAllowedTimeShift = 90;	// Timeshift allowed between client and server (gex-lm)
    int		iMaximumVersionMaj=-1;	// Highest Version# allowed to be ran (based on maintenance contract)
    int		iMaximumVersionMin=-1;	// Highest Version# allowed to be ran (based on maintenance contract)

    // ProductID:0= Examinator, 1=Examinator for Credence,2=Examinator for SZ, etc...4=ExaminatorDB
    mPrivate->ReadCryptedFile(&lProductID);

    // <LicenseType>  <TotalLicensesOrdered> <Edition Type> <Products-for-Monitoring> <PluginSupport>
    mPrivate->ReadCryptedFile(szString);			// License Type , MAximum concurrent licenses.
    r=sscanf(szString,"%d %d %d %d %d %d %d %d",
                 &iLicenseType,&iLicensesOrdered,&iEditionType,&iMonitorProducts,
                 &iOptionalModules,&iAllowedTimeShift,&iMaximumVersionMaj,&iMaximumVersionMin);
    if (r==EOF)
        GSLOG(4, "failed to interpret a licence string");
    // If old license file: enable SYA option!
    if (iFileVersion < 101)
        iOptionalModules |= 0x10;

    // HACK for GTM: in order to do minimal changes in LicMan (knowing we will
    // switch to Flexera in V7.1), GTM is an optional module of GEXPRO. But
    // internally, we already use a specific Product ID
    if(productInfo->isGTM())
    {
        // Application started in GTM mode
        // If GTM activated in license file, switch Product ID
        if( (lProductID == GEX_DATATYPE_ALLOWED_DATABASE) &&
            (iOptionalModules & GEX_OPTIONAL_MODULE_GTM))
            lProductID = GEX_DATATYPE_GTM;
        // If license does not allow GTM, reject license file
        if(lProductID != GEX_DATATYPE_GTM)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL,"Your License doesn't allow you to run the 'GTM' module.");
            // Clean close of the license file
            fclose(getCryptedFile());
            setCryptedFile (NULL);
            setInternalError(LPMessage::eReject,"Your License doesn't allow you to run the 'GTM' module.");
            return false;
        }
    }

    // Set Product ID
    //productInfo->setProductID(mPrivate->IntProductEnum(lProductID, iOptionalModules));

    GS::LPPlugin::LicenseProvider::GexProducts productID;
    GS::LPPlugin::LicenseProvider::GexOptions productOptions;
    mPrivate->LegacyProductToEnum(lProductID, iOptionalModules, productID, productOptions);
    productInfo->setProductID(productID);


    // Set number of products allowed for monitoring (if any)
    productInfo->setMonitorProducts(iMonitorProducts);

    // if YM desired, let s remove PAT option if any
    //if (mDetectedArguments.contains("YM"))
      //  iOptionalModules=iOptionalModules & !GEX_OPTIONAL_MODULE_PATMAN;

    // Set Flags for options supported: Plugin support, PAT AEC-Q001 support,...
    //productInfo->setOptionalModules(iOptionalModules);
    unsigned int Options = GS::LPPlugin::LicenseProvider::eNoOptions;
    mPrivate->LegacyOptionsToEnum(iOptionalModules, Options);
    productInfo->setOptionalModules(Options);


    // Set Edition type: Standard, Pro
    switch(iEditionType)
    {
        case 0:
        default:
            productInfo->setEditionID(GEX_EDITION_STD);	// Standard Edition
            break;
        case 1:
            productInfo->setEditionID(GEX_EDITION_ADV);	// Pro Edition
            break;
    }

    // If 'Evaluation' release: refuse to run if license was created with an earlier release!
    if((iLicenseType == 2)
        && (nLicenseMajorVersion < GEX_APP_VERSION_MAJOR || (nLicenseMajorVersion == GEX_APP_VERSION_MAJOR && nLicenseMinorVersion < GEX_APP_VERSION_MINOR)))
    {
        QString strErrorMessage =
            QString("ActivationKeyDialog::ReadLicenseFile : Version found in the license file is lower than the software version (%1.%2 < %3.%4)")
            .arg(nLicenseMajorVersion).arg(nLicenseMinorVersion).arg(GEX_APP_VERSION_MAJOR).arg(GEX_APP_VERSION_MINOR);
        GSLOG(SYSLOG_SEV_DEBUG,strErrorMessage.toLatin1().constData());
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);
        setInternalError(LPMessage::eReject, QString("Version found in the license file is lower than the software version (%1.%2 < %3.%4)")
                         .arg(nLicenseMajorVersion).arg(nLicenseMinorVersion).arg(GEX_APP_VERSION_MAJOR).arg(GEX_APP_VERSION_MINOR));
        return false;
    }

    if(iLicenseType == 0)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        // GEX license type: 0=floating, 1= standalone, 2=Evaluation
        // License file doesn't exist...so abort, and popup registration window!
        // Unless last Launch was in 'Client/Server' mode and was successful
        GSLOG(SYSLOG_SEV_DEBUG,QString("License file doesn't exist...so abort, and popup registration window! \n iLicenseType(%1) bValidityCheckOnly(%2)")
              .arg(iLicenseType).arg(bValidityCheckOnly).toLatin1().constData());
        if(bValidityCheckOnly == false)
        {
            if(get_private_profile_int(getIniClientSection().toLatin1().constData(),"ServerFound",0, getConfigFilePath().toLatin1().constData()))
            {
                GSLOG(SYSLOG_SEV_DEBUG,"StartRunningServer");
                char szString[GEX_MAX_PATH+1];
                get_private_profile_string(getIniClientSection().toLatin1().constData(),"Server","localhost",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());
                setEditField1(szString);

                get_private_profile_string(getIniClientSection().toLatin1().constData(),"SocketPort","4242",szString,GEX_MAX_PATH,
                                           getConfigFilePath().toLatin1().constData());
                setEditField2(szString);
                return StartRunningServer(productInfo);
            }
        }
        setInternalError(LPMessage::eReject,"starting in server mode");
        return false;	// License file is for a server!
    }

    mPrivate->ReadCryptedFile(szString);					// Computer name
    if(mPrivate->GetSystemInfo().strHostName != szString)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        setInternalError(LPMessage::eReject,"License file not generated for this machine");
        return false;
    }

    mPrivate->ReadCryptedFile(szString);					// Account logged when GEX executed....(ignored in license filter)
    mPrivate->ReadCryptedFile(szString);					// ProductKey....(ignored in license filter)
    setProductKey(szString);
    mPrivate->ReadCryptedFile(szString);					// HostID

    // Check if user has a valid maintenance contract...
    if(iMaximumVersionMaj >= 0 && iMaximumVersionMin >= 0)
    {
        // Extract Version# of this client application...
        int	iVersionMaj,iVersionMin;
        QString strUserRevision = getAppConfigData("AppFullName").toString(); //GS::Gex::GexLicenseProvider::GetInstance().Get("AppFullName").toString();	// eg: "Visual Examinator - V6.1 B147 (Beta)"
        strUserRevision = strUserRevision.section('-',1);		// eg: " V6.1 B147 (Beta)"
        strUserRevision = strUserRevision.section('V',1);		// eg: "6.1 B147 (Beta)"
        iVersionMaj = strUserRevision.section('.',0,0).toInt();
        strUserRevision = strUserRevision.section('.',1);		// eg: "1 B147 (Beta)"
        strUserRevision = strUserRevision.section(' ',0,0);		// eg: "1"
        iVersionMin = strUserRevision.section('.',0,0).toInt();

        // Check validity X.Y
        bool bRejectUser=false;
        if(iVersionMaj < iMaximumVersionMaj)
            bRejectUser=false;
        else if(iVersionMaj > iMaximumVersionMaj)
            bRejectUser=true;
        else
            // Major version 'X' matching... check if minor version
            bRejectUser = (iVersionMin > iMaximumVersionMin);

        // Check if release not under maintenance contract
        if(bRejectUser)
        {
            // Clean close of the license file
            fclose(getCryptedFile());
            setCryptedFile (NULL);
            setInternalError(LPMessage::eReject,"maintenance contract expired");

            return false;
        }
    }

    bool	bHostIdMatching;
    if(szString != mPrivate->GetSystemInfo().strHostID)
        bHostIdMatching = false;
    else
        bHostIdMatching = true;

    // HostID okay...but if it is '?', then the network board MUST match...
    bool bIgnoreNetworkID;
    if(*szString == '?')
      bIgnoreNetworkID = false;
    else
      bIgnoreNetworkID = true;

    mPrivate->ReadCryptedFile(szString);					// DiskID (='?' if Unix)
    if(mPrivate->GetSystemInfo().strDiskID != szString)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        return false;
    }

    mPrivate->ReadCryptedFile(szString);					// 1st Ethernet NetworkBoardID (only used to track CPU that did the 'Evaluation'
    if((bHostIdMatching == false) &&
        (mPrivate->GetSystemInfo().strNetworkBoardsIDs.indexOf(szString) < 0) &&
        (bIgnoreNetworkID == false))
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        return false;	// None of the Ethernet boards we have match the Board used for the license request...so its a different computer: Exit!
    }

    mPrivate->ReadCryptedFile(szString);					// List of ALL Ethernet NetworkBoardIDs
    mPrivate->ReadCryptedFile(&lData);					// Processors (=0 if Unix)
    if(lData != mPrivate->GetSystemInfo().lNumberOfProcessors)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        return false;
    }

    mPrivate->ReadCryptedFile(&lData);					// Processor type (=0 if Unix)
//    if(lData != cSystemInfo.lProcessorType)
//    {
//        // Clean close of the license file
//        fclose(hCryptedFile);
//        hCryptedFile = NULL;

//        return false;
//    }

    mPrivate->ReadCryptedFile(&lData);					// Processor level (=0 if Unix)
    if(lData != mPrivate->GetSystemInfo().lProcessorLevel)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        return false;
    }

    mPrivate->ReadCryptedFile(&lData);					// Processor revision (=0 if Unix)
    if(lData != mPrivate->GetSystemInfo().lProcessorRevision)
    {
        // Clean close of the license file
        fclose(getCryptedFile());
        setCryptedFile (NULL);

        return false;
    }

    mPrivate->ReadCryptedFile(szString);					// Maintenance expiration date (YYYY MM DD)
    if (iFileVersion > 100)
    {
        yy=0,mm=0,dd=0;
        int r=sscanf(szString,"%d %d %d",&yy,&mm,&dd);
        if (r==EOF)
            GSLOG(4, QString("failed ot interpret Maintenance Exp date '%1'").arg(szString).toLatin1().constData());
        sprintf(szString,"%d %d %d",yy,mm,dd);
        QDate MaintenanceExpirationDate;
        if(MaintenanceExpirationDate.setDate(yy,mm,dd) == false)
        {
            // Clean close of the license file
            fclose(getCryptedFile());
            setCryptedFile (NULL);

            if(bValidityCheckOnly == false)
            {
                GSLOG(4,
                       QString("%1%2").arg("Internal ERROR: Failed checking maintenance expiration date.\nPlease contact Quantix support at ").arg(GEX_EMAIL_SUPPORT).toLatin1().constData());
                *pbCriticalError = true;
                setInternalError(LPMessage::eReject,QString("%1%2").arg("Internal ERROR: Failed checking maintenance expiration date.\nPlease contact Quantix support at ")
                                 .arg(GEX_EMAIL_SUPPORT).toLatin1().constData());
                return false;
            }
            else
                return false;	// Failed processing license file...behave as if first installation: license request process
        }
        setLPData("MaintenanceExpirationDate", MaintenanceExpirationDate);
    }


    mPrivate->ReadCryptedFile(szString);					// Spare field#1
    mPrivate->ReadCryptedFile(szString);					// Spare field#2
    mPrivate->ReadCryptedFile(szString);					// Spare field#3

    mPrivate->uChecksum &= 0xffff;
    unsigned long uComputedChecksum = mPrivate->uChecksum;	// Get checksum of file...without checksum field!
    mPrivate->ReadCryptedFile(szString);					// Spare field#5...or checksum

    // Clean close of the license file
    fclose(getCryptedFile());
    setCryptedFile (NULL);

    if(strcmp(szString,"?") != 0)
    {
        // Verify checksum...
        unsigned long uFileChecksum=0;
        int r=sscanf(szString,"%lu",&uFileChecksum);
        if (r==EOF)
            GSLOG(4, QString("failed ot interpret File Checksum '%s'").arg(szString).toLatin1().constData());
        if(uFileChecksum != uComputedChecksum)
        {
            return false;	// Invalid checksum...file corrupted
        }
    }

    // GEX license type: 0=floating, 1= standalone, 2=Evaluation
    // License file is okay...so set the running mode flag: Standalone or Evaluation
    switch(iLicenseType)
    {
        case 1: // Standalone.
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);
            break;
        case 2: // Evaluation (Pro Edition)
            productInfo->setEditionID(GEX_EDITION_ADV);
            // fall into 'Evaluation'
        default: // Should never happen
            productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);
            break;
    }

    return true;
}

bool GexLicenseProvider::StartRunning(ProductInfo *productInfo)
{

    // Read selection mode
    int iSelectionMode = getSelectionMode();

  /*  QVariantList runningModes = getRunningModeAvailable();
    QString runningMode = runningModes[iSelectionMode].toString().section("|",1,1);

    // Saves in config file the current running mode selected.
    write_private_profile_string("Startup","RunningMode",
      runningMode.toLatin1().constData(), getConfigFilePath().toLatin1().constData()); */

    // If running 'Gex-Monitoring', to 'Server' anyway.
    //if(iForceRunningMode == GEX_DATATYPE_GEX_MONITORING)
    //	iSelectionMode = 1;	// Running mode = 'Server'.
    productInfo->setProductID(getProduct());
    bool ret = false;
    switch(iSelectionMode)
    {
        case GEX_RUNNINGMODE_STANDALONE:	// Standalone running mode
            ret = StartRunningStandalone(productInfo);
            break;
        case GEX_RUNNINGMODE_CLIENT:	// Client/Server running mode
            ret = StartRunningServer(productInfo);
            break;
        default:
            ret = StartRunningEvaluation(productInfo);
            break;
    }
    return ret;
}

///////////////////////////////////////////////////////////
// User requested to Start as 'Standalone'
///////////////////////////////////////////////////////////
bool GexLicenseProvider::StartRunningServer(ProductInfo *productInfo)
{
//    GSLOG(SYSLOG_SEV_DEBUG, QString("Start Running Server getEditField1(%1) getEditField2(%2)...")
    GSLOG(SYSLOG_SEV_DEBUG, QString("Start Running Server getEditField1(%2)...")
          .arg(getEditField1()).toLatin1().constData());
    QString strString;

    // Saves running mode.
    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_CLIENT);

    // Write server name
    strString = getEditField1().section(":", 0, 0);
    write_private_profile_string(getIniClientSection().toLatin1().constData(),
      "Server", strString.toLatin1().constData(), getConfigFilePath().toLatin1().constData());

    // Write socket port #
    strString = getEditField1().section(":", 1);;
    write_private_profile_string(getIniClientSection().toLatin1().constData(),
      "SocketPort", strString.toLatin1().constData(), getConfigFilePath().toLatin1().constData());

    GSLOG(SYSLOG_SEV_DEBUG, QString("OnConnectToServer...").toLatin1().constData());
    OnConnectToServer();
    if(mPrivate->m_pGexLmSocket)
    {
        if(mPrivate->m_pGexLmSocket->waitForConnected(-1))
            return true;
        else
        {
            QString lExtendedMessage;
            if(mPrivate->m_pGexLmSocket)
                lExtendedMessage = mPrivate->m_pGexLmSocket->errorString();


            QString lMessage = QString("Failed to connect to the server.\n"
                                       "The license manager is down,\n"
                                       "or the Server name / IP is incorrect!\n"
                                       "%1\n").arg(lExtendedMessage);

            if(!getAppConfigData("isDAEMON").toBool())
            {
                QStringList lUserChoiceList = QStringList () << lMessage
                                                             << "Ok"
                                                             << QString()
                                                             << QString();
                int lStatus = 0;
                emit userChoice(&lUserChoiceList, lStatus);
            }
            else
            {
                GSLOG(SYSLOG_SEV_NOTICE, lMessage.toLatin1().constData());
            }

            return false;
        }
    }
    return false;
}

///////////////////////////////////////////////////////////
// User requested to Start as 'Standalone'
///////////////////////////////////////////////////////////
bool GexLicenseProvider::StartRunningEvaluation(ProductInfo *productInfo)
{
    // Saves running mode.
    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_EVALUATION);

    // Default
    setLicenseeName ("Evaluation");

    // Create License Request for the 4days 'Evaluation' file.
    CreateLicenseRequestFile("Evaluation");
    createEvaluationLicense(productInfo);

    if(false)
    {
        QString notificationMess = buildUserNotificationMessage();
        emit notifyUser(&notificationMess);
        return false;
    }

    //emit sendLPMessage(LPMessage(LPMessage::eReject,"Evaluation file created"));
    setInternalError(LPMessage::eReject,"Evaluation file created");
    return false;
}

bool GexLicenseProvider::createEvaluationLicense(ProductInfo */*product*/)
{
    // End of 1st step process message:
    QString strString = "Thanks for entering your identification information.\n";
    strString += "To receive your license file, do the following:\n";
    strString += "Email the file: ";
    strString += mPrivate->strLicenseFile;

    strString += "\nTo: "+QString(GEX_EMAIL_LICENSE)+"\n";
    strString += "\nWithin a few minutes, an auto-reply message will\n";
    strString += "be returned to you with the license file and\n";
    strString += "the installation instructions.\n\nEnjoy GEX!";

    // Under windows, try to launch a Mail window!
#ifdef _WIN32
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;
    bool bWin95=true;	// If Win95, do not try to send the email 9would crash!).

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
    // If that fails, try using the OSVERSIONINFO structure.
    memset(&osvi,0,sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
    if (!bOsVersionInfoEx)
    {
        // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx((OSVERSIONINFO *)&osvi);
    } // if

    switch (osvi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        bWin95 = false;	// We have NT3, NT4, 2000, or XP Operating system!
        break;

    case VER_PLATFORM_WIN32_WINDOWS:
        if (osvi.dwMajorVersion > 4)
            bWin95 = false;	// Windows ME or higher
        else
            if (osvi.dwMajorVersion == 4)
            {
                if (osvi.dwMinorVersion >= 90)
                    bWin95 = false;	// Windows ME or higher
                else
                    if (osvi.dwMinorVersion >= 10)
                        bWin95 = false;	// Windows98
                    else
                        bWin95 = true;	// Windows 95
            } // if
        break;
    } // switch


    if(bWin95 == true)
    {
        // Do not send email, simply display a message box!
//        GS::Gex::Message::information("GEX License", strString);
        emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                     QString("Message::information|%1|%2").arg("GEX License").arg(strString)));
    }
    else
    {
        // Send the email!
        int iStatus = 0;
//        QMessageBox::information(NULL,"GEX License",
//                                               strString,
//                                               "Outlook users: Create Email Now!",
//                                               "Other users: email manually", 0, 0, 1 );

        QStringList userChoiceList = QStringList () << strString
                                                    << "Outlook users: Create Email Now!"
                                                    << "Other users: email manually"
                                                    << QString();
        emit userChoice(&userChoiceList, iStatus);

        if(iStatus != 0)
        {
//            QMessageBox::information(NULL,"GEX License",strString);
//            emit sendLPMessage(LPMessage(LPMessage::eExtended,
//                                         QString("Message::information|%1|%2").arg("GEX License").arg(strString)));
            userChoiceList.clear();
            QStringList userChoiceList = QStringList () << strString
                                                        << "OK"
                                                        << QString()
                                                        << QString();

            emit userChoice(&userChoiceList, iStatus);
            return true;
        }

        QString strEmail;
        strEmail = "mailto:"+QString(GEX_EMAIL_LICENSE)+"?subject=Request for GEX license activation";
        strEmail += "&body=%0A====> LAST STEP to perform:%0A";
        strEmail += "   1. ATTACH the file ";
        strEmail += "'" + mPrivate->strLicenseFile + "' to this email%0A";
        strEmail += "   2. Send this message!%0A%0A";
        strEmail += "Within few minutes, an auto-reply message will%0A";
        strEmail += "be returned to you with the license file and%0A";
        strEmail += "the installation instructions.%0A%0AEnjoy GEX!";
        // Launch email shell
        ULONG_PTR ulpStatus;
        ulpStatus = (ULONG_PTR) (ShellExecuteA(NULL,
                                               "open",
                                               strEmail.toLatin1().constData(),
                                               NULL,
                                               NULL,
                                               SW_SHOWNORMAL));
        iStatus = (int )(ulpStatus);
        if(iStatus <= 32)
        {
            // Failed creating
            strString = "*Failed* trying to create the email reply\nYou have to manually create it:\n\n" + strString;
//            GS::Gex::Message::warning("GEX License", strString);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("Message::information|%1|%2").arg("GEX License").arg(strString)));
        }
    }
#else
    // Under Unix, can't send email automatically: so simply display the instructions!
//    GS::Gex::Message::information("GEX License", strString);
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("Message::information|%1|%2").arg("GEX License").arg(strString)));

    // Also print this message to the console...so user still has the message after GEX closes!
    printf("\n\n\n\n");
    printf("\n************************************************\n");
    printf("%s",strString.toLatin1().constData());
    printf("\n************************************************\n");
#endif
    return true;
}

///////////////////////////////////////////////////////////
// User requested to Start as 'Standalone'
///////////////////////////////////////////////////////////
bool GexLicenseProvider::StartRunningStandalone(ProductInfo *productInfo)
{
    QString strString;
    bool	bCriticalError;

    // Saves running mode.
    productInfo->setLicenseRunningMode(GEX_RUNNINGMODE_STANDALONE);

    setLicenseeName(getEditField1());	// Licensee name
    if(getLicenseeName().isEmpty() == true)
    {
        GSLOG(4, "A valid user name is required for the license activation");
        //editField1->setFocus();
        //emit sendLPMessage(LPMessage(LPMessage::eReject,"A valid user name is required for the license activation"));
        setInternalError(LPMessage::eReject,"A valid user name is required for the license activation");
        return false;
    }

    // Get ProductKey entered by the user.
    bool	bBadKey=false;
    strString = getEditField2();	// License Key ID
    strString = strString.toUpper();	// uppercase
    if(strString.isEmpty() == true)
        bBadKey = true;
    if(strString.at(7) != '-')
        bBadKey = true;
    strString.truncate(4);
    if(strString != "GEX-")
        bBadKey = true;
    if(bBadKey == true)
    {
        GSLOG(4, "A valid Product Key is required for the license activation (eg: GEX-XYZ-123456)");
        //getEditField2();
        //emit sendLPMessage(LPMessage(LPMessage::eReject,"GEX-LP A valid Product Key is required for the license activation (eg: GEX-XYZ-123456)"));
        setInternalError(LPMessage::eReject,"A valid Product Key is required for the license activation (eg: GEX-XYZ-123456)");
        return false;
    }

    // Write user name
    strString = getEditField1();
    write_private_profile_string("Standalone","Username", strString.toLatin1().constData(), getConfigFilePath().toLatin1().constData());

    // Write KeyID entered
    strString = getEditField2();
    write_private_profile_string("Standalone","KeyID", strString.toLatin1().constData(), getConfigFilePath().toLatin1().constData());

    // Read license file (if any), if return true, then we have a local valid license!...then no need
    // to create a gex_request.txt !
    bool bAcceptLicense = true;
    bool bReadLicense = ReadLicenseFile(productInfo, true, &bCriticalError);

    bReadLicense = bReadLicense && (getEditField2() == getProductKey());

    if(bReadLicense)
        bAcceptLicense = mPrivate->CheckDisabledSupportPerpetualLicense(getProductKey());
    if(bReadLicense == false || bCriticalError || !bAcceptLicense)
    {
        // Local license file doesn't exist or not correct, then
        // consider we activate GEX and create a License request file

        if(!bAcceptLicense)
            GSLOG(4,
            "Your maintenance contract has expired. You can't run this recent release.\nContact the Quantix sales team to upgrade your contract.");

        // Create License Request 'Standalone' file...and forced Exit from GEX
        ProductInfo::getInstance()->setProductID(getProduct());
        setLicenseeName(getEditField1());
        setProductKey(getEditField2());
        CreateLicenseRequestFile("Standalone");
        createEvaluationLicense(productInfo);
        //emit sendLPMessage(LPMessage(LPMessage::eReject,"standalone license not granted"));
        setInternalError(LPMessage::eReject,"standalone license not granted");
        return false;

    }
    else
    {
        // We have a local VALID license...so allow GEX to start!
        // Clears the auto-launch config file: next run won't launch GEX in Server mode
        write_private_profile_string(getIniClientSection().toLatin1().constData(),"ServerFound","0", getConfigFilePath().toLatin1().constData());
        //done(1);
        emit sendLPMessage(LPMessage(LPMessage::eAccept,"standalone license granted"));
        return true;
    }
}

///////////////////////////////////////////////////////////
// Creates the license request file (for Evaluation
//	or Standalone running modes)
///////////////////////////////////////////////////////////
void	GexLicenseProvider::CreateLicenseRequestFile(QString strLicenseType)
{
    QString strString;
    // Check if license file is valid: $GEX_PATH/gex_license.txt
    mPrivate->strLicenseFile= getAppConfigData("UserFolder").toString();//GS::Gex::GexLicenseProvider::GetInstance().Get("UserFolder").toString(); //GetUserFolder(strLicenseFile);

    // Build license request file path+name
    mPrivate->strLicenseFile += "/gex_request.txt";// Path+file name of license key request file.

    mPrivate->hCryptedFile=fopen(mPrivate->strLicenseFile.toLatin1().constData(),"wb");
    if(mPrivate->hCryptedFile == NULL)
    {
        strString = "ERROR: Failed creating license request file:\n";
        strString += mPrivate->strLicenseFile;
        GSLOG(4,strString.toLatin1().constData());
//        done(0);
        return ;
    }

    // Get ProductKey entered by the user.
    strString = getEditField2();
    // write into the file the information we need !
    mPrivate->WriteCryptedFile(getAppConfigData("AppFullName").toString().toLatin1().data());				// 'Galaxy Examinator - VX.Y.Z'
    mPrivate->WriteCryptedFile(time(NULL));					// Current time/date
    strString = "Gex_Request " + strLicenseType;
    mPrivate->WriteCryptedFile(strString.toLatin1().constData());	// License request is from a GEX node...license type is 'Standalone' or 'Evaluation'
    mPrivate->WriteCryptedFile(mPrivate->strLicenseeName.toLatin1().constData());// User name

    // Platform : Windows, Solaris, HP-UX
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strPlatform.toLatin1().constData());
    // Computer name
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strHostName.toLatin1().constData());
    // Account logged when GEX executed.
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strAccountName.toLatin1().constData());

    strString = getEditField2();
    mPrivate->WriteCryptedFile(strString.toLatin1().constData());				// ProductKey
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strHostID.toLatin1().constData());	// HostID
    // DiskID (='?' if Unix)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strDiskID.toLatin1().constData());
    // 1st EthernetBoardID detected (MAC unique board ID)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strNetworkBoardID.toLatin1().constData());
    // List of ALL EthernetBoardID detected (MAC unique board ID)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().strNetworkBoardsIDs.toLatin1().constData());
    // Processors (=0 if Unix)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().lNumberOfProcessors);
    // Processor type (=0 if Unix)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().lProcessorType);
    // Processor level (=0 if Unix)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().lProcessorLevel);
    // Processor revision (=0 if Unix)
    mPrivate->WriteCryptedFile(mPrivate->GetSystemInfo().lProcessorRevision);
    // HACK for GTM: in order to do minimal changes in LicMan (knowing we will
    // switch to Flexera in V7.1), GTM is an optional module of GEXPRO. But
    // internally, we already use a specific Product ID
    if(mPrivate->mLegacyProduct/*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct()*/ == GEX_DATATYPE_GTM)
        strString = QString::number(GEX_DATATYPE_ALLOWED_DATABASE);
    else
        strString = QString::number(mPrivate->mLegacyProduct/*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct()*/);
    mPrivate->WriteCryptedFile(strString.toLatin1().constData());	// Product family type ID 0=Examinator, etc...
    mPrivate->WriteCryptedFile("?");							// Spare field#2
    mPrivate->WriteCryptedFile("?");							// Spare field#3
    mPrivate->WriteCryptedFile("?");							// Spare field#4
    mPrivate->WriteCryptedFile("?");							// Spare field#5
    mPrivate->WriteCryptedFile("?");							// Spare field#6


    fclose(mPrivate->hCryptedFile);
}

FILE * GexLicenseProvider::getCryptedFile()
{
    return mPrivate->hCryptedFile;
}

void GexLicenseProvider::setCryptedFile(FILE *file){
  mPrivate->hCryptedFile = file;
}

QString	GexLicenseProvider::getConfigFilePath()
{
    return mPrivate->strConfigFilePath;
}

QString GexLicenseProvider::getIniClientSection()
{
    return getAppConfigData("IniClientSection").toString();

}

QString GexLicenseProvider::getLicenseFile()
{
    return mPrivate->strLicenseFile;
}

QString GexLicenseProvider::getLicenseeName()
{
    return mPrivate->strLicenseeName;
}

QString GexLicenseProvider::getProductKey()
{
    return mPrivate->strProductKey;
}

QString GexLicenseProvider::getRunningMode()
{
    return mPrivate->strRunningMode;
}
unsigned long GexLicenseProvider::getChecksum()
{
    return mPrivate->uChecksum;
}

void GexLicenseProvider::setConfigFilePath(const QString &strString)
{
    mPrivate->strConfigFilePath = strString;
}

void GexLicenseProvider::setLicenseFile(const QString &strString)
{
    mPrivate->strLicenseFile = strString;
}

void GexLicenseProvider::setLicenseeName(const QString &strString)
{
    mPrivate->strLicenseeName = strString;
}

void GexLicenseProvider::setProductKey(const QString &strString)
{
    mPrivate->strProductKey = strString;
}

void GexLicenseProvider::setRunningMode(const QString &strString)
{
    mPrivate->strRunningMode = strString;
}

void GexLicenseProvider::setChecksum(unsigned long ulVal)
{
    mPrivate->uChecksum = ulVal;
}

QString GexLicenseProvider::getEditField2()
{
    return mPrivate->mEditField2;
}

void GexLicenseProvider::setEditField2(const QString &strString)
{
    mPrivate->mEditField2 = strString;
}

QString GexLicenseProvider::getEditField1()
{
    return mPrivate->mEditField1;
}

void GexLicenseProvider::setEditField1(const QString &strString)
{
    mPrivate->mEditField1 = strString;
}

int GexLicenseProvider::getSelectionMode()
{
    return mPrivate->miSelectionMode;
}

void GexLicenseProvider::setSelectionMode(int iVal)
{
    mPrivate->miSelectionMode = iVal;
}

GexLicenseProviderPrivate::GexLicenseProviderPrivate(LicenseProvider::GexProducts Product, QVariantMap &appConfigData)
    :mAppConfigData(appConfigData),mProduct(Product)
{
    mInternalError = GexLicenseProvider::eLPLibNoError;
    mInternalErrorMessage = "";
    mInitError = true;
    hCryptedFile = NULL;
    m_pGexLmSocket = NULL;
    mSocketStatusTimer = NULL;
    // GexLm connection
    m_strGexLmServerName = "?";
    m_strGexLmServerIP = "?";
    m_uiGexLmServerPort = 4242;
    m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;
    mLastSocketError = -2 ;
    miSelectionMode = 0;
}

GexLicenseProviderPrivate::~GexLicenseProviderPrivate()
{
    GSLOG(7, QString(" hCryptedFile : %1").arg(hCryptedFile?"not NULL !":"NULL").toLatin1().data() );
}

bool GexLicenseProviderPrivate::SetLegacyStuff(LicenseProvider::GexProducts Product)
{
    //Transform the Enum Product Type to Legacy Product Type
    int legacyProduct, legacyOptions;
    if(!ProductEnumToLegacy(Product, legacyProduct, legacyOptions))
    {
        GSLOG(SYSLOG_SEV_WARNING, "Cannot convert enum to product type");
        mInternalError = LicenseProvider::eLPProductNotFound;
        mInternalErrorMessage = QString("The requested product/edition (%1) is not supported by this license provider").arg((int)Product);
        return false;
    }
    else
    {
        mLegacyProduct = legacyProduct;
        mLegacyOptions = legacyOptions;
    }

    // Save server info into local file for further use...
    // Build access path to the .INI file !
#if defined unix || __MACH__
    // Get Home folder
    strConfigFilePath= mAppConfigData["UserFolder"].toString();//GS::Gex::GexLicenseProvider::GetInstance().Get("UserFolder").toString();    //GetUserFolder(strConfigFilePath);

    // Create .examinator folder to save all local info.
    {
        QDir cDir;
        strConfigFilePath += QString(GEX_LOCAL_UNIXFOLDER);
        cDir.mkdir(strConfigFilePath);
    }
#else
    // MS world: Get Home folder
    GSLOG(7, QString("1.*strConfigFilePath = mAppConfigData[UserFolder] (%1)")
       .arg(mAppConfigData["UserFolder"].toString()).toLatin1().data() );
    strConfigFilePath = mAppConfigData["UserFolder"].toString();//GS::Gex::GexLicenseProvider::GetInstance().Get("UserFolder").toString();    //GetUserFolder(strConfigFilePath);
#endif

    // Build path to local configuration file
    switch(mLegacyProduct/*GS::Gex::GexLicenseProvider::GetInstance().GetCommandLineOptions().GetProduct()*/)
    {
        case GEX_DATATYPE_ALLOWED_ANY:
        case GEX_DATATYPE_ALLOWED_DATABASE:	// ExaminatorDB
        case GEX_DATATYPE_GEX_TOOLBOX:		// Examinator ToolBox
        case GEX_DATATYPE_GEX_MONITORING:	// ExaminatorMonitoring
        case GEX_DATATYPE_GEX_PATMAN:		// Galaxy PAT-Man
        case GEX_DATATYPE_GTM:              // GTM (Galaxy Tester Monitor)
        default:
            // Config file for info to access PRIOR to process scripts.
            GSLOG(7, QString("2.strConfigFilePath = GetLocalConfigFilePath (%1)").arg(GetLocalConfigFilePath()).toLatin1().data() );
            strConfigFilePath = GetLocalConfigFilePath();
            break;
        case GEX_DATATYPE_ALLOWED_DATABASEWEB: // ExaminatorWeb
            // Config file for info to access PRIOR to process scripts.
            strConfigFilePath = strConfigFilePath + GEXWEB_CLIENT_NODE;
            break;
    }

    return true;
}

QString GexLicenseProviderPrivate::GetLocalConfigFilePath()
{
    QString strLocalConfigFilePath  = "";
    QString strUserFolderPath       = mAppConfigData["UserFolder"].toString();

    GSLOG(SYSLOG_SEV_DEBUG, "GetLocalConfigFilePath ...");
    GSLOG(SYSLOG_SEV_DEBUG,
          QString("1. strUserFolderPath= mAppConfigData[ UserFolder ] (%1)")
          .arg(strUserFolderPath).toLatin1().data() );

#if defined unix || __MACH__
    strUserFolderPath += GEX_LOCAL_UNIXFOLDER;
#endif

    QString strServerProfileFolder  = ProductInfo::getInstance()->GetProfileFolder();
    QString lServerProfile          = ProductInfo::getInstance()->GetProfileVariable();

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("1. Looking for profile variable %1").arg(lServerProfile).toLatin1().data() );

    if(strServerProfileFolder.isNull() == false &&
       strServerProfileFolder.isEmpty() == false)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("1. Profile folder found %1").arg(strServerProfileFolder).toLatin1().data());

        // Get env variable
        CGexSystemUtils::NormalizePath(strServerProfileFolder);

        // Server profile path  + file name
        strLocalConfigFilePath = strServerProfileFolder + GEX_CLIENT_NODE;

        // If file doesn't exist ni server profile folder, copy it from user folder to server profile folder
        if (!QFile::exists(strLocalConfigFilePath))
            QFile::copy(strUserFolderPath + GEX_CLIENT_NODE, strLocalConfigFilePath);
    }

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("1. strLocalConfigFilePath (%1)").arg(strLocalConfigFilePath).toLatin1().data());

    // If no location has been set, set it to the default location
    if (strLocalConfigFilePath.isEmpty())
        strLocalConfigFilePath = strUserFolderPath + GEX_CLIENT_NODE;

    GSLOG(SYSLOG_SEV_DEBUG,
          QString("=> strLocalConfigFilePath (%1)").arg(strLocalConfigFilePath).toLatin1().data() );

    return strLocalConfigFilePath;
}

///////////////////////////////////////////////////////////
// Check if this is a perpetual license, if so, ensure it has
// active maintenance to allow running this release!
///////////////////////////////////////////////////////////
bool GexLicenseProviderPrivate::CheckDisabledSupportPerpetualLicense(QString strCurrentLicense)
{
    QString strMessage = " CurrentLicense = " +strCurrentLicense;
    GSLOG(7, strMessage.toLatin1().data() );
    QDate clCompilationDate = QDate::fromString(QString(__DATE__).simplified(), "MMM d yyyy");
    QDate MaintenanceExpirationDate = mAppConfigData["MaintenanceExpirationDate"].toDate();

    if (MaintenanceExpirationDate != QDate(2007,4,20))
    {
        if (clCompilationDate <= MaintenanceExpirationDate)
            return true;
        else
            return false;	// This is a hold license without maintenance contract: refuse to run it!
    }

    // Holds list of perpetual licenses:
    // o format is <Product Key>[|<Maintenance expiration date>]
    // o if no maintenance date, the corrsponding Product key has no maintenance and should be rejected
    // o if a maintenance date is specified, the key is rejected if this date is prior the compilation date
    const char *	szPerpetualLicenses[]={
        "GEX-GFC-139104885",				// PAT-Man: ADI / External Foundry
        "GEX-CMA-120877281",				// PAT-Man: ADI / Cambridge
        "GEX-DBM-126143538",				// PAT-Man: ADI / Philippines
        "GEX-NBA-120877116|Nov 30 2012",	// PAT-Man: ADI / Wilmington
        "GEX-OKL-114138061",				// PAT-Man: ADI / Analog Limerick (Main server)
        "GEX-CCD-143336257",				// PAT-Man: ADI / Analog Limerick (Backup server)
        "GEX-JBB-129753400|Dec 31 2010",	// Yield-Man: TSMC Taiwan
        "GEX-MCF-256083275|Jun 15 2011",	// GEX-PAT : SSSL, UK
        "GEX-EPI-247023891|Mar 01 2011",	// GEX-PAT : Advanced Silicon

        //"GEX-IIB-107897255",				// GEXP-1: HongKong Science & Technology parks corp.

        //"GEX-HGE-234980486|Nov 15 2009",	// Galaxy - GEXDBP (+plugin).
        //"GEX-KGD-110592905",				// Galaxy - GEXDBP (+plugin).
        //"GEX-DNF-63227378",				// Examinator: GEXDBP-SS5 (Plugin + MO + PAT).
        //"GEX-BME-186791136",				// PAT-Man: Micronas
        //"GEX-EPI-247023891|Mar 02 2011",	// Examinator-PAT: Advanced Silicons

        ""	// End of list
    };

    QDate clMaintenanceExpirationDate;
    QString	strProductKey, strMaintenanceExpiration;

    const char * ptLicense;
    int	iIndex = 0;
    do
    {
        // Get license key from list
        ptLicense=szPerpetualLicenses[iIndex++];

        // Check if current license is in the exclusion list...
        if(*ptLicense)
        {
            strProductKey = QString(ptLicense).section("|", 0, 0);
            strMaintenanceExpiration = QString(ptLicense).section("|",1,1).simplified();
            if(strProductKey.toLower() == strCurrentLicense.toLower())
            {
                strMessage = "Perpetual license rejected because no valid maintenance contract";
                if(!strMaintenanceExpiration.isEmpty())
                {
                    clMaintenanceExpirationDate = QDate::fromString(strMaintenanceExpiration, "MMM dd yyyy");
                    if(!clMaintenanceExpirationDate.isValid())
                        clMaintenanceExpirationDate = QDate::fromString(strMaintenanceExpiration, "MMM d yyyy");
                    if(clMaintenanceExpirationDate.isValid())
                    {
                        if(clCompilationDate <= clMaintenanceExpirationDate)
                            return true;
                        strMessage += " (maintenance expired: " + clMaintenanceExpirationDate.toString("MMM dd yyyy") + " < ";
                        strMessage += clCompilationDate.toString("MMM dd yyyy") + ")";
                    }
                    else
                        strMessage += " (invalid maintenance expiration date: " + strMaintenanceExpiration + ")";
                }
                strMessage += ".";
                GSLOG(SYSLOG_SEV_EMERGENCY,strMessage.toLatin1().constData());

                return false;	// This is a old license without maintenance contract: refuse to run it!
            }
        }
    }
    while(*ptLicense);
    return true;
}

///////////////////////////////////////////////////////////
// Encrypt then write string to file.
///////////////////////////////////////////////////////////
void	GexLicenseProviderPrivate::WriteCryptedFile(const char *szString)
{
    CCryptoFile ascii;	// Buffer used for crypting data!
    char*		szAsciiKey;

    ascii.SetBufferFromBuffer(szString, (strlen(szString)+1)*sizeof(char));
    ascii.Encrypt_Buffer((unsigned char*)GEX_CRYPTO_KEY, GEX_KEY_BYTES);
    ascii.GetBufferToHexa(&szAsciiKey);
    fprintf(hCryptedFile,"%s\n",szAsciiKey);
    free(szAsciiKey);
}

///////////////////////////////////////////////////////////
// Encrypt then write data to file.
///////////////////////////////////////////////////////////
void	GexLicenseProviderPrivate::WriteCryptedFile(long lData)
{
    CCryptoFile ascii;	// Buffer used for crypting data!
    char	szString[2048];

    sprintf(szString,"%ld",lData);
    WriteCryptedFile(szString);
}


///////////////////////////////////////////////////////////
// Read one line from License file...ignore blank lines
///////////////////////////////////////////////////////////
bool GexLicenseProviderPrivate::ReadFileLine(char *szLine,int iMaxChar,FILE *hFile)
{
  *szLine = 0;
  QString strLine;

  // Loop until non-empty line found...or end of file!
  while(!feof(hFile))
  {
    if(fgets(szLine,iMaxChar,hFile) == NULL)
        return false;	// failed reading line...probably end of file!

    // Remove all leading spaces
    strLine = szLine;
    strLine = strLine.trimmed();
    strcpy(szLine,strLine.toLatin1().constData());
    if(strLine.isEmpty() == false)
        return true;	// Found valid (non empty string)
  };
  // end of file
  return false;
}

///////////////////////////////////////////////////////////
// Decrypt a string from license file.
///////////////////////////////////////////////////////////
void	GexLicenseProviderPrivate::ReadCryptedFile(char *szString)
{
    CCryptoFile ascii;	// Buffer used for de-crypting data!
    char*		szAsciiKey;

    *szString=0;
    // Read one line from file...but skip any blank line!
    if(ReadFileLine(szString,2047,hCryptedFile) == false)
        return;	// End of file, or error reading line in file.
    ascii.SetBufferFromHexa(szString);
    ascii.Decrypt_Buffer((unsigned char*)GEX_CRYPTO_KEY,GEX_KEY_BYTES);
    ascii.GetBuffer(&szAsciiKey);
    strcpy(szString,szAsciiKey);

    // Update checksum (all characters without CR-LF marker)
    char* szAsciiKeyTemp = szAsciiKey;
    while(*szAsciiKey)
    {
        uChecksum += *szAsciiKey;
        szAsciiKey++;
    };
    free(szAsciiKeyTemp);
}

///////////////////////////////////////////////////////////
// Decrypt a (long int) from file.
///////////////////////////////////////////////////////////
void GexLicenseProviderPrivate::ReadCryptedFile(long *lData)
{
    CCryptoFile ascii;	// Buffer used for de-crypting data!
    char	szString[2048];

    ReadCryptedFile(szString);
    *lData = 0;
    //int r=			// ToDo : analyse the return code of sscanf
    sscanf(szString,"%ld",lData);
    //if (r==EOF) // ToDo : return false
}

bool GexLicenseProviderPrivate::ProductEnumToLegacy(GS::LPPlugin::LicenseProvider::GexProducts eProduct, int &legacyProduct, int &legacyOptions)
{
    legacyOptions = 0;
    switch(eProduct)
    {
        case GS::LPPlugin::LicenseProvider::eNoProduct :
    default:
            return false;
            break;
        case GS::LPPlugin::LicenseProvider::eLtxcOEM :
            legacyProduct = GEX_DATATYPE_ALLOWED_LTXC;
            break;
        case GS::LPPlugin::LicenseProvider::eAslOEM :
            legacyProduct = GEX_DATATYPE_ALLOWED_CREDENCE_ASL;
            break;
        case GS::LPPlugin::LicenseProvider::eSzOEM :
            legacyProduct = GEX_DATATYPE_ALLOWED_SZ;
            break;
        case GS::LPPlugin::LicenseProvider::eSapphireOEM :
            legacyProduct = GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE;
            break;
        case GS::LPPlugin::LicenseProvider::eExaminatorEval :
            return false;
            break;
        case GS::LPPlugin::LicenseProvider::eExaminator :
            legacyProduct = GEX_DATATYPE_ALLOWED_ANY;
            break;
        case GS::LPPlugin::LicenseProvider::eExaminatorPro :
            legacyProduct = GEX_DATATYPE_ALLOWED_DATABASE;
            break;
        case GS::LPPlugin::LicenseProvider::eExaminatorPAT :
            legacyProduct = GEX_DATATYPE_ALLOWED_DATABASE;
            legacyOptions |= GEX_OPTIONAL_MODULE_PATMAN;
            break;
        case GS::LPPlugin::LicenseProvider::eYieldMan :
            legacyProduct = GEX_DATATYPE_GEX_MONITORING;
            break;
        case GS::LPPlugin::LicenseProvider::ePATMan :
            legacyProduct = GEX_DATATYPE_GEX_PATMAN;
            break;
        case GS::LPPlugin::LicenseProvider::eGTM :
            legacyProduct = GEX_DATATYPE_GTM;
            break;
    }
    return true;

}

bool GexLicenseProviderPrivate::LegacyProductToEnum(int legacyProduct, int legacyOptions,
                                                    GS::LPPlugin::LicenseProvider::GexProducts &product,
                                                    GS::LPPlugin::LicenseProvider::GexOptions &)
{
    switch(legacyProduct)
    {
    default :
        return false;
        break;
    case GEX_DATATYPE_ALLOWED_LTXC :
        product = GS::LPPlugin::LicenseProvider::eLtxcOEM;
        break;
    case GEX_DATATYPE_ALLOWED_CREDENCE_ASL :
        product = GS::LPPlugin::LicenseProvider::eAslOEM;
        break;
    case GEX_DATATYPE_ALLOWED_SZ :
        product = GS::LPPlugin::LicenseProvider::eSzOEM;
        break;
    case GEX_DATATYPE_ALLOWED_CREDENCE_SAPPHIRE :
        product = GS::LPPlugin::LicenseProvider::eSapphireOEM;
        break;
    case GEX_DATATYPE_ALLOWED_DATABASE :
    {
        if(legacyOptions & GEX_OPTIONAL_MODULE_PATMAN)
            product = GS::LPPlugin::LicenseProvider::eExaminatorPAT;
        else
            product = GS::LPPlugin::LicenseProvider::eExaminatorPro;
    }
        break;
    case GEX_DATATYPE_GEX_MONITORING :
        product = GS::LPPlugin::LicenseProvider::eYieldMan;
        break;
    case GEX_DATATYPE_GEX_PATMAN :
        product = GS::LPPlugin::LicenseProvider::ePATMan;
        break;
    case GEX_DATATYPE_GTM :
        product = GS::LPPlugin::LicenseProvider::eGTM;
        break;

    case GEX_DATATYPE_ALLOWED_DATABASEWEB :
        product = GS::LPPlugin::LicenseProvider::eExaminatorPro;
        break;
    case GEX_DATATYPE_GEX_TOOLBOX :
        product = GS::LPPlugin::LicenseProvider::eExaminator;
        break;
    case GEX_DATATYPE_ALLOWED_TERADYNE:
        product = GS::LPPlugin::LicenseProvider::eNoProduct ;
        break;
    case GEX_DATATYPE_GEX_YIELD123:
        product = GS::LPPlugin::LicenseProvider::eNoProduct ;
        break;
    case GEX_DATATYPE_ALLOWED_ANY :
        product = GS::LPPlugin::LicenseProvider::eExaminator;
        break;

    }
    return true;
}

bool GexLicenseProviderPrivate::LegacyOptionsToEnum(int legacyOptions, unsigned int &Options)
{
    if(legacyOptions & GEX_OPTIONAL_MODULE_SYL_SBL)
        Options |= GS::LPPlugin::LicenseProvider::eSYA;
    if(legacyOptions & GEX_OPTIONAL_MODULE_GENEALOGY)
        Options |= GS::LPPlugin::LicenseProvider::eGenealogy;
    if(legacyOptions & GEX_OPTIONAL_MODULE_TDR)
        Options |= GS::LPPlugin::LicenseProvider::eTDR;
    return true;
}


void GexLicenseProvider::CloseGexLmSocket(void)
{
    // Make sure socket connection gets free'ed
    if(getGexLmSocket())
    {
        disconnect(getGexLmSocket(), 0, 0, 0);

        // abort method calls close()
        getGexLmSocket()->abort();

        getGexLmSocket()->deleteLater();
        setGexLmSocket(NULL);
    }
    if(mPrivate->mSocketStatusTimer)
    {
        disconnect(mPrivate->mSocketStatusTimer, 0, 0, 0);

        // abort method calls close()
        mPrivate->mSocketStatusTimer->deleteLater();
        mPrivate->mSocketStatusTimer = NULL;
    }

    mPrivate->mLastSocketError = -2;
}

void GexLicenseProvider::OnConnectToServer()
{
    GSLOG(7, QString("On connect to server (LmSocket=%1").arg(mPrivate->m_pGexLmSocket?"ok":"NULL")
          .toLatin1().data() );

    QString strMessage;
    char szString[GEX_MAX_PATH+1];

    // Delete previous socket if exists (means it lost connection with the server)
    if(mPrivate->m_pGexLmSocket)
    {
        strMessage = "\tClosing Gex-Lm socket before connecting.";
        GSLOG(SYSLOG_SEV_INFORMATIONAL,strMessage.toLatin1().constData());
        CloseGexLmSocket();
    }

    // create the socket and connect various of its signals
    mPrivate->m_pGexLmSocket = new QTcpSocket;
    mPrivate->m_pGexLmSocket->moveToThread(thread());
    mPrivate->mSocketStatusTimer = new QTimer;
    mPrivate->mSocketStatusTimer->moveToThread(thread());
    mPrivate->mLastSocketError = -2 ;

    connect(mPrivate->mSocketStatusTimer, SIGNAL(timeout())
            , this, SLOT(OnCheckSocketStatus()));
    connect(mPrivate->m_pGexLmSocket, SIGNAL(connected())
             , this,SLOT(socketConnected()) );
    connect(mPrivate->m_pGexLmSocket, SIGNAL(disconnected())
             , this, SLOT(socketConnectionClosed()) );
    connect(mPrivate->m_pGexLmSocket, SIGNAL(readyRead())
             , this, SLOT(socketReadyRead()) );
    connect(mPrivate->m_pGexLmSocket, SIGNAL(error(QAbstractSocket::SocketError))
             ,this, SLOT(socketError(QAbstractSocket::SocketError)) );

    // connect to the server
    QString strServerName;   // Server name (if running in Client/Server mode)
    quint16 iSocketPort;   // Socket port # (if running in client/server mode)

    // Get server name
    get_private_profile_string(getIniClientSection().toLatin1().constData(),
                               "Server", "localhost", szString, GEX_MAX_PATH,
                               getConfigFilePath().toLatin1().constData());
    strServerName = szString;

    // Get socket port #
    iSocketPort = get_private_profile_int(getIniClientSection().toLatin1().constData(),
                                          "SocketPort", 4242,
                                          getConfigFilePath().toLatin1().constData());
    mPrivate->m_uiGexLmServerPort = iSocketPort;

    // Try to resolve server name
    hostent  *hServer;
    in_addr  inAddr;

    hServer = gethostbyname(szString);
    if(hServer)
    {
        // Specified server name is a valid name
        memcpy(&(inAddr.s_addr), hServer->h_addr, hServer->h_length);
        strServerName = inet_ntoa(inAddr);
        mPrivate->m_strGexLmServerName = szString;
        mPrivate->m_strGexLmServerIP = strServerName;
    }
    else
    {
        // Specified server name is an IP address
        setGexLmServerIP(QString(szString));
    }
    setLPData("ServerIP", mPrivate->m_strGexLmServerIP);
    setLPData("ServerName", mPrivate->m_strGexLmServerName);
    setLPData("SocketPort", QString::number(mPrivate->m_uiGexLmServerPort));


    // Show process bar...step 1 of 30 : timeout will occur if fails connecting to server within 30 seconds., hide '%' text
    //UpdateProgressStatus(true, GEX_CLIENT_TRY_CONNECT_TIMEOUT, 1, false);
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("UpdateProgressStatus|%1|%2|%3|%4")
                                 .arg(true).arg(GEX_CLIENT_TRY_CONNECT_TIMEOUT).arg(1).arg(false)));

    QString strString = " Connecting to: ";
    strString += szString;
    strString += "[" + strServerName + "]:";
    strString += QString::number(iSocketPort);
    strString += "         ";

    //UpdateLabelStatus(strString);
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("UpdateLabelStatus|%1").arg(strString)));

    // YM history log
//    GetSchedulerEngine().AppendMoHistoryLog("","Gex-Lm socket", strString);
//    emit sendLPMessage(LPMessage(LPMessage::eExtended,
//                                 QString("AppendMoHistoryLog|%1|%2|%3").arg("").arg("Gex-Lm socket").arg(strString)));

    strMessage = "Gex-Lm socket:" + strString;
    GSLOG(SYSLOG_SEV_INFORMATIONAL,strMessage.toLatin1().constData());

    // Socket connection request.
    mPrivate->m_pGexLmSocket->connectToHost(strServerName, iSocketPort);

    // Entering in status GEX_CLIENT_TRY_CONNECT
    mPrivate->dtSocketEvent = QDateTime::currentDateTime();
    mPrivate->m_iSocketStatus = GEX_CLIENT_TRY_CONNECT;

    // Extract Version# of this client application...
    int iVersionMaj,iVersionMin;
    // eg: "Visual Examinator - V6.1 B147 (Beta)"
    QString strRevision =  getAppConfigData("AppFullName").toString();//GetInstance().Get("AppFullName").toString();
    strRevision = strRevision.section('-',1);  // eg: " V6.1 B147 (Beta)"
    strRevision = strRevision.section('V',1);  // eg: "6.1 B147 (Beta)"
    iVersionMaj = strRevision.section('.',0,0).toInt();
    strRevision = strRevision.section('.',1);  // eg: "1 B147 (Beta)"
    strRevision = strRevision.section(' ',0,0);  // eg: "1"
    iVersionMin = strRevision.section('.',0,0).toInt();

    // Request a concurrent license
    QDateTime clDateTime = QDateTime::currentDateTime();
    QString strQueryServer;

    // Command string to send to server : GEX_RequestAdvLicense; <Local_ClientTime> ; <VersionMaj> ; <VersionMin> ; <ComputerPlatform> ; <ComputerName> ; <User> ; <GexFullAppName>
    // note Local_ClientTime is in format: YYYY MM DD HH MM
    strQueryServer = "GEX_RequestAdvLicense;";
    strQueryServer += clDateTime.toString("yyyy MM dd hh mm");
    strQueryServer += ";";
    strQueryServer += QString::number(iVersionMaj);  // Client node Software version (Major version#)
    strQueryServer += ";";
    strQueryServer += QString::number(iVersionMin);  // Client node Software version (Minor version#)
    strQueryServer += ";";
    strQueryServer += mPrivate->GetSystemInfo().strPlatform;
    strQueryServer += ";";
    strQueryServer += mPrivate->GetSystemInfo().strHostName;
    strQueryServer += ";";
    strQueryServer += mPrivate->GetSystemInfo().strAccountName;
    strQueryServer += ";";
    strQueryServer += getAppConfigData("AppFullName").toString();
    strQueryServer += ";";
    // 6953
    strQueryServer += getAppConfigData("ClientApplicationId").toString();
    mPrivate->mSocketStatusTimer->start(GEX_LP_SOCKET_STATUS_INTERVAL);
    // Send command to server (encrypted).
    WriteLineToSocket(strQueryServer);
//    QEventLoop loop;
//    loop.exec();
}

QString GexLicenseProvider::WriteLineToSocket(QString strMessage)
{
    //GSLOG(5, "%s", strMessage.toLatin1().data());

    CCryptoFile ascii; // Buffer used for crypting data!
    QTextStream os(mPrivate->m_pGexLmSocket);
    char*  szCryptedString=NULL;

    if (ascii.SetBufferFromBuffer(strMessage.toLatin1().constData(), (strMessage.length()+1)*sizeof(char))==0)
        GSLOG(3, "SetBufferFromBuffer failed !");
    if (ascii.Encrypt_Buffer((BYTE*)GEX_CRYPTO_KEY, GEX_KEY_BYTES)==0)
        GSLOG(3, "Encrypt_Buffer failed !");
    ascii.GetBufferToHexa(&szCryptedString);

    // Send command line server
    os << szCryptedString << "\n";
    free(szCryptedString);

    return "ok";
}

QString GexLicenseProvider::ReadLineFromSocket(void)
{
    CCryptoFile ascii; // Buffer used for crypting data!
    char*  szAsciiKey;
    QString  strServerMessage;

    strServerMessage = mPrivate->m_pGexLmSocket->readLine();
    strServerMessage = strServerMessage.trimmed(); // Remove leading '\n'
    ascii.SetBufferFromHexa(strServerMessage.toLatin1().constData());
    ascii.Decrypt_Buffer((BYTE*)GEX_CRYPTO_KEY,GEX_KEY_BYTES);
    ascii.GetBuffer(&szAsciiKey);
    QString strAsciiKey = szAsciiKey;
    free(szAsciiKey);
    return strAsciiKey;
}

///////////////////////////////////////////////////////////
// Client/Server running mode: Server is present!
///////////////////////////////////////////////////////////
void GexLicenseProvider::socketConnected()
{
    GSLOG(7, QString("socketConnected (LmSocket=%1").arg(mPrivate->m_pGexLmSocket?"ok":"NULL")
          .toLatin1().data() );

    // Success connecting to server: no check that we have a license granted yet!
    setSocketEvent(QDateTime::currentDateTime());
    mPrivate->m_iSocketStatus = GEX_CLIENT_GET_LICENSE;

    // Saves in confi file, so next run we don't ask again the server name!
    write_private_profile_string(getIniClientSection().toLatin1().constData(),"ServerFound","1",
                                 getConfigFilePath().toLatin1().constData());
}

void GexLicenseProvider::socketConnectionClosed()
{
    GSLOG(7, QString("socketConnectionClosed (LmSocket=%1").arg(mPrivate->m_pGexLmSocket?"ok":"NULL")
          .toLatin1().data() );

    QString strMessage;

    // About to exit....switch Socket status to idle (so no procee bar is updated!)
    mPrivate->m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;

    // Hide process bar
    //UpdateProgressStatus(true, 0, 0, false);
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
    //UpdateLabelStatus();
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("UpdateLabelStatus|%1").arg("")));

    // YM history log
    strMessage = "'connection closed' signal received.";
    //GetSchedulerEngine().AppendMoHistoryLog("","Gex-Lm socket", strMessage);
    emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                 QString("AppendMoHistoryLog|%1|%2|%3").arg("").arg("Gex-Lm socket").arg(strMessage)));

    // Trace Massage
    strMessage = "Socket connection closed : Gex-Lm socket: " + strMessage;
    GSLOG(SYSLOG_SEV_NOTICE, strMessage.toLatin1().constData());

    strMessage =
        "Server closed the connection!\n"
        "Either :\n"
        "- the GalaxySemi license manager on the server was closed\n"
        "- the network is experiencing problems\n"
            "- the software was not replying during a too much long time.";


    disconnectionSender(QString("%1|%2").arg(GS::Error::SERVER_CONNECTION_CLOSE).arg(strMessage));
}

void GexLicenseProvider::OnCheckSocketStatus()
{
//    GSLOG(7, QString("OnCheckSocketStatus (LmSocket=%1").arg(mPrivate->m_pGexLmSocket?"ok":"NULL")
//          .toLatin1().data() );
//    QString senderMessage = QString("<< %1 >> OnCheckSocketStatus details : %2 %3")
//            .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
//            .arg(QString().sprintf("%p", sender()))
//            .arg(senderSignalIndex());

//    GSLOG(7, senderMessage.toLatin1().data() );


//    GSLOG(7, QString("OnCheckSocketStatus (%1)").arg(getSocketStatus()).toLatin1().data() );

    // Check if Client/Server GEX currently waiting for a connection to complete...
    QString		lMessage;
    QDateTime	lNow;
    int			lSecs = 0;
    mPrivate->mSocketStatusTimer->stop();
    // Check socket action status (if any). Note: if gex in standalone or demo, status = GEX_CLIENT_SOCKET_IDLE
    switch(getSocketStatus())
    {
    // We are waiting for connection to server to complete...
    case GEX_CLIENT_TRY_CONNECT:
        lSecs   = getSocketEvent().secsTo(QDateTime::currentDateTime());

        // Show process bar...step 1 of 30 : timeout will occur if fails connecting to server
        // within 30 seconds.(hide '%' text)
        //UpdateProgressStatus(true, GEX_CLIENT_TRY_CONNECT_TIMEOUT, lSecs, false);
        emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                     QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(GEX_CLIENT_TRY_CONNECT_TIMEOUT).arg(lSecs).arg(false)));

        if (lSecs > GEX_CLIENT_TRY_CONNECT_TIMEOUT)
        {
            if (!getAppConfigData("HiddenMode").toBool())
            {
                // Reset status... avoids multiple callbacks
                // to this function until user clicks 'OK'
                setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
                lMessage =
                        "Failed to connect to the server.\n"
                        "The license manager is down,\n"
                        "or the Server name / IP is incorrect!\n";


                disconnectionSender(QString("%1|%2").arg(GS::Error::NETWORK_TIMEOUT).arg(lMessage));

                //if (ShouldConnectAgain((const int)GS::Error::NETWORK_TIMEOUT,lMessage))
                //{
                //    // Do not close application,
                //    // user confirmed to Try to connect to server again
                //    break;
                //}

                // Clears the auto-launch config file :
                // next run will display welcome page to
                // allow changing the hostname/port# if needed
                write_private_profile_string(getIniClientSection().
                                             toLatin1().constData(),
                                             "ServerFound", "0",
                                             getConfigFilePath().
                                             toLatin1().constData());
            }

            // Abort program
            GSLOG(SYSLOG_SEV_ERROR, " OnCheckSocketStatus : GEX_CLIENT_TRY_CONNECT_TIMEOUT");
            //            Exit(GS::Error::CLIENT_TRY_CONNECT_TIMEOUT);
            //return;
        }
        break;

        // Waiting License acknowledge from server
    case GEX_CLIENT_GET_LICENSE:
        lSecs = getSocketEvent().secsTo(QDateTime::currentDateTime());

        // Show process bar...step 1 of 30 : timeout will occur if fails connecting to server
        // within 30 seconds. Hide '%' text
        //        UpdateProgressStatus(true, GEX_CLIENT_TRY_CONNECT_TIMEOUT, lSecs, false);
        emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                     QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(GEX_CLIENT_TRY_CONNECT_TIMEOUT).arg(lSecs).arg(false)));

        if(lSecs > GEX_CLIENT_TRY_CONNECT_TIMEOUT)
        {
            // Reset status...avoids multiple callbacks to this function until user clicks 'OK'!
            setSocketStatus(GEX_CLIENT_SOCKET_IDLE);

            // Connection failed as no reply from server arrived...abort!
            lMessage = "The license manager 'gex-lm' on the server is not responding\nThe server or network may be down\n";

            //            if (ShouldConnectAgain((const int)GS::Error::NETWORK_TIMEOUT, lMessage))
            //                break;	// Do not close application, user confirmed to Try to connect to server again...
            disconnectionSender(QString("%1|%2").arg(GS::Error::NETWORK_TIMEOUT).arg(lMessage));

            // Clears the auto-launch config file: next run will display welcome page...to allow changing the hostname/port# if needed.
            write_private_profile_string(getIniClientSection().toLatin1().constData(),
                                         "ServerFound","0",
                                         getConfigFilePath().toLatin1().constData());

            // Abort program!
            GSLOG(SYSLOG_SEV_ERROR, " OnCheckSocketStatus : GEX_CLIENT_GET_LICENSE");

            //Exit(EXIT_FAILURE);
            //return;
        }
        break;

        // GEX license granted...GEX now simply need to tell periodically to the server that it is still 'ALIVE'
    case GEX_CLIENT_REGISTERED:
        lNow    = QDateTime::currentDateTime();
        lSecs   = getSocketEvent().secsTo(lNow);

        if(lSecs > GEX_CLIENT_ALIVE_TIMEOUT)
        {
            // Send message to server.
            WriteLineToSocket("GEX_NodeAlive");
            // Reset timer value...so next 'Alive' message is in 1 minute.
            setSocketEvent(lNow);
        }

        // If not Yield-Man/PAT-Server/GTM:
        // check if node ready + client user has been idle too long...in which case propose to close the application
        if(!(mPrivate->mProductInfo->isYieldMan())
                && !(mPrivate->mProductInfo->isPATMan())
                && !mPrivate->mProductInfo->isGTM())
        {
            lSecs			= mPrivate->mLastUserActivity.secsTo(lNow);
            int lAutoClose	= getAppConfigData("TimeBeforeAutoClose").toInt();
            if((lAutoClose > 0) && (lAutoClose < lSecs))
            {
                // Reset status...avoids multiple callbacks to this function until user clicks 'OK'!
                setSocketStatus(GEX_CLIENT_SOCKET_IDLE);

                lMessage = "Software was idle for too long: license has been released to other ";
                lMessage += "users.\nNote: You can change the timeout value from the 'Options' ";
                lMessage += "tab,\nin the section 'Environment preferences'";

                //                if (ShouldConnectAgain((const int)GS::Error::SOFTWARE_IDLE, lMessage) == false)
                //                {
                //                    GSLOG(SYSLOG_SEV_NOTICE, "Reconnection manually refused");
                //                    Exit(EXIT_FAILURE);	// User asked not to try again but abort...
                //                    return;
                //                }
                disconnectionSender(QString("%1|%2").arg(GS::Error::SOFTWARE_IDLE).arg(lMessage));
                GSLOG(SYSLOG_SEV_ERROR, " OnCheckSocketStatus : GEX_CLIENT_REGISTERED");
            }
        }
        break;

        // All licenses already in use, prompt user if want to try again?
    case GEX_CLIENT_ALLUSED:
        // Reset status...avoids multiple callbacks to this function until user clicks 'OK'!
        setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
        lMessage = "Sorry, all licenses are already in use!\nIf this happens too often, ";
        lMessage += "contact Quantix\nto order more licenses: " + QString(GEX_EMAIL_SALES);
        //        if (ShouldConnectAgain( (const int)GS::Error::ALL_LICENSES_USED, lMessage) == false)
        //        {
        //            GSLOG(SYSLOG_SEV_WARNING, "GEX_CLIENT_ALLUSED");
        //            Exit(EXIT_FAILURE);	// User asked not to try again but abort...
        //            return;
        //        }
        disconnectionSender(QString("%1|%2").arg(GS::Error::ALL_LICENSES_USED).arg(lMessage));
        GSLOG(SYSLOG_SEV_ERROR, " OnCheckSocketStatus : GEX_CLIENT_ALLUSED");
        break;

    case GEX_CLIENT_SOCKET_IDLE:	// IDle...
        break;

    default:
        break;
    }
    mPrivate->mSocketStatusTimer->start(GEX_LP_SOCKET_STATUS_INTERVAL);
}

QTcpSocket* GexLicenseProvider::getGexLmSocket(){
    return mPrivate->m_pGexLmSocket;
}

void GexLicenseProvider::setGexLmSocket(QTcpSocket*	pGexLmSocket){
    mPrivate->m_pGexLmSocket = pGexLmSocket;

}

quint16 GexLicenseProvider::getGexLmServerPort(){
    return mPrivate->m_uiGexLmServerPort;
}

void GexLicenseProvider::setGexLmServerPort(quint16 uiPort){
    mPrivate->m_uiGexLmServerPort = uiPort;

}

QString	GexLicenseProvider::getGexLmServerName(){
    return mPrivate->m_strGexLmServerName;

}

void GexLicenseProvider::setGexLmServerName(const QString&strName ){
    mPrivate->m_strGexLmServerName = strName;

}

QString GexLicenseProvider::getGexLmServerIP(){
    return mPrivate->m_strGexLmServerIP;

}

void GexLicenseProvider::setGexLmServerIP(const QString &strIp){
    mPrivate->m_strGexLmServerIP = strIp;
}

QDateTime GexLicenseProvider::getSocketEvent(){
    return mPrivate->dtSocketEvent;

}

void GexLicenseProvider::setSocketEvent(const QDateTime &rDateTime){
    mPrivate->dtSocketEvent = rDateTime;
}

int GexLicenseProvider::getSocketStatus(){
    return mPrivate->m_iSocketStatus;

}

void GexLicenseProvider::setSocketStatus(int iVal){
    mPrivate->m_iSocketStatus = iVal;
}

///////////////////////////////////////////////////////////
// Client/Server running mode:
///////////////////////////////////////////////////////////
void GexLicenseProvider::socketReadyRead()
{
//    GSLOG(7, QString("socketReadyRead (LmSocket=%1").arg(mPrivate->m_pGexLmSocket?"ok":"NULL")
//          .toLatin1().data() );
    // Make sure socket is still alive
    if(!mPrivate->m_pGexLmSocket)
        return;
    mPrivate->mSocketStatusTimer->stop();
    // Server has sent a message...process it.
    QString strServerMessage;
    while(mPrivate->m_pGexLmSocket->canReadLine())
    {
        // Make sure socket is still alive
        if(!mPrivate->m_pGexLmSocket)
            break;

        // Read line, then parse it.
        strServerMessage = ReadLineFromSocket();
        //GSLOG( 7, "socket ready read : %s", strServerMessage.toLatin1().data() );
        if(strServerMessage.startsWith("GEX_ACCEPT") == true)
        {
            // Received license ok string: GEX_ACCEPT ; YYYY MM DD; ProductID
            QString strString;
            int  yy=0,mm=0,dd=0;

            // OK. set new client state
            mPrivate->m_iSocketStatus = GEX_CLIENT_REGISTERED;

            // Get License expiration date from string received
            strString = strServerMessage.section(';',1,1); // Expiration date
            sscanf(strString.toLatin1().constData(),"%d %d %d",&yy,&mm,&dd);

            QDate ed;
            ed.setDate(yy,mm,dd);
            setLPData("ExpirationDate",ed);

            // Get Running mode: Examinator (default), ExaminatorDB?, etc...UNLESS we're running Examinator-MONITORING!
            int  iProductID = GEX_DATATYPE_ALLOWED_ANY;
            strString = strServerMessage.section(';',2,2); // ProductID
            if(strString.isEmpty() == false)
                sscanf(strString.toLatin1().constData(),"%d",&iProductID);

            // Get edition type
            int  iEditionType = GEX_EDITION_STD;
            strString = strServerMessage.section(';',3,3); // EditionType: 0=Standard, 1=Professional
            if(strString.isEmpty() == false)
            {
                sscanf(strString.toLatin1().constData(),"%d",&iEditionType);
                switch(iEditionType)
                {
                case 0:
                default:
                    iEditionType = GEX_EDITION_STD;
                    break;
                case 1:
                    iEditionType = GEX_EDITION_ADV;
                    break;
                }
            }

            // Get nb. of allowed products for Monitoring
            int  iMonitorProducts = 0;
            strString = strServerMessage.section(';',4,4); // Maximum ProductsIDs allowed in Monitoring (-1 = unlimited)
            if(strString.isEmpty() == false)
                sscanf(strString.toLatin1().constData(),"%d",&iMonitorProducts);

            // Get optional modules: Bit0=Plugin, Bit1=PAT, Bit2=Y123-Web etc...
            int uOptionalModules = 0;
            strString = strServerMessage.section(';',5,5);
            if(strString.isEmpty() == false)
                sscanf(strString.toLatin1().constData(),"%d",&uOptionalModules);

            // HACK for GTM: in order to do minimal changes in LicMan (knowing we will
            // switch to Flexera in V7.1), GTM is an optional module of GEXPRO. But
            // internally, we already use a specific Product ID
            if(mPrivate->mProductInfo->isGTM())
            {
                // Application started in GTM mode
                // If GTM activated in license file, switch Product ID
                if( (iProductID == GEX_DATATYPE_ALLOWED_DATABASE) &&
                        (uOptionalModules & GEX_OPTIONAL_MODULE_GTM))
                    iProductID = GEX_DATATYPE_GTM;
                // If license does not allow GTM, reject license file
                if(iProductID != GEX_DATATYPE_GTM)
                {
                    GSLOG(SYSLOG_SEV_EMERGENCY,"Your License doesn't allow you to run the 'GTM' module. System will exit now.");
                    QString m="Your current license does not allow you to run the 'GTM' module.\nYou need to upgrade your license to do so.\nContact "+QString(GEX_EMAIL_SALES)+" for more details.\n\nGEX will now exit.";
//                    Message::information(Get("AppFullName").toString(), m);
//                    Exit(EXIT_FAILURE);
                    emit sendLPMessage(LPMessage(LPMessage::eReject,m));
                    break;
                }
            }

            // Set edition type received from server
            mPrivate->mProductInfo->setEditionID(iEditionType);

            // Set nb. of allowed products for Monitoring
            mPrivate->mProductInfo->setMonitorProducts(iMonitorProducts);

            // If running mode is 'Gex-Monitoring', we need to have 'iMonitorProducts' > 0!
            if(mPrivate->mProductInfo->isMonitoring() && iMonitorProducts == 0)
            {
                GSLOG(SYSLOG_SEV_EMERGENCY,"Your License doesn't allow you to run the 'Yield-Man' module. System will exit now.");
                QString m="Your current license does not allow you to run the 'Yield-Man' module.\nYou need to upgrade your license to do so.\nContact "+QString(GEX_EMAIL_SALES)+" for more details.\n\nGEX will now exit.";
//                Message::information(Get("AppFullName").toString(),  m);
                emit sendLPMessage(LPMessage(LPMessage::eReject,m));
//                Exit(EXIT_FAILURE);
                break;
            }

            // Run YM even if the licence has PAT
            if ((iProductID == GEX_DATATYPE_GEX_MONITORING)
                || ((iProductID == GEX_DATATYPE_ALLOWED_DATABASE) && !(mPrivate->mLegacyOptions & GEX_OPTIONAL_MODULE_PATMAN)))
            {
                uOptionalModules =
                        uOptionalModules & ~GEX_OPTIONAL_MODULE_PATMAN;
            }

            unsigned int lpOptions = LicenseProvider::eNoOptions;

            mPrivate->LegacyOptionsToEnum(uOptionalModules, lpOptions);
            mPrivate->mProductInfo->setOptionalModules(lpOptions);

            if ((iProductID == GEX_DATATYPE_GEX_PATMAN) && ! (uOptionalModules & GEX_OPTIONAL_MODULE_PATMAN))
            {
                mPrivate->mProductInfo->setProductID(eYieldMan);

                QString m =
                        "Your license does not support the PAT module.\n"
                        "The application will be started in Yield-Man mode.";
                //Message::information(Get("AppFullName").toString(),  m);
                emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                             QString("Message::information|%1").arg(m)));

            }
            else if ((mPrivate->mLegacyOptions & GEX_OPTIONAL_MODULE_PATMAN) &&
                     ! (uOptionalModules & GEX_OPTIONAL_MODULE_PATMAN))
            {

                //                GS::Gex::LicenseProvider::getInstance()->setPatSupport(false);

                QString m =
                        "Your license does not support the PAT module.\n"
                        "The application will be started in Examinator-Pro mode.";
//                Message::information(Get("AppFullName").toString(), m);
                emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                             QString("Message::information|%1").arg(m)));
            }

//            // Rebuild script paths in case the running mode has changed dynamically (from license file ID or server ID provided)
//            // InitScriptsPath(true);
//            emit sendLPMessage(LPMessage(LPMessage::eExtended,
//                                         QString("InitScriptsPath|%1").arg(true)));


            // Admin Server
            strString = strServerMessage.section(';',6,6).simplified();
            mPrivate->mYMAdminDBConfig = strString;
//            emit sendLPMessage(LPMessage(LPMessage::eSetYMAdminDBConfig,
//                                                     strString));
            // Note: GEX will periodically tell the server that he is still alive...
            mPrivate->dtSocketEvent = QDateTime::currentDateTime();

//            if(mPrivate->m_eClientState == eState_ReConnect)
//                SetClientState(eState_NodeReady);   // If reconnect, no additional scripts to be loaded, switch client state to ready
//            else
//            {
//                SetClientState(eState_LicenseGranted); // If first connect, the client still has some tasks to perform before being ready (run scripts corresponding to the license type, user scripts...)

//                qApp->postEvent(this, new QEvent(QEvent::Type(EVENT_LICENSE_GRANTED)));
//            }
            emit sendLPMessage(LPMessage(LPMessage::eAccept,"GEX-LM accept to grant license"));
        }
        else if(strServerMessage.startsWith("GEX_REJECT_ALLUSED") == true)
        {
            // License request rejected as all licenses are already in use

            // Set flag to prompt user if want to try again or a abort!
            mPrivate->m_iSocketStatus = GEX_CLIENT_ALLUSED;
            // Hide process bar
            //UpdateProgressStatus(true, 0, 0, false);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
//            UpdateLabelStatus();
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateLabelStatus|%1").arg("")));
        }
        else if(strServerMessage.startsWith("GEX_REJECT_BADOS") == true)
        {
            // Server is not of same OS platform as GEX....refuse license!...we have to EXIT GEX!

            // About to exit....switch Socket status to idle (so no procee bar is updated!)
            mPrivate->m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;
            // Hide process bar
            //UpdateProgressStatus(true, 0, 0, false);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
            //UpdateLabelStatus("");
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateLabelStatus|%1").arg("")));

//            Message::information(Get("AppFullName").toString(),
//                                 "Sorry, License server is not running on same Operating system.\nGEX will now exit.");
//            Exit(EXIT_FAILURE);
            emit sendLPMessage(LPMessage(LPMessage::eReject,"Sorry, License server is not running on same Operating system.\nGEX will now exit."));
            break;
        }
        else if(strServerMessage.startsWith("GEX_REJECT_BADTIMEZONE") == true)
        {
            // Server is not on same timezone...refuse to service this client!

            // About to exit....switch Socket status to idle (so no procee bar is updated!)
            mPrivate->m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;
            // Hide process bar
            //UpdateProgressStatus(true, 0, 0, false);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
            //UpdateLabelStatus("");
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateLabelStatus|%1").arg("")));

//            Message::information(Get("AppFullName").toString(),
//                                 "Sorry, Server is not on your time zone.\nGEX will now exit.");
            //Exit(EXIT_FAILURE);
             emit sendLPMessage(LPMessage(LPMessage::eReject,"Sorry, Server is not on your time zone.\nGEX will now exit."));
            break;
        }
        else if(strServerMessage.startsWith("GEX_REJECT_MAINTENANCE_CONTRACT") == true)
        {
            // About to exit....switch Socket status to idle (so no procee bar is updated!)
            mPrivate->m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;
            // Hide process bar
            //UpdateProgressStatus(true, 0, 0, false);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
            //UpdateLabelStatus("");
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateLabelStatus|%1").arg("")));

//            Message::information(Get("AppFullName").toString(),
//                                 "Your maintenance contract has expired. You can't run this recent release.\nContact the Quantix sales team to upgrade your contract.\nGEX will now exit.");
            //Exit(EXIT_FAILURE);
             emit sendLPMessage(LPMessage(LPMessage::eReject,"Your maintenance contract has expired. You can't run this recent release.\nContact the Quantix sales team to upgrade your contract.\nGEX will now exit."));
            break;
        }
        else if(strServerMessage.startsWith("GEX_BAD_COMMAND") == true)
        {
            GSLOG(3, "GexLM returned GEX_BAD_COMMAND !");
            // About to exit....switch Socket status to idle (so no process bar is updated!)
            mPrivate->m_iSocketStatus = GEX_CLIENT_SOCKET_IDLE;
            // Hide process bar
            //UpdateProgressStatus(true, 0, 0, false);
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateProgressStatus|%1|%2|%3|%4").arg(true).arg(0).arg(0).arg(false)));
            //UpdateLabelStatus("");
            emit sendLPMessage(LPMessage(LPMessage::eExtended,
                                         QString("UpdateLabelStatus|%1").arg("")));

//            Message::information(Get("AppFullName").toString(),
//                                 "Network error or old License Manager (recent version required).\nGEX will now exit.");
            emit sendLPMessage(LPMessage(LPMessage::eReject,"Network error or old License Manager (recent version required).\nGEX will now exit."));
            //Exit(EXIT_FAILURE);
            break;
        }
    };
    mPrivate->mSocketStatusTimer->start(GEX_LP_SOCKET_STATUS_INTERVAL);
}

///////////////////////////////////////////////////////////
// Client/Server running mode:
///////////////////////////////////////////////////////////
void GexLicenseProvider::socketError(QAbstractSocket::SocketError socketError)
{
    GSLOG(7, QString("socketError (LmSocket=%1) ()").arg(mPrivate->m_pGexLmSocket?"ok":"NULL").toLatin1().constData());
    QString extendedMessage = mPrivate->m_pGexLmSocket->errorString();
    QString senderAdress = QString().sprintf("%p", sender());
    QString senderMessage = QString("%1 %2").arg(senderAdress).arg(senderSignalIndex());
    GSLOG(7, QString(">> socketError Error (%1) (senderMessage=%2) extendedMessage(%3)").arg(socketError).arg(senderMessage).arg(extendedMessage).toLatin1().constData());

     // Error code may be one of:
    // QSocket::ErrConnectionRefused - if the connection was refused
    // QSocket::ErrHostNotFound - if the host was not found
    // QSocket::ErrSocketRead - if a read from the socket failed
    // Message("socket error!");
    setSocketStatus(GEX_CLIENT_SOCKET_IDLE);
    QString lMessage =
            QString("Failed to connect to the server.\n"
                    "The license manager is down,\n"
                    "or the Server name / IP is incorrect!\n"
                    "%1\n").arg(extendedMessage);

    if(mPrivate->mLastSocketError != socketError)
    {
        mPrivate->mLastSocketError = socketError;
        if(mPrivate->mLastSocketError != QAbstractSocket::RemoteHostClosedError)
        {
            if(!mPrivate->mInitError)
                disconnectionSender(QString("%1|%2|%3").arg(GS::Error::NETWORK_TIMEOUT).arg(lMessage).arg(mPrivate->mInitError?1:0));
    }
        mPrivate->mInitError = false;
    }

}



void GexLicenseProvider::disconnectionSender(const QString &message)
{
    GSLOG(7, QString("disconnectionSender (LmSocket=%1) ()").arg(mPrivate->m_pGexLmSocket?"ok":"NULL").toLatin1().constData());
    GEX_ASSERT(message.split("|").count() == 2);
    GSLOG(SYSLOG_SEV_DEBUG, QString(">> Emit disconnection with message :<%1>").arg(message).toLatin1().constData());
    emit sendLPMessage(LPMessage(LPMessage::eDisconnected, message));
    mPrivate->mSocketStatusTimer->stop();
}

}
}
