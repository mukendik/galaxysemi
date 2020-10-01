#ifndef CSL_ENGINE_H
#define CSL_ENGINE_H

#include <QObject>
#include <QString>

namespace GS
{
namespace Gex
{

class CSLEnginePrivate;

/*! \class  CSLStatus
    \brief  The CSLStatus class holds the status of CSL script executed.
*/
class CSLStatus : public QObject
{
    Q_OBJECT
public:

    /*!
      @brief    Constructs a default CSL status object
    */
     CSLStatus(QObject* parent=0);

    /*!
      @brief    Destroys the CSL status object
    */
     ~CSLStatus();

     /*!
       @brief    Copy constructor
     */
     CSLStatus(const CSLStatus& other);

    /*!
      @brief    Assignment operator
    */
     CSLStatus& operator=(const CSLStatus& other);

    /*!
      @brief    Returns true if the CSL script executed has failed, otherwise false.
    */
     Q_INVOKABLE bool            IsFailed() const;

    /*!
      @brief    Returns true if the CSL script executed has been aborted, otherwise false.
    */
     Q_INVOKABLE bool            IsAborted() const;

    /*!
      @brief    Returns true if the CSL script executed was a startup script, otherwise false.
    */
     Q_INVOKABLE bool            IsStartup() const;

    /*!
      @brief    Returns the error message of the CSL script executed. The message is empty if the
                script was successfully executed.
    */
     Q_INVOKABLE const QString&  GetErrorMessage() const;

private:

    bool        mFailed;
    bool        mAborted;
    bool        mStartup;
    QString     mErrorMessage;

    friend class CSLEngine;
};

/*! \class  CSLEngine
    \brief  The CSLEngine class provides an environment for executing CSL scripts. This class is a
            singleton as it is not allowed to run multiple CSL scripts concurrently.
*/
class CSLEngine : public QObject
{
    Q_OBJECT
public:

    enum CslType
    {
        CSL_STARTUP     = 0x01,
        CSL_ARGUMENT    = 0x02,
        CSL_REGULAR     = 0x04,
        CSL_ANY         = 0x07
    };

    /*!
      @brief    Returns a reference to the CSL Engine instance. The CSL Engine is allocated if it
                has not been already done.
      */
    static CSLEngine& GetInstance();

    /*!
      @brief    Destroy the CSL engine object
    */
    static void        Destroy();


    /*!
      @brief    Execute the \a scriptName CSL script.
      @returns  The status of the executed CSL script
      @sa       IsRunning
      */
    Q_INVOKABLE CSLStatus       RunScript(const QString& scriptName);

    /*!
      @brief    Execute the \a scriptName CSL script given as argument of the executable.

      @returns  The status of the executed CSL script

      @sa       IsRunning
      */
    Q_INVOKABLE CSLStatus       RunArgumentScript(const QString& scriptName);

    /*!
      @brief    Execute the \a scriptName CSL startup script.

      @returns  The status of the executed CSL startup script

      @sa       IsRunning
      */
    Q_INVOKABLE CSLStatus       RunStartupScript(const QString& scriptName);

    /*!
      @brief    Requests the CSL engine to abort the current script executed.

      @sa       IsAbortRequested
      */
    void            AbortScript();

    /*!
      @brief    Returns true if the CSL engine is currently executing a CSL script, otherwise false
    */
    bool            IsRunning(CslType lType = CSL_ANY) const;

    /*!
      @brief    Returns true an abort has been requested for the the CSL engine being executed,
                otherwise false
    */
    bool            IsAbortRequested() const;

    /*!
      @brief    Returns the name of the CSL script being executed when CSL engine is running, or the
                name of the last CSL script executed when CSL engine is stopped.
    */
    const QString&  GetScriptName() const;

public slots:
    /*!
      @brief    Execute the \a scriptName CSL script.
      @returns  A string: ok or error:...
      @sa       IsRunning
      */
    QString     RunCSL(const QString& scriptName);

protected:

    /*!
      @brief    Checks the CSL version of the \a scriptName CSL script. If the version is too old,
                the script is updated to the new format and  the old script is renamed with .bak
                extension.

      @param    scriptName  Name of the CSL script to check.
      */
    void        CheckCSLVersion(const QString &scriptName);


    bool        IsRunnable() const;

    /*!
      @brief    Execute the \a scriptName CSL script. This method is the same for startup and
                non-startup scripts.

      @returns  The status of the executed CSL script

      @sa       IsRunning
      */
    CSLStatus   Run(const QString& scriptName);

private:

    /*!
      @brief    Constructs a CSL Engine object with the given \a parent.
      */
    CSLEngine(QObject * parent);

    /*!
      @brief    Destroys this CSL Engine.
      */
    virtual ~CSLEngine();

    Q_DISABLE_COPY(CSLEngine)

    static CSLEngine *  mInstance;

    CSLEnginePrivate *  mPrivate;

signals:

    /*!
      @brief    This signal is sent when a CSL script execution starts. The \a scriptName of the CSL
                script is passed and \a startup script status.
      */
    void    scriptStarted(const QString& scriptName, bool startup);

    /*!
      @brief    This signal is sent when a CSL script execution finishes. The \a cslStatus of the
                executed CSL script is passed.
      */
    void    scriptFinished(const GS::Gex::CSLStatus& cslStatus);

    /*!
      @brief    This signal is sent when a particulat event occurs during the CSL script execution
                (e.g.: script exception old CSL version detected, ...). The event \a logMessage is
                passed.
      */
    void    scriptLog(const QString& logMessage);
};

}   // namespace Gex
}   // namespase GS

#endif // CSL_ENGINE_H
