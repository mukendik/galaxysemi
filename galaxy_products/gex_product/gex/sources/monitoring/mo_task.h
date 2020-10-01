#ifndef MO_TASK_H
#define MO_TASK_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QVariant>
#include "mo_task.h"

#define MO_TASK_STATUS_NOERROR	0
#define MO_TASK_STATUS_WARNING	1
#define MO_TASK_STATUS_ERROR	2

class TaskProperties;
class GexMoDataPumpTaskData;
class GexMoFileConverterTaskData;
class GexDatabaseEntry;

class SchedulerGui;

///
/// \brief The CGexMoTaskItem class represents a task that can be executed by the scheduler
/// it's an abstract class that must be subclassed for each new task type
///
class CGexMoTaskItem : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CGexMoTaskItem)

    ///
    /// \brief Task unique identifier as found in the ym_admin_db
    ///
    int         m_iTaskId;

public slots:
    ///
    /// \brief Gets the task type
    /// \return a task type defined in GEX_CONSTANTS
    ///
    virtual int GetTaskType() = 0;
    // return the Name(TaskType)
    virtual QString GetTypeName() = 0;

    virtual bool                HasComputedLimits() { return false;}
    virtual int                 GetActiveProdVersionId() { return -1;}
    //virtual TaskProperties*     GetProperties() { return 0;}
    // from gexmo_conf.XML file
    bool        IsLocal();
    // from ym_admin_db
    bool        IsUploaded();
    // get the unique id of the task (an integer which should be > 0)
    int         GetID();
    // Set the ID : todo : move me to private
    void        SetID(int id);
    // Get the 'Name' (most of the time the title of the task)
    QString     GetName() { return m_strName; }

    // return if task enable and ready to be launched
    virtual bool IsUsable(bool bCheckExecutionWindows=false, bool bCheckFrequency=false);
    // return if task is executable with ExecuteTask
    virtual bool IsExecutable();
    // return if task is for PAT process
    bool  IsForPAT();
    // return if task is for Yield-Man process
    bool  IsForYieldMan();
    // return task enable status. Does not check if mo scheduler ready,...
    bool GetEnabledState();
    // set the enable status to b
    bool SetEnabledState(bool b);
    // Return the associated database if exist (from DataPump, Yield, or Outlier)
    virtual QString GetDatabaseName();
    // Set the associated database if possible
    virtual bool SetDatabaseName(QString DatabaseName);
    // Return the Priority string
    QString   GetPriorityLabel();
    // Return the Priority int
    int       GetPriority();
    // Return the Frequency string
    virtual QString GetFrequencyLabel();
    // Timer based: check for tasks to execute...
    // return false if day of week is not matching!
    bool      AddDateFrequency(QDateTime *pDateTime,int iFrequency,int iDayOfWeek=-1);
    // For DataPump/Converter, return the INPUT folder
    virtual QString GetDataFilePath();
    // For DataPump/Converter, return the extension filter
    virtual QString GetDataFileExtension();
    // For DataPump/Converter, return the true if have to scan folder
    virtual bool IsDataFileScanSubFolder();
    // return the error status if any
    QString   GetErrorStatus();

public:

    // return the list of TaskType for YM or PAT
    static QStringList GetTypeListFor(bool PAT);
    // return the list of Name(TaskType) for YM or PAT
    static QStringList GetNameListFor(bool PAT);

    CGexMoTaskItem(QObject* parent=NULL);
protected:
    CGexMoTaskItem(const CGexMoTaskItem* orig);
public:
    virtual CGexMoTaskItem* Duplicate(QString copyName) = 0;
    virtual ~CGexMoTaskItem();

    virtual QDir::SortFlags GetDataFileSort();

    // for load-balancing
    bool    TraceExecution(QString Command,QString Status,QString Summary);
    bool    TraceUpdate(QString Command,QString Status,QString Summary);
    bool    TraceInfo();

    // Update ym_actions to indicate step DONE
    // when task is DataPump and have Yieldtasks associated
    bool    SetActionMng(int ActionId,QString Command);
    bool    UpdateActionMng(QString Summary);
    QString GetActionMngAttribute(QString key);
    QMap<QString, QString> GetActionMngAttributes() {return mActionMngAttributes;}

    QMap<QString, QString> mActionMngAttributes;
    int       iTaskMngId;

    // YieldManDb
    int       m_iDatabaseIndex;   // use for link with pDatabaseEntries because DatabaseId can be uploaded
    int       m_iNodeId;
    int       m_iUserId;
    int       m_iGroupId;
    int       m_iDatabaseId;
    int       m_iPermissions;
    QString   m_strDatabaseName;
    // Actually the title of the task
    // stored in AdminDB in 2 tables
    // Moved as an attribute
    QString   m_strName;
    QString   m_strLastInfoMsg;
    // as defined by GEXMO_TASK_... in gexmo_constants.h
    int       m_iStatus;
    bool      m_bEnabledState;
    QDateTime m_clCreationDate;
    QDateTime m_clExpirationDate;
    QDateTime m_clLastUpdate;
    QDateTime m_LastStatusUpdate;
    time_t    m_tLastExecuted;    // Last time it was executed.

    // Attribute accessor
    void          saveAttributesToTextStream(QTextStream& ts);

    /*! \brief Reset attributes :
         - reset public and private attributes
         - never in public slot
    */
    void ResetAllAttributes();
    /*! \brief Reset Private attributes :
         - reset private attributes
         - never in public slot
    */
    void ResetPrivateAttributes();
    /*! \brief Update Private attributes :
         - update Private attributes from members
         - never in public slot
    */
    virtual void UpdatePrivateAttributes();
    /*! \brief Set Private attribute :
         - Update the value of a private attribute
         - never in public slot
    */
    bool SetPrivateAttribute(const QString &key, const QVariant &value);
    bool UpdateAttribute(const QString &key, const QVariant &value);

    ///
    /// \brief Executes the task
    /// \param the data file on which the task must be executed, if any
    /// \return
    ///
    virtual QString Execute(QString DataFile);

    ///
    /// \brief Saves the current task to the provided XMLStream
    /// \param XMLStream
    ///
    virtual void SaveTaskToXML(QTextStream& XMLStream) = 0;

    ///
    /// \brief Loads the task data from the database
    /// \return
    ///
    virtual bool LoadTaskDataFromDb() = 0;

    ///
    /// \brief Saves the task data to the database
    /// \return
    ///
    virtual bool SaveTaskDataToDb();

    ///
    /// \brief Delete the task data from the database,
    ///  more specifically from the TDR
    /// \return
    ///
    virtual bool DeleteTaskDetails();

    ///
    /// \brief Checks the current task status
    /// \param checkDatabaseOption
    /// \param checkDatabase
    /// \param dbEntry
    /// \param refDb
    /// \param checkFolderOption
    /// \param checkFolder
    /// \param folderList
    ///
    void CheckTaskStatus(bool checkDatabaseOption, bool checkFolderOption);
private:
    virtual void CheckTaskStatusInternal(bool checkDatabaseOption, bool checkFolderOption) = 0;
protected:
    void CheckFolder(QStringList folderList);
    void CheckDatabase(bool checkDatabaseOption);

public:
    ///
    /// \brief Invokes the matching task creation method from the provided SchedulerGui
    /// \param gui
    /// \param readOnly
    /// \return
    ///
    virtual CGexMoTaskItem* CreateTask(SchedulerGui* gui, bool readOnly) = 0;

    ///
    /// \brief Invokes the matching list view update method from the provided SchedulerGui
    /// \param gui
    /// \param nCurrentRow
    /// \param allowEdit
    ///
    virtual void UpdateListView(SchedulerGui* gui, int nCurrentRow, bool allowEdit) = 0;

    virtual QString GetPropertiesTitle();
    virtual GexMoDataPumpTaskData* GetDataPumpData();
    virtual GexMoFileConverterTaskData* GetConverterData();

public slots:
    QVariant GetAttribute(const QString &key) const;
    bool SetAttribute(const QString &key, const QVariant &value);
    const QMap<QString, QVariant> GetAttributes();
    bool HasProperties();

    //virtual void            SetProperties(TaskProperties* properties) = 0;
    //virtual TaskProperties* GetProperties() = 0;

protected:
    TaskProperties* mTaskProperties;

    virtual bool CheckExecutionWindow();
    virtual bool CheckFrequency();
};

#endif // MO_TASK_H
