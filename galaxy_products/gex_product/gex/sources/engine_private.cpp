#include <QApplication>
#include <QDir>
#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <gqtl_log.h>
#include <gqtl_webserver.h>
#include <gqtl_sysutils.h>
#include <gqtl_filelock.h>
#include <QNetworkCookie>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    #include <tlhelp32.h>
    #undef CopyFile
#endif

#include "engine.h"
#include "engine_private.h"
#include "gex_scriptengine.h"
#include "product_info.h"
#include "scheduler_engine.h"
#include "admin_engine.h"
#include "report_options.h"
#include "message.h"
#include "gex_constants.h"

extern GexScriptEngine* pGexScriptEngine;
extern CReportOptions ReportOptions;
extern bool UpdateGexScriptEngine(CReportOptions * pReportOptions); // todo : move me in engine

namespace GS
{
    namespace Gex
    {
        EnginePrivate::EnginePrivate(QObject* lParent) : QObject(lParent),
            //mAppFullName("?"), // GEX_APP_EXAMINATOR
            mWebServer(this),
            mExitCode(0),
            mAdminEngine(this),
            mSchedulerEngine(this),
            mDatabaseEngine(this),
            mSYAEngine(),
            mSPMEngine(),
            mLastUserActivity(QDateTime::currentDateTime())
  #ifdef GCORE15334
          ,
            mTesterServer(this)
  #endif
        {
            setObjectName("GSEnginePrivate");
            setProperty("AppVersionMajor", GEX_APP_VERSION_MAJOR);
            setProperty("AppVersionMinor", GEX_APP_VERSION_MINOR);
            setProperty("AppVersionBuild", GEX_APP_VERSION_BUILD);
            setProperty("ExpirationDate", QDate(2007,4,20));
            setProperty("MaintenanceExpirationDate", QDate(2007,4,20));
            setProperty("AppBuildDate", QDate::fromString(__DATE__, "MMM dd yyy"));

            mNetworkAccessManager.setObjectName("GSNetworkAccessManager");
            mNetworkCookieJar.setObjectName("GSNetworkCookieJar");
            mWebServer.setObjectName("GSWebServer");
            mScriptEngineDebugger.setObjectName("GSScriptEngineDebugger");

            /*
            QString minor_string=QString::number(GEX_APP_VERSION_MINOR);
            float lMinor=((float)GEX_APP_VERSION_MINOR)
                    /((float)GS_POW(10,minor_string.size()));
            // fix me : version 7.1 gives 7.0999999999999999
            mMap.insert("AppVersion", ((float)GEX_APP_VERSION_MAJOR)+lMinor);
            */

            QString lApplicationDir;
            int r=CGexSystemUtils::GetApplicationDirectory(lApplicationDir);

            // HTH test
            QCoreApplication::addLibraryPath(lApplicationDir);

            //mMap.insert("ApplicationDir", lApplicationDir); //mApplicationDir=lApplicationDir;
            setProperty("ApplicationDir", lApplicationDir);

            if(r != CGexSystemUtils::NoError)
            {
                GSLOG(3, "Unable to retrieve the app dir");
            }

            QString lUserFolder;
            QString lRes=RetrieveUserFolder(lUserFolder);
            GSLOG(7, QString("RetrieveUserFolder : lRes (%1) lUserFolder(%2)").arg(lRes).arg(lUserFolder).toLatin1().constData());

            if ( lRes.startsWith("error") )
            {
                mUserFolder=QDir::homePath();
                GSLOG(7, QString("lRes error (%1);").arg(mUserFolder.toString()).toLatin1().constData());
                GSLOG(3,
                  QString("Impossible to retrieve user folder using old school function: %1. Using Qt function : %2")
                        .arg(lRes).arg(mUserFolder.toString()).toLatin1().data());
            }
            else
                mUserFolder=lUserFolder;

            GSLOG(7, QString("Set(UserFolder, %1);").arg(mUserFolder.toString()).toLatin1().constData());
            Set("UserFolder", mUserFolder);

            mClientApplicationId=GetClientApplicationId();

            // 6320 : QTimer not perfect
            //mDailyTimer.connect(&mDailyTimer, SIGNAL(timeout()), this, SLOT(OnDailyTimeout()) );
            // each day : 5 184 000 000 ms : too big for an int
            //mDailyTimer.start(1000*60*60*60); // each hour ?
            m_eClientState=GS::Gex::Engine::eState_Init;     // PYC, 06/09/2011, valgrind return, SetClientState() calls GetClientState() !
            m_ePreviousClientState = GS::Gex::Engine::eState_Init;

            mlicenseReleaseRequest = false;
            mlicensePassive = false;
            mInitializationSequenceDone = false;
        }

        EnginePrivate::~EnginePrivate()
        {
            GSLOG(SYSLOG_SEV_DEBUG, "EnginePrivate destruction...");
        }

        QVariant& EnginePrivate::Get(const QString key)
        {
            if (key=="UserFolder")
                return mUserFolder;
            if (key=="ClientApplicationId")
                return mClientApplicationId;
            /*
             if (key=="TempFolder")
                return QVariant(mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
                // now stored in properties
            */

            //if (property
            //if (!mMap.contains(key))
                //return mInvalidVariant;
            //return mMap[key];
            static QVariant lVar;
            lVar=property(key.toLatin1().data());
            return lVar;
        }

        QVariant& EnginePrivate::Set(const QString key, QVariant value)
        {
            // Read ony variables :
            if (key=="AppFullName" || key=="ApplicationDir"
                    || key=="AppVersion" || key=="AppVersionMajor" || key=="AppVersionMinor"
                    || key=="GalaxySemiFolder"|| key=="TempFolder" || key == "AppBuildDate")
            {
                static QVariant lVar;
                lVar=property(key.toLatin1().data());
                return lVar;
                //return mMap[key];
            }

            // Dynamic update
            if (key=="UserFolder")
            {
                // Dynamic update
                //mMap.insert("GalaxySemiFolder", mUserFolder.toString()+QDir::separator()+"GalaxySemi");
                setProperty("GalaxySemiFolder", mUserFolder.toString()+QDir::separator()+"GalaxySemi");
                //mMap.insert("TempFolder",mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
                setProperty("TempFolder", mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
            }

            //mMap.insert(key, value);
            setProperty(key.toLatin1().data(), value);
            static QVariant lVar=property(key.toLatin1().data());
            //return mMap[key];
            return lVar;
        }


        QString EnginePrivate::OnEngineTimerTimeout()
        {
            GSLOG(6, "On engine timer timeout...");
            QString lResult;
            /*
            QFile scriptFile( GS::Gex::Engine::GetInstance().Get("ApplicationDir").toString()+"/scripts/daily_script.js" );
            if (scriptFile.exists())
            {
                if (scriptFile.open(QIODevice::ReadOnly))
                {
                    QTextStream stream(&scriptFile);
                    QString contents = stream.readAll();
                    scriptFile.close();
                    QScriptValue v=pGexScriptEngine->evaluate(contents);
                    GSLOG(SYSLOG_SEV_NOTICE, QString("Execution of daily_script.js script returned : %1").arg(
                             v.toString().toLatin1().data() );
                    lResult=v.toString();
                }
            }
            */
            return lResult;
        }

        QString	EnginePrivate::RetrieveUserFolder(QString &strPath)
        {
#if defined unix || __MACH__
            if(CGexSystemUtils::GetUserHomeDirectory(strPath) != CGexSystemUtils::NoError)
            {
                char	szString[2048];
                char	*ptChar = getenv("HOME");
                if(ptChar != NULL)
                    strPath  = ptChar;	// the HOME variable is valid !
                else
                {
                    // We miss the HOME variable...then assume file is in the current directory
                    ptChar = getcwd(szString,2047);
                    if(ptChar != NULL)
                        strPath  = szString;
                    else
                        return "error";
                }
            }
#endif

#ifdef _WIN32
            char szString[2048]="";
            // Script files to be in the User profile folder
            int ret = CGexSystemUtils::GetUserHomeDirectory(strPath);
            GSLOG(6, QString("CGexSystemUtils::GetUserHomeDirectory Return %1: '%2'").arg(ret).arg(strPath).toLatin1().constData());
            GSLOG(6, QString("Home according to Qt QDir='%1'").arg(QDir::homePath()).toLatin1().data() );
            if( ret != CGexSystemUtils::NoError)
            {
                // Windows95 doesn't support Profilefolder, keep using Windows folder...
                GetWindowsDirectoryA(szString,2047);
                GSLOG(7, QString("GetWindowsDirectoryA Return (%1)").arg(szString).toLatin1().constData());
                strPath  = szString;
            }
#endif
            GSLOG(7, QString("RetrieveUserFolder : strPath (%1)").arg(strPath).toLatin1().constData());
            return "ok";
        }

        ///////////////////////////////////////////////////////////
        // Return Client Id to avoid using several node if the new
        // instance is ran by the same user
        ///////////////////////////////////////////////////////////
        QString EnginePrivate::GetClientApplicationId()
        {
            QString strClientId = "";
            QString strSessionId = "";
        #if defined unix || __MACH__
            // Get the session id of the current process ( identical to getsid(getpid()) )
            pid_t pidSessionId = getsid(0);
            if (pidSessionId != -1)
                strSessionId = QString::number(pidSessionId);
        #else
            strSessionId = QString(getenv("SESSIONNAME"));
        #endif
            // If no session information -> 1 client = 1 node
            if (strSessionId.isEmpty())
                return strClientId;

            strClientId += strSessionId;
            strClientId += "|";
            strClientId += mSystemInfo.strHostName;   // Computer name
            strClientId += "|";
            strClientId += mSystemInfo.strAccountName;  // Account logged when GEX executed.
            strClientId += "|";
            strClientId += mSystemInfo.strHostID;   // HostID
            strClientId += "|";
            strClientId += mSystemInfo.strDiskID;   // DiskID (='?' if Unix)
            strClientId += "|";
            strClientId += mSystemInfo.strNetworkBoardID; // 1st EthernetBoardID detected (MAC unique board ID)
            QList<QHostAddress> lstHosts = QNetworkInterface::allAddresses();
            for (int i = 0; i < lstHosts.size() ; ++i)
            {
                if (lstHosts[i].toString() != "127.0.0.1")
                {
                    strClientId += "|";
                    strClientId += lstHosts[i].toString();
                }
            }

            if (GS::LPPlugin::ProductInfo::getInstance()->isMonitoring() || GS::LPPlugin::ProductInfo::getInstance()->isGTM())
            {
                strClientId += "|" ;
                strClientId += getenv(LPPlugin::ProductInfo::getInstance()->GetProfileVariable().toLatin1().constData());
            }

            return strClientId;
        }

        QString EnginePrivate::OverloadUserFolder()
        {
            char *  ptChar = 0;
            QString lServerProfile;

            lServerProfile = GS::LPPlugin::ProductInfo::getInstance()->GetProfileVariable();

            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Overload user folder searching for %1 env var").
                  arg(lServerProfile).toLatin1().constData());

            ptChar = getenv(lServerProfile.toLatin1().constData());

            if(ptChar != NULL)
            {
                GSLOG(SYSLOG_SEV_NOTICE,
                      QString("Overload user folder with %1 found: %2").
                      arg(lServerProfile).arg(ptChar).toLatin1().constData());

                // Get env variable
                QString strFolder = ptChar;
                CGexSystemUtils::NormalizePath(strFolder);

                QString strMessage = lServerProfile + " loaded (" + strFolder + ")";
                GSLOG(SYSLOG_SEV_INFORMATIONAL, strMessage.toLatin1().data());

                QDir clDir;
                // Check if dir exists or can be created
                if(QFile::exists(strFolder) == true)
                    mUserFolder = strFolder;
                else if(clDir.mkpath(strFolder))
                    mUserFolder = strFolder;
                clDir.mkpath(mUserFolder.toString()+QDir::separator()+"GalaxySemi");
                clDir.mkpath(mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");

                Set("UserFolder", mUserFolder);

                GSLOG(SYSLOG_SEV_INFORMATIONAL,
                      QString("User folder set to %1").
                      arg(mUserFolder.toString()).toLatin1().constData());

                /*
                // no more usefull: use instead GSEngine.Get("UserFolder");
                if (pGexScriptEngine)
                    pGexScriptEngine->globalObject().setProperty(PROP_GEX_USER_FOLDER,
                                                                 mUserFolder.toString(),
                                                                 QScriptValue::ReadOnly);
                */
            }
            else
            {
#ifdef GSDAEMON
                // according to tech specs, let's quit if not found.
                if (!ptChar)
                {
                    QString lError = "error: mandatory env var " + lServerProfile + " not found.";
                    return lError;
                }
#else
                if (GS::LPPlugin::ProductInfo::getInstance()->isGTM() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    GSLOG(SYSLOG_SEV_WARNING,
                          QString("Server profile variable %1 not found").
                          arg(lServerProfile).toLatin1().constData());
                }
                else
                    GSLOG(SYSLOG_SEV_NOTICE,
                          QString("Client profile variable %1 not found").
                          arg(lServerProfile).toLatin1().constData());
#endif
            }

            setProperty("TempFolder", mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"temp");
            setProperty("LogsFolder", mUserFolder.toString()+QDir::separator()+"GalaxySemi"+QDir::separator()+"logs");

            return "ok";
        }

        QString EnginePrivate::NetworkGet(const QString&lUrl)
        {
            GSLOG(5, QString("Network get %1").arg(lUrl).toLatin1().data());
            QNetworkRequest nr;
            nr.setUrl(QUrl(lUrl));
            QNetworkReply* reply=mNetworkAccessManager.get(nr);
            if (!reply)
            {
                GSLOG(4, "NetworkReply NULL");
                return "error : NetworkReply NULL";
            }
            while (!reply->isFinished()) // reply->isFinished() or isRunning() never ends...
            {
                //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("NetworkReply : running:%1, finished:%2").arg(.arg(
                //GSLOG(SYSLOG_SEV_INFORMATIONAL, "NetworkReply : running:%1.arg( finished:%2").arg(
                //       reply->isRunning()?"yes":"no", reply->isFinished()?"yes":"no" );
                QCoreApplication::instance()->QCoreApplication::processEvents();
                //QThread::currentThread()->wait(100);
            }
            GSLOG(6, QString("NetworkReply : StatusCode=%1")
                  .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                  .toLatin1().data() );
            GSLOG(6, QString("NetworkReply : ReasonPhrase=%1")
                  .arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                  .toLatin1().data() );
            QByteArray rba=reply->readAll();
            GSLOG(6, QString("NetworkReply : %s").arg(QString(rba)).toLatin1().data() );
            if (reply->error()!=QNetworkReply::NoError)
            {
                GSLOG(5, QString("NetworkReply : error : %1").arg(reply->errorString()).toLatin1().data());
                return "error : "+reply->errorString();
            }

            return QString(rba);
        }

        QString EnginePrivate::NetworkPost(const QString &url, const QString &data , const QString &contentType)
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Network post on %1 sending %2")
                  .arg(url).arg(data).toLatin1().data());
            GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("content type: %1")
                  .arg(contentType.isEmpty() ? "Not specified" : contentType).toLatin1().constData());
            QNetworkRequest nr;
            nr.setUrl(QUrl(url));
            if (contentType.isEmpty() == false)
            {
                nr.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
            }

            QNetworkReply* reply=mNetworkAccessManager.post(nr, data.toLatin1());
            if (!reply)
            {
                GSLOG(4, "NetworkReply NULL");
                return "error : NetworkReply NULL";
            }
            while (!reply->isFinished()) // reply->isFinished() or isRunning() never ends...
            {
                //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("NetworkReply : running:%1, finished:%2").arg(.arg(
                //GSLOG(SYSLOG_SEV_INFORMATIONAL, "NetworkReply : running:%1.arg( finished:%2").arg(
                //       reply->isRunning()?"yes":"no", reply->isFinished()?"yes":"no" );
                QCoreApplication::instance()->QCoreApplication::processEvents();
                //QThread::currentThread()->wait(100);
            }
            GSLOG(6, QString("NetworkReply : StatusCode=%1")
                  .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                  .toLatin1().data() );
            GSLOG(6, QString("NetworkReply : ReasonPhrase=%1")
                  .arg(reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                  .toLatin1().data() );
            QByteArray rba=reply->readAll();
            GSLOG(6, QString("NetworkReply : %s")
                  .arg(QString(rba)).toLatin1().data() );
            if (reply->error()!=QNetworkReply::NoError)
            {
                GSLOG(4, QString("NetworkReply : error : %1")
                      .arg(reply->errorString())
                      .toLatin1().data());
                return "error : "+reply->errorString();
            }

            return "ok";
        }

        QString	EnginePrivate::UpdateProductStringName()
        {
            QString lAppFullName;
            int     lProductId = GS::LPPlugin::ProductInfo::getInstance()->getProductID();

            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("Update Product String Name product %1...").
                  arg(lProductId).toLatin1().constData());

            switch(lProductId)
            {
                case GS::LPPlugin::LicenseProvider::ePATMan:            // Galaxy PAT-Man
                    lAppFullName = GEX_APP_PATMAN;
                    break;

                case GS::LPPlugin::LicenseProvider::ePATManEnterprise:  // Galaxy PAT-Man Enterprise
                    lAppFullName = GEX_APP_PME;
                    break;

                case GS::LPPlugin::LicenseProvider::eYieldMan:          // Galaxy Yield-Man
                    lAppFullName = GEX_APP_EXAMINATOR_MO;
                    break;

                case GS::LPPlugin::LicenseProvider::eYieldManEnterprise:// Galaxy Yield-Man Enterprise
                    lAppFullName = GEX_APP_YME;
                    break;

                case GS::LPPlugin::LicenseProvider::eGTM:               // Galaxy Tester Monitor
                    lAppFullName = GEX_APP_GTM;
                    break;

                case GS::LPPlugin::LicenseProvider::eLtxcOEM:           // OEM-Examinator for LTXC
                    lAppFullName = GEX_APP_EXAMINATOR_OEM_LTXC;
                    break;

                case GS::LPPlugin::LicenseProvider::eSzOEM:             // Examinator SPACE: Examinator OEM version for Credence SZ
                    lAppFullName = GEX_APP_EXAMINATOR_OEM_SZ;
                    break;

                case GS::LPPlugin::LicenseProvider::eExaminatorPro:     // Examinator-Pro
                    lAppFullName = GEX_APP_EXAMINATOR_PRO;
                    break;

                case GS::LPPlugin::LicenseProvider::eExaminatorPAT:     // Examinator-PAT
                    lAppFullName = GEX_APP_EXAMINATOR_DB_PAT;
                    break;

#ifdef TER_GEX
            case GS::LPPlugin::LicenseProvider::eTerOEM:                // Teradyne-Examinator OEM
                lAppFullName = GEX_APP_EXAMINATOR_OEM_TER;
                break;

            case GS::LPPlugin::LicenseProvider::eTerProPlus:            // Teradyne-Examinator-Pro+
                lAppFullName = GEX_APP_EXAMINATOR_TERPROPLUS;
                break;
#endif

                default:                                                // 'Examinator' is the application name.
                    lAppFullName = GEX_APP_EXAMINATOR;
                    break;
            }

            if(mlicensePassive)
                lAppFullName += " (Passive License)";

            //mMap.insert("AppFullName", lAppFullName);
            setProperty("AppFullName", lAppFullName);
            QCoreApplication::setApplicationName(lAppFullName.section(" - ",0,0).simplified());

            GSLOG(5, QString("New AppFullName=%1").arg(lAppFullName).toLatin1().data() );

            return "ok";
        }

        QString EnginePrivate::LoginToWeb(const QString &url, const QString &post_data)
        {
            GSLOG(6, QString("Network post on %1 sending %2 ")
                  .arg(url).arg(post_data).toLatin1().data());
            //Example for Sisense : "http://prismweb.galaxyec7.com/account/sso",
            QUrl the_url(url);
            QNetworkRequest nr;
            nr.setUrl(the_url);
            //nr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

            // If not set, QT5 now outputs a messy message in console...
            nr.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

            //QByteArray jsonData;
            //jsonData.append("{\"username\":\"admin\",\"password\":\"sisense\"}");
            //nr.setHeader(QNetworkRequest::ContentLengthHeader, jsonData.size());
            //nr.setRawHeader("params-json", jsonData);
            //nr.setRawHeader("User-Agent", "Mozilla/5.0");
            //nr.setRawHeader("Accept", "application/json"); // check me
            //nr.setRawHeader("Accept", "text/html"); // check me
            //nr.setRawHeader("Connection", "Keep-Alive"); // check me
            //nr.setRawHeader("Connection", "close");
            //nr.setRawHeader("Cache-Control", "max-age=0" ); // check me
            //nr.setAttribute(QNetworkRequest::CookieSaveControlAttribute,QVariant(true));

            QNetworkReply* reply=mNetworkAccessManager.post(nr, post_data.toLatin1()); // post_data.toUtf8() .toAscci() ?
            if (!reply)
            {
                GSLOG(4, "NetworkReply NULL");
                return "error : NetworkReply NULL";
            }
            while (!reply->isFinished()) // reply->isFinished() or isRunning() never ends...
            {
                //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("NetworkReply : running:%1, finished:%2").arg(.arg(                //GSLOG(SYSLOG_SEV_INFORMATIONAL, "NetworkReply : running:%1.arg( finished:%2").arg(
                //       reply->isRunning()?"yes":"no", reply->isFinished()?"yes":"no" );
                QCoreApplication::instance()->QCoreApplication::processEvents();
                //QThread::currentThread()->wait(100);
            }
            GSLOG(6, QString("NetworkReply : %1 bytes available, %2 bytes to write, %3 buffer size")
                  .arg(reply->bytesAvailable()).arg(reply->bytesToWrite()).arg(reply->readBufferSize())
                  .toLatin1().data() );
            GSLOG(6, QString("NetworkReply : StatusCode=%1")
                  .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
                  .toLatin1().data() );
            GSLOG(6, QString("NetworkReply : ReasonPhrase=%1")
                  .arg( reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString())
                  .toLatin1().data() );
            GSLOG(7, QString("NetworkReply : RedirectionTarget=%1")
                  .arg(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString())
                  .toLatin1().data() );
            GSLOG(7, QString("NetworkReply : rawHeader=%1")
                  .arg(QString(reply->rawHeader("authentication")))
                  .toLatin1().data() );
            //GSLOG(7, "NetworkReply : params-json=%s", QString(reply->rawHeader("params-json")).toLatin1().data() );
            //QList<QByteArray> lba=reply->rawHeaderList();
            //foreach(QByteArray ba, lba)
            //  GSLOG(SYSLOG_SEV_NOTICE, QString("NetworkReply : rawheader : %1:%2").arg( QString(ba)).toLatin1().constData(), QString(reply->rawHeader(ba))).toLatin1().constData() );.arg(            //  GSLOG(SYSLOG_SEV_NOTICE, "NetworkReply : rawheader : %1:%2").arg( QString(ba)).toLatin1().constData().arg( QString(reply->rawHeader(ba))).toLatin1().constData() );

            QByteArray rba=reply->readAll();
            // For Sisense : NetworkReply should be about : {"success":false,"reasone":"invalid parameters"} on failure

            GSLOG(6, QString("NetworkReply : %1").arg( QString(rba))
                  .toLatin1().data() );
            if (reply->error()!=QNetworkReply::NoError)
            {
                GSLOG(4, QString("NetworkReply : error : %1")
                      .arg( reply->errorString())
                      .toLatin1().data());
                return "error : "+reply->errorString();
            }
            // In case of clean cookie in header (not like Sisense replies where cookie are inside the body of the reply)
            //QNetworkCookie nc; nc.parseCookies(rba);
            // It seems Sisense does not use standard Set-Cookie header to give the cookie to install. The cookie params are inside the raw reply...
            /*
                    QList<QNetworkCookie> lnc=QNetworkCookie::parseCookies(reply->rawHeader("Set-Cookie")); // or rba ?
                    if (lnc.isEmpty())
                    {
                        return "error : cannot parse any cookies from reply";
                    }
                    GSLOG(6, QString("%1 cookies found in reply : first cookie name:%2 domain:%3 value:%4")
                             .arg(lnc.size())
                             .arg(QString(lnc.first().name()))
                             .arg(lnc.first().domain())
                             .arg(QString(lnc.first().value()))
                             .toLatin1().data() );
                    lnc.first().setDomain("galaxyec7.com");
                    lnc.first().setValue();
                    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Cookie : name:%1 domain:%2 value size:%3").arg(
                             QString(lnc.first().name()), lnc.first().domain().toLatin1().data(), lnc.first().value().size() );
                    */
            // Sisense replies contains these keywords
            // ToDo : parse me with a JSON lib
            if (!rba.contains("authentication"))
                return "error : reply does not contain an authentication attribute";
            if (!rba.contains("ticket"))
                return "error : reply does not contain any ticket";
            if (!rba.contains("isauthenticated\":true"))
                return "error : reply does not contain isauthenticated:true";

            QNetworkCookie nc;

            QScriptEngine lSE;
            QScriptValue lSV=lSE.evaluate("r="+QString(rba), QDir::homePath()+"/GalaxySemi/temp/networkreply.txt");
            if (lSV.isNull() || !lSV.isValid() || lSV.isError())
                return "error : evaluation result null or invalid or error " + lSV.toString();
            if (lSE.hasUncaughtException())
                return "error : evaluate exception : " + lSE.uncaughtException().toString();
            QScriptValue lAuth=lSV.property("authentication");
            if (lAuth.isNull() || !lAuth.isValid())
                return "error : no or bad authentication property found in reply";
            //GSLOG(SYSLOG_SEV_NOTICE, QString("Authentication found = %1").arg( lAuth.toString()).toLatin1().constData() );

            /*
                    int domainPos=rba.indexOf("domain\":\"");
                    QByteArray domain=rba.mid(domainPos+9, rba.indexOf(",\"isauthenticated")-domainPos-10 );
                    GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Domain found in reply : %1").arg( domain.data());
                    */
            nc.setDomain( lAuth.property("domain").toString() ); // nc.setDomain("galaxyec7.com");
            nc.setName( lAuth.property("cookiename").toString().toLatin1() ); // ".prism" in "cookiename":".prism"
            //int ticket_pos=rba.indexOf("ticket");
            //int cookiename_pos=rba.indexOf("cookiename");
            //QByteArray ticket=rba.mid(ticket_pos+9, 264); // "ticket":".....
            //GSLOG(SYSLOG_SEV_INFORMATIONAL, QString("Ticket found (%1-%2) : %3").arg( ticket_pos, cookiename_pos.arg( ticket.data());

            QScriptValue lTicket=lAuth.property("ticket");
            if (!lTicket.isValid())
                return "error : ticket unfindable or not valid";
            nc.setValue( lTicket.toString().toLatin1() );

            QList<QNetworkCookie> lnc; lnc.append(nc);
            // The url must be clean, no extensions like /account/...
            //example : QUrl("http://si.galaxysemi.com") QUrl("http://prismweb.galaxyec7.com")
            bool b=mNetworkCookieJar.setCookiesFromUrl(lnc, QUrl(the_url.scheme()+"://"+the_url.host()) );
            if (!b)
                return "error : cannot fill NetworkCookieJar";
            mNetworkAccessManager.setCookieJar(&mNetworkCookieJar);

            return "ok : login ok, cookie registered.";
        }


        bool EnginePrivate::CheckValidInstance(bool lLockOnly)
        {
            char szString[2048]="";
            int iErrorCode=0;

            #ifdef _WIN32

            CGFileLock clFileLock;
            QString  strLockFile = Engine::GetInstance().Get("UserFolder").toString() + "/.gex_lockfile" +
                    QString(getenv("SESSIONNAME")) + ".txt";

            CGexSystemUtils::NormalizePath(strLockFile);
            clFileLock.SetLockFileName(strLockFile.toLatin1().data());

            if (lLockOnly)
            {
                clFileLock.Lock();
                return true;
            }

            #ifdef QT_DEBUG
                #define EXEC_NAME "gexd.exe"
            #else
                #define EXEC_NAME "gex.exe"
            #endif

            // Under Windows,
            // a. Ensure the application mame is gex.exe
            // b. Ensure it is not already listed in the processes!
            GetModuleFileNameA(NULL, szString, 2047);

            #ifndef QT_DEBUG
                QString strString = szString;
                iErrorCode = strString.lastIndexOf( EXEC_NAME , -1, Qt::CaseInsensitive);
            #endif
            if(iErrorCode < 0)
            {
                // This application was renamed!...Refuse to run it.
                Message::information(Get("AppFullName").toString(),
                                     "ERROR: Program name must be 'gex.exe'.\nGEX will now exit!");
                return false;
            }

            // Check if gex.exe is running
            HANDLE   hSS = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            PROCESSENTRY32 info;
            info.dwSize = sizeof(PROCESSENTRY32);
            unsigned int uiNbGexProcesses = 0;
            BOOL   moreProcs = Process32First(hSS, &info);
            while(moreProcs)
            {
                if(wcsicmp(info.szExeFile, TEXT( EXEC_NAME )) == 0)
                    uiNbGexProcesses++;
                moreProcs = Process32Next(hSS, &info);
            }
            CloseHandle(hSS);

            // If first GEX instance, create user LOCK
            // If not first instance, allow a second instance only if the user LOCK exists (same user trying to start another instance)
            if(uiNbGexProcesses == 1)
                // First instance, create user lock, and accept instance
                clFileLock.Lock();
            else if(clFileLock.Lock())
            {
                // At least one other GEX instance is running, and the user Lock is successful,
                // so the other instance has been started by another user: REJECT this new instance!!
                Message::information(Get("AppFullName").toString(),
                                     "ERROR: Another user is running a GEX instance on this machine.\nGEX will now exit!");
                return false;
            }
            #endif

            #if defined __MACH__
                Q_UNUSED(szString);
                Q_UNUSED(iErrorCode);
                Q_UNUSED(lLockOnly);
            #endif

            #if defined unix
            if(lLockOnly)
                return true;

            // First: Check that argv[0] is gex
            char szLine[2048];
            char szProcess[256];
            const char* ptChar;
            FILE *hFile;
            int  iGrxProcesses;
            long lGexProcessID;
            bool bIsGexApp;
            bool bFoundCurrentGex;
            time_t lStartTime=time(NULL);
            QString strString = GS::Gex::Engine::GetInstance().GetCommandLineOptions().GetArguments().at(0);
        #ifdef QT_DEBUG
            bIsGexApp = (strString.endsWith("gexd") || strString.endsWith("gexappd"));
        #else
            bIsGexApp = (strString.endsWith("gex") || strString.endsWith("gexapp"));
        #endif

            if(bIsGexApp == false)
            {
                // This application was renamed!...Refuse to run it.
                Message::information(Get("AppFullName").toString(),
                                     "ERROR: Program name must be 'gex' or 'gexapp'.\nGEX will now exit!");
                return false;
            }

            // Second: Check if we have only one gex process running.
        #if defined(hpux) || defined(__linux__)
            strString = "ps -e > ";
        #endif
        #ifdef __sun__
            strString = "/usr/bin/ps -A > ";
        #endif

            ptChar = getenv("HOME");
            if(ptChar == NULL)
            {
                ptChar = "ERROR: The environment variable HOME is not set...\nyou must set it to the path of your HOME directory.\nGEX will now exit!";
                Message::information(Get("AppFullName").toString(), ptChar);
                return false;
            }
            sprintf(szString,"%s/gex_list_processes.txt",ptChar);
            unlink(szString); // Remove file.
            // Makes string 'ps -A | grep gex > <$HOME>/gex_list_processes.txt'
            strString += szString;
            iErrorCode = system(strString.toLatin1().constData());
            // With CentOS4.8, the system function returns -1, although the file gets created (error message: no child process).
            // So force return code to 0. Then we check if we can read the expected file for 4".
            iErrorCode = 0;
            if( iErrorCode == -1 )
            {
                // This application was renamed!...Refuse to run it.
                Message::information(Get("AppFullName").toString(),
                                     "ERROR: Failed reading GEX processes running.\nVerify you do not run 'gex' as an alias or from a shell file!\nGEX will now exit!");
                return false;
            }
            else
            {
                // Wait until file with list of 'gex' processes created.
                if (WEXITSTATUS(iErrorCode)) {
                    // Nothing to do
                }
                // Read file and see how many of the processes are 'GEX'
        read_file:
                if(time(NULL)-lStartTime > 4)
                {
                    // Timeout...it took over 4 seconds to check processes...and still not okay!
                    Message::information(Get("AppFullName").toString(),
                                         "ERROR: Timeout reading GEX processes running.\nVerify you do not run 'gex' as an alias or from a shell file!\nGEX will now exit!");
                    return false;
                }
                iGrxProcesses = 0;
                bFoundCurrentGex = false;
                hFile = fopen(szString,"r");
                if(hFile)
                {
                    while(!feof(hFile))
                    {
                        *szLine = 0;
                        lGexProcessID = 0;
                        *szProcess = 0;
                        // Read one line of the process file
                        // <ProcessID> <TTY> <Time> <Name>
                        if (fgets(szLine, 2047, hFile) == NULL) {
                            //FIXME: error ?
                        }
                        if(strstr(szLine, "gex") != NULL)  // make sure the line contains string "gex" (replaces usage of grep in unix command)
                        {
                            sscanf(szLine,"%ld %*s %*s %s",&lGexProcessID,szProcess);
                            // Get process name
                            strString = szProcess;
                        #ifdef QT_DEBUG
                                            if((strString == "gexd") || (strString == "gexappd"))
                        #else
                                            if((strString == "gex") || (strString == "gexapp"))
                        #endif
                            {
                                // Found a GEX process in the list !
                                iGrxProcesses++;
                                // Check if this process is current GEX running (it MUST appear in the file...unless hacker is providing another file!)
                                if(lGexProcessID == getpid())
                                {
                                    bFoundCurrentGex = true; // We have found current GEX process!
                                }
                            }
                        }
                    };
                    fclose(hFile);
                }
                if(bFoundCurrentGex == false)
                    goto read_file;
                unlink(szString); // Remove file.
                if(iGrxProcesses != 1)
                {
                    // Another GEX is already running !...Refuse to run it.
                    Message::information(Get("AppFullName").toString(),
                                         "ERROR: Another GEX instance is already running on this machine.\nGEX will now exit!");
                    return false;
                }
            }
            #endif

            return true;
        }

        void EnginePrivate::InitLocalConfigFilePath()
        {
            // Local config file name
            mLocalConfigFile = Get("UserFolder").toString();

            QString lProfileFolder = GS::LPPlugin::ProductInfo::getInstance()->GetProfileFolder();

            // If a server or client profile exists, do not add the unix folder on linux
            if (lProfileFolder.isNull() || lProfileFolder.isEmpty())
            {
#if defined unix || __MACH__
                mLocalConfigFile += GEX_LOCAL_UNIXFOLDER;
#endif
            }

            mLocalConfigFile +=  GEX_CLIENT_NODE;
        }

        QString EnginePrivate::InitScriptsPath()
        {
            GSLOG(SYSLOG_SEV_INFORMATIONAL, "Initializing default script path");

            // Creates scripts name in USER Folder
            if(GS::LPPlugin::ProductInfo::getInstance()->isGTM())
            {
                mStartupScript    = Get("GalaxySemiFolder").toString() + GTM_SCRIPT_CONFIG_NAME;
                mAssistantScript    = Get("GalaxySemiFolder").toString() + GTM_SCRIPT_ASSISTANT_NAME;

            }
            else if(GS::LPPlugin::ProductInfo::getInstance()->isOEM() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminator() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
            {
                mStartupScript    = Get("UserFolder").toString()+ GEX_SCRIPT_CONFIG_NAME;
                mAssistantScript  = Get("UserFolder").toString()+ GEX_SCRIPT_ASSISTANT_NAME;
            }
            else if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring()
                    || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO()
                    || GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT())
            {
                mStartupScript    = Get("UserFolder").toString() + GEXDB_SCRIPT_CONFIG_NAME;
                mAssistantScript  = Get("UserFolder").toString() + GEXDB_SCRIPT_ASSISTANT_NAME;

                if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan())
                {
                    mStartupScript    = Get("GalaxySemiFolder").toString() + PAT_SCRIPT_CONFIG_NAME;
                    mAssistantScript  = Get("GalaxySemiFolder").toString() + PAT_SCRIPT_ASSISTANT_NAME;
                }
                else if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan())
                {
                    mStartupScript    = Get("GalaxySemiFolder").toString() + YM_SCRIPT_CONFIG_NAME;
                    mAssistantScript  = Get("GalaxySemiFolder").toString() + YM_SCRIPT_ASSISTANT_NAME;
                }

                mMoTasksFile      = Get("UserFolder").toString()+ GEXMO_TASKS_FILE;
                mMoTasksXmlFile   = Get("UserFolder").toString()+ GEXMO_TASKS_XML_FILE;

            }
            else
            {
                GSLOG(SYSLOG_SEV_WARNING,
                      QString("Unknown Quantix Product ID runnning %1")
                      .arg(GS::LPPlugin::ProductInfo::getInstance()->getProductID()).toLatin1().data());
                return "error : Unknown Quantix Product ID running when initializing default scripts path";
            }

            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("\tStartup script set to '%1'").arg(mStartupScript).toLatin1().data());
            GSLOG(SYSLOG_SEV_DEBUG,
                  QString("\tAssistant script set to '%1'").arg(mAssistantScript).toLatin1().data());

            return "ok";
        }

        QString EnginePrivate::ActivateEngines()
        {
            GSLOG(6, "Activate engines...");
            if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
            {
                // Initialize Database Transaction module.
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().Activate();
                // Initialize Scheduler engine module.
                GS::Gex::Engine::GetInstance().GetSchedulerEngine().Activate();
            }
            else if(GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPRO() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorPAT() ||
                    GS::LPPlugin::ProductInfo::getInstance()->isExaminatorTerProPlus())
            {
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().Activate();

                if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() ||
                        GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
                {
                    GS::Gex::Engine::GetInstance().GetSchedulerEngine().Activate();
                }
            }

            return "ok";
        }

    } // Gex
} // GS
