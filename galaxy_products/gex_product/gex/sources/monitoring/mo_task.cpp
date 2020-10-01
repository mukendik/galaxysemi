#include <QStringList>
#include "gqtl_log.h"

#include "mo_task.h"

#include "task_properties.h"

#include "engine.h"
#include "scheduler_engine.h"
#include "db_engine.h"
#include "mo_email.h"

#include "gex_database_entry.h"

#include "gexmo_constants.h"
#include "product_info.h"

// LOAD-BALANCING
#include "browser_dialog.h"
#include "admin_engine.h"

///////////////////////////////////////////////////////////
// Task Item initialization
///////////////////////////////////////////////////////////
CGexMoTaskItem::CGexMoTaskItem(QObject* parent)
    : QObject(parent)
{
    setObjectName("TaskItem");
    iTaskMngId        = -1;
    m_iTaskId         = -1;
    m_iNodeId         = -1;
    m_iUserId         =  1;
    m_iGroupId        =  0;
    m_iDatabaseId     =  0;
    m_iDatabaseIndex  =  0;
    m_iPermissions    =  0;
    m_iStatus         = -1;
    m_tLastExecuted   =  0;

    m_bEnabledState   = true;
    m_clLastUpdate    = GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
    m_LastStatusUpdate= GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
    m_strLastInfoMsg  = "Task not checked";

    mTaskProperties   = NULL;
}

CGexMoTaskItem::CGexMoTaskItem(const CGexMoTaskItem* orig)
    : QObject(orig->parent())
{
    setObjectName("TaskItem");
    iTaskMngId        = -1;
    m_iTaskId         = orig->m_iTaskId;
    m_iNodeId         = -1;
    m_iUserId         =  orig->m_iUserId;
    m_iGroupId        =  orig->m_iGroupId;
    m_iDatabaseId     =  0;
    m_iDatabaseIndex  =  0;
    m_strDatabaseName = orig->m_strDatabaseName;
    m_iPermissions    =  orig->m_iPermissions;
    m_iStatus         = -1;
    m_tLastExecuted   =  0;

    m_bEnabledState   = true;
    m_clLastUpdate    = GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
    m_LastStatusUpdate= GS::Gex::Engine::GetInstance().GetServerDateTime().addYears(-10);
    m_strLastInfoMsg  = "Task not checked";

    mTaskProperties   = NULL;
}

CGexMoTaskItem::~CGexMoTaskItem()
{
    if(mTaskProperties != NULL)
    {
        delete mTaskProperties;
    }
}

int CGexMoTaskItem::GetID()
{
    return m_iTaskId;
}
void CGexMoTaskItem::SetID(int id)
{
    m_iTaskId=id;
}

bool CGexMoTaskItem::IsLocal()
{
    return m_iTaskId<=0;
}

bool CGexMoTaskItem::IsUploaded()
{
    return !IsLocal();
}

bool CGexMoTaskItem::SetEnabledState(bool b)
{
    m_bEnabledState=b;
    return true;
}

QVariant CGexMoTaskItem::GetAttribute(const QString &key) const
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->GetAttribute(key);
    }
    return QVariant();
}

bool CGexMoTaskItem::SetAttribute(const QString &key, const QVariant &value)
{
    if(key.startsWith("Action"))
        return true;

    bool bStatus=false;
    QVariant lValue = GetAttribute(key);

    if(mTaskProperties != NULL)
    {
        bStatus = mTaskProperties->SetAttribute(key, value);
    }

    if(bStatus && (lValue != value))
        m_clLastUpdate = GS::Gex::Engine::GetInstance().GetServerDateTime();

    return bStatus;
}

const QMap<QString, QVariant> CGexMoTaskItem::GetAttributes()
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->GetAttributes();
    }

    return QMap<QString, QVariant>();
}

// never in public slot
void CGexMoTaskItem::ResetAllAttributes()
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->ResetAllAttributes();
    }
}

void CGexMoTaskItem::ResetPrivateAttributes()
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->ResetPrivateAttributes();
    }
}

void CGexMoTaskItem::UpdatePrivateAttributes()
{
    ResetPrivateAttributes();

    if(mTaskProperties != NULL)
    {
        return mTaskProperties->UpdatePrivateAttributes();
    }

    if(m_tLastExecuted > 0)
        SetPrivateAttribute("LastExecuted",QDateTime::fromTime_t(m_tLastExecuted).toString("yyyy-MM-dd hh:mm:ss"));
}

bool CGexMoTaskItem::SetPrivateAttribute(const QString &key, const QVariant &value)
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->SetPrivateAttribute(key,value);
    }

    return false;
}

bool CGexMoTaskItem::UpdateAttribute(const QString &key, const QVariant &value)
{
    if(mTaskProperties != NULL)
    {
        return mTaskProperties->UpdateAttribute(key,value);
    }

    return false;
}

void CGexMoTaskItem::saveAttributesToTextStream(QTextStream& ts)
{
    QMap<QString, QVariant> Attributes = GetAttributes();
    foreach(const QString &k, Attributes.keys())
    {
        // to be sure not to save invalid
        if (Attributes.value(k).isValid())
            ts << k<<"="<<Attributes.value(k).toString()<<endl;
    };
}

///////////////////////////////////////////////////////////
// Check if this task can be use by GexMo
///////////////////////////////////////////////////////////
bool CGexMoTaskItem::GetEnabledState()
{
    return m_bEnabledState;
}

bool CGexMoTaskItem::IsExecutable()
{
    return false;
}

bool CGexMoTaskItem::IsUsable(bool bCheckExecutionWindows, bool bCheckFrequency)
{

    // If the task is disabled by the user
    if(!m_bEnabledState)
        return false;

    // If the task has some error
    if(m_iStatus == MO_TASK_STATUS_ERROR)
        return false;

    if(bCheckExecutionWindows && !CheckExecutionWindow())
    {
        return false;
    }

    if(bCheckFrequency && !CheckFrequency())
    {
        return false;
    }

    // Else task is active
    return true;
}

bool CGexMoTaskItem::CheckExecutionWindow()
{
    return true;
}

bool CGexMoTaskItem::CheckFrequency()
{
    return true;
}

void CGexMoTaskItem::CheckTaskStatus(bool checkDatabaseOption, bool checkFolderOption)
{
    this->m_iStatus = MO_TASK_STATUS_NOERROR;
    this->m_strLastInfoMsg =  "";

    if(GS::LPPlugin::ProductInfo::getInstance()->isPATMan() && !this->IsForPAT())
    {
        this->m_iStatus = MO_TASK_STATUS_ERROR;
        this->m_strLastInfoMsg =  "Not a PAT task. This task can be processed only by a Yield-Man license...";
        return;
    }

    if(GS::LPPlugin::ProductInfo::getInstance()->isYieldMan() && !this->IsForYieldMan())
    {
        this->m_iStatus = MO_TASK_STATUS_ERROR;
        this->m_strLastInfoMsg =  "Not a Yield-Man task. This task can be processed only by a PAT-Man license...";
        return;
    }

    if(!mTaskProperties)
    {
        this->m_iStatus = MO_TASK_STATUS_ERROR;
        this->m_strLastInfoMsg = "Task failed to load its parameters";
        GSLOG(SYSLOG_SEV_ERROR, QString("Task %1: %2")
              .arg(this->m_strName)
              .arg(this->m_strLastInfoMsg).toLatin1().data());
        return;
    }

    this->CheckTaskStatusInternal(checkDatabaseOption, checkFolderOption);
}

void CGexMoTaskItem::CheckFolder(QStringList folderList)
{
    QDir lDir;
    QString lDirPathStg;

    bool lStatus = true;
    while(!folderList.isEmpty())
    {
        lDirPathStg = folderList.takeFirst().trimmed();
        if(lDirPathStg.isEmpty())
        {
            continue;
        }

        if(QDir::isRelativePath(lDirPathStg))
        {
            QString lTmpPath = QDir::cleanPath(QDir::currentPath()+QDir::separator()+lDirPathStg);
            // Check if dir exists or Check if file exists
            lDir.setPath(lTmpPath);
            if(lDir.exists() || lDir.exists(lDirPathStg))
            {
                lDirPathStg = lTmpPath;
            }
        }

        lDirPathStg = QDir::cleanPath(lDirPathStg);

        // Check if dir exists
        lDir.setPath(lDirPathStg);
        if(lDir.exists())
        {
            continue;
        }
        else // if dir doesn't exists try to create it
        {
            if (lDir.mkpath(lDirPathStg)) continue;
        }

        // If not a Dir
        // Check if file exists
        if(lDir.exists(lDirPathStg))
        {
            continue;
        }


        // Else not accessible
        lStatus = false;
        break;
    }
    if(!lStatus)
    {
        this->m_iStatus = MO_TASK_STATUS_ERROR;
        this->m_strLastInfoMsg = "Unable to access or create mandatory directory for this task: " + lDirPathStg;
        GSLOG(SYSLOG_SEV_ERROR, QString("Task %1: %2")
              .arg(this->m_strName)
              .arg(this->m_strLastInfoMsg).toLatin1().data());
    }
}

void CGexMoTaskItem::CheckDatabase(bool checkDatabaseOption)
{
    QString strDataBaseName = this->GetDatabaseName();
    this->SetDatabaseName(strDataBaseName);

    GexDatabaseEntry *pDatabaseEntry=NULL;
    // ptTask->m_iDatabaseId can be 0 with DatabaseName if Task just loaded from Local file
    if((this->m_iDatabaseId == 0) && (!strDataBaseName.isEmpty()))
    {
        // Check if database is referenced
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDataBaseName,false);
        if(pDatabaseEntry)
        {
            this->m_iDatabaseId = pDatabaseEntry->m_nDatabaseId;
        }
    }

    QString strReferencedDatabase;
    if((this->m_iDatabaseId != 0) && (pDatabaseEntry==NULL))
    {
        // Check if database is referenced
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(this->m_iDatabaseId);
        if(pDatabaseEntry)
        {
            strReferencedDatabase = pDatabaseEntry->LogicalName();
        }
    }

    if(strDataBaseName.startsWith("[Local]") || strDataBaseName.startsWith("[Server]"))
    {
        SetDatabaseName(strDataBaseName.section("]",1).trimmed());
    }

    if(!strDataBaseName.isEmpty() && (strDataBaseName != strReferencedDatabase))
    {
        pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(strDataBaseName,false);
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
                && pDatabaseEntry && pDatabaseEntry->m_pExternalDatabase)
        {
            GexDbPlugin_ID *pPlugin = pDatabaseEntry->m_pExternalDatabase->GetPluginID();
            if( pPlugin && pPlugin->m_pPlugin && pPlugin->m_pPlugin->m_pclDatabaseConnector
                    && (pDatabaseEntry->LogicalName() != pDatabaseEntry->m_strDatabaseRef))
            {
                // Find the referenced database
                pDatabaseEntry = GS::Gex::Engine::GetInstance().GetDatabaseEngine().FindDatabaseEntry(
                            pPlugin->pluginName(),
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strHost_Name,
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_uiPort,
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDriver,
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strSchemaName,
                            pPlugin->m_pPlugin->m_pclDatabaseConnector->m_strDatabaseName);
                if(pDatabaseEntry)
                {
                    // Update the referenced database
                    this->m_iDatabaseId = pDatabaseEntry->m_nDatabaseId;
                    this->SetDatabaseName(pDatabaseEntry->LogicalName());
                    strDataBaseName = this->GetDatabaseName();
                    if(strDataBaseName.startsWith("[Local]") || strDataBaseName.startsWith("[Server]"))
                    {
                        SetDatabaseName(strDataBaseName.section("]",1).trimmed());
                    }
                }
            }
        }
    }


    if(pDatabaseEntry)
    {
        if(checkDatabaseOption)
        {
            // Recheck database only if DB.StatusChecked < Task.StatusUpdate
            if(pDatabaseEntry->m_LastStatusChecked < this->m_LastStatusUpdate)
            {
                GS::Gex::Engine::GetInstance().GetDatabaseEngine().CheckDatabaseEntryStatus(pDatabaseEntry, false);
            }
        }

        this->m_iDatabaseId = pDatabaseEntry->m_nDatabaseId;

        if(!(pDatabaseEntry->StatusFlags() & STATUS_CONNECTED))
        {
            // Database not connected
            // Disable this task
            this->m_iStatus = MO_TASK_STATUS_ERROR;
            this->m_strLastInfoMsg = "Task was disabled. [Database " + this->GetDatabaseName() + " not connected]";
            return;
        }

        if(!(pDatabaseEntry->IsCompatible()))
        {
            // FLEXIBLE CONSOLIDATION
            if(pDatabaseEntry->IsUpToDate())
            {
                // Database Consolidation error
                // Do not disable this task but warning
                this->m_iStatus = MO_TASK_STATUS_WARNING;
                if(pDatabaseEntry->m_strLastInfoMsg.contains(":"))
                {
                    this->m_strLastInfoMsg = "Task has a warning. [ " + pDatabaseEntry->m_strLastInfoMsg.section(":",1) + " ]";
                }
                else
                {
                    this->m_strLastInfoMsg = "Task has a warning. [ " + pDatabaseEntry->m_strLastInfoMsg + " ]";
                }
                return;
            }
            else
            {
                // Database not uptodate
                // Do not disable this task but warning
                this->m_iStatus = MO_TASK_STATUS_ERROR;
                this->m_strLastInfoMsg = "Task was disabled. [Database " + this->m_strDatabaseName + " not uptodate]";
                return;
            }
        }
    }
    else
    {
        this->m_iStatus = MO_TASK_STATUS_ERROR;
        this->m_iDatabaseId = 0;
        this->m_strLastInfoMsg = "Task must have a valid Database referenced.";
        if(strDataBaseName.isEmpty())
        {
            this->m_strLastInfoMsg += " [Empty database name]";
        }
        else
        {
            this->m_strLastInfoMsg += " [" + strDataBaseName + ": invalid database]";
        }
    }
}

bool CGexMoTaskItem::IsForPAT()
{
    return GetTypeListFor(true).contains(QString::number(GetTaskType()));
}

bool CGexMoTaskItem::IsForYieldMan()
{
    return GetTypeListFor(false).contains(QString::number(GetTaskType()));
}

/// \brief CGexMoTaskItem::GetNameListFor
/// \param PAT: considere (bool PAT) as an enum i.e. true indicates for PAT, false indicates for YieldMan
/// \return the list of TaskType for YM or PA
QStringList CGexMoTaskItem::GetNameListFor(bool PAT)
{
    QStringList TaskNameList;

    TaskNameList << "OLD_DATAPUMP";
    TaskNameList << "STATUS";
    TaskNameList << "AUTOADMIN";

    if(PAT)
    {
        TaskNameList << "PATPUMP";
        TaskNameList << "OUTLIER_REMOVAL";
    }
    else
    {
        // If NOT RDB, hide RDB button
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
            // Disable RDB support
            TaskNameList << "TRIGGERPUMP";
            TaskNameList << "YIELDMONITOR";
            TaskNameList << "YIELDMONITOR_RDB";
            TaskNameList << "SPM";
            TaskNameList << "SYA";
        }

        TaskNameList << "DATAPUMP";
        TaskNameList << "CONVERTER";
        TaskNameList << "REPORTING";
        TaskNameList << "INCREMENTAL_UPDATE";
    }

    return TaskNameList;
}


/// \brief CGexMoTaskItem::GetTypeListFor
/// \param PAT: considere (bool PAT) as an enum i.e. true indicates for PAT, false indicates for YieldMan
/// \return the list of TaskType for YM or PA
QStringList CGexMoTaskItem::GetTypeListFor(bool PAT)
{
    QStringList TaskList;

    TaskList << QString::number(GEXMO_TASK_OLD_DATAPUMP);
    TaskList << QString::number(GEXMO_TASK_STATUS);
    TaskList << QString::number(GEXMO_TASK_AUTOADMIN);

    if(PAT)
    {
        TaskList << QString::number(GEXMO_TASK_PATPUMP);
        TaskList << QString::number(GEXMO_TASK_OUTLIER_REMOVAL);
    }
    else
    {
        // If NOT RDB, hide RDB button
        if(GS::LPPlugin::ProductInfo::getInstance()->isDBPluginAllowed())
        {
            // Disable RDB support
            TaskList << QString::number(GEXMO_TASK_TRIGGERPUMP);
            TaskList << QString::number(GEXMO_TASK_YIELDMONITOR);
            TaskList << QString::number(GEXMO_TASK_YIELDMONITOR_RDB);
            TaskList << QString::number(GEXMO_TASK_SPM);
            TaskList << QString::number(GEXMO_TASK_SYA);
        }

        TaskList << QString::number(GEXMO_TASK_DATAPUMP);
        TaskList << QString::number(GEXMO_TASK_CONVERTER);
        TaskList << QString::number(GEXMO_TASK_REPORTING);
        TaskList << QString::number(GEXMO_TASK_INCREMENTAL_UPDATE);
    }

    return TaskList;
}

QString CGexMoTaskItem::GetDatabaseName()
{
    return m_strDatabaseName;
}

bool CGexMoTaskItem::SetDatabaseName(QString DatabaseName)
{
    m_strDatabaseName = DatabaseName;

    return true;
}

///////////////////////////////////////////////////////////
QString CGexMoTaskItem::GetPriorityLabel()
{
    QString Priority;
    switch(GetPriority())
    {
    case 0: Priority="Low";
        break;
    case 1: Priority="Medium";
        break;
    case 2: Priority="High";
        break;
    default: Priority="";
    }
    return Priority;
}

///////////////////////////////////////////////////////////
int CGexMoTaskItem::GetPriority()
{
    int Priority = -1;
    if(IsUploaded())
        Priority = GetAttribute("Priority").toInt();

    return Priority;
}



///////////////////////////////////////////////////////////
QString CGexMoTaskItem::GetFrequencyLabel()
{
    return "";
}


///////////////////////////////////////////////////////////
/// pDataTime is the Last Execution of the task
/// iFrequency is an integer
///     0 to 14 for minute and hour
///     15 to 24 for day, week and month
///////////////////////////////////////////////////////////
bool CGexMoTaskItem::AddDateFrequency(QDateTime *pDateTime,int iFrequency,int iDayOfWeek/*=-1*/)
{
    bool    bValidDayOfWeek=true;
    QDate   cCurrentDate = QDate::currentDate();
    int     iToday = cCurrentDate.dayOfWeek()-1;    // 0=Monday,...6=Sunday

    // If in Y123 mode, set frequency to every second
    //    if(GS::LPPlugin::ProductInfo::getInstance()->isY123WebMode())
    //    {
    //        *pDateTime = pDateTime->addSecs(1);
    //        return true;
    //    }

    // GCORE-17185 - Scheduled reports running at wrong time
    // For Frequency based on minute hour => use the Time
    // For Frequency based on day week month => ignore the Time and use only the Date
    if(iFrequency >= GEXMO_TASKFREQ_1DAY)
    {
        // Based on day
        // Remove the Time
        *pDateTime = QDateTime(pDateTime->date());
    }
    switch(iFrequency)
    {
    case    GEXMO_TASKFREQ_1MIN:                // Task every: 1 minute
        *pDateTime = pDateTime->addSecs(60);
        break;

    case    GEXMO_TASKFREQ_2MIN:                // Task every: 2 minutes
        *pDateTime = pDateTime->addSecs(2*60);
        break;

    case    GEXMO_TASKFREQ_3MIN:                // Task every: 3 minutes
        *pDateTime = pDateTime->addSecs(3*60);
        break;

    case    GEXMO_TASKFREQ_4MIN:                // Task every: 4 minutes
        *pDateTime = pDateTime->addSecs(4*60);
        break;

    case    GEXMO_TASKFREQ_5MIN:                // Task every: 5 minutes
        *pDateTime = pDateTime->addSecs(5*60);
        break;

    case    GEXMO_TASKFREQ_10MIN:               // Task every: 10 minutes
        *pDateTime = pDateTime->addSecs(10*60);
        break;

    case    GEXMO_TASKFREQ_15MIN:               // Task every: 15 minutes
        *pDateTime = pDateTime->addSecs(15*60);
        break;

    case    GEXMO_TASKFREQ_30MIN:               // Task every: 30 minutes
        *pDateTime = pDateTime->addSecs(30*60);
        break;

    case    GEXMO_TASKFREQ_1HOUR:               // Task every: 1 hour
        *pDateTime = pDateTime->addSecs(60*60);
        break;

    case    GEXMO_TASKFREQ_2HOUR:               // Task every: 2 hours
        *pDateTime = pDateTime->addSecs(2*60*60);
        break;

    case    GEXMO_TASKFREQ_3HOUR:               // Task every: 3 hours
        *pDateTime = pDateTime->addSecs(3*60*60);
        break;

    case    GEXMO_TASKFREQ_4HOUR:               // Task every: 4 hours
        *pDateTime = pDateTime->addSecs(4*60*60);
        break;

    case    GEXMO_TASKFREQ_5HOUR:               // Task every: 5 hours
        *pDateTime = pDateTime->addSecs(5*60*60);
        break;

    case    GEXMO_TASKFREQ_6HOUR:               // Task every: 6 hours
        *pDateTime = pDateTime->addSecs(6*60*60);
        break;

    case    GEXMO_TASKFREQ_12HOUR:              // Task every: 12 hours
        *pDateTime = pDateTime->addSecs(12*60*60);
        break;

    case    GEXMO_TASKFREQ_1DAY:                // Task every: 1 day
        *pDateTime = pDateTime->addDays(1);
        break;

    case    GEXMO_TASKFREQ_2DAY:                // Task every: 2 days
        *pDateTime = pDateTime->addDays(2);
        break;

    case    GEXMO_TASKFREQ_3DAY:                // Task every: 3 days
        *pDateTime = pDateTime->addDays(3);
        break;

    case    GEXMO_TASKFREQ_4DAY:                // Task every: 4 days
        *pDateTime = pDateTime->addDays(4);
        break;

    case    GEXMO_TASKFREQ_5DAY:                // Task every: 5 days
        *pDateTime = pDateTime->addDays(5);
        break;

    case    GEXMO_TASKFREQ_6DAY:                // Task every: 6 days
        *pDateTime = pDateTime->addDays(6);
        break;

    case    GEXMO_TASKFREQ_1WEEK:               // Task every: 1 week
        *pDateTime = pDateTime->addDays(7);
        if(iToday != iDayOfWeek) bValidDayOfWeek = false;
        break;

    case    GEXMO_TASKFREQ_2WEEK:               // Task every: 2 weeks
        *pDateTime = pDateTime->addDays(14);
        if(iToday != iDayOfWeek) bValidDayOfWeek = false;
        break;

    case    GEXMO_TASKFREQ_3WEEK:               // Task every: 3 weeks
        *pDateTime = pDateTime->addDays(21);
        if(iToday != iDayOfWeek) bValidDayOfWeek = false;
        break;

    case    GEXMO_TASKFREQ_1MONTH:              // Task every: 1 month
        *pDateTime = pDateTime->addMonths(1);
        if(iToday != iDayOfWeek) bValidDayOfWeek = false;
        break;
    }

    // return 'true' if this task can be executed today!
    if(iDayOfWeek >= 0)
        return bValidDayOfWeek;
    return true;
}


QString CGexMoTaskItem::GetDataFilePath()
{
    return "";
}

QString CGexMoTaskItem::GetDataFileExtension()
{
     return "";
}

bool CGexMoTaskItem::IsDataFileScanSubFolder()
{
    return false;
}

QDir::SortFlags CGexMoTaskItem::GetDataFileSort()
{
    QDir::SortFlags SortFile;

    bool bValid;
    SortFile = (QDir::SortFlags) GetAttribute("SortFlags").toInt(&bValid);
    if(!bValid)
        SortFile = QDir::Time;

    return SortFile;
}

QString CGexMoTaskItem::GetErrorStatus()
{
    QString strError;
    if(m_iStatus == MO_TASK_STATUS_ERROR)
        strError = m_strLastInfoMsg;
    return strError;
}

bool CGexMoTaskItem::TraceExecution(QString Command,QString Status,QString Summary)
{
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
    {
        // Process contains the command ligne
        // Arg1=v1|Arg2=v2|...
        int     FileSize = 0;
        QString FileName;
        QString TaskStatus = Status.trimmed();
        QString TaskSummary = Summary.trimmed();
        QString TaskComment;
        if(mActionMngAttributes.contains("ActionId"))
        {
            // Include the ActionId
            TaskComment = "action_id=" + mActionMngAttributes["ActionId"];
        }

        if(Command.contains("FileName="))
        {
            QMap<QString,QString> Args = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Command);
            FileName = Args["FileName"];
            FileSize = QFileInfo(FileName).size();
        }
        if(Status == "START")
            TaskStatus = Status;
        else if(Status == "INFO")
            TaskStatus = Status;
        else if(Status == "WARNING")
            TaskStatus = Status;
        else if(Status.isEmpty() || Status.startsWith("ok:",Qt::CaseInsensitive))
            TaskStatus = "PASS";
        else if(Status.startsWith("delay:",Qt::CaseInsensitive))
            TaskStatus = "DELAY";
        else
            TaskStatus = "FAIL";

        if(Status.startsWith("ok:",Qt::CaseInsensitive)
                || Status.startsWith("delay:",Qt::CaseInsensitive)
                || Status.startsWith("error:",Qt::CaseInsensitive))
            TaskSummary = Status.section(":",1).trimmed();

        if(Summary.contains("Status=",Qt::CaseInsensitive))
        {
            QMap<QString,QString> mapSummary = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Summary.toLower());
            QString StatusNumber = mapSummary["status"];
            if(StatusNumber == "0") TaskStatus = "PASS";
            else if(StatusNumber == "1") TaskStatus = "FAIL";
            else if(StatusNumber == "2") TaskStatus = "FAIL";
            else if(StatusNumber == "3") TaskStatus = "DELAY";

            // Get the last size after uncompress
            int Size = mapSummary["stdffilesize"].toInt();
            if(Size > FileSize)
                FileSize = Size;
        }

        if((TaskStatus == "FAIL") && !TaskSummary.contains("error=",Qt::CaseInsensitive))
            TaskSummary = "error="+TaskSummary;

        GS::Gex::Engine::GetInstance().GetAdminEngine()
                .AddNewEvent("EXECUTION",GetTypeName(),FileSize
                             ,m_iTaskId,m_iDatabaseId
                             ,Command,TaskStatus,TaskSummary,TaskComment);
    }
    return true;
}

bool CGexMoTaskItem::TraceUpdate(QString Command,QString Status,QString Summary)
{
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && IsUploaded())
    {
        // Update the Events flow
        GS::Gex::Engine::GetInstance().GetAdminEngine()
                .AddNewEvent("UPDATE",GetTypeName(),
                             0,m_iTaskId,0,Command,
                             Status,Summary,"");

        if(((Command == "DELETE") || (Command == "EDIT")) && (Status == "PASS"))
            GS::Gex::Engine::GetInstance().GetAdminEngine()
                    .CleanActions("mutex IS NULL AND task_id="+QString::number(m_iTaskId));
    }
    return true;
}

bool CGexMoTaskItem::TraceInfo()
{
    QDateTime cCurrentDateTime=GS::Gex::Engine::GetInstance().GetClientDateTime();
    QString strErrorMessage = cCurrentDateTime.toString("[d MMMM yyyy h:mm:ss] ");
    strErrorMessage	+= "["+m_strName+"] - "+ m_strLastInfoMsg + "\n";

    GS::Gex::Engine::GetInstance().GetSchedulerEngine().AppendMoHistoryLog(strErrorMessage);

    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated() && IsUploaded())
    {
        // Update the Events flow
        if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated())
            GS::Gex::Engine::GetInstance().GetAdminEngine().AddNewEvent("UPDATE",GetTypeName(),
                                                                        0,m_iTaskId,0,"",
                                                                        "INFO","["+m_strName+"] - "+ m_strLastInfoMsg,"");
    }
    return true;
}

bool CGexMoTaskItem::SetActionMng(int ActionId,QString Command)
{
    mActionMngAttributes.clear();
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && ActionId > 0)
    {
        mActionMngAttributes = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Command);
        mActionMngAttributes["ActionId"] = QString::number(ActionId);
    }
    return true;
}

bool CGexMoTaskItem::UpdateActionMng(QString Summary)
{
    // The UpdateActionMng is used to trace the status of the execution of the task
    // when this execution has different steps
    // ie for DATAPUMP, 1 INSERTION step and a list of MONITORING task
    // After the INSERTION process, we can update the ym_actions to indicate that the 'insertionstatus' is DONE
    // If the process is stopped just after this step (YM deamon stop or crash)
    // the ym_actions keep the trace of the insertion and restart the action at this point
    // GCORE-4202
    // Because of the spider environement or for collision reason ?
    // May be this update can lock the row before and the row after and cause some 'Lock wait timeout'
    if(GS::Gex::Engine::GetInstance().GetAdminEngine().IsActivated()
            && (mActionMngAttributes.contains("ActionId"))
            && (mActionMngAttributes["ActionId"].toInt() > 0))
    {
        foreach(const QString &key, GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Summary).keys())
            mActionMngAttributes[key] = GS::Gex::Engine::GetInstance().GetAdminEngine().SplitCommand(Summary)[key];

        GS::Gex::Engine::GetInstance().GetAdminEngine().UpdateAction(
                    mActionMngAttributes["ActionId"].toInt(),
                GS::Gex::Engine::GetInstance().GetAdminEngine().JoinCommand(mActionMngAttributes));

    }
    return true;
}

QString CGexMoTaskItem::GetActionMngAttribute(QString key)
{
    QString value;

    // Case insensitive
    if(mActionMngAttributes.contains(key))
        value = mActionMngAttributes[key];
    else if(mActionMngAttributes.contains(key.toLower()))
        value = mActionMngAttributes[key.toLower()];

    return value;
}

bool CGexMoTaskItem::SaveTaskDataToDb()
{
    return GS::Gex::Engine::GetInstance().GetSchedulerEngine().SaveDbTaskAttributes(this);
}

bool CGexMoTaskItem::DeleteTaskDetails()
{
    return true;
}

QString CGexMoTaskItem::Execute(QString DataFile)
{
    return "";
}

QString CGexMoTaskItem::GetPropertiesTitle()
{
    return "";
}

bool CGexMoTaskItem::HasProperties()
{
    return (mTaskProperties != NULL);
}

GexMoDataPumpTaskData* CGexMoTaskItem::GetDataPumpData()
{
    return NULL;
}

GexMoFileConverterTaskData* CGexMoTaskItem::GetConverterData()
{
    return NULL;
}
