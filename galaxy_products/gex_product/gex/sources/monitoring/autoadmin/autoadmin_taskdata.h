#ifndef AUTOADMIN_TASKDATA_H
#define AUTOADMIN_TASKDATA_H

#include <time.h>
#include <QString>
#include <QTime>
#include "task_properties.h"

const QString C_ShellYieldPass      = "ShellYieldPass";
const QString C_ShellSpmPass        = "ShellSpmPass";
const QString C_ShellSyaPass        = "ShellSyaPass";
const QString C_ShellPatPass        = "ShellPatPass";

const QString C_ShellYieldAlarm     = "ShellYieldAlarm";
const QString C_ShellSpmAlarm       = "ShellSpmAlarm";
const QString C_ShellSyaAlarm       = "ShellSyaAlarm";
const QString C_ShellPatAlarm       = "ShellPatAlarm";

const QString C_ShellYieldAlarmCritical     = "ShellYieldAlarmCritical";
const QString C_ShellSpmAlarmCritical       = "ShellSpmAlarmCritical";
const QString C_ShellSyaAlarmCritical       = "ShellSyaAlarmCritical";
const QString C_ShellPatAlarmCritical       = "ShellPatAlarmCritical";

const QString C_ShellYieldPassIsEnabled     = "ShellYieldPassIsEnabled";
const QString C_ShellSpmPassIsEnabled       = "ShellSpmPassIsEnabled";
const QString C_ShellSyaPassIsEnabled       = "ShellSyaPassIsEnabled";
const QString C_ShellPatPassIsEnabled       = "ShellPatPassIsEnabled";

const QString C_ShellYieldAlarmIsEnabled    = "ShellYieldAlarmIsEnabled";
const QString C_ShellSpmAlarmIsEnabled      = "ShellSpmAlarmIsEnabled";
const QString C_ShellSyaAlarmIsEnabled      = "ShellSyaAlarmIsEnabled";
const QString C_ShellPatAlarmIsEnabled      = "ShellPatAlarmIsEnabled";

const QString C_ShellYieldAlarmCriticalIsEnabled    = "ShellYieldAlarmCriticalIsEnabled";
const QString C_ShellSpmAlarmCriticalIsEnabled      = "ShellSpmAlarmCriticalIsEnabled";
const QString C_ShellSyaAlarmCriticalIsEnabled      = "ShellSyaAlarmCriticalIsEnabled";
const QString C_ShellPatAlarmCriticalIsEnabled      = "ShellPatAlarmCriticalIsEnabled";

// Auto Admin: Compression mode
#define	GEXMO_AUTOADMIN_COMPRESSION_NEVER		0	// Never Compress Databases
#define	GEXMO_AUTOADMIN_COMPRESSION_ALWAYS		1	// Always Compress Databases

// Log file contents
#define	GEXMO_AUTOADMIN_LOG_EXCEPTIONS		0	// Log exceptions only
#define	GEXMO_AUTOADMIN_LOG_DETAILS			1	// Log all tasks details on top of exceptions

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;

class GexMoAutoAdminTaskData : public TaskProperties
{
public:
    GexMoAutoAdminTaskData(QObject* parent);	// Constructor

    GexMoAutoAdminTaskData& operator= (const GexMoAutoAdminTaskData &copy);


    QString mTitle;                 ///< Task title.
    int     mKeepReportDuration;    ///< 0= 1 day, 1= 2 days, 3= 3 days...
    int     mLogContents;           ///< GEXMO_AUTOADMIN_LOG_EXCEPTIONS, GEXMO_AUTOADMIN_LOG_DETAILS...
    QTime   mStartTime;             ///< Task starting time every day.
    QString mEmailFrom;             ///< Email 'From' address
    QString mEmailNotify;           ///< Email addresses to notify.
    bool    mSendInsertionLogs;     ///< To send or not daily insertion logs
    QDate   mLastInsertionLogsSent;

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // AUTOADMIN_TASKDATA_H
