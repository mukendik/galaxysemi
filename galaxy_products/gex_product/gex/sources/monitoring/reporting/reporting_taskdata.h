#ifndef REPORTING_TASKDATA_H
#define REPORTING_TASKDATA_H

#include <time.h>
#include "task_properties.h"

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;

class GexMoReportingTaskData : public TaskProperties
{
public:
    GexMoReportingTaskData(QObject* parent);	// Constructor

    GexMoReportingTaskData& operator= (const GexMoReportingTaskData &copy);

    QString	strTitle;			// Task title.
    QString	strScriptPath;		// Script file to execute.
    int		iFrequency;			// Task frequency.
    int		iDayOfWeek;			// Day of Week to execute task (0= Monday, ...6=Sunday)
    bool	bExecutionWindow;	// 'true' if only execute task during a given time window
    QTime	cStartTime;			// Task starting time (if time window activated)
    QTime	cStopTime;			// Task ending time (if time window activated)
    int		iNotificationType;	// Email zipped report, or email URL to report, etc...
    QString strEmailFrom;		// Email address of sender.
    QString	strEmailNotify;		// Email addresses to notify.
    bool	bHtmlEmail;			// 'true' if email to be sent in HTML format

    int     mPriority;

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // REPORTING_TASKDATA_H
