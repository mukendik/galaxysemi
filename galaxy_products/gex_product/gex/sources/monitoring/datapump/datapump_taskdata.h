#ifndef DATAPUMP_TASKDATA_H
#define DATAPUMP_TASKDATA_H

#include <time.h>
#include <QString>
#include <QDir>
#include <QTime>
#include <QMap>
#include <QVariant>
#include "task_properties.h"

#define GEX_DATAPUMP_DATATYPE_TEST	0
#define GEX_DATAPUMP_DATATYPE_WYR	1

class CGexMoTaskItem;

class GexMoDataPumpTaskData : public TaskProperties
{
public:
    GexMoDataPumpTaskData(QObject* parent);	// Constructor

    GexMoDataPumpTaskData& operator= (const GexMoDataPumpTaskData &copy);

    //! \brief Default script when not allowed
    const static QString sIllegalScript;
    //! \brief Default Pre Script
    const static QString sDefaultPreScript;
    //! \brief Default POST Script
    const static QString sDefaultPostScript;
    //! \brief Default script for Data pump
    const static QString sDefaultDataPumpPreScript;
    //! \brief Script run before (insertion, PAT, ...)
    const static QString sPreScriptAttrName;
    //! \brief Script run after (insertion, PAT, ...)
    const static QString sPostScriptAttrName;

    //! \brief Bool to activate or not the Post Script
    const static QString sPostScriptActivatedAttrName;
    //! \brief Bool to activate or not the Pre Script
    const static QString sPreScriptActivatedAttrName;

    // new attribute to take care of values of the statistical agents configuration
    const static QString sStatisticalAgentsConfigurationADRAttrName;

    QString         strTitle;               // Task title.
    QString         strDataPath;            // Folder to scan for incoming test data files (or FTP URL)
    bool            bScanSubFolders;        // 'true', if also scan sub-folders
    QDir::SortFlags eSortFlags;

    QString         strImportFileExtensions;// list of files extensions to import (eg: *.stdf;*.wat)
    QString         strDatabaseTarget;      // Database to receive incoming data files
    unsigned int    uiDataType;             // Type of data to be inserted if GEXDB database (Test Data, Weekly Yield Report...)
    QString         strTestingStage;        // Testing stage of data to be inserted if WYR
    int             iFrequency;             // Task frequency. (use only for local task)
    int             iDayOfWeek;             // Day of Week to execute task (0= Monday, ...6=Sunday)
    bool            bExecutionWindow;       // 'true' if only execute task during a given time window
    int             iPostImport;            // Tells what to do with files after being read: rename, move, delete.
    QString         strPostImportMoveFolder;// Move/FTP location of source file after insertion
    int             iPostImportFailure;     // Tells what to do with BAD files
    QString         strPostImportFailureMoveFolder;// Where to move BAD files
    int             iPostImportDelay;       // Tells what to do with DELAYED files
    QString         strPostImportDelayMoveFolder;// Where to move DELAYED files

    QTime           cStartTime;                 // Task starting time (if time window activated)
    QTime           cStopTime;                  // Task ending time (if time window activated)
    bool            bCheckYield;                // 'true' if Check test data file yield level
    QString         strYieldBinsList;           // List of bins to monitor (Good bins)
    int             iAlarmLevel;                // Ranges in 0-100
    long            lMinimumyieldParts;         // Minimum parts required to check the yield.
    QString         strEmailFrom;               // Email address of sender.
    QString         strEmailNotify;             // Email addresses to notify.
    bool            bHtmlEmail;                 // 'true' if email to be sent in HTML format

    // Other options
    bool            bRejectSmallSplitlots_NbParts;      // Reject files with less than N parts
    unsigned int    uiRejectSmallSplitlots_NbParts;
    bool            bRejectSmallSplitlots_GdpwPercent;  // Reject files with less than p*GDPW parts
    double          lfRejectSmallSplitlots_GdpwPercent;
    bool            bExecuteBatchAfterInsertion;        // Set to true if a batch file should be executed on insertion
    QString         strBatchToExecuteAfterInsertion;    // Full name of the batch file to be executed on insertion
    int             nMaxPartsForTestResultInsertion;    // For files with more parts, raw test results will not be inserted (-1 disables this option)
    bool            bRejectFilesOnPassBinlist;          // Activate option to reject files with PASS hard bins not in specified list
    QString         strPassBinListForRejectTest;        // Pass Binlist for above option

    int             mPriority;

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // DATAPUMP_TASKDATA_H
