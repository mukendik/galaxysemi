#include "gex_pat_processing.h"
#include "pat_json_updater.h"
#include "pat_defines.h"
#include "pat_engine.h"
#include "pat_info.h"
#include "engine.h"

#include <gqtl_log.h>

namespace GS
{
namespace Gex
{

unsigned PATProcessing::sNumOfInstances=0;

const QString PATProcessing::sKeyComment="Comment";
const QString PATProcessing::sKeyStatus="Status";
const QString PATProcessing::sKeyTotalGoodBeforePAT="TotalGoodBeforePAT";

PATProcessing::PATProcessing(QObject* parent) : QObject(parent)
{
    sNumOfInstances++;

    setObjectName(QString("PatProcessing %1").arg(sNumOfInstances) );
    clear();
}

PATProcessing::PATProcessing(const PATProcessing& o)
    : QObject(o.parent())
{
    (*this)=o;
}

PATProcessing::~PATProcessing()
{
    sNumOfInstances--;
}

#define CHECK_FOR_SCRIPT_ALLOWED false

bool PATProcessing::RegisterCustomVariable(const QString& lKeyName)
{
    if (dynamicPropertyNames().contains(lKeyName.toLatin1()))
        return false;
    setProperty(lKeyName.toLatin1().data(), QVariant());
    return true;
}

void PATProcessing::clear(void)
{
    if (CHECK_FOR_SCRIPT_ALLOWED)
        return;

    dtProcessStartTime  = QDateTime();
    dtProcessEndTime    = QDateTime();

//    strTriggerFile.clear();
    strSources.clear();
    strOptionalSource.clear();
    strOptionalOutput.clear();
    mOutputFolderSTDF.clear();
    mOutputDatalogFormat    = GEX_TPAT_DLG_OUTPUT_NONE;
    mOutputIncludeExtFails  = false;
    bUpdateTimeFields       = false;	// Set to 'true' if want to update STDF MIR TimeStamp info with PAT processing date.

    bEmbedStdfReport = true;	// embed PAT-Man HTML report in the STDF file
    bOutputMapIsSoftBin = true;	// 'true' if wafermap output is based on SOFT-BIN, 'false' if HARD-Bin wafermlap to create.
    bLogDPAT_Details = false;
    bLogNNR_Details = false;
    bLogIDDQ_Delta_Details = false;
    bReportStdfOriginalLimits = false;	// 'true' if report Original test limits in PTR records.

    iGenerateReport = GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM;	// Production PAT processing will generate report only if Yield alarm detected...
    iReportNaming = GEXMO_OUTLIER_REPORT_NAME_TIMESTAMP;		// PAT report name based on unique timestamp

    bRotateWafer = true;			// Wafermap to create: wafermap rotation enabled
    iGrossDiePerWafer = -1;		// No GrossDiePerWfaer count defined
    iGoodMapOverload_MissingStdf = -1;
    iGoodMapOverload_FailStdf = -1;
    fValidWaferThreshold_GrossDieRatio = 0.0F;
    bDeleteIncompleteWafers = false;

    strOutputReportMode.clear();
    strOutputReportFormat.clear();
    strCustomerName.clear();
    strSupplierName.clear();
    bHtmlEmail = false;

    mYieldAlarmLoss=-1;		// PAT Yield loss to trigger an alarm
    mAlarmType=0;				// 0= Yield alarm is yield loss %, 1= yield alarm is on the number os PAT failing parts

    strEmailTitle.clear();
    strEmailFrom.clear();
    strEmailTo.clear();

    iNotificationType = 0;
    iExceptionLevel = 0;

    strRecipeFile.clear();
    strRecipeOverloadFile.clear();
    strCompositeFile.clear();
    strLogFilePath.clear();

    // No mirroring of the dies (default)
    bDeleteDataSource=false;	// 'true' if must delete test data files after sucecssful PAt processing.
    bRetest_HighestBin = false;		// 'true' if retest policy is to promote highest bin instead of last bin

    strPostProcessShell.clear();
    strProcessPart = "all";			// Type of parts to process in the report generation (NO effect on PAT limits computation)
    strProcessList = " ";			// List of parts (associated with above 'strProcessPart').

    strWaferFileFullName.clear();
    mYieldThreshold.clear();

    mPatAlarm = false;

    // Miscelaneous variables
    strOperation = "";				// Use for STM-PAT
    strProd_RecipeName = "";
    strProd_RecipeVersion = "";

    m_strLogFileName.clear();
    m_strTemporaryLog.clear();
    m_strProductName.clear();

    mStdfRefLocation.clear();
    mExternalRefLocation.clear();
    mCoordSystemAlignment.clear();
    mGeometryCheck.clear();
    mOnOppositeAxisDir.clear();
    mStdfOrientation            = WOrientationDefault;
    mStdfXDirection             = eDefault;
    mStdfYDirection             = eDefault;
    mExternalOrientation        = WOrientationDefault;
    mExternalXDirection         = eDefault;
    mExternalYDirection         = eDefault;
    mExternalFailOverload       = -1;
    mMergeMaps                  = false;
    mMergeBinGoodMapMissingSTDF = -1;

    mOutputMapNotch         = -1;
    mOutputMapXDirection    = eDefault;
    mOutputMapYDirection    = eDefault;

    mOutputFormat.clear();

    mProcessID.clear();
    mMergeBinRuleJSHook.clear();
    mReticleStepInfo = "";

    // Set dynamic properties allowed
    setProperty(sKeyTotalGoodBeforePAT.toLatin1().constData(), QVariant(0));
    setProperty("TotalGoodAfterPAT", QVariant(0));
    setProperty("TotalPATFails", QVariant(0));
    setProperty("PPATFails", QVariant(0));
    setProperty("MVPATFails", QVariant(0));
    setProperty("NNRPATFails", QVariant(0));
    setProperty("IDDQPATFails", QVariant(0));
    setProperty("GDBNPATFails", QVariant(0));
    setProperty("ClusteringPATFails", QVariant(0));
    setProperty("ZPATFails", QVariant(0));
    setProperty("ReticlePATFails", QVariant(0));
    setProperty(sKeyStatus.toLatin1().data(), QVariant(0)); // default : 'passed'
    setProperty(sKeyComment.toLatin1().data(), QVariant(""));

    setProperty("ReticleStepInfo", QVariant(mReticleStepInfo));

    // Name of the outlier removal task used
    setProperty(GS::PAT::Trigger::sPATOutlierRemovalTask, QVariant(""));
    // Source of the PAT Yield Limit (recipe, trigger, outlier removal task)
    setProperty(GS::PAT::Trigger::sPATYieldLimitSource,   QVariant(PYLFromUnknown));
}

bool PATProcessing::Set(const QString key, QVariant value)
{
    QString strParameter = value.toString().trimmed();
    bool bOk=false;

    if (key=="TriggerFile")
    {
        strTriggerFile=value.toString();
        return true;
    }

    if (key=="DataSource")
    {
        strSources.append(value.toString());
        return true;
    }

    if (key=="DataSources")
    {
        strSources=value.toStringList();
        return true;
    }

    // Define input optional data file
    if(key.compare("OptionalSource",Qt::CaseInsensitive) == 0)
    {
        strOptionalSource = strParameter;
        return true;
    }

    // Customer name field to write in Wafermap file (Semi85)
    if(key.compare("CustomerName",Qt::CaseInsensitive) == 0)
    {
        strCustomerName = strParameter;
        return true;
    }

    // Supplier name field to write in Wafermap file (Semi85)
    if(key.compare("SupplierName",Qt::CaseInsensitive) == 0)
    {
        strSupplierName = strParameter;
        return true;
    }

    if(key.compare("OutputMap",Qt::CaseInsensitive) == 0 ||
       key.compare("OutputMapBinType",Qt::CaseInsensitive) == 0)
    {
        if (value=="soft_bin")
            bOutputMapIsSoftBin = true;
        else
            bOutputMapIsSoftBin = false;
        return true;
    }

    // Log file: include DPAT test details?
    if(key.compare("LogDPAT_Details",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bLogDPAT_Details = true;
        else
            bLogDPAT_Details = false;
        return true;
    }

    if(key.compare("LogNNR_Details",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bLogNNR_Details = true;
        else
            bLogNNR_Details = false;
        return true;
    }

    // Log file: include IDDQ-Delta test details?
    if(key.compare("LogIDDQ_Delta_Details",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bLogIDDQ_Delta_Details = true;
        else
            bLogIDDQ_Delta_Details = false;
        return true;
    }

    if(key.compare("ReportStdfOriginalLimits",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
          bReportStdfOriginalLimits = true;	// Report PAT + Original test limits in PTR records.
        else
          bReportStdfOriginalLimits = false;	// Only report PAT limits
        return true;
    }

    //EmbedStdfReport
    if(key.compare("EmbedStdfReport",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bEmbedStdfReport = true;
        else
            bEmbedStdfReport = false;
        return true;
    }

    //GoodMapOverload_MissingStdf
    if(key.compare("GoodMapOverload_MissingStdf",Qt::CaseInsensitive) == 0)
    {
        if (strParameter.startsWith("0x", Qt::CaseInsensitive))
        {
            // remove "Ox" to convert the number into base 10 number
            strParameter.remove(0, 2);
            iGoodMapOverload_MissingStdf = strParameter.toInt(&bOk, 16);
            if(!bOk)
                iGoodMapOverload_MissingStdf = -1;

        }
        else
        {
            iGoodMapOverload_MissingStdf = strParameter.toInt(&bOk);

            if(!bOk)
                iGoodMapOverload_MissingStdf = -1;
        }

        return true;
    }

    //GoodMapOverload_FailStdf
    if(key.compare("GoodMapOverload_FailStdf",Qt::CaseInsensitive) == 0)
    {
        if (strParameter.startsWith("0x", Qt::CaseInsensitive))
        {
            // remove "Ox" to convert the number into base 10 number
            strParameter.remove(0, 2);
            iGoodMapOverload_FailStdf = strParameter.toInt(&bOk, 16);
            if(!bOk)
                iGoodMapOverload_FailStdf = -1;

        }
        else
        {
            iGoodMapOverload_FailStdf = strParameter.toInt(&bOk);

            if(!bOk)
                iGoodMapOverload_FailStdf = -1;
        }

        return true;
    }

    //UpdateTimeFields
    if(key.compare("UpdateTimeFields",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bUpdateTimeFields = true;
        else
            bUpdateTimeFields = false;
        return true;
    }

    //Shell
    if(key.compare("Shell",Qt::CaseInsensitive) == 0)
    {
        strPostProcessShell= strParameter;
        return true;
    }

    // OutputMapNotch
    if(key.compare("Notch",Qt::CaseInsensitive) == 0 ||
       key.compare("OutputMapNotch",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Up", Qt::CaseInsensitive))
            mOutputMapNotch = 12;	// Notch UP = '12 hours' direction
        else
        if(strParameter.startsWith("Right", Qt::CaseInsensitive))
            mOutputMapNotch = 3;	// Notch RIGHT = '3 hours' direction
        else
        if(strParameter.startsWith("Down", Qt::CaseInsensitive))
            mOutputMapNotch = 6;	// Notch DOWN = '6 hours' direction
        else
        if(strParameter.startsWith("Left", Qt::CaseInsensitive))
            mOutputMapNotch = 9;	// Notch LEFT = '9 hours' direction
        else
            mOutputMapNotch = -1;// Leave Notch to default direction
        return true;
    }

    //  Define inkless wafermap positive X direction
    if(key.compare("OutputMapPosX",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.compare("Right", Qt::CaseInsensitive) == 0)
            // Positive X towards the right, except if one specified in STDF
            mOutputMapXDirection = PATProcessing::ePositiveRight;
        else if(strParameter.compare("Left", Qt::CaseInsensitive) == 0)
            // Positive X towards the left, except if one specified in STDF
            mOutputMapXDirection = PATProcessing::ePositiveLeft;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }
        return true;
    }

    //  Define inkless wafermap positive Y direction
    if(key.compare("OutputMapPoxY",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.compare("Up", Qt::CaseInsensitive) == 0)
            // Positive Y towards the up, except if one specified in STDF
            mOutputMapYDirection = PATProcessing::ePositiveUp;
        else if(strParameter.compare("Down", Qt::CaseInsensitive) == 0)
            // Positive Y towards the down, except if one specified in STDF
            mOutputMapYDirection = PATProcessing::ePositiveDown;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }

        return true;
    }

    //  Define inkless wafermap rotation flag
    if(key.compare("RotateWafer",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Disabled", Qt::CaseInsensitive))
            bRotateWafer = false;
        return true;
    }

    if(key.compare("PosX",Qt::CaseInsensitive) == 0 ||
       key.compare("STDFPosX",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Right", Qt::CaseInsensitive))
            // Positive X towards the right, except if one specified in STDF
            mStdfXDirection = PATProcessing::ePositiveRight;
        else if(strParameter.startsWith("Left", Qt::CaseInsensitive))
            // Positive X towards the left, except if one specified in STDF
            mStdfXDirection = PATProcessing::ePositiveLeft;
        else if(strParameter.startsWith("Force_Right", Qt::CaseInsensitive))
            // Positive X towards the right, whatever in STDF
            mStdfXDirection = PATProcessing::ePositiveForceRight;
        else if(strParameter.startsWith("Force_Left", Qt::CaseInsensitive))
            // Positive X towards the left, whatever in STDF
            mStdfXDirection = PATProcessing::ePositiveForceLeft;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }
        return true;
    }

    //  Define inkless wafermap positive Y direction
    if(key.compare("PosY",Qt::CaseInsensitive) == 0 ||
       key.compare("STDFPosY",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Up", Qt::CaseInsensitive))
            // Positive Y towards the up, except if one specified in STDF
            mStdfYDirection = PATProcessing::ePositiveUp;
        else if(strParameter.startsWith("Down", Qt::CaseInsensitive))
            // Positive Y towards the down, except if one specified in STDF
            mStdfYDirection = PATProcessing::ePositiveDown;
        else if(strParameter.startsWith("Force_Up", Qt::CaseInsensitive))
            // Positive Y towards the up, whatever in STDF
            mStdfYDirection = PATProcessing::ePositiveForceUp;
        else if(strParameter.startsWith("Force_Down", Qt::CaseInsensitive))
            // Positive Y towards the down, whatever in STDF
            mStdfYDirection = PATProcessing::ePositiveForceDown;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }

        return true;
    }

    if(key.compare("GrossDie",Qt::CaseInsensitive) == 0)
    {
        iGrossDiePerWafer = strParameter.toInt();
        return true;
    }

    if(key.compare("PatAlarmLevel",Qt::CaseInsensitive) == 0)
    {
        // Check if parameter is pointing to a file...
        QFile	cFile(strParameter);
        mYieldAlarmLoss = -1;
        if(cFile.exists())
        {
            // Read file contents and look for a yield threshold
            if (cFile.open(QIODevice::ReadOnly) == false)
                return false;	// Failed to open limit file

            QTextStream hLimitFile(&cFile);
            // Check if valid header...or empty!
            strParameter = hLimitFile.readLine();
        }
        // Not a file...extract yield threshold value.
        strParameter = strParameter.replace("%"," ").trimmed();
        mYieldAlarmLoss = strParameter.toFloat();
        return true;
    }

    // Gross Die Ratio that should be used to check if a wafer should be accepted
    if(key.compare("ValidWaferThreshold_GrossDieRatio",Qt::CaseInsensitive) == 0)
    {
        fValidWaferThreshold_GrossDieRatio = strParameter.toFloat();
        return true;
    }

    if(key.compare("DeleteIncompleteWafers",Qt::CaseInsensitive) == 0)
    {
        if (value.toString().startsWith("yes") || value==1)
            bDeleteIncompleteWafers = true;
        else
            bDeleteIncompleteWafers = false;
        return true;
    }

    if (key.compare("Output",Qt::CaseInsensitive)==0)
    {
        // Where to create the file. Remove extra space at start and end,
        // replaced sequence of '\t', '\n', '\v', '\f', '\r' and ' ' with a single space
        strParameter = strParameter.simplified().trimmed();

        // Output format e.g.: 'STDF', 'TSMC' 'G85' ...
        QString strFormat       = strParameter.section(' ',0,0);
        QString strPath         = strParameter.section(' ', 1, 1);
        QString lParameter      = strParameter.section(' ', 2, 2);

        if(strFormat.indexOf("STDF",0,Qt::CaseInsensitive) >= 0)
        {
            mOutputDatalogFormat  = GEX_TPAT_DLG_OUTPUT_STDF;
            mOutputFolderSTDF     = strPath;   // Path where the STDF file has to be created

            if (lParameter.compare("IncludeExternalFail", Qt::CaseInsensitive) == 0)
                mOutputIncludeExtFails = true;
            else
                mOutputIncludeExtFails = false;
        }
        else if(strFormat.indexOf("ATDF",0,Qt::CaseInsensitive) >= 0)
        {
            mOutputDatalogFormat    = GEX_TPAT_DLG_OUTPUT_ATDF;
            mOutputFolderSTDF       = strPath;	// Path where the STDF file has to be created

            if (lParameter.compare("IncludeExternalFail", Qt::CaseInsensitive) == 0)
                mOutputIncludeExtFails = true;
            else
                mOutputIncludeExtFails = false;
        }
        else
        {
            QPair<MapBinType, QString> lMapInfo(MapDefaultBin, strPath);

            if (lParameter.compare("soft_bin", Qt::CaseInsensitive) == 0)
                lMapInfo.first  = MapSoftBin;
            else if (lParameter.compare("hard_bin", Qt::CaseInsensitive) == 0)
                lMapInfo.second = MapHardBin;
            else if (lParameter.isEmpty() == false)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Invalid bin type (%1) for output map %2").arg(lParameter).arg(strFormat)
                      .toLatin1().constData());

                return false;
            }

            mOutputFormat.insert(strFormat, lMapInfo);
        }

        return true;
    }

    if(key.compare("ReportMode",Qt::CaseInsensitive) == 0)
    {
        // E.g: 'YieldAlarm', 'Always' 'Never' ...
        if(strParameter.startsWith("PatYieldAlarm", Qt::CaseInsensitive))
        {
            // PAT report generated if: PAT Yield loss too high (default)
            iGenerateReport = GEXMO_OUTLIER_REPORT_ON_PAT_YIELD_ALARM;
        }
        else
        if(strParameter.startsWith("Always", Qt::CaseInsensitive))
        {
            // PAT report ALWAYS generated
            iGenerateReport = GEXMO_OUTLIER_REPORT_ALWAYS;
        }
        else
        if(strParameter.startsWith("Never", Qt::CaseInsensitive))
        {
            // PAT report NEVER generated
            iGenerateReport = GEXMO_OUTLIER_REPORT_NEVER;
        }
        return true;
    }

    if(key.compare("ReportName",Qt::CaseInsensitive) == 0)
    {
        // E.g: 'YieldAlarm', 'Always' 'Never' ...
        if(strParameter.startsWith("TimeStamp", Qt::CaseInsensitive))
        {
            // PAT report name based on unique timestamp
            iReportNaming = GEXMO_OUTLIER_REPORT_NAME_TIMESTAMP;
        }
        else
        if(strParameter.startsWith("TriggerName", Qt::CaseInsensitive))
        {
            // PAT report name based on trigger file name.
            iReportNaming = GEXMO_OUTLIER_REPORT_NAME_TRIGGERFILE;
        }
        return true;
    }

    if(key.compare("Recipe",Qt::CaseInsensitive) == 0)
    {
        strRecipeFile = strParameter;
        return true;
    }

    if(key.compare("RecipeOverload",Qt::CaseInsensitive) == 0)
    {
        strRecipeOverloadFile = strParameter;
        return true;
    }

    // Select COMPOSITE file: holds exclusion zones, 3D neighborhood
    if(key.compare("CompositeFile",Qt::CaseInsensitive) == 0)
    {
        strCompositeFile = strParameter;
        return true;
    }

    // Select Reticle Location file
    if(key.compare("ReticleStepInfo",Qt::CaseInsensitive) == 0)
    {
        mReticleStepInfo = strParameter;
        return true;
    }

    if(key.compare("LogFile",Qt::CaseInsensitive) == 0)
    {
        strLogFilePath= strParameter;
        return true;
    }

    // Delete source test data files after successful PAT processing.
    if(key.compare("DeleteDataSource",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("yes", Qt::CaseInsensitive))
            bDeleteDataSource = true;
        else
            bDeleteDataSource = false;
        return true;
    }

    if (key.compare("RetestPolicy", Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("highest_die", Qt::CaseInsensitive))
            bRetest_HighestBin = true;
        else
            bRetest_HighestBin = false;
        return true;
    }

    // Overload MIR product name.
    if(key.compare("Product",Qt::CaseInsensitive) == 0)
    {
        m_strProductName= strParameter;
        return true;
    }

    if(key.compare("STDFRefLocation", Qt::CaseInsensitive) == 0)
    {
        mStdfRefLocation = strParameter;
        return true;
    }

    if(key.compare("STDFNotch", Qt::CaseInsensitive) == 0)
    {
        if(strParameter.compare("Up", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationUp;
        else if(strParameter.compare("Right", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationRight;
        else if(strParameter.compare("Down", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationDown;
        else if(strParameter.compare("Left", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationLeft;
        else if(strParameter.compare("ForceUp", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationForceUp;
        else if(strParameter.compare("ForceRight", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationForceRight;
        else if(strParameter.compare("ForceDown", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationForceDown;
        else if(strParameter.compare("ForceLeft", Qt::CaseInsensitive) == 0)
            mStdfOrientation = WOrientationForceLeft;
        else
            mStdfOrientation = WOrientationDefault;

        return true;
    }

    if(key.compare("ExternalNotch", Qt::CaseInsensitive) == 0)
    {
        if(strParameter.compare("Up", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationUp;
        else if(strParameter.compare("Right", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationRight;
        else if(strParameter.compare("Down", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationDown;
        else if(strParameter.compare("Left", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationLeft;
        else if(strParameter.compare("ForceUp", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationForceUp;
        else if(strParameter.compare("ForceRight", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationForceRight;
        else if(strParameter.compare("ForceDown", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationForceDown;
        else if(strParameter.compare("ForceLeft", Qt::CaseInsensitive) == 0)
            mExternalOrientation = WOrientationForceLeft;
        else
            mExternalOrientation = WOrientationDefault;

        return true;
    }

    if(key.compare("ExternalPosX",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Right", Qt::CaseInsensitive))
            // Positive X towards the right, except if one specified in STDF
            mExternalXDirection = PATProcessing::ePositiveRight;
        else if(strParameter.startsWith("Left", Qt::CaseInsensitive))
            // Positive X towards the left, except if one specified in STDF
            mExternalXDirection = PATProcessing::ePositiveLeft;
        else if(strParameter.startsWith("Force_Right", Qt::CaseInsensitive))
            // Positive X towards the right, whatever in STDF
            mExternalXDirection = PATProcessing::ePositiveForceRight;
        else if(strParameter.startsWith("Force_Left", Qt::CaseInsensitive))
            // Positive X towards the left, whatever in STDF
            mExternalXDirection = PATProcessing::ePositiveForceLeft;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }

        return true;
    }

    //  Define inkless wafermap positive Y direction
    if(key.compare("ExternalPosY",Qt::CaseInsensitive) == 0)
    {
        if(strParameter.startsWith("Up", Qt::CaseInsensitive))
            // Positive Y towards the up, except if one specified in STDF
            mExternalYDirection = PATProcessing::ePositiveUp;
        else if(strParameter.startsWith("Down", Qt::CaseInsensitive))
            // Positive Y towards the down, except if one specified in STDF
            mExternalYDirection = PATProcessing::ePositiveDown;
        else if(strParameter.startsWith("Force_Up", Qt::CaseInsensitive))
            // Positive Y towards the up, whatever in STDF
            mExternalYDirection = PATProcessing::ePositiveForceUp;
        else if(strParameter.startsWith("Force_Down", Qt::CaseInsensitive))
            // Positive Y towards the down, whatever in STDF
            mExternalYDirection = PATProcessing::ePositiveForceDown;
        else
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());

            return false;
        }

        return true;
    }

    if(key.compare("ExternalRefLocation", Qt::CaseInsensitive) == 0)
    {
        mExternalRefLocation = strParameter;
        return true;
    }

    if(key.compare("ExternalFailOverload", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.startsWith("0x", Qt::CaseInsensitive))
        {
            // remove "Ox" to convert the number into base 10 number
            strParameter.remove(0, 2);
            mExternalFailOverload = strParameter.toInt(&bOk, 16);
            if(!bOk)
                mExternalFailOverload = -1;

        }
        else
        {
            mExternalFailOverload = strParameter.toInt(&bOk);

            if(!bOk)
                mExternalFailOverload = -1;
        }

        return true;
    }

    if(key.compare("CoordSystemAlignment", Qt::CaseInsensitive) == 0)
    {
        mCoordSystemAlignment = strParameter;
        return true;
    }

    if(key.compare("GeometryCheck", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.compare("STDFSmaller", Qt::CaseInsensitive) != 0 &&
            strParameter.compare("Disable", Qt::CaseInsensitive) != 0)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Invalid value %1 for GTF key %2").arg(strParameter).arg(key)
                  .toLatin1().constData());
            return false;
        }

        mGeometryCheck = strParameter;
        return true;
    }

    if(key.compare("MergeMaps", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.compare("true", Qt::CaseInsensitive) == 0)
            mMergeMaps = true;
        else
            mMergeMaps = false;

        return true;
    }

    if (key.compare("MergeBin_GoodMap_MissingSTDF", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.startsWith("0x", Qt::CaseInsensitive))
        {
            // remove "Ox" to convert the number into base 10 number
            strParameter.remove(0, 2);
            mMergeBinGoodMapMissingSTDF = strParameter.toInt(&bOk, 16);
            if(!bOk)
                mMergeBinGoodMapMissingSTDF = -1;

        }
        else
        {
            mMergeBinGoodMapMissingSTDF = strParameter.toInt(&bOk);

            if(!bOk)
                mMergeBinGoodMapMissingSTDF = -1;
        }

        return true;
    }

    if (key.compare("AutoAlignBoundaryBins", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.isEmpty() == false)
            mAutoAlignBoundaryBins.SetRange(strParameter);
        else
            mAutoAlignBoundaryBins.Clear();

        return true;
    }

    if (key.compare("ReticleStepInfo", Qt::CaseInsensitive) == 0)
    {
        if (strParameter.isEmpty() == false)
            mReticleStepInfo = strParameter;
        else
            GSLOG(SYSLOG_SEV_WARNING, "Empty reticle location file");

        return true;
    }

    if (key.compare(GS::PAT::Trigger::sMergeBinRuleJSHook, Qt::CaseInsensitive) == 0)
    {
        mMergeBinRuleJSHook = strParameter;
        return true;
    }

    if (key.compare(GS::PAT::Trigger::sProcessID, Qt::CaseInsensitive) == 0)
    {
        mProcessID = strParameter;
        return true;
    }

    if (key.compare(GS::PAT::Trigger::sOnOppositeAxisDir, Qt::CaseInsensitive) == 0)
    {
        if (strParameter.isEmpty() == false)
            mOnOppositeAxisDir = strParameter;
        else
            GSLOG(SYSLOG_SEV_WARNING, QString("Empty key %1").arg(key).toLatin1().constData());
        return true;
    }

    // check if key is an allowed property
    //if (dynamicPropertyNames().contains(key.toLatin1()))
    {
        // modif to allow to set dynamic keys
        setProperty(key.toLatin1().constData(), value);
        return true;
    }

    return false;
}

QVariant PATProcessing::Get(const QString key)
{
    if (key.compare("DataSource",Qt::CaseInsensitive)==0 )
        return strSources;
    if (key.compare("DataSources",Qt::CaseInsensitive)==0 )
        return strSources;
    if (key.compare("OptionalSource",Qt::CaseInsensitive)==0 )
        return strOptionalSource;
    if (key.compare("CustomerName",Qt::CaseInsensitive)==0 )
        return strCustomerName;
    if (key.compare("SupplierName",Qt::CaseInsensitive)==0 )
        return strSupplierName;
    if (key.compare("OutputMap",Qt::CaseInsensitive)==0 ||
        key.compare("OutputMapBinType",Qt::CaseInsensitive)==0)
        return bOutputMapIsSoftBin;
    if(key.compare("LogDPAT_Details",Qt::CaseInsensitive) == 0)
        return bLogDPAT_Details;
    if(key.compare("LogNNR_Details",Qt::CaseInsensitive) == 0)
        return bLogNNR_Details;
    if(key.compare("LogIDDQ_Delta_Details",Qt::CaseInsensitive) == 0)
        return bLogIDDQ_Delta_Details;
    if(key.compare("ReportStdfOriginalLimits",Qt::CaseInsensitive) == 0)
        return bReportStdfOriginalLimits;
    if(key.compare("EmbedStdfReport",Qt::CaseInsensitive) == 0)
        return bEmbedStdfReport;
    if(key.compare("GoodMapOverload_MissingStdf",Qt::CaseInsensitive) == 0)
        return iGoodMapOverload_MissingStdf;
    if(key.compare("GoodMapOverload_FailStdf",Qt::CaseInsensitive) == 0)
        return iGoodMapOverload_FailStdf;
    if(key.compare("UpdateTimeFields",Qt::CaseInsensitive) == 0)
        return bUpdateTimeFields;
    if(key.compare("Shell",Qt::CaseInsensitive) == 0)
        return strPostProcessShell;
    if(key.compare("Notch",Qt::CaseInsensitive) == 0 ||
       key.compare("OutputMapNotch",Qt::CaseInsensitive) == 0)
        return mOutputMapNotch;
    if(key.compare("OutputMapPosX",Qt::CaseInsensitive) == 0)
        return mOutputMapXDirection;
    if(key.compare("OutputMapPosY",Qt::CaseInsensitive) == 0)
        return mOutputMapYDirection;
    if(key.compare("RotateWafer",Qt::CaseInsensitive) == 0)
        return bRotateWafer;
    if(key.compare("PosX",Qt::CaseInsensitive) == 0 ||
       key.compare("STDFPosX",Qt::CaseInsensitive) == 0)
        return mStdfXDirection;
    if(key.compare("PosY",Qt::CaseInsensitive) == 0 ||
       key.compare("STDFPosY",Qt::CaseInsensitive) == 0)
        return mStdfYDirection;
    if(key.compare("GrossDie",Qt::CaseInsensitive) == 0)
        return iGrossDiePerWafer;
    if(key.compare("PatAlarmLevel",Qt::CaseInsensitive) == 0)
        return mYieldAlarmLoss;
    if(key.compare("ValidWaferThreshold_GrossDieRatio",Qt::CaseInsensitive) == 0)
        return fValidWaferThreshold_GrossDieRatio;
    if(key.compare("DeleteIncompleteWafers",Qt::CaseInsensitive) == 0)
        return bDeleteIncompleteWafers;
    if (key.compare("Output",Qt::CaseInsensitive)==0)
    {
        QString lResult = QString::number(mOutputDatalogFormat) + " " + mOutputFolderSTDF;

        if (mOutputIncludeExtFails)
            lResult += " IncludeExternalFail";

        return lResult;
    }
    if(key.compare("ReportMode",Qt::CaseInsensitive) == 0)
        return iGenerateReport;
    if(key.compare("ReportName",Qt::CaseInsensitive) == 0)
        return iReportNaming;
    if(key.compare("Recipe",Qt::CaseInsensitive) == 0)
        return strRecipeFile;
    if(key.compare("RecipeOverload",Qt::CaseInsensitive) == 0)
        return strRecipeOverloadFile;
    if(key.compare("CompositeFile",Qt::CaseInsensitive) == 0)
        return strCompositeFile;
    if (key.compare("ReticleStepInfo", Qt::CaseInsensitive) == 0)
        return mReticleStepInfo;
    if(key.compare("LogFile",Qt::CaseInsensitive) == 0)
        return strLogFilePath;
    if(key.compare("DeleteDataSource",Qt::CaseInsensitive) == 0)
        return bDeleteDataSource;
    if (key=="RetestPolicy")
    {
        if (bRetest_HighestBin)
            return QVariant("highest_die");
        else
            return QVariant("");
    }
    if(key.compare("Product",Qt::CaseInsensitive) == 0)
        return m_strProductName;
    if(key.compare("STDFRefLocation", Qt::CaseInsensitive) == 0)
        return mStdfRefLocation;
    if(key.compare("STDFNotch", Qt::CaseInsensitive) == 0)
        return mStdfOrientation;
    if(key.compare("ExternalRefLocation", Qt::CaseInsensitive) == 0)
        return mExternalRefLocation;
    if(key.compare("ExternalNotch", Qt::CaseInsensitive) == 0)
        return mExternalOrientation;
    if(key.compare("ExternalPosX", Qt::CaseInsensitive) == 0)
        return mExternalXDirection;
    if(key.compare("ExternalPosY", Qt::CaseInsensitive) == 0)
        return mExternalYDirection;
    if(key.compare("ExternalFailOverload", Qt::CaseInsensitive) == 0)
        return mExternalFailOverload;
    if(key.compare("CoordSystemAlignment", Qt::CaseInsensitive) == 0)
        return mCoordSystemAlignment;
    if(key.compare("GeometryCheck", Qt::CaseInsensitive) == 0)
        return mGeometryCheck;
    if(key.compare("MergeMaps", Qt::CaseInsensitive) == 0)
    {
        if (mMergeMaps)
            return QVariant("true");
        else
            return QVariant("false");
    }
    if(key.compare("MergeBin_GoodMap_MissingSTDF", Qt::CaseInsensitive) == 0)
        return mMergeBinGoodMapMissingSTDF;
    if(key.compare("AutoAlignBoundaryBins", Qt::CaseInsensitive) == 0)
        return mAutoAlignBoundaryBins.GetRangeList();
    if (key.compare(GS::PAT::Trigger::sMergeBinRuleJSHook, Qt::CaseInsensitive) == 0)
        return mMergeBinRuleJSHook;
    if (key.compare(GS::PAT::Trigger::sProcessID, Qt::CaseInsensitive) == 0)
        return mProcessID;
    if (key.compare(GS::PAT::Trigger::sOnOppositeAxisDir, Qt::CaseInsensitive) == 0)
        return mOnOppositeAxisDir;

    if (dynamicPropertyNames().contains(key.toLatin1()))
        return property(key.toLatin1().constData());

    return QVariant(); // invalid
}

QString PATProcessing::GetPatSummary(const QString version)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    if(lPatInfo == 0)
        return "";

    QJsonDocument               lJjsonDocument;
    GS::PAT::PatJsonUpdater     lPatJsonUpdater;
    lJjsonDocument = lPatJsonUpdater.buildJsonReport( GS::Gex::PATEngine::GetInstance().GetContext(), version);

    return lJjsonDocument.toJson();
}

QString PATProcessing::GetPatSummary(const QString version, const QString outputFile)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

    if(lPatInfo == 0)
        return "";

    QJsonDocument           lJjsonDocument;
    GS::PAT::PatJsonUpdater lPatJsonUpdater;
    lJjsonDocument = lPatJsonUpdater.buildJsonReport(lPatInfo , version);

    QFile lFile(outputFile);
    lFile.open(QIODevice::WriteOnly | QIODevice::Text);
    lFile.write(lJjsonDocument.toJson());
    lFile.close();

    return lJjsonDocument.toJson();
}

void PATProcessing::WritePatSummary(const QString& jsonString, const QString fileName)
{
    QFile lFile(fileName);
    lFile.open(QIODevice::WriteOnly | QIODevice::Text);
    lFile.write(jsonString.toLatin1());
    lFile.close();
}

CWaferMap* PATProcessing::GetWaferMap(const QString& lType)
{
    CPatInfo* lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();
    if (lPatInfo==0)
        return NULL;

    if (lType=="soft")
        return &lPatInfo->m_AllSitesMap_Sbin; ////m_Stdf_HbinMap;
    if (lType=="hard")
        return &lPatInfo->m_AllSitesMap_Hbin;
    if (lType=="prober")
        return &lPatInfo->m_ProberMap;
    if (lType=="inputsoft")
        return &lPatInfo->m_Stdf_SbinMap;
    if (lType=="inputhard")
        return &lPatInfo->m_Stdf_HbinMap;
    return NULL;
}

///////////////////////////////////////////////////////////
// '=' operator
///////////////////////////////////////////////////////////
PATProcessing& PATProcessing::operator=(const PATProcessing& cTriggerFields)
{
    if (this != &cTriggerFields)
    {
        dtProcessStartTime      = cTriggerFields.dtProcessStartTime;
        dtProcessEndTime        = cTriggerFields.dtProcessEndTime;
        strTriggerFile          = cTriggerFields.strTriggerFile;
        strSources              = cTriggerFields.strSources;
        strOptionalSource       = cTriggerFields.strOptionalSource;
        strOptionalOutput       = cTriggerFields.strOptionalOutput;
        mOutputFolderSTDF       = cTriggerFields.mOutputFolderSTDF;
        mOutputDatalogFormat    = cTriggerFields.mOutputDatalogFormat;
        mOutputIncludeExtFails  = cTriggerFields.mOutputIncludeExtFails;
        iGenerateReport         = cTriggerFields.iGenerateReport;
        iReportNaming           = cTriggerFields.iReportNaming;
        bUpdateTimeFields       = cTriggerFields.bUpdateTimeFields;
        bEmbedStdfReport        = cTriggerFields.bEmbedStdfReport;
        bOutputMapIsSoftBin     = cTriggerFields.bOutputMapIsSoftBin;	// 'true' if wafermap output is based on SOFT-BIN, 'false' if HARD-Bin wafermlap to create.
        bLogDPAT_Details        = cTriggerFields.bLogDPAT_Details;
        bLogNNR_Details         = cTriggerFields.bLogNNR_Details;
        bLogIDDQ_Delta_Details  = cTriggerFields.bLogIDDQ_Delta_Details;
        bReportStdfOriginalLimits = cTriggerFields.bReportStdfOriginalLimits; // 'true' if report original test limits in PTR records.
        iGrossDiePerWafer = cTriggerFields.iGrossDiePerWafer;
        fValidWaferThreshold_GrossDieRatio = cTriggerFields.fValidWaferThreshold_GrossDieRatio;
        bDeleteIncompleteWafers = cTriggerFields.bDeleteIncompleteWafers;
        iGoodMapOverload_MissingStdf = cTriggerFields.iGoodMapOverload_MissingStdf;
        iGoodMapOverload_FailStdf = cTriggerFields.iGoodMapOverload_FailStdf;
        bRotateWafer = cTriggerFields.bRotateWafer;
        strOutputReportMode = cTriggerFields.strOutputReportMode;
        strOutputReportFormat = cTriggerFields.strOutputReportFormat;
        strCustomerName = cTriggerFields.strCustomerName;
        strSupplierName = cTriggerFields.strSupplierName;
        mYieldAlarmLoss = cTriggerFields.mYieldAlarmLoss;
        mAlarmType = cTriggerFields.mAlarmType;
        bHtmlEmail = cTriggerFields.bHtmlEmail;
        strEmailTitle = cTriggerFields.strEmailTitle;
        strEmailFrom = cTriggerFields.strEmailFrom;
        strEmailTo = cTriggerFields.strEmailTo;
        iNotificationType = cTriggerFields.iNotificationType;
        strRecipeFile = cTriggerFields.strRecipeFile;
        strRecipeOverloadFile = cTriggerFields.strRecipeOverloadFile;
        strLogFilePath = cTriggerFields.strLogFilePath;
        strCompositeFile = cTriggerFields.strCompositeFile;
        mReticleStepInfo = cTriggerFields.mReticleStepInfo;
        bDeleteDataSource = cTriggerFields.bDeleteDataSource;
        bRetest_HighestBin = cTriggerFields.bRetest_HighestBin;
        strWaferFileFullName = cTriggerFields.strWaferFileFullName;
        mYieldThreshold = cTriggerFields.mYieldThreshold;
        mPatAlarm = cTriggerFields.mPatAlarm;
        m_strLogFileName = cTriggerFields.m_strLogFileName;
        m_strTemporaryLog = cTriggerFields.m_strTemporaryLog;
        strPostProcessShell = cTriggerFields.strPostProcessShell;
        m_strProductName = cTriggerFields.m_strProductName;

        strOperation =  cTriggerFields.strOperation;				// Use for STM-PAT
        strProd_RecipeName = cTriggerFields.strProd_RecipeName;
        strProd_RecipeVersion = cTriggerFields.strProd_RecipeVersion;

        mStdfRefLocation            = cTriggerFields.mStdfRefLocation;
        mStdfOrientation            = cTriggerFields.mStdfOrientation;
        mStdfXDirection             = cTriggerFields.mStdfXDirection;
        mStdfYDirection             = cTriggerFields.mStdfYDirection;
        mExternalOrientation        = cTriggerFields.mExternalOrientation;
        mExternalXDirection         = cTriggerFields.mExternalXDirection;
        mExternalYDirection         = cTriggerFields.mExternalYDirection;
        mExternalRefLocation        = cTriggerFields.mExternalRefLocation;
        mExternalFailOverload       = cTriggerFields.mExternalFailOverload;
        mCoordSystemAlignment       = cTriggerFields.mCoordSystemAlignment;
        mGeometryCheck              = cTriggerFields.mGeometryCheck;
        mMergeMaps                  = cTriggerFields.mMergeMaps;
        mMergeBinGoodMapMissingSTDF = cTriggerFields.mMergeBinGoodMapMissingSTDF;
        mMergeBinRuleJSHook         = cTriggerFields.mMergeBinRuleJSHook;
        mAutoAlignBoundaryBins      = cTriggerFields.mAutoAlignBoundaryBins;
        mOnOppositeAxisDir     = cTriggerFields.mOnOppositeAxisDir;

        mOutputMapNotch             = cTriggerFields.mOutputMapNotch;
        mOutputMapXDirection        = cTriggerFields.mOutputMapXDirection;
        mOutputMapYDirection        = cTriggerFields.mOutputMapYDirection;

        mOutputFormat               = cTriggerFields.mOutputFormat;

        // Properties are belongs to QObject, and need to be explicitly copied as
        // Qobject cannot be copied
        QList<QByteArray> lDynProperties = dynamicPropertyNames();

        for (int lIdx = 0; lIdx < lDynProperties.count(); ++lIdx)
            setProperty(lDynProperties.at(lIdx).constData(), property(lDynProperties.at(lIdx).constData()));
    }

    return *this;
}

}   // namespace Gex
}   // namespace GS
