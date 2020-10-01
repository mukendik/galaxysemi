#ifndef ENGINE_PRIVATE_H
#define ENGINE_PRIVATE_H

#include <QScriptEngineDebugger>
#include <QDate>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QVariant>

#include <gqtl_sysutils.h>
#include <gqtl_webserver.h>

#include "gex_version.h"
#include "read_system_info.h"
#include "temporary_files_manager.h"
#include "scheduler_engine.h"
#include "db_engine.h"
#include "gex_options_handler.h"
#include "sya_engine.h"
#include "spm_engine.h"
#include "tasks_manager_engine.h"
#include "vfei_server.h"
#include "admin_engine.h"
#include "command_line_options.h"
#include "testerserver.h" // for GTM
#include "pat_engine.h"

namespace GS
{
    namespace Gex
    {
        class EnginePrivate : public QObject
        {
            Q_OBJECT

            Q_DISABLE_COPY(EnginePrivate)

            //friend GS::Gex::Engine; ?
            friend class AdminEngine;
            friend class SchedulerEngine;
            //friend class DatabaseCenter;

            QVariant mInvalidVariant;
            QVariant mGSFolder;

            public:
                EnginePrivate(QObject* lParent);
                virtual ~EnginePrivate();

            public slots:

                QString OnEngineTimerTimeout();

                // Retrieve the user folder from some env variables (HOME).
                // Get Home user path (or Windows folder if not other choice)
                // todo use Qt functions ?
                // returns "error..." or "ok"
                QString	RetrieveUserFolder(QString &strPath);

                //
                QString OverloadUserFolder();

                //
                QVariant& Get(const QString key);
                QVariant& Set(const QString key, QVariant value);

                QString NetworkGet(const QString&lUrl);
                QString NetworkPost(const QString &url, const QString &data, const QString &contentType);

                QString LoginToWeb(const QString &url, const QString &post_data);

                QString	UpdateProductStringName();

                // ActivateEngines : DBEngine, SchedulerEngine, ... according to current Product
                QString ActivateEngines();

            public:
                //QMap<QString, QVariant> mMap; // moved to QObject internal properties map in order to be JSON listed

                // global NAM to be used by probably all Gex network requests
                // in order to track network activities,
                // reuse global third party cookies (BI, HelpConsole,...)
                QNetworkAccessManager mNetworkAccessManager;
                // containing all cookies registered in a gex session
                QNetworkCookieJar   mNetworkCookieJar;
                //! \brief WesServer for WebService
                GS::QtLib::WebServer mWebServer;

                QVariant     mUserFolder; // Move me to the map

                QTimer	mScriptTimer;
                QScriptEngineDebugger mScriptEngineDebugger;

                QString     mLastAccessedFolder;

                /*!
                 * \var mExitCode
                 */
                int mExitCode;

                // Engines
                GS::Gex::AdminEngine        mAdminEngine;
                GS::Gex::SchedulerEngine    mSchedulerEngine;
                GS::Gex::DatabaseEngine     mDatabaseEngine;
                GS::Gex::SYAEngine          mSYAEngine;
                GS::Gex::SPMEngine          mSPMEngine;
//                PATEngine mPATEngine;

                //QTcpSocket mGexLmSocket; // Anis: could we remove that ?

                // All system info: hostname, host id, board id, etc...
                ReadSystemInfo mSystemInfo;
                QVariant mClientApplicationId;

                // A string used for unique identification of the current app
                QString GetClientApplicationId();

                // Ensures only one instance of GEX is running on the CPU!
                // if lLockOnly, it will just lock the gex_lockfile and return true
                // if not lLockOnly, check gex exec is runned only once, if not, popup a message and return false
                bool CheckValidInstance(bool lLockOnly);

                /*!
                    @brief  Initialize file path of the local config file
                    */
                void InitLocalConfigFilePath();

                /*!
                    @brief  Initialize the scripts path used by the products as well as the file
                            path for Monitoring task files
                    */
                QString InitScriptsPath();

                // Usefull for many functions
                CGexSystemUtils mSysUtil;

                //! \brief UserProfile: could be anything, could be overload by XXX_CLIENT/SERVER_PROFILE,...
                // Move me to the map
                QString mUserProfile;

                /*!
                  @brief    Full File name of the assistant script (<path>/.gex_assistant.csl)
                  */
                QString                 mAssistantScript;

                /*!
                  @brief    Full File name of the assistant script (<path>/.galaxy_examinator.csl)
                  */
                QString                 mStartupScript;

                /*!
                  @brief    Full File name of the old tasks configuration file (<path>/.gexmo_tasks.conf)
                  */
                QString                 mMoTasksFile;

                /*!
                  @brief    Full File name of the new tasks configuration file (<path>/.gexmo_tasks.xml)
                  */
                QString                 mMoTasksXmlFile;

                /*!
                  @brief    Full File name of the local configuration file (<path>/.gex_localconfig.conf)
                  */
                QString                 mLocalConfigFile;

                /*!
                  @brief    Holds the options parsed from the command line
                  */
                CommandLineOptions      mCommandLineOptions;

                /*!
                  @brief    Holds the timestamp of the last action of the user.
                  */
                QDateTime               mLastUserActivity;

                TemporaryFilesManager   mTempFilesManager;
                // Task Manager to manage and get tasks status
                TasksManagerEngine      mTasksManager;

                // Default Gex options
                OptionsHandler          mOptionsHandler;

#ifdef VFEI_SERVER
                // Virtual Factory Equipment Interface (TCP/IP): Allows external
                // application to PAT-Man
                VFEIServer mVFEIServer;
#endif

#ifdef GCORE15334
                // Only used by the GTM product but passive/inactive until Start() is called
                TesterServer mTesterServer;
#endif
                //CPatInfo mPatInfo;
                //GexMainwindow::ClientState mClientState; // todo : move me outside of GexMainWindow
                // Contains the client's current state (Init, Ready,...)
                unsigned int m_eClientState;
                // Contains the last client's state
                unsigned int m_ePreviousClientState;
                // Anis ?
                bool mlicenseReleaseRequest;

                // Active-Passive license
                bool mlicensePassive;

                // Initialization sequence status
                bool mInitializationSequenceDone;
        };
    } // Gex
} // GS

#endif // ENGINE_PRIVATE_H
