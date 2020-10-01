#ifndef GEX_PAT_PROCESSING_H
#define GEX_PAT_PROCESSING_H

#include "gqtl_utils.h"

#include <QObject>
#include <QDateTime>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QVariant>

// When to create a PAT Report in production
#define	GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM		0
#define	GEXMO_OUTLIER_REPORT_ALWAYS					1
#define	GEXMO_OUTLIER_REPORT_NEVER					2

// How to build the PAT report file name...
#define	GEXMO_OUTLIER_REPORT_NAME_TIMESTAMP			0	// PAT report name based on unique timestamp
#define	GEXMO_OUTLIER_REPORT_NAME_TRIGGERFILE		1	// PAT report name based on trigger file name.

class CWaferMap;

namespace GS
{
namespace Gex
{

class PATProcessing : public QObject
{
    Q_OBJECT

    static unsigned sNumOfInstances;

public slots:
    // return true if key ok and value set else false
    bool Set(const QString key, QVariant value);
    QVariant Get(const QString key);

    QString GetPatSummary   (const QString version);
    QString GetPatSummary   (const QString version, const QString outputFile);
    void    WritePatSummary (const QString& jsonString, const QString fileName);

public:

    static const QString sKeyComment;
    static const QString sKeyStatus;
    static const QString sKeyTotalGoodBeforePAT;

    enum eAxisDirections
    {
        ePositiveLeft,
        ePositiveRight,
        ePositiveUp,
        ePositiveDown,
        ePositiveForceLeft,
        ePositiveForceRight,
        ePositiveForceUp,
        ePositiveForceDown,
        eDefault
    };

    enum WaferOrientation
    {
        WOrientationDefault,
        WOrientationUp,
        WOrientationRight,
        WOrientationDown,
        WOrientationLeft,
        WOrientationForceUp,
        WOrientationForceRight,
        WOrientationForceDown,
        WOrientationForceLeft
    };

    enum MapBinType
    {
        MapDefaultBin,
        MapSoftBin,
        MapHardBin
    };

    enum PATYieldLimitSource
    {
        PYLFromUnknown = -1,
        PYLFromRecipe,
        PYLFromTrigger,
        PYLFromTask
    };

    //! \brief // Constructor for the structure holding all details about file to PAT process
    PATProcessing(QObject* parent=0);
    //! \brief Copy constructor
    PATProcessing(const PATProcessing&);

    //! \brief Clear structure
    Q_INVOKABLE void clear(void);
    //! \brief Register in the dyn properties this new key. Retuns false if key already registered.
    //! Keys are case insensitive. If Set is called on an unregistered key, it will return false.
    Q_INVOKABLE bool RegisterCustomVariable(const QString& lKeyName);

    //! \brief Get the current output wafermap
    Q_INVOKABLE CWaferMap* GetWaferMap(const QString& lType);

    virtual ~PATProcessing();	// Destructor

    // assignment '=' operator
    PATProcessing& operator=(const PATProcessing& source);

    QDateTime	dtProcessStartTime;// DateTime when Processing is started
    QDateTime	dtProcessEndTime; // DateTime when Processing is finished
    QString		strTriggerFile;		// Full path to trigger file processed.
    // Holds input test data files (if more than one, need to merge them)
    QStringList	strSources;
    QString		strOptionalSource;      // Optional input. E.g: KLA/INF
    QString		strOptionalOutput;      // If Optional input file updated, holds the new name used after update
    bool		bUpdateTimeFields;		// 'true' if overloads MIR Start & Setup time with current date.
    bool		bEmbedStdfReport;		// 'true' if embed PAT-Man HTML report in STDF output file.
    bool		bOutputMapIsSoftBin;	// 'true' if wafermap output is based on SOFT-BIN, 'false' if HARD-Bin wafermlap to create.
    bool		bLogDPAT_Details;		// 'true' if log file includes DPAT test details
    bool		bLogNNR_Details;		// 'true' if log file includes NNR test details
    bool		bLogIDDQ_Delta_Details;	// 'true' if log file includes IDDQ-Delta test details
    bool		bReportStdfOriginalLimits; // 'true' if report original test limits in STDF.PTR records
    int			iGenerateReport;		// On PatYielAlarm, or Always or Never.
    int			iReportNaming;			// How to name PAT report: based on timestamp (default), or over trigger file...
    bool		bRotateWafer;			// set to true if wafermap should be rotated to get the wafer flat into the position defined by the Notch field
    int			iGrossDiePerWafer;		// gross die count per wafer (-1 if not defined)
    int			iGoodMapOverload_MissingStdf; // if >0, holds binning to overload Map-Bin1 when STDF die is missing
    int			iGoodMapOverload_FailStdf; // if >0, holds binning to overload Map-Bin1 when STDF die is failing
    float		fValidWaferThreshold_GrossDieRatio;	// Gross Die Ratio that should be used to check if a wafer should be accepted
    bool		bDeleteIncompleteWafers;// Flag specifying if incomplete wafers should be deleted
    QString		strOutputReportMode;	// Report output creation mode 'always' or on 'alarm' only
    QString		strOutputReportFormat;	// Type of report to create (PDF, PPT, Word, ...)
    QString		strCustomerName;		// Customer name field to write in Wafermap file (Semi85)
    QString		strSupplierName;		// Supplier name field to write in Wafermap file (Semi85)
    bool		bHtmlEmail;				// true=HTML or false=Text
    QString		strEmailTitle;			// Email title
    QString		strEmailFrom;			// Email 'From' addresse
    QString		strEmailTo;				// Email 'To' addresses
    int			iNotificationType;		// 0= Report attached to email, 1= keep on server, only email its URL
    int			iExceptionLevel;		// 0=Standard, 1=Critical...
    QString		strRecipeFile;			// Full path to custom recipe file.
    QString		strRecipeOverloadFile;	// Full path to a recipe 'overload' which (if exists) can hold info such as PAT binning.
    QString		strCompositeFile;		// COMPOSITE file: holds exclusion zones, 3D neighborhood
    QString		strLogFilePath;			// LogFile path. If empty, disable LogFile generation.
    bool		bDeleteDataSource;		// 'true' if must delete test data files after sucecssful PAt processing.
    bool		bRetest_HighestBin;		// 'true' if retest policy is to promote highest bin instead of last bin
    QString		strPostProcessShell;	// Shell command to call once trigger file processed
    QString		strProcessPart;			// Type of parts to process in the report generation (NO effect on PAT limits computation)
    QString		strProcessList;			// List of parts (associated with above 'strProcessPart').

    QString     mReticleStepInfo;       // The location of the reticle file

    // Results - Filled at execution time
    QString		strWaferFileFullName;	// WaferMap output data file path+name (created at run time)

    // Yield Loss Alarm
    QString		mYieldThreshold;		// Yield level and yield threshold selected.
    bool		mPatAlarm;				// True if Yield loss exceeds threshold
    double		mYieldAlarmLoss;		// PAT Yield loss to trigger an alarm
    int			mAlarmType;				// 0=% of yield loss, 1=number PAT parts failures

    // Miscelaneous fields, not always used
    QString		strOperation;			// For now, only used for ST-PAT
    QString		strProd_RecipeName;		// For now, only used for ST-PAT, holds Production recipe name
    QString		strProd_RecipeVersion;	// For now, only used for ST-PAT, holds Production recipe version

    // Variables used when processing Trigger files.
    QFile		m_LogFile;
    QTextStream m_hLogFile;
    QString		m_strLogFileName;
    QString		m_strTemporaryLog;
    QString		m_strProductName;

    // Variables used for maps alignment
    QString                 mStdfRefLocation;
    WaferOrientation        mStdfOrientation;
    int                     mStdfXDirection;
    int                     mStdfYDirection;
    WaferOrientation        mExternalOrientation;
    QString                 mExternalRefLocation;
    int                     mExternalXDirection;
    int                     mExternalYDirection;
    int                     mExternalFailOverload;
    QString                 mCoordSystemAlignment;
    QString                 mGeometryCheck;
    GS::QtLib::Range        mAutoAlignBoundaryBins;
    QString                 mOnOppositeAxisDir;

    // Variables used for maps merge
    bool                    mMergeMaps;
    int                     mMergeBinGoodMapMissingSTDF;
    QString                 mMergeBinRuleJSHook;

    int         mOutputMapNotch;		// External map to create: notch position (12=up,3=right,6=down,9=left,-1=same as STDF file)
    int         mOutputMapXDirection;   // wafermap to create: positive X direction to force (left, right, unspecified)
    int			mOutputMapYDirection;			// wafermap to create: positive Y direction to force (up, down, unspecified)

    // Output STDF/ATDF
    QString		mOutputFolderSTDF;      // STDF output data file FOLDER path
    int			mOutputDatalogFormat;	// Defined the output datalog file to ctreate: none, STDF, ATDF
    bool        mOutputIncludeExtFails;

    QString     mProcessID;

    // Output format
    QMap<QString, QPair<MapBinType, QString> >  mOutputFormat;
};

}   // namespace Gex
}   // namespace GS

#endif // GEX_PAT_PROCESSING_H
