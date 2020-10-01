#ifndef YIELD_TASKDATA_H
#define YIELD_TASKDATA_H

#include <QDate>
#include <QObject>
#include "task_properties.h"
#include "gexdb_plugin_base.h"

// Email report format
#define	GEXMO_YIELD_EMAILREPORT_TEXT	0
#define	GEXMO_YIELD_EMAILREPORT_WORD	1
#define	GEXMO_YIELD_EMAILREPORT_EXCEL	2
#define	GEXMO_YIELD_EMAILREPORT_PPT		3
#define	GEXMO_YIELD_EMAILREPORT_PDF		4


// SYL/SBL: Validity Period
#define	GEXMO_YIELD_SYA_PERIOD_1W		0
#define	GEXMO_YIELD_SYA_PERIOD_1M		1
#define	GEXMO_YIELD_SYA_PERIOD_2M		2
#define	GEXMO_YIELD_SYA_PERIOD_3M		3
#define	GEXMO_YIELD_SYA_PERIOD_6M		4
#define	GEXMO_YIELD_SYA_PERIOD_1Y		5

#define YIELDMO_TASK_ATTR_OUTLIERED_DISTRIBS "OutlieredDistribs"
#define YIELDMO_TASK_ATTR_SYA_BIN_EXCLUSION  "SYABinExclusion"

class GexMoYieldMonitoringTaskData : public TaskProperties
{
public:
    GexMoYieldMonitoringTaskData(QObject* parent);	// Constructor

    GexMoYieldMonitoringTaskData& operator= (const GexMoYieldMonitoringTaskData &copy);

    static const QStringList sCheckTypes;
    QString strTitle;                   // Task title.
    QString strDatabase;                // Database to focus on
    QString strTestingStage;            // Testing stage tables to focus on
    QString strProductID;               // ProductID to look for : 7170
    QString strYieldBinsList;           // List of bins to monitor
    int     iBiningType;                // Binning type: 0=Good bins, 1=Failing bins
    int     iAlarmLevel;                // Ranges in 0-100
    int     iAlarmIfOverLimit;          // 0= alarm if Yield Under Limit, 1= alarm if Yield Over limit
    long    lMinimumyieldParts;         // Minimum parts required to check the yield.
    QString strSblFile;                 // Full path to SBL/YBL data file (for Statistical Bin Limits monitoring)
    QString strEmailFrom;               // 'From' Email addresses of email notification
    QString strEmailNotify;             // Email addresses to notify.
    bool    bHtmlEmail;                 // 'true' if email to be sent in HTML format
    int     iEmailReportType;           // GEXMO_YIELD_EMAILREPORT_xxx  :ASCII text in BODY, Excel, Word,  PPT, PDF
    int     iNotificationType;          // 0= Report attached to email, 1= keep on server, only email its URL
    int     iExceptionLevel;            // 0=Standard, 1=Critical...
    // SYA (SYL-SBL) (SQL RDB only)
    bool    bSYA_active_on_datafile_insertion;  // the rule will be considered on datafile insertion (splitlot)
    bool    bSYA_active_on_trigger_file;        // the rule will be considered on GalaxySemi trigger .gtf file
    OutlierRule eSYA_Rule;              // Rule: 0=N*Sigma, 1=N*IQR,...
    QString strSYA_Rule;                // Name of the rule (for recording in GEXDB)
    float   fSYA_N1_value;              // N1 parameter
    float   fSYA_N2_value;              // N2 parameter
    int     iSYA_LotsRequired;          // Minimum Total lots required for computing new SYL-SBL
    int     iSYA_Period;                // Period for reprocessing SYL/SBL: 0=1week,1=1Month,2...
    QString strSYA_SBL1_LL_Disabled;    // List of Binnings for which the SBL1 LL should be disabled
    QString strSYA_SBL1_HL_Disabled;    // List of Binnings for which the SBL1 HL should be disabled
    QString strSYA_SBL2_LL_Disabled;    // List of Binnings for which the SBL2 LL should be disabled
    QString strSYA_SBL2_HL_Disabled;    // List of Binnings for which the SBL2 HL should be disabled
    bool    bSYA_SYL1_LL_Disabled;      // True if SYL1 LL should be disabled
    bool    bSYA_SYL1_HL_Disabled;      // True if SYL1 HL should be disabled
    bool    bSYA_SYL2_LL_Disabled;      // True if SYL2 LL should be disabled
    bool    bSYA_SYL2_HL_Disabled;      // True if SYL2 HL should be disabled
    bool    bSYA_IgnoreDataPointsWithNullSigma;	// Set to true if datapoints with null sigma should be ignored
    bool    bSYA_IgnoreOutliers;        // Set to true if outliers shoud be ignored
    bool    bSYA_UseGrossDie;           // Set to true if Gross Die should be used (when available)
    int     iSYA_MinDataPoints;         // Minimum datapoints (wafers, lots if FT) to compute SYL/SBL
    // key is bin number (-1 = All Pass)
    // values are : <field , value> : (field = "RuleType", "N1", "N2", ...
    // example : mapBins_rules[1]=QMap<>{("RuleType", 1), ("N1", 6.0), ("N2", 4.0)}
    QMap<int, QMap<QString, QVariant> > mapBins_rules;

    void    clear(void);                    // Reset variables

    /*! \brief Update Private attributes :
           - update Private attributes from members
           - never in public slot
      */
    void UpdatePrivateAttributes();
};

#endif // YIELD_TASKDATA_H
