#ifndef GEX_ENGINE_H
#define GEX_ENGINE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTcpSocket>
#include <QTimer>
#include <QNetworkCookieJar>
#include <QScriptEngineDebugger>
#include <QVariant>
#include <QDate>
#include <QEvent>
#include <QStringList>
#include <QProcess>

/*
  The engine singleton class contains all core members
    for the non-GUI process of Gex (Exa, YM, PM, ...)
*/

// Used for encrypting data.
#define GEX_CRYPTO_KEY		"(gex@galaxysemi.com)"
#define GEX_KEY_BYTES		12

class ReadSystemInfo;
class TemporaryFilesManager;


namespace GS
{
namespace LPPlugin
{
class GEXMessage;
class LPMessage;
class LicenseProvider;
}

namespace Gex
{
class CommandLineOptions;
class CSLStatus;
class DatabaseEngine;
class OptionsHandler;
class OptionsMap;
class AdminEngine;
class SchedulerEngine;
class TasksManagerEngine;

class Engine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString mClientState READ GetClientStateName)

    Q_PROPERTY(bool mHasTasksRunning READ HasTasksRunning)

    Q_DISABLE_COPY(Engine)

    // Private destructor to prevent several instantiation
    Engine(QObject* parent);
    virtual ~Engine();
    static Engine* mInstance;
    class EnginePrivate *mPrivate;

signals:
    void    sExit(int lExitCode);
    void    sStarting();
    void    sReady();
    void    sLicenseGranted();
    void    sProgressStatus(bool, int, int, bool);
    void    sLabelStatus(QString);
    void    sHideProgress();

public:

    enum EventType
    {
        EVENT_START             = QEvent::User+1,
        EVENT_LICENSE_READY     = QEvent::User+2,
        EVENT_LICENSE_GRANTED   = QEvent::User+3,
        EVENT_USER_ACTIVITY     = QEvent::User+4
    };

    static Engine& GetInstance();

    // Use one bit for each values, as the WaitForClientState(unsigned int uiStates) function can be used
    // with a OR on different states to wait for one of the specified states.
    enum ClientState
    {
        eState_Init				= 0x01,		// Node initializing
        eState_Connect			= 0x02,		// Connect to license server
        eState_ReConnect		= 0x04,		// Connect to license server
        eState_LicenseGranted	= 0x08,		// License granted
        eState_NodeReady		= 0x10,		// Node fully ready (license OK and all startup scripts executed)
        eState_Exit				= 0x20		// Node will exit
    };

    void customEvent(QEvent * e);

    ///< brief return the list of file names that shouldn't be notifiyed
    QList<QString> GetListNotNotifiyedFile();

    ///< brief Add the file name to the list of file names that shouldn't be notifiyed
    void AddFileToNotNotifiyed(QString fileName);

public slots:
    /*!
       * \fn OnExit
       */
    bool OnExit();
    /*!
       * \fn Exit
       */
    bool Exit(int lExitCode);
    /*!
       * \fn Exit
       * \brief Called once at the very beginning
       */
    void OnStart();
    // called once when the license is ready
    void OnLicenseReady();
    // called once the license is granted
    void OnLicenseGranted();
    // Called when a task starts
    void OnStartTask(int lType);
    //  called once on software close
    //void OnClose(); // does not seem to be usefull anymore

    //
    static bool Destroy();

    static bool RemoveFileFromDisk(const QString& lFileName);

    //! \brief Points to relevant product name.
    // iProductId = GEX_DATATYPE_ALLOWED_DATABASE, GEX_DATATYPE_ALLOWED_DATABASEWEB, GEX_DATATYPE_GEX_PATMAN, ...
    // uOptionalModules = GEX_OPTIONAL_MODULE_PATMAN,...
    QString UpdateProductStringName();
    // Login to given web server using any post data authentication (JSON,...)
    QString LoginToWeb(const QString &url, const QString &post_data);
    //
    QString NetworkPost(const QString &url, const QString &data, const QString& contentType = QString());
    //
    QString NetworkGet(const QString &url);

    // Download given url : "ftp://..." or "http://...." or ...
    QString Download(QString url, QString targetfile);

    //
    QNetworkAccessManager&    GetNAM();
    //
    QNetworkCookieJar&        GetNetworkCookieJar();
    //
    QScriptEngineDebugger&    GetScriptEngineDebbugger();
    //
    class SchedulerEngine&    GetSchedulerEngine();
    //
    class DatabaseEngine&     GetDatabaseEngine();
    //
    class SYAEngine&          GetSYAEngine();
    //
    class SPMEngine&          GetSPMEngine();
    //
    TemporaryFilesManager&    GetTempFilesManager();
    //
    TasksManagerEngine&       GetTaskManager();
    //
    ReadSystemInfo&           GetSystemInfo();

    class AdminEngine&        GetAdminEngine();
#ifdef GCORE15334
    class TesterServer&       GetTesterServer();
#endif
//    class PATEngine&          GetPATEngine();

    // Options methods
    const OptionsHandler &    GetOptionsHandler() const;

    /*! \brief Generic Get accessor : example : Get("AppFullName")
                    Available keys : "AppFullName", "UserFolder", "ClientApplicationId",
                    "AppVersionMajor", "AppVersionMinor", "AppVersionBuild", "ApplicationDir"
                    "GalaxySemiFolder": UserFolder/GalaxySemi
                    "TempFolder", "LogsFolder" :
                    Returns an invalid QVariant if property not found
                */
    QVariant  Get(const QString);


    // Set the value for this key
    bool      Set(const QString key, QVariant value);
    //
    //QString GetAppFullName(); // Replaced by Get("AppFullName")
    //
    // Exec the shell script with QProcess : assynchrone or not
    QVariant  ExecShellScript(const QString&, bool assynchrone=true);

    /**
     * @brief ExecProcess.  Execute a script. In synchronous mode the log output are displayed in the GEX log file
     * @param programm :    the script that must be launch
     * @param argument :    the arguments passed to the script if any
     * @param synchrone :   true (default), the process will wait until the end of the script unless the timeout is reached
     *                  :   false, the scrip is launched in asynchronous mode.
     * @param msecond :     value of the time out (default 3000 msec).
     * @return :            Whether or not the script has been exxecuted correctly (in synchronous mode only)
     */
    QVariant ExecProcess(const QString& programm, const QString& argument,  bool synchrone=true, int msecond = 3000);


    //QString Get("ApplicationDir").toString(); // replaced by Get("ApplicationDir")
    // Overload/recompute the user folder
    // with the profile env var if found
    // Does nothing if profile env variable  not found
    // Should be called only on server mode
    QString OverloadUserFolder();

    // get last accessed folder
    QString GetLastAccessedFodler();
    void    UpdateLastAccessedFolder(const QString& folder);
    // get UserFolder/GalaxySemi/temp
    // todo: use QDir::tempPath() ?
    //QString GetTempFolder(); // replaced by Get("TempFolder")

    /*!
                  @brief    Returns the full file name of the assistant script.
                  */
    const QString& GetAssistantScript() const;

    /*!
                  @brief    Returns the full file name of the startup script.
                  */
    const QString& GetStartupScript() const;

    /*!
                  @brief    Returns the full file name of the old format of the
                            configuration tasks file.
                  */
    const QString& GetOldTasksFileName() const;

    /*!
                  @brief    Returns the full file name of the xml configuration
                            tasks file
                  */
    const QString& GetTasksXmlFileName() const;

    /*!
                  @brief    Returns the full file name of the local configuration file
                  */
    const QString& GetLocalConfigFileName() const;

    /*!
                  @brief    Returns a reference to the Command Line Options
                  */
    CommandLineOptions& GetCommandLineOptions();

    /*!
                  @brief    Return the timestamp of the last user activity.
                  */
    const QDateTime&    GetLastUserActivity() const;

    /*!
                  @brief    Return the list of profiles existing on the computer.
                  */
    QStringList GetProfiles();

    /*!
                  @brief    Return the default profile script. The string is empty if there is no
                            default profile script.
                  */
    QString     GetDefaultProfileScript();

    /*!
                  @brief    Return the default profile user. The string is empty if there is no
                            default user profile.
                  */
    QString     GetDefaultUserProfile();

    /*!
                  @brief    Return true is a default profile script exists, otherwise returns false.
                  */
    bool        IsExistingDefaultProfile();

    /*!
                  @brief    Sets the \a lDateTime of the last user activity.

                  @param    lDateTime    Timestamp of the last user action.
                  */
    void        SetLastUserActivity();

    /*!
                  @brief    Sets the \a scriptName of the startup script file

                  @param    scriptName    Name of the startup script file
                  */
    void        SetStartupScript(const QString& scriptName);

    /*!
                  @brief    Returns true if we are in Admin Server mode, and connected

                  @param    userConnected    ??
                  */
    bool        IsAdminServerMode(bool userConnected = false);

    // Returns true if have some task running
    /*!
                  @brief    Returns true if tasks are still running
                  */
    bool         HasTasksRunning();

    /*!
                  @brief    Returns the currentDateTime from ym_admin_db connector if any
                  */
    QDateTime    GetServerDateTime();
    /*!
                  @brief    Returns the currentDateTime where the Appl is running
                  */
    QDateTime    GetClientDateTime();

    QDate GetExpirationDate();
    // Todo ! move socketReadyRead in engine
    void  SetExpirationDate(const QDate &d);
    void  SetMaintenanceExpirationDate(const QDate &d);

    //
    QDate GetMaintenanceExpirationDate();
    //CPatInfo& GetPatInfo() { return mPatInfo; }
    int   CheckForLicense(void);

    bool  SetOptionsMap(const OptionsMap &optionsMap);

#ifdef VFEI_SERVER
    /* WARNING: This method should be removed once the onStart()
                  method from GexMainwindow will be moved to the Engine. */
    void  StartVFEIServer();
#endif

    // Retrieve state as a string
    QString   GetClientStateName(void);

    // Set client state
    void      SetClientState(ClientState eNewClientState);

    // Client state set/get
    ClientState   GetClientState(void);
    QString       GetClientStateName(ClientState eClientState);
    // Waits until client node in specified state
    void WaitForClientState(ClientState eState);
    // Waits until client node in one of the specified states (OR of states)
    void WaitForClientState(unsigned int uiStates);

    // Returns true if the node has a valid license : node ready and LicenseGranted
    bool HasNodeValidLicense();

    void UpdateProgressStatus(bool lSet, int lRange, int lStep, bool lShow = true);
    void UpdateLabelStatus(const QString& lMessage = "");
    void HideProgress();

    // Request a GTL token and return the status and additional info if needed
    void RequestGTLLicense(qintptr libraryID, bool &status, QString &info);
    // Release a GTL license for library libraryID
    void ReleaseGTLLicense(int libraryID);
    // Free the license stuff just before quitting the application
    void FreeLicenseProvider();

    /*!
     * \fn SetStdoutHandler
     */
    void SetStdoutHandler(bool lSet);

protected slots:

    void OnCSLScriptFinished(const GS::Gex::CSLStatus &lStatus);
protected:
    /*!
                 * \fn ShouldConnectAgain
                 */
    bool ShouldConnectAgain(const int errnb,
                            const QString& lMessage);
signals:
    void sendGEXMessage(const GS::LPPlugin::GEXMessage &);
public slots:
    void processLPMessage(const GS::LPPlugin::LPMessage &);
    void licenseReleaseRequest();
    void autoCloseReached(QString &message);
    // Slot  used to request to connect again with the license server connection is lost.
    void ConnectToLPAgain();

private:
    QList<QString> mListNotNotifiyedFile;
    void handleDisconnection(const QString &lpMessage);
    void handleExtendedMessage(const QString &lpMessage);
    void setYMAdminDBConfig(const QString &);
    void sendYMAdminDBConfig();

    QProcess* launchScriptProcess;

public:
    void disconnectFromLP(const QString &str);
    void reconnectToLP(const QString &str);
    void activityNotification(const QString &str);
    bool GetLicensePassive();
    //Return the global timout returned by the license server
    int GetServerGlobalTimeout();
};
}
}

Q_DECLARE_METATYPE(GS::Gex::Engine::ClientState)

#endif // ENGINE_H
