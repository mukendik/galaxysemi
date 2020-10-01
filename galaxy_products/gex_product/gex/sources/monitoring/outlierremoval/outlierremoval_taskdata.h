#ifndef OUTLIERREMOVAL_TASKDATA_H
#define OUTLIERREMOVAL_TASKDATA_H

#include <QObject>

#include <QString>
#include <QDir>
#include <QTime>
#include <QMap>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "task_properties.h"

#define	GEXMO_OUTLIER_EMAILREPORT_TEXT		0
#define	GEXMO_OUTLIER_EMAILREPORT_WORD		1
#define	GEXMO_OUTLIER_EMAILREPORT_EXCEL		2
#define	GEXMO_OUTLIER_EMAILREPORT_PPT		3
#define	GEXMO_OUTLIER_EMAILREPORT_PDF		4

// Forward declaration (in scheduler_engine.h)
class CGexMoTaskItem;
class GexDatabaseEntry;


// Classes for Composite PAT: pre-processing a full lot to identify exclusion zones, 3D neighborhood, etc...
class CGexCompositePatWaferInfo
{
public:
    void		clear();			// Reset fields

    int			iWaferID;
    QStringList	strSources;			// Holds input test data files (if more than one, need to merge them)
    QString		strDataFile;		// Data file to process: in case source requires merging multiple files.
    QString		strOptionalSource;	// Optional input. E.g: KLA/INF
};
typedef QMap<int, CGexCompositePatWaferInfo> CompositeWaferMaps;

class CGexCompositePatProcessing : public QObject
{
    Q_OBJECT
public:
    CGexCompositePatProcessing(QObject* parent=0);		// Constructor
    //! \brief Copy constructor in case we need one day to publish to the scirpt engine
    //CGexCompositePatProcessing(const CGexCompositePatProcessing &lOther);

public:
    //! \brief Set members through keys strings
    Q_INVOKABLE bool Set(const QString &lKey, const QString &lValue);

    QString		strTriggerFile;			// Full path to composite trigger file processed.
    QString		strRecipeFile;			// Full path to custom recipe file.
    QString		strLogFilePath;			// LogFile path. If empty, disable LogFile generation.
    QString		strOutputReportFormat;	// Type of report to create (PDF, PPT, Word, ...)
    bool		bSwapDieX;				// 'true' if mirror dies on the X axis
    bool		bSwapDieY;				// 'true' if mirror dies on the Y axis
    bool		bRetest_HighestBin;		// 'true' if retest policy is to promote highest bin instead of last bin
    QString		strPostProcessShell;	// Shell command to call once trigger file processed
    QString		strProductName;
    QString		strCustomerName;		// Customer name field to write in Wafermap file (Semi85)
    QString		strSupplierName;		// Supplier name field to write in Wafermap file (Semi85)

    QString		strCompositeFile;		// COMPOSITE file created (holds exclusion zones, 3D neighborhood)
    CompositeWaferMaps cWaferMaps;		// holds info aout each wafer in Lot.

    // Variables used when processing Trigger files.
    QFile       m_LogFile;
    QTextStream m_hLogFile;
    QString     m_strLogFileName;
    QString     m_strTemporaryLog;

    FILE	*m_hReportFile;
    QString	m_strReportFileName;
    QString m_strTemporaryReport;
};

class GexMoOutlierRemovalTaskData : public TaskProperties
{
public:
    GexMoOutlierRemovalTaskData(QObject* parent);	// Constructor

    GexMoOutlierRemovalTaskData& operator= (const GexMoOutlierRemovalTaskData &copy);

    QString strTitle;           // Task title.
    QString strDatabase;        // Database to focus on
    QString strTestingStage;    // Testing stage tables to focus on
    QString strProductID;       // ProductID to look for
    int     iAlarmType;         // 0= Trigger alarm of given % of parts are fail. 1= Trigger alarm of given # of parts are fail
    double  lfAlarmLevel;       // Alarm level (if higher failure rate than this field, trigger alarm)
    long    lMinimumyieldParts; // Minimum parts required to check the yield.
    long    lCompositeEtestAlarm; // Maximum number of dies discrepancies (between E-Test and STDF wafmap)
    long    lCompositeExclusionZoneAlarm; // Maximum number of dies rejected on the exclusion zone wafere (stacked wafer)
    QString strEmailFrom;       // Email address of sender.
    QString strEmailNotify;     // Email addresses to notify.
    bool    bHtmlEmail;         // 'true' if email to be sent in HTML format
    bool    bNotifyShapeChange; // 'true' = notify by email if distribution detected is different from historical data.
    int     iEmailReportType;   // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, CSV, Word,  PPT, PDF
    int     iNotificationType;  // 0= Report attached to email, 1= keep on server, only email its URL
    int     iExceptionLevel;    // 0=Standard, 1=Critical...

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // OUTLIERREMOVAL_TASKDATA_H
