#ifdef GCORE15334
#include "cbinning.h"
#include "stdf.h"
#include "stdf_head_and_site_number_decipher.h"
#include "engine.h"
#include "gex_pat_processing.h"
#include "gex_report.h"
#include "gqtl_log.h"
#include "pat_outlier_finder_ws.h"
#include "pat_engine.h"
#include "pat_definition.h"
#include "pat_global.h"
#include "pat_stdf_updater.h"
#include "read_system_info.h"
#include "stdfrecords_v4.h"
#include "gex_pat_constants_extern.h"
#include "pat_part_filter.h"
#include <QApplication>

#define STDF_CN_STRING_MAX		255

#define BINTYPE_SOFT            0
#define BINTYPE_HARD            1

extern CGexReport *     gexReport;
extern CReportOptions	ReportOptions;

extern double       ScalingPower(int iPower);

#include <QHash>


namespace GS
{
namespace Gex
{

class PATStdfUpdatePrivate
{
public:

    PATStdfUpdatePrivate();
    ~PATStdfUpdatePrivate();

    bool                mBRCreated;
    bool                mMRRFound;
    bool                mPCRCreated;
    bool                mWCRFound;
    bool                mExtFailuresAdded;
    int                 mPassCount;
    int                 mPassCurrent;
    BYTE                mTesterHead;
    long                mWaferStartT;
    long                mWaferEndT;
    long                mRunID;
    long                mCurrentInternalPartID;
    QMap<QString, long>	mMapPartID;
    QString             mErrorMessage;
    QString             mInputFileName;
    QString             mOutputFileName;
    QString             mSTDFCompliancy;
    GS::StdLib::Stdf	mInputFile;
    GS::StdLib::Stdf	mOutputFile;
    PATOutlierFinderWS  mEngine;
    PATProcessing       mSettings;
    CPatInfo *          mContext;
    CGexFileInGroup *   mFile;
    QMap<unsigned int, CPatSite *>  mPATSites;
    QMap<int,int>                   mTestPerSite;
    QHash<QPair<unsigned long,unsigned char>, GQTL_STDF::Stdf_MPR_V4> mMPRFirstOccurence;

    bool                IsLastPass() const;
};

PATStdfUpdatePrivate::PATStdfUpdatePrivate()
    : mBRCreated(false), mMRRFound(false), mPCRCreated(false), mWCRFound(false), mExtFailuresAdded(false),
      mPassCount(1), mPassCurrent(0), mTesterHead(0), mWaferStartT(0), mWaferEndT(0),
      mRunID(0), mCurrentInternalPartID(0), mEngine(PATEngine::GetInstance().GetContext()),
      mContext(PATEngine::GetInstance().GetContext()), mFile(NULL)
{
    mSTDFCompliancy = ReportOptions.GetOption("dataprocessing", "stdf_compliancy").toString();
}

PATStdfUpdatePrivate::~PATStdfUpdatePrivate()
{
    qDeleteAll(mPATSites.values());
    mPATSites.clear();
}

bool PATStdfUpdatePrivate::IsLastPass() const
{
    return mPassCurrent == mPassCount;
}

PATStdfUpdate::PATStdfUpdate(QObject *parent)
    : QObject(parent), mPrivate(new PATStdfUpdatePrivate)
{
}

PATStdfUpdate::~PATStdfUpdate()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

bool PATStdfUpdate::Execute(const QString &lInputFileName, const QString &lOutputFileName)
{
    mPrivate->mInputFileName    = lInputFileName;
    mPrivate->mOutputFileName   = lOutputFileName;
    mPrivate->mErrorMessage.clear();

    // Check if input file exists
    if(QFile::exists(mPrivate->mInputFileName) == false)
    {
        // Error. STDF file doesn't exist
        mPrivate->mErrorMessage = "*PAT Error* File not found:" + mPrivate->mInputFileName;
        return false;
    }

    if (mPrivate->mContext == NULL)
    {
        mPrivate->mErrorMessage = "Context pointer is null";
        return false;
    }

    CGexGroupOfFiles *  lGroup = NULL;

    // The WS PAT process must handle only one group of file.
    if (gexReport->getGroupsList().count() == 1)
    {
        lGroup = gexReport->getGroupsList().first();
        mPrivate->mFile = (!lGroup || lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

        // No datasets handler found
        if (mPrivate->mFile == NULL)
        {
            mPrivate->mErrorMessage = "No datasets handler found.";
            return false;
        }
    }
    else
    {
        mPrivate->mErrorMessage = "Too many groups of file detected.";
        return false;
    }

    // Check if input file can be open
    if(mPrivate->mInputFile.Open(mPrivate->mInputFileName.toLatin1().constData(), STDF_READ,
                                 1000000L) != GS::StdLib::Stdf::NoError)
    {
        // Error. Can't open STDF file in read mode!
        mPrivate->mErrorMessage = "*PAT Error* Failed reading file (probably corrupted):" +
                                  mPrivate->mInputFileName;
        return false;
    }

    if(mPrivate->mOutputFile.Open(mPrivate->mOutputFileName.toLatin1().constData(),
                                  STDF_WRITE) != GS::StdLib::Stdf::NoError)
    {
        // Failed to create patman output file
        mPrivate->mErrorMessage = "*PAT Error* Failed to create output file.\nDisk full or protection issue?:\nFile: " +
                                  mPrivate->mOutputFileName;

        return false;
    }

    // Make sure we recreate the STDF output in the same CPU format as it was
    // (sothat records we dump and reecords we recreate are in the right and same CPU type!)
    mPrivate->mOutputFile.SetStdfCpuType(mPrivate->mInputFile.GetStdfCpuType());

    if (WriteFAR() == false)
    {
        mPrivate->mErrorMessage = "*PAT Error* Failed to write FAR record in the output file";
        return false;
    }

    if (WriteATR() == false)
    {
        mPrivate->mErrorMessage = "*PAT Error* Failed to write ATR record in the output file";
        return false;
    }

    if (ProcessFile() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to process file %1").arg(mPrivate->mInputFileName).toLatin1().constData());

        return false;
    }
    return true;
}

const QString &PATStdfUpdate::GetErrorMessage() const
{
    return mPrivate->mErrorMessage;
}

void PATStdfUpdate::SetPATSettings(const PATProcessing &lSettings)
{
    mPrivate->mSettings = lSettings;
}

bool PATStdfUpdate::CheckParameterValue(CPatDefinition *lPatDef, int lSite, const CTest &lTest,
                                        double lResult, CPatSite *lPATSite, bool& lWrittenRecord)
{
    if (lPatDef == NULL)
        return false;

    // Check if Test fails the STATIC PAT Limits: ignore STATIC PAT limits test if the Bin assigned is negative, or Sigma = 0
    bool lApplyStaticPAT = true;

    if(lPatDef->m_lfRobustSigma == 0 && mPrivate->mContext->GetRecipeOptions().bIgnoreIQR0)
        lApplyStaticPAT = false;
    else if(lPatDef->m_lFailStaticBin < 0)
        lApplyStaticPAT = false;

    // Check if we have to ignore some data samples...
    switch(lPatDef->m_SamplesToIgnore)
    {
        case GEX_TPAT_IGNOREDATA_NONEID:		// Do not ignore any data sample...
            break;

        case GEX_TPAT_IGNOREDATA_NEGATIVEID:	// Ignore NEGATIVE samples
            if(lResult < 0)
                return false;
            break;

        case GEX_TPAT_IGNOREDATA_POSITIVEID:	// Ignore POSITIVE samples
            if(lResult > 0)
                return false;
            break;
    }

    // Stop-on-fail mode: If SPAT failure in this flow, simply ignore followinng records until PRR
    if(mPrivate->mContext->GetRecipeOptions().bStopOnFirstFail && lPATSite && lPATSite->lPatBin >= 0)
    {
        // Do NOT keep original record!
        lWrittenRecord = true;
        return false;
    }

    // If Static PAT enabled...check sample value over Static PAT limits.
    if(lApplyStaticPAT)
    {
        // If Robust Range == 0, then see if we still use the Static PAT limits
        if(lResult < lPatDef->m_lfLowStaticLimit)
        {
            if (lPATSite)
            {
                // Failing STATIC PAT limits
                // Get PAT bin
                lPATSite->lPatBin = lPatDef->m_lFailStaticBin;

                // Update count of LOW STATIC failures
                lPatDef->m_lStaticFailuresLow_AllParts++;

                // Update total failures in test
                lPatDef->m_TotalFailures_AllParts++;

                // Update total failures in flow
                lPATSite->lOutliersFound++;

                // Add test number is Outlier list for this run.
                CPatFailingTest lFailTest;
                lFailTest.mTestNumber       = lTest.lTestNumber;
                if (lTest.lPinmapIndex >= 0)
                    lFailTest.mPinIndex = lTest.lPinmapIndex;
                else
                    lFailTest.mPinIndex         = -1;
                lFailTest.mTestName         = lTest.strTestName;
                lFailTest.mFailureMode      = GEX_TPAT_FAILMODE_STATIC_L;	// Static LOW PAT failure
                lFailTest.mFailureDirection = -1;           // Negative failure (under low PAT limit)
                lFailTest.mSite             = lSite;        // Keeps track of site#
                lFailTest.mValue            = lResult;      // Keeps track of the outlier result value.

                lPATSite->cOutlierList.append(lFailTest);

                // Flag failure type: Static PAT failure
                lPATSite->bFailType = GEX_TPAT_BINTYPE_STATICFAIL;
            }

            return true;
        }

        if(lResult > lPatDef->m_lfHighStaticLimit)
        {
            if (lPATSite)
            {
                // Failing STATIC PAT limits
                // Get PAT bin
                lPATSite->lPatBin = lPatDef->m_lFailStaticBin;

                // Update count of HIGH STATIC failures
                lPatDef->m_lStaticFailuresHigh_AllParts++;

                // Update total failures in test
                lPatDef->m_TotalFailures_AllParts++;

                // Update total failures in flow
                lPATSite->lOutliersFound++;

                // Add test number is Outlier list for this run.
                CPatFailingTest lFailTest;
                lFailTest.mTestNumber       = lTest.lTestNumber;
                if (lTest.lPinmapIndex >= 0)
                    lFailTest.mPinIndex = lTest.lPinmapIndex;
                else
                    lFailTest.mPinIndex         = -1;
                lFailTest.mTestName         = lTest.strTestName;
                lFailTest.mFailureMode      = GEX_TPAT_FAILMODE_STATIC_H;	// Static High PAT failure
                lFailTest.mFailureDirection = +1;           // Positive failure (over high PAT limit)
                lFailTest.mSite             = lSite;        // Keeps track of site#
                lFailTest.mValue            = lResult;      // Keeps track of the outlier result value.

                lPATSite->cOutlierList.append(lFailTest);

                // Flag failure type: Static PAT failure
                lPATSite->bFailType = GEX_TPAT_BINTYPE_STATICFAIL;
            }

            return true;
        }
    }

    // Check if Test fails the Dynamic PAT Limits: ignore Dynamic PAT limits test if the Bin assigned is negative.
    if(lPatDef->m_lFailDynamicBin >= 0)
    {
        // Get Dynamic limits set (according to Site#)
        GS::PAT::DynamicLimits lDynLimits = lPatDef->GetDynamicLimits(lSite);

        // Check if passes any of the limits sets
        if((lResult >= lDynLimits.mLowDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] &&
            lResult <= lDynLimits.mHighDynamicLimit1[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]) ||
           (lResult >= lDynLimits.mLowDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR] &&
            lResult <= lDynLimits.mHighDynamicLimit2[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR]))
            return false;

        // check if on the low side of the limit sets
        int	lFirstFailingLimitsSet = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;

        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR;
            lIdx >= GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            --lIdx)
        {
            if(lResult < lDynLimits.mLowDynamicLimit1[lIdx] ||
               lResult < lDynLimits.mLowDynamicLimit2[lIdx])
            {
                // Keep track of first limits set that failed for this test
                if(lFirstFailingLimitsSet == GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR)
                    lFirstFailingLimitsSet = lIdx;

                // Update count of LOW Dynamic failures
                lPatDef->m_lDynamicFailuresLow_AllParts[lIdx]++;

                // Check if this failure is within the Severity limits activated...
                // (eg: tests is a Medium outlier, while test's settings is to fail 'Far' outliers only.)
                if(lPatDef->m_iOutlierLimitsSet <= lIdx)
                {
                    if (lPATSite)
                    {
                        // DPAT outlier
                        // Update total failures in test
                        lPatDef->m_TotalFailures_AllParts++;

                        // Update fail count of the test for this testing site
                        if(lPatDef->m_DynamicFailCountPerSite.find(lSite) == lPatDef->m_DynamicFailCountPerSite.end())
                            lPatDef->m_DynamicFailCountPerSite[lSite] = 1;
                        else
                            lPatDef->m_DynamicFailCountPerSite[lSite] = lPatDef->m_DynamicFailCountPerSite[lSite] +1;

                        // Failing Dynamic Low limits
                        lPATSite->lPatBin = lPatDef->m_lFailDynamicBin;

                        // Update total failures in flow
                        lPATSite->lOutliersFound++;

                        // Add test number is Outlier list for this run.
                        CPatFailingTest lFailTest;
                        lFailTest.mTestNumber       = lTest.lTestNumber;
                        if (lTest.lPinmapIndex >= 0)
                            lFailTest.mPinIndex = lTest.lPinmapIndex;
                        else
                            lFailTest.mPinIndex         = -1;
                        lFailTest.mTestName         = lTest.strTestName;;
                        lFailTest.mFailureMode          = lFirstFailingLimitsSet;	// Dynamic PAT failure
                        lFailTest.mFailureDirection     = -1;       // Negative failure (under low Dynamic PAT limit)
                        lFailTest.mSite                 = lSite;	// Keeps track of site#
                        lFailTest.mValue                = lResult;  // Keeps track of the outlier result value.

                        lPATSite->cOutlierList.append(lFailTest);

                        // Flag failure type: Dynamic PAT failure
                        lPATSite->bFailType = GEX_TPAT_BINTYPE_DYNAMICFAIL;
                    }

                    return true;
                }
            }
        }

        lFirstFailingLimitsSet = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
        for(int lIdx = GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR;
            lIdx >= GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR;
            --lIdx)
        {
            if(lResult > lDynLimits.mHighDynamicLimit1[lIdx] ||
               lResult > lDynLimits.mHighDynamicLimit2[lIdx])
            {
                // Keep track of first limits set that failed for this test
                if(lFirstFailingLimitsSet == GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR)
                    lFirstFailingLimitsSet = lIdx;

                // Update count of HIGH Dynamic failures
                lPatDef->m_lDynamicFailuresHigh_AllParts[lIdx]++;


                // Check if this failure is within the Severity limits activated...(eg: tests is a Medium outlier, while test's settings is to fail 'Far' outliers only.)
                if(lPatDef->m_iOutlierLimitsSet <= lIdx)
                {
                    if (lPATSite)
                    {
                        // DPAT failure
                        // Update total failures in test
                        lPatDef->m_TotalFailures_AllParts++;

                        // Update fail count of the test for this testing site
                        if(lPatDef->m_DynamicFailCountPerSite.find(lSite) == lPatDef->m_DynamicFailCountPerSite.end())
                            lPatDef->m_DynamicFailCountPerSite[lSite] = 1;
                        else
                            lPatDef->m_DynamicFailCountPerSite[lSite] = lPatDef->m_DynamicFailCountPerSite[lSite] +1;

                        // Failing Dynamic High limits
                        lPATSite->lPatBin = lPatDef->m_lFailDynamicBin;

                        // Update total failures in flow
                        lPATSite->lOutliersFound++;

                        // Add test number is Outlier list for this run.
                        CPatFailingTest lFailTest;

                        lFailTest.mTestNumber       = lTest.lTestNumber;
                        if (lTest.lPinmapIndex >= 0)
                            lFailTest.mPinIndex = lTest.lPinmapIndex;
                        else
                            lFailTest.mPinIndex         = -1;
                        lFailTest.mTestName         = lTest.strTestName;
                        lFailTest.mFailureMode          = lFirstFailingLimitsSet;	// Dynamic PAT failure
                        lFailTest.mFailureDirection     = +1;               // Positive failure (over high Dynamic PAT limit)
                        lFailTest.mSite                 = lSite;            // Keeps track of site#
                        lFailTest.mValue                = lResult;          // Keeps track of the outlier result value.

                        lPATSite->cOutlierList.append(lFailTest);

                        // Flag failure type: Dynamic PAT failure
                        lPATSite->bFailType = GEX_TPAT_BINTYPE_DYNAMICFAIL;
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

bool PATStdfUpdate::ProcessFile()
{
    int     lStatus          = GS::StdLib::Stdf::NoError;
    GS::StdLib::StdfRecordReadInfo lRecordHeader;

    // One or Two passes required:
    // 1 pass only: if do NOT consider PAT failures in bad-neigbhorhood algorithm
    // 2 passes required: if need to consider PAT failures in bad-neigbhorhood algorithm
    if (mPrivate->mContext->GetRecipeOptions().mGPAT_IgnorePPatBins == true)
    {
        // Only one pass required
        mPrivate->mPassCount = 1;
    }
    else
    {
        // Two passes required: PAss one to identify PPAT, second for MVPAT and/or GPAT
        mPrivate->mPassCount = 2;
    }

    do
    {
        // Increment the pass#
        ++mPrivate->mPassCurrent;

        if (mPrivate->IsLastPass())
        {
            if (mPrivate->mEngine.AnalyzeWaferSurface() == false)
            {
                mPrivate->mErrorMessage = "Failed to analyze Wafer surface.";
                GSLOG(SYSLOG_SEV_ERROR, "Failed to analyze Wafer surface.");
                return false;
            }
        }

        if (mPrivate->mPassCurrent == 1)
        {
            if (mPrivate->mEngine.ComputeMVPATOutliers() == false)
            {
                mPrivate->mErrorMessage = "Failed to compute Multi-Variate PAT outliers.";
                return false;
            }
        }

        lStatus = mPrivate->mInputFile.LoadRecord(&lRecordHeader);

        while(lStatus == GS::StdLib::Stdf::NoError)
        {
            if (ProcessRecord(lRecordHeader) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR, "Failed to process STDF record during PAT processing.")
                return false;
            }

            // Read one record from STDF file.
            lStatus = mPrivate->mInputFile.LoadRecord(&lRecordHeader);
        };

        // Close input stdf file name
        mPrivate->mInputFile.Close();

        if (lStatus != GS::StdLib::Stdf::EndOfFile)
        {
            mPrivate->mErrorMessage =  "*PAT Error* Failed reading file (probably corrupted):" +
                                       mPrivate->mInputFileName;
            return false;
        }

        // There is still one or more pass to perform, then do reopen the STDF input file!
        if (mPrivate->IsLastPass() == false)
        {
            // Check if input file can be open
            if(mPrivate->mInputFile.Open(mPrivate->mInputFileName.toLatin1().constData(), STDF_READ,
                                         1000000L) != GS::StdLib::Stdf::NoError)
            {
                // Error. Can't open STDF file in read mode!
                mPrivate->mErrorMessage = "*PAT Error* Failed reading file (probably corrupted):" +
                                          mPrivate->mInputFileName;
                return false;
            }
        }
    } while (mPrivate->IsLastPass() == false);

    // In case MRR was miissing, we can't process this abnormal file!
    if(mPrivate->mMRRFound == false)
    {
        // Write PIR/PRR records for external map failures
        if (WriteExternalMapFailures() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to write External Map failures records");
            return false;
        }

        // Write DTR records (with PAT results) + TSR + HBR + SBR + WCR + PCR
        if (WritePATRecords() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to write PAT records");
            return false;
        }

        // create missing MRR
        GS::StdLib::StdfRecordReadInfo lRecordHeader;

        lRecordHeader.iRecordType       = 1;
        lRecordHeader.iRecordSubType    = 20;	// MRR

        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
        mPrivate->mOutputFile.WriteDword(time(NULL) + (mPrivate->mWaferEndT - mPrivate->mWaferStartT));

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write MRR record";
            return false;
        }
    }

    // Close Output STDF file
    mPrivate->mOutputFile.Close();

    return true;
}

bool PATStdfUpdate::ProcessRecord(GS::StdLib::StdfRecordReadInfo &lRecordHeader)
{
    bool lWrittenRecord = false;

    // Process STDF record read.
    switch(lRecordHeader.iRecordType)
    {
        case 0:
            // Ignore FAR in file as we've already written it!
            if (lRecordHeader.iRecordSubType == 10)
                lWrittenRecord = true;
            break;

        case 1:
        {
            switch(lRecordHeader.iRecordSubType)
            {
                case 10:
                    // Process MIR records... Type = 1:10
                    if (ProcessMIR(lWrittenRecord) == false)
                        return false;

                    // Create list of testing sites to process: during first pass
                    if(mPrivate->mPassCurrent == 1)
                    {
                        // Compute limits
                        if(mPrivate->mEngine.ComputePatLimits(mPrivate->mContext->GetSiteList(), true) == false)
                        {
                            // Invalid file...
                            mPrivate->mErrorMessage = "*PAT Error* Failed computing Dynamic PAT limits";
                            GSLOG(SYSLOG_SEV_ERROR, "Failed to compute DPAT limits");
                            return false;
                        }

                        // Tells performing STDF file generation
                        Engine::GetInstance().UpdateLabelStatus(" Updating STDF file...");
                        // Make sure screen is refreshed.
                        QCoreApplication::processEvents();
                    }
                    break;


                case 20:
                    // MRR: last record in file, so just before it, dump our HBR & SBR
                    mPrivate->mMRRFound = true;

                    // Write MRR
                    if (ProcessMRR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process MRR record");
                        return false;
                    }
                    break;

                case 30:
                    // Process PCR records... Type = 1:30
                    ProcessPCR(lWrittenRecord);
                    break;

                case 40:
                    // Process HBR records... Type = 1:40
                    if (ProcessBinRecords(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process HBR record");
                        return false;
                    }
                    break;

                case 50:
                    // Process SBR records... Type = 1:50
                    if (ProcessBinRecords(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process SBR record");
                        return false;
                    }
                    break;
            }
            break;
        }

        case 2:
            switch(lRecordHeader.iRecordSubType)
            {
                case 10:
                    // Process WIR records... Type = 2:10
                    if (ProcessWIR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process WIR record");
                        return false;
                    }
                    break;

                case 20:
                    // Process WRR records... Type = 2:20
                    if (ProcessWRR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process WRR record");
                        return false;
                    }

                    break;

                case 30:// Process WCR records... Type = 2:30
                    mPrivate->mWCRFound = true;
                    break;
            }
            break;

        case 5:
            switch(lRecordHeader.iRecordSubType)
            {
                case 10:
                    // Process PIR records... Type = 5:10
                    ProcessPIR();
                    break;
                case 20:
                    // Process PRR records... Type = 5:20
                    if (ProcessPRR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process PRR record");
                        return false;
                    }
                    break;
            }
            break;

        case 10:
            switch(lRecordHeader.iRecordSubType)
            {
                case 30:
                    // Process TSR records... Type = 10:30
                    // Ignore Parametric and Multi-Parametric TSR as we rewrite our own!
                    if (ProcessTSR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process TSR record");
                        return false;
                    }
                    break;
            }
            break;

        case 15:
            switch(lRecordHeader.iRecordSubType)
            {
                case 10:
                    // Process PTR records... Type = 15:10
                    // Only write Parametric records if FULL STDF mode enabled
                    if (ProcessPTR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process PTR record");
                        return false;
                    }
                    break;
                case 15:
                    // Process MPR records... Type = 15:15
                    // Only write Parametric records if FULL STDF mode enabled
                    if (ProcessMPR(lWrittenRecord) == false)
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process MPR record");
                        return false;
                    }
                    break;
            }
            break;

        case 50:
            switch( lRecordHeader.iRecordSubType )
            {
                // GDR : 50...10
                case 10 :
                {
                    GQTL_STDF::Stdf_GDR_V4 gdr;
                    gdr.Read( mPrivate->mInputFile );

                    // set the deciphering mode if needed
                    GQTL_STDF::HeadAndSiteNumberDecipher::SetDecipheringModeInStdf
                        ( gdr, mPrivate->mInputFile );
                    break;
                }
                // DTR : 50...30
                case 30 :
                {
                    GQTL_STDF::Stdf_DTR_V4 dtr;
                    if( ! dtr.Read( mPrivate->mInputFile ) )
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to process DTR record");
                        return false;
                    }

                    ReadDTR( dtr );

                    break;
                }
            }
            break;

        default:
            break;
    }

    // If this record must be copied 'as-is', then do it!
    if(!lWrittenRecord & mPrivate->IsLastPass())
        mPrivate->mOutputFile.DumpRecord(&lRecordHeader, &mPrivate->mInputFile, true);

    return true;
}

void PATStdfUpdate::ReadDTR( const GQTL_STDF::Stdf_DTR_V4 &/*dtr*/ )
{
    // process reticle infos if any
//    ProcessReticleInformationsIn( dtr );
}

/******************************************************************************!
 * \fn ProcessReticleInformationsIn
 ******************************************************************************/
//void
//PATStdfUpdate::ProcessReticleInformationsIn(const GQTL_STDF::Stdf_DTR_V4 &dtr)
//{

//}

bool PATStdfUpdate::ProcessTSR(bool &lWrittenRecord)
{
    if(mPrivate->IsLastPass())
    {
        if (mPrivate->mInputFile.GetStdfVersion() == GEX_STDFV4)
        {
            GQTL_STDF::Stdf_TSR_V4 lTSRRecord;
            if (lTSRRecord.Read(mPrivate->mInputFile) == false)
            {
                mPrivate->mErrorMessage = "*PAT Error* Failed to read TSR record";
                return false;
            }

            // Ignore non functional TSR as we write our own
            if (lTSRRecord.m_c1TEST_TYP != 'F')
                lWrittenRecord = true;
        }
        else
            lWrittenRecord = true;
    }

    return true;
}

bool PATStdfUpdate::ProcessWIR(bool &lWrittenRecord)
{
    if(mPrivate->IsLastPass())
    {
        // Checks if PAT running at ST-Microelectronics customer site
        bool lST_MicroPAT = (getenv(PATMAN_USER_ST) != NULL);	// eg: "GEX_PROMIS_DATA_PATH"

        // WIR
        char	lTmpString[STDF_CN_STRING_MAX+1];
        BYTE	lData;

        // Record header
        GS::StdLib::StdfRecordReadInfo lRecordHeader;
        lRecordHeader.iRecordType       = 2;
        lRecordHeader.iRecordSubType    = 10;	// WIR

        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);

        // Save tester head# (may be used in HBR/SBR if creating STDF output file)
        mPrivate->mInputFile.ReadByte(&mPrivate->mTesterHead);      // Head
        mPrivate->mInputFile.ReadByte(&lData);                      // pad (stdf V3), test site (stdf V4)
        mPrivate->mInputFile.ReadDword(&mPrivate->mWaferStartT);	// START_T
        if(lST_MicroPAT)
            lData = 0xFF;					// ST PAT: forces site group = 255

        mPrivate->mWaferEndT = mPrivate->mWaferStartT;	// Copy in case WRR missing!

        mPrivate->mOutputFile.WriteByte(mPrivate->mTesterHead);
        mPrivate->mOutputFile.WriteByte(lData);
        if(mPrivate->mSettings.bUpdateTimeFields)
            mPrivate->mOutputFile.WriteDword(time(NULL));		// Overwrite START_T
        else
            mPrivate->mOutputFile.WriteDword(mPrivate->mWaferStartT);


        // WaferID
        if(mPrivate->mInputFile.ReadString(lTmpString) >= 0)
            mPrivate->mOutputFile.WriteString(lTmpString);

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write WIR record";
            return false;
        }

        lWrittenRecord = true;
    }

    return true;
}

bool PATStdfUpdate::ProcessWRR(bool &lWrittenRecord)
{
    if(mPrivate->IsLastPass())
    {
        // Write PIR/PRR records for external map failures
        if (WriteExternalMapFailures() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to write External Map failures records");
            return false;
        }

        // Checks if PAT running at ST-Microelectronics customer site
        bool lST_MicroPAT = (getenv(PATMAN_USER_ST) != NULL);	// eg: "GEX_PROMIS_DATA_PATH"

        // Update WRR
        char	lTmpString[STDF_CN_STRING_MAX+1];
        BYTE	lByteData;
        long	lLongData;

        // Record header
        GS::StdLib::StdfRecordReadInfo lRecordHeader;
        lRecordHeader.iRecordType       = 2;
        lRecordHeader.iRecordSubType    = 20;	// WRR

        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);

        switch(mPrivate->mInputFile.GetStdfVersion())
        {
        default :
                break;

            case GEX_STDFV4:
                // Head
                mPrivate->mInputFile.ReadByte(&lByteData);
                mPrivate->mOutputFile.WriteByte(lByteData);
                // site group
                mPrivate->mInputFile.ReadByte(&lByteData);
                if(lST_MicroPAT)
                    lByteData = 0xFF;		// ST PAT: forces site group = 255
                mPrivate->mOutputFile.WriteByte(lByteData);

                // FINISH_T
                mPrivate->mInputFile.ReadDword(&mPrivate->mWaferEndT);
                if(mPrivate->mSettings.bUpdateTimeFields)
                {
                    if(lST_MicroPAT)
                        mPrivate->mOutputFile.WriteDword(time(NULL));
                    else
                        mPrivate->mOutputFile.WriteDword(time(NULL) + (mPrivate->mWaferEndT - mPrivate->mWaferStartT));
                }
                else
                    mPrivate->mOutputFile.WriteDword(mPrivate->mWaferEndT);

                // PART_CNT
                mPrivate->mInputFile.ReadDword(&lLongData);
                mPrivate->mOutputFile.WriteDword(lLongData);
                // RTST_CNT
                mPrivate->mInputFile.ReadDword(&lLongData);
                mPrivate->mOutputFile.WriteDword(lLongData);
                // ABRT_CNT
                mPrivate->mInputFile.ReadDword(&lLongData);
                mPrivate->mOutputFile.WriteDword(lLongData);
                // GOOD_CNT: Deduct total outliers.
                if(mPrivate->mInputFile.ReadDword(&lLongData) == GS::StdLib::Stdf::NoError)
                    lLongData -= mPrivate->mContext->m_lstOutlierParts.count();
                mPrivate->mOutputFile.WriteDword(lLongData);
                // FUNC_CNT
                mPrivate->mInputFile.ReadDword(&lLongData);
                mPrivate->mOutputFile.WriteDword(lLongData);
                // WAFER_ID
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                // FABWF_ID
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                // FRAME_ID
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                // MASK_ID
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                // USR_DESC
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                // EXC_DESC
                mPrivate->mInputFile.ReadString(lTmpString);
                mPrivate->mOutputFile.WriteString(lTmpString);
                break;
        }

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write WRR record";
            return false;
        }

        lWrittenRecord = true;

        // Write our custom HBR & SBR (with failing PAT) ...unless already done.
        return WritePATBinRecords();
    }

    return true;
}

bool PATStdfUpdate::ProcessBinRecords(bool &lWrittenRecord)
{
    lWrittenRecord = true;

    return WritePATBinRecords();
}

bool PATStdfUpdate::ProcessMIR(bool &lWrittenRecord)
{
    // MIR
    long	lSetupT = 0;
    long    lStartT = 0;
    char	lTmpString[STDF_CN_STRING_MAX+1];
    long	lCurrentPos;
    long    lPos;

    // Some MIR fields
    BYTE bStation,bmode_code,brtst_code,bprot_cod,bcmode_code;
    int	wburn_time;
    QString strString,strLotID,strPartType,strNodeName,strTesterType,strJobName;
    QString strJobRev;
    QString strSublot;
    QString strOperator;
    QString strExecType;
    QString strExecVersion;
    QString strTestCode;
    QString strTemperature;
    QString strUserText;
    QString strAuxFile;
    QString strPackageType;
    QString strFamilyID;
    QString strDateCode;
    QString strFacilityID;
    QString strFloorID;
    QString strProcID;
    QString strFreq;
    QString strSpecName;
    QString strSpecVersion;
    QString strFlowID;
    QString strSetupID;
    QString strDesignerRev;
    QString strEngID;
    QString strROMcode;
    QString strSerialNumber;
    QString strSuperName;

    // Checks if PAT running at ST-Microelectronics customer site
    bool bST_MicroPAT = (getenv(PATMAN_USER_ST) != NULL);	// eg: "GEX_PROMIS_DATA_PATH"

    switch(mPrivate->mInputFile.GetStdfVersion())
    {
        case GEX_STDFV4:

            lPos = mPrivate->mInputFile.GetReadRecordPos();				// Save current offset position in read buffer
            mPrivate->mInputFile.ReadDword((long *)&lSetupT);				// Setup_T
            mPrivate->mInputFile.ReadDword((long *)&lStartT);				// Start_T

            // Check if need to overload the Start & Setup dates...
            if(mPrivate->mSettings.bUpdateTimeFields)
            {
                lCurrentPos = mPrivate->mInputFile.GetReadRecordPos();	// Save current offset position in read buffer
                mPrivate->mInputFile.SetReadRecordPos(lPos);				// Force offset to point over 'Setup_T' date
                mPrivate->mInputFile.OverwriteReadDword(lSetupT);		// Overwrite Setup_T
                mPrivate->mInputFile.OverwriteReadDword(lSetupT);		// Overwrite Start_T
                mPrivate->mInputFile.SetReadRecordPos(lCurrentPos);		// Revert read-buffer offset position.
                lSetupT = lStartT = time(NULL);
            }

            mPrivate->mInputFile.ReadByte(&bStation);				// stat #
            mPrivate->mInputFile.ReadByte(&bmode_code);			// mode_code
            mPrivate->mInputFile.ReadByte(&brtst_code);			// rtst_code
            mPrivate->mInputFile.ReadByte(&bprot_cod);				// prot_cod #
            mPrivate->mInputFile.ReadWord(&wburn_time);			// burn_time
            mPrivate->mInputFile.ReadByte(&bcmode_code);			// cmode_code
            mPrivate->mInputFile.ReadString(lTmpString);		// Lot ID
            strLotID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Part Type
            strPartType = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Node name
            strNodeName = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Tester type
            strTesterType = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Job name
            strJobName = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Job revision
            strJobRev = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Sublot
            strSublot = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Operator
            strOperator = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Exec type
            strExecType = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Exec version
            strExecVersion = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Test code
            strTestCode = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Temperature
            strTemperature = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// User Text
            strUserText = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Aux file
            strAuxFile = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Package type
            strPackageType = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Family ID
            strFamilyID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Date code
            strDateCode = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Facility ID
            strFacilityID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Floor ID
            strFloorID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Proc ID
            strProcID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Freq.
            strFreq = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Spec name
            strSpecName = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Spec version
            strSpecVersion = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Flow ID
            strFlowID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Setup ID
            strSetupID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Designer Rev
            strDesignerRev = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Eng ID
            strEngID = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// ROM code
            strROMcode = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Serial number
            strSerialNumber = lTmpString;
            mPrivate->mInputFile.ReadString(lTmpString);		// Super name
            strSuperName = lTmpString;

            // IF ST customer, rewrite MIR record...
            if(bST_MicroPAT && mPrivate->IsLastPass())
            {
                // Recipe name & Version
                GS::StdLib::StdfRecordReadInfo lRecordHeader;
                QFileInfo   cFileInfo(mPrivate->mSettings.strRecipeFile);	// ENG recipe path+name <path>/R7105XXL_Version1_patman_1.0.csv
                QString     strSection          = cFileInfo.baseName();			// eg: 'R7105XXL_Version1_patman_1.0.csv'
                QString     strRecipeName       = strSection.section('_',0,0);  // eg: 'R7105XXL'
                QString     strRecipeVersion    = strSection.section('_',1,1);  // eg: 'Version1'

                strRecipeVersion = strRecipeVersion.section("Version",1);
                if(strRecipeVersion.isEmpty())
                    strRecipeVersion = strRecipeVersion.section("Version",1);

                // If Production recipe, overload above info!
                if(mPrivate->mSettings.strProd_RecipeName.isEmpty() == false)
                {
                    strRecipeName       = mPrivate->mSettings.strProd_RecipeName;
                    strRecipeVersion    = mPrivate->mSettings.strProd_RecipeVersion;
                }

                // MIR header
                lRecordHeader.iRecordType = 1;
                lRecordHeader.iRecordSubType = 10;

                mPrivate->mOutputFile.WriteHeader(&lRecordHeader);

                lSetupT = cFileInfo.created().toTime_t();	// Recipe creation time.
                lStartT = time(NULL);
                mPrivate->mOutputFile.WriteDword(lSetupT);
                mPrivate->mOutputFile.WriteDword(lStartT);

                // Clear optional variables
                strOperator = "";
                strTestCode = "";
                strTemperature = "";
                strPackageType = "";
                strSetupID = "";

                // Fields overload
                strUserText = cFileInfo.fileName();	// ENGineering Recipe (ex: R7105XXX_patman_1.0.csv)
                bcmode_code = 'A';
                strTesterType = "PATMAN";
                strProcID = "PAT";
                strFreq = mPrivate->mSettings.strOperation; // CAM PAT Operation
                strFlowID = "PAT";
                /// FOR ST PURPOSE ONLY, NOT USED SO FAR
//                if(mPrivate->mContext->cOptionsPat.cWorkingFlow.m_strFlow.isEmpty() == false)
//                    strFlowID += mPrivate->mContext->cOptionsPat.cWorkingFlow.m_strFlow.right(1);
//                else
//                    strFlowID += "1";// No flow defined, assume flow 1!
                strSpecName = strRecipeName;
                strSpecVersion = strRecipeVersion;
                // eg: Galaxy PAT-Man - V6.3 Build 154
                strString = Engine::GetInstance().Get("AppFullName").toString();
                strExecType = strString.section(" - V",0,0).trimmed();
                strExecVersion = "V" + strString.section(" - V",1,1).trimmed();
                cFileInfo.setFile(mPrivate->mSettings.strSources.first());
                strAuxFile = cFileInfo.baseName() + "." + cFileInfo.completeSuffix(); // Original STDF file

                // Drift: none for now, use PAT recipe instead
                strJobName = strSpecName;
                strJobRev = strSpecVersion;

                // Node name = computer running PATMAN
                strNodeName = Engine::GetInstance().GetSystemInfo().strHostName;

                // Write fields
                mPrivate->mOutputFile.WriteByte(bStation);			// stat #
                mPrivate->mOutputFile.WriteByte(bmode_code);			// mode_code
                mPrivate->mOutputFile.WriteByte(brtst_code);			// rtst_code
                mPrivate->mOutputFile.WriteByte(bprot_cod);			// prot_cod #
                mPrivate->mOutputFile.WriteWord(wburn_time);			// burn_time
                mPrivate->mOutputFile.WriteByte(bcmode_code);		// cmode_code
                mPrivate->mOutputFile.WriteString((strLotID.isEmpty() ? "": strLotID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strPartType.isEmpty() ? "": strPartType.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strNodeName.isEmpty() ? "": strNodeName.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strTesterType.isEmpty() ? "": strTesterType.toLatin1().constData()));

                mPrivate->mOutputFile.WriteString((strJobName.isEmpty() ? "": strJobName.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strJobRev.isEmpty() ? "": strJobRev.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strSublot.isEmpty() ? "": strSublot.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strOperator.isEmpty() ? "": strOperator.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strExecType.isEmpty() ? "": strExecType.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strExecVersion.isEmpty() ? "": strExecVersion.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strTestCode.isEmpty() ? "": strTestCode.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strTemperature.isEmpty() ? "": strTemperature.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strUserText.isEmpty() ? "": strUserText.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strAuxFile.isEmpty() ? "": strAuxFile.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strPackageType.isEmpty() ? "": strPackageType.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strFamilyID.isEmpty() ? "": strFamilyID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strDateCode.isEmpty() ? "": strDateCode.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strFacilityID.isEmpty() ? "": strFacilityID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strFloorID.isEmpty() ? "": strFloorID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strProcID.isEmpty() ? "": strProcID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strFreq.isEmpty() ? "": strFreq.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strSpecName.isEmpty() ? "": strSpecName.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strSpecVersion.isEmpty() ? "": strSpecVersion.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strFlowID.isEmpty() ? "": strFlowID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strSetupID.isEmpty() ? "": strSetupID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strDesignerRev.isEmpty() ? "": strDesignerRev.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strEngID.isEmpty() ? "": strEngID.toLatin1().constData()));
                mPrivate->mOutputFile.WriteString((strROMcode.isEmpty() ? "": strROMcode.toLatin1().constData()));
                //mPrivate->mOutputFile.WriteString((strSerialNumber.isEmpty() ? "": strSerialNumber.toLatin1().constData()));
                //mPrivate->mOutputFile.WriteString((strSuperName.isEmpty() ? "": strSuperName.toLatin1().constData()));

                lWrittenRecord = true;

                if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                {
                    mPrivate->mErrorMessage = "*PAT Error* Failed to write MIR record";
                    return false;
                }
            }
            break;

    default:
            // Invalid STDF version
            return false;
    }

    return true;
}


bool PATStdfUpdate::ProcessMPRInMergeMode(CPatDefinition*& aPatDef,
                                          GQTL_STDF::Stdf_MPR_V4 aMPRRecord,
                                          bool& aOutlier,
                                          CTest*& aTest,
                                          CPatSite* aPATSite,
                                          unsigned short aSiteNumber)
{
    float lResult;
    bool lWrittenRecord(false);

    // Returns pointer to correct cell. If cell doesn't exist ; it's created...Test# mapping enabled.
    if(mPrivate->mFile->FindTestCell(aMPRRecord.m_u4TEST_NUM, GEX_PTEST, &aTest, true, true,
                                     aMPRRecord.m_cnTEST_TXT.toLatin1().data()) !=1)
        return true;

    if(aTest == NULL)
    {
        // No Parameter name found in this record...then rely on Parameter Number only...
        if(mPrivate->mFile->FindTestCell(aMPRRecord.m_u4TEST_NUM, GEX_PTEST, &aTest) !=1)
            return true;
    }

    if(aTest->ldSamplesValidExecs < mPrivate->mContext->GetRecipeOptions().iMinimumSamples)
        return true;

    // Get the Pat definition for this test
    aPatDef = mPrivate->mContext->GetPatDefinition(aTest->lTestNumber, aTest->lPinmapIndex, aTest->strTestName);

    if (aPatDef)
    {

        int     lCount          = aMPRRecord.m_u2RSLT_CNT;

        // If more results than Pin indexes...ignore the leading results
        if(lCount > aMPRRecord.m_u2RTN_ICNT)
            lCount = aMPRRecord.m_u2RTN_ICNT;

        // Read each result in array...and process it!
        for(int lIdx = 0; lIdx < lCount; ++lIdx)
        {
            // Read Pin Parametric test result.
            lResult = aMPRRecord.m_kr4RTN_RSLT[lIdx];

            // Check if value is a outlier.
            aOutlier |= CheckParameterValue(aPatDef, aSiteNumber, *aTest, lResult, aPATSite, lWrittenRecord);

            if (lWrittenRecord)
                return true;
        }
    }
    // Return false to write the record
    return false;
}

bool PATStdfUpdate::ProcessMPRInSplitMode(CPatDefinition *&aPatDef,
                                          GQTL_STDF::Stdf_MPR_V4 aMPRRecord,
                                          bool& aOutlier,
                                          CTest *&aTest,
                                          CPatSite *aPATSite,
                                          unsigned short aSiteNumber)
{
    bool lWrittenRecord(false);
    int  lCount = aMPRRecord.m_u2RSLT_CNT;
    float lResult;

    // If more results than Pin indexes...ignore the leading results
    if(lCount > aMPRRecord.m_u2RTN_ICNT)
        lCount = aMPRRecord.m_u2RTN_ICNT;

    // Read each result in array...and process it!
    for(int lIdx = 0; lIdx < lCount; ++lIdx)
    {
        if(mPrivate->mFile->FindTestCell(aMPRRecord.m_u4TEST_NUM, lIdx, &aTest, true, true,
                                         aMPRRecord.m_cnTEST_TXT.toLatin1().data()) !=1)
            return true;
        // Get the Pat definition for this test
        aPatDef = mPrivate->mContext->GetPatDefinition(aTest->lTestNumber, aTest->lPinmapIndex, aTest->strTestName);

        if (aPatDef)
        {
            // Read Pin Parametric test result.
            lResult = aMPRRecord.m_kr4RTN_RSLT[lIdx];

            // Check if value is a outlier.
            aOutlier |= CheckParameterValue(aPatDef, aSiteNumber, *aTest, lResult, aPATSite, lWrittenRecord);

            // Don't write/dump the original record
            if (lWrittenRecord)
                return true;
        }
    }
    return false;
}

bool PATStdfUpdate::ProcessMPR(bool& lWrittenRecord)
{
    unsigned int        lMapIndex;
    CTest *             lTest       = NULL;
    CPatSite *          lPATSite    = NULL;
    CPatDefinition *    lPatDef     = NULL;

    // First read MPR completely
    GQTL_STDF::Stdf_MPR_V4 lMPRFirstRecord;     // Contains the first occurence of the MPR record for the combination Head/Site/Test number
    GQTL_STDF::Stdf_MPR_V4 lMPRRecord;
    lMPRRecord.Read(mPrivate->mInputFile);

    // Keep first occurence of each MPR records for static informations overall sites
    if (mPrivate->mMPRFirstOccurence.contains(QPair<unsigned long, unsigned char>(lMPRRecord.m_u4TEST_NUM, lMPRRecord.m_u1HEAD_NUM)) == false)
        mPrivate->mMPRFirstOccurence.insert(QPair<unsigned long, unsigned char>(lMPRRecord.m_u4TEST_NUM, lMPRRecord.m_u1HEAD_NUM), lMPRRecord);
    else
        lMPRFirstRecord = mPrivate->mMPRFirstOccurence.value(QPair<unsigned long, unsigned char>(lMPRRecord.m_u4TEST_NUM, lMPRRecord.m_u1HEAD_NUM));

    // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
    unsigned short siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn(mPrivate->mInputFile, lMPRRecord);

    lMPRRecord.m_u1HEAD_NUM = GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn(mPrivate->mInputFile, lMPRRecord);

    lMapIndex = (lMPRRecord.m_u1HEAD_NUM << 16) | siteNumber;

    // Check if this MPR is within a valid PIR/PRR block.
    if(mPrivate->mPATSites.contains(lMapIndex))
        lPATSite = mPrivate->mPATSites.value(lMapIndex);

    // If looking at processing clean STDF files only, then look if a PIR/PRR bloc is defined...
    if (mPrivate->mSTDFCompliancy.compare("stringent") == 0)
    {
        if (lPATSite == NULL || lPATSite->bPirSeen == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, "PIR/MPR mismatch for head_num, this MPR will be ignored");
            return true;
        }
    }

    if (mPrivate->mFile && mPrivate->mContext->GetSiteList().indexOf(siteNumber) >= 0)
    {
        bool lOutlier = false;
        if (mPrivate->mFile->m_eMPRMergeMode == CGexFileInGroup::NO_MERGE)
        {
            lWrittenRecord = ProcessMPRInSplitMode(lPatDef, lMPRRecord, lOutlier, lTest,lPATSite, siteNumber);
        }
        else
        {
            lWrittenRecord = ProcessMPRInMergeMode(lPatDef, lMPRRecord, lOutlier, lTest, lPATSite, siteNumber);
        }

        if (lWrittenRecord) return true;

        if (lPatDef)
        {
            GS::PAT::DynamicLimits lDynLimits = lPatDef->GetDynamicLimits(siteNumber);
            if (mPrivate->IsLastPass())
            {
                GQTL_STDF::Stdf_MPR_V4 lNewRecord;
                bool lCreateNewRecord   = false;
                char lOptFlag           = 0;

                // Check if test result is already fail (if so don't look for a outlier)
                if((lMPRRecord.m_b1TEST_FLG & STDF_MASK_BIT6) == STDF_MASK_BIT6 &&
                   (lMPRRecord.m_b1TEST_FLG & STDF_MASK_BIT7) == 0)
                    lOutlier = false;	// Test is fail, so ignore if it is also an outlier!

                // Check if limits defined: if no, need to overload with PAT limits
                if(((lMPRRecord.m_b1OPT_FLAG & STDF_MASK_BIT4) == 0)
                        && ((lMPRRecord.m_b1OPT_FLAG & STDF_MASK_BIT5) == 0))
                    lCreateNewRecord = true;

                // If first outlier instance for this test# (on this site), ensure we write the PAT limits.
                if(lPatDef->m_NewPTR_PerSite[siteNumber] != 1)
                {
                    // Force to write PTR with PAT limits
                    lCreateNewRecord = true;
                    lPatDef->m_NewPTR_PerSite[siteNumber] = 1;
                }

                // Rewrite PTR with PAT limits.
                if(lCreateNewRecord)
                {
                    //
                    lNewRecord.SetTEST_NUM(lMPRRecord.m_u4TEST_NUM);
                    lNewRecord.SetHEAD_NUM(lMPRRecord.m_u1HEAD_NUM);
                    lNewRecord.SetSITE_NUM(lMPRRecord.m_u1SITE_NUM);

                    // Test failing PAT limits. Flag it!
                    if(lOutlier)
                    {
                        lMPRRecord.m_b1TEST_FLG &= 077;     // Clear Bit7 (PASS flag)
                        lMPRRecord.m_b1TEST_FLG |= 0200;	// Tells 'PASS/FAIL' flag is valid
                    }

                    lNewRecord.SetTEST_FLG(lMPRRecord.m_b1TEST_FLG);
                    lNewRecord.SetPARM_FLG(lMPRRecord.m_b1PARM_FLG);
                    lNewRecord.SetRTN_ICNT(lMPRRecord.m_u2RTN_ICNT);
                    lNewRecord.SetRSLT_CNT(lMPRRecord.m_u2RSLT_CNT);

                    for (int lIdx = 0; lIdx < lMPRRecord.m_u2RTN_ICNT; ++lIdx)
                        lNewRecord.SetRTN_STAT(lIdx, lMPRRecord.m_kn1RTN_STAT[lIdx]);

                    for (int lIdx = 0; lIdx < lMPRRecord.m_u2RSLT_CNT; ++lIdx)
                        lNewRecord.SetRTN_RSLT(lIdx, lMPRRecord.m_kr4RTN_RSLT[lIdx]);

                    lNewRecord.SetTEST_TXT(lMPRRecord.m_cnTEST_TXT);
                    lNewRecord.SetALARM_ID(lMPRRecord.m_cnALARM_ID);

                    // Extract PAT limits involved
                    float	lLowLimit;
                    float   lHighLimit;

                    if(lPatDef->m_lFailDynamicBin < 0)
                    {
                        // Dynamic PAT disabled
                        if(lPatDef->m_lFailStaticBin < 0)
                        {
                            // PAT fully disabled, keep standard limits
                            lLowLimit   = lTest->GetCurrentLimitItem()->lfLowLimit;
                            lHighLimit  = lTest->GetCurrentLimitItem()->lfHighLimit;
                        }
                        else
                        {
                            // Only STATIC PAT is enabled for this test
                            lLowLimit  = lPatDef->m_lfLowStaticLimit;
                            lHighLimit = lPatDef->m_lfHighStaticLimit;
                        }
                    }
                    else
                    {
                        // Dynamic PAT is enabled
                        lLowLimit  = gex_min(lDynLimits.mLowDynamicLimit1[lPatDef->m_iOutlierLimitsSet],
                                             lDynLimits.mLowDynamicLimit2[lPatDef->m_iOutlierLimitsSet]);
                        lHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[lPatDef->m_iOutlierLimitsSet],
                                             lDynLimits.mHighDynamicLimit2[lPatDef->m_iOutlierLimitsSet]);
                    }

                    // Ensure 'Valid Test limits' flag is set
                    // Clear top 4 bits = valid limits.
                    lOptFlag = lMPRRecord.m_b1OPT_FLAG & 0xf;

                    // Check if must include original test limits...
                    if(mPrivate->mSettings.bReportStdfOriginalLimits)
                        lOptFlag &= ~(014);	// ensure relevant flags in right place (clear BIT2 and 3)

                    // Check if LL or HL is infinite
                    if(lLowLimit <= -3.4e+38F || lLowLimit >= 3.4e+38F)
                        lOptFlag |= 0100; // bit 6 set = no Low limit
                    if(lHighLimit >= 3.4e+38F || lHighLimit <= -3.4e+38F)
                        lOptFlag |= 0200; // bit 7 set = no High limit

                    lNewRecord.SetOPT_FLAG(lOptFlag);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRES_SCAL))
                        lNewRecord.SetRES_SCAL(lMPRRecord.m_i1RES_SCAL);
                    else
                        lNewRecord.SetRES_SCAL(lMPRFirstRecord.m_i1RES_SCAL);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLLM_SCAL))
                        lNewRecord.SetLLM_SCAL(lMPRRecord.m_i1LLM_SCAL);
                    else
                        lNewRecord.SetRES_SCAL(lMPRFirstRecord.m_i1LLM_SCAL);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHLM_SCAL))
                        lNewRecord.SetHLM_SCAL(lMPRRecord.m_i1HLM_SCAL);
                    else
                        lNewRecord.SetRES_SCAL(lMPRFirstRecord.m_i1HLM_SCAL);

                    lNewRecord.SetLO_LIMIT(lLowLimit);
                    lNewRecord.SetHI_LIMIT(lHighLimit);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposSTART_IN))
                        lNewRecord.SetSTART_IN(lMPRRecord.m_r4START_IN);
                    else
                        lNewRecord.SetSTART_IN(lMPRFirstRecord.m_r4START_IN);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposINCR_IN))
                        lNewRecord.SetINCR_IN(lMPRRecord.m_r4INCR_IN);
                    else
                        lNewRecord.SetINCR_IN(lMPRFirstRecord.m_r4INCR_IN);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposUNITS))
                        lNewRecord.SetUNITS(lMPRRecord.m_cnUNITS);
                    else
                        lNewRecord.SetUNITS(lMPRFirstRecord.m_cnUNITS);

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRTN_INDX))
                    {
                        for (int lIdx = 0; lIdx < lMPRRecord.m_u2RTN_ICNT; ++lIdx)
                            lNewRecord.SetRTN_INDX(lIdx, lMPRRecord.m_ku2RTN_INDX[lIdx]);
                    }
                    else if (lMPRFirstRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposRTN_INDX))
                    {
                        for (int lIdx = 0; lIdx < lMPRFirstRecord.m_u2RTN_ICNT; ++lIdx)
                            lNewRecord.SetRTN_INDX(lIdx, lMPRFirstRecord.m_ku2RTN_INDX[lIdx]);
                    }
                    else
                    {
                        GSLOG(SYSLOG_SEV_ERROR, "Failed to update MPR record due to null pointer for RTN_INDEX field");
                        mPrivate->mErrorMessage = "*PAT Error* Failed to update MPR record";
                        return false;
                    }

                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposUNITS_IN))
                        lNewRecord.SetUNITS_IN(lMPRRecord.m_cnUNITS_IN);
                    else
                        lNewRecord.SetUNITS_IN(lMPRFirstRecord.m_cnUNITS_IN);

                    // C_RESFMT,xxxFMT
                    // ANSI C result format string
                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_RESFMT))
                        lNewRecord.SetC_RESFMT(lMPRRecord.m_cnC_RESFMT);
                    else
                        lNewRecord.SetC_RESFMT(lMPRFirstRecord.m_cnC_RESFMT);

                    // ANSI C low limit format string
                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_LLMFMT))
                        lNewRecord.SetC_LLMFMT(lMPRRecord.m_cnC_LLMFMT);
                    else
                        lNewRecord.SetC_LLMFMT(lMPRFirstRecord.m_cnC_LLMFMT);

                    // ANSI C high limit format string
                    if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposC_HLMFMT))
                        lNewRecord.SetC_HLMFMT(lMPRRecord.m_cnC_HLMFMT);
                    else
                        lNewRecord.SetC_HLMFMT(lMPRFirstRecord.m_cnC_HLMFMT);

                    // Write original test limits if option enabled.
                    if(mPrivate->mSettings.bReportStdfOriginalLimits)
                    {
                        // LO_SPEC = Original Test Limits.
                        lNewRecord.SetLO_SPEC(lTest->GetCurrentLimitItem()->lfLowLimit);
                        // HI_SPEC = Original Test Limits.
                        lNewRecord.SetHI_SPEC(lTest->GetCurrentLimitItem()->lfHighLimit);
                    }
                    else
                    {
                        // LO_SPEC
                        if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposLO_SPEC))
                            lNewRecord.SetLO_SPEC(lMPRRecord.m_r4LO_SPEC);
                        else
                            lNewRecord.SetLO_SPEC(lMPRFirstRecord.m_r4LO_SPEC);

                        // HI_SPEC
                        if (lMPRRecord.IsFieldValid(GQTL_STDF::Stdf_MPR_V4::eposHI_SPEC))
                            lNewRecord.SetHI_SPEC(lMPRRecord.m_r4HI_SPEC);
                        else
                            lNewRecord.SetHI_SPEC(lMPRFirstRecord.m_r4HI_SPEC);
                    }

                    // Write record to disk
                    if (lNewRecord.Write(mPrivate->mOutputFile) == false)
                    {
                        mPrivate->mErrorMessage = "*PAT Error* Failed to write MPR record";
                        return false;
                    }

                    // Do not write original PTR record
                    lWrittenRecord = true;
                }
            }
        }
    }

    return true;
}

bool PATStdfUpdate::ProcessMRR(bool &lWrittenRecord)
{
    if(mPrivate->IsLastPass())
    {
        // Write PIR/PRR records for external map failures
        if (WriteExternalMapFailures() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to write External Map failures records");
            return false;
        }

        // Write DTR records (with PAT results) + TSR + HBR + SBR + WCR + PCR
        if (WritePATRecords() == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to write PAT records");
            return false;
        }

        // Checks if PAT running at ST-Microelectronics customer site
        bool lST_MicroPAT = (getenv(PATMAN_USER_ST) != NULL);	// eg: "GEX_PROMIS_DATA_PATH"

        // Record header
        GS::StdLib::StdfRecordReadInfo lRecordHeader;
        lRecordHeader.iRecordType       = 1;
        lRecordHeader.iRecordSubType    = 20;	// MRR

        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);

        switch(mPrivate->mInputFile.GetStdfVersion())
        {
        default :
            break;

            case GEX_STDFV4:
            {
                long lData;
                mPrivate->mInputFile.ReadDword(&lData);	// END_T

                if(mPrivate->mSettings.bUpdateTimeFields)
                {
                    if(lST_MicroPAT)
                        mPrivate->mOutputFile.WriteDword(time(NULL));	// Overwrite END_T
                    else
                        mPrivate->mOutputFile.WriteDword(time(NULL) + (mPrivate->mWaferEndT - mPrivate->mWaferStartT));		// Overwrite END_T
                }
                else
                    mPrivate->mOutputFile.WriteDword(lData);

                break;
            }
        }

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write MRR record";
            return false;
        }

        lWrittenRecord = true;
    }

    return true;
}

void PATStdfUpdate::ProcessPCR(bool &lWrittenRecord)
{
    if(mPrivate->IsLastPass())
    {
        // If no PAT failures, keep current PCR!
        if(mPrivate->mContext == NULL || (mPrivate->mContext->m_lstOutlierParts.count() == 0))
        {
            // Flag PCR written, no need to create custom one (with PAT failures)
            mPrivate->mPCRCreated   = true;
            lWrittenRecord          = false;
        }
        else
            // We've got some PAT failures so a new PCR will need to be written!
            lWrittenRecord = true;
    }
}

void PATStdfUpdate::ProcessPIR()
{
    unsigned int    lMapIndex;
    BYTE            lSite;
    CPatSite *      lPATSite = NULL;
    unsigned short siteNumber = 0;

    // PIR
    switch(mPrivate->mInputFile.GetStdfVersion())
    {
    default :
        break;

        case GEX_STDFV4:
            // Save tester head# (may be used in HBR/SBR if creating STDF output file)
            // head number
            mPrivate->mInputFile.ReadByte(&mPrivate->mTesterHead);
            // site number
            mPrivate->mInputFile.ReadByte(&lSite);

            GQTL_STDF::Stdf_PIR_V4 pir;
            pir.SetHEAD_NUM( mPrivate->mTesterHead );
            pir.SetSITE_NUM( lSite );

            siteNumber =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
                ( mPrivate->mInputFile, pir );

            mPrivate->mTesterHead =
                GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
                ( mPrivate->mInputFile, pir );

            // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
            lMapIndex = (mPrivate->mTesterHead << 16) | siteNumber;

            if (mPrivate->mPATSites.contains(lMapIndex) == false)
            {
                lPATSite = new CPatSite;
                mPrivate->mPATSites.insert(lMapIndex, lPATSite);
            }
            else
                lPATSite = mPrivate->mPATSites[lMapIndex];

            // Tells we enter in a PIR/PRR block
            lPATSite->bPirSeen = true;

            // Reset test count in run
            mPrivate->mTestPerSite[lSite] = 0;
            break;
    }
}

bool PATStdfUpdate::ProcessPRR(bool& lWrittenRecord)
{
    char            lTmpString[STDF_CN_STRING_MAX+1];
    BYTE            lSite;
    BYTE            lPartFlag;
    bool            lSavePatBinInfo         = false;
    bool            lOriginalGoodPart       = true;
    bool            lWaferSurfaceException  = false;
    bool            lIsOutlierPart          = false;
    int             lIntData;
    int             lSeekDieXY          = 0;
    int             lPartSoftBin        = 65535;
    int             lPartHardBin        = 65535;
    int             lDieX               = -32768;
    int             lDieY               = -32768;
    int             lSoftBin;
    int             lHardBin;
    unsigned int    lMapIndex;
    unsigned int    lFailType           = 0;
    long            lReadOffsetPartFlag = 0;
    long            lRecordReadOffset   = 0;
    QString         lPATFailFamily      = "PPAT";
    CPatSite *      lPATSite            = NULL;

    // Keep track of the PRR number (part #)
    ++mPrivate->mRunID;

    mPrivate->mInputFile.ReadByte(&mPrivate->mTesterHead);      // head number
    mPrivate->mInputFile.ReadByte(&lSite);                      // site number

    GQTL_STDF::Stdf_PRR_V4 prr;
    prr.SetHEAD_NUM(mPrivate->mTesterHead);
    prr.SetSITE_NUM(lSite);

    unsigned short siteNumber =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn
        (mPrivate->mInputFile, prr);

    mPrivate->mTesterHead =
        GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn
        (mPrivate->mInputFile, prr);

    // Check if run without datalog samples
    if(mPrivate->mTestPerSite[siteNumber] == 0)
        mPrivate->mContext->lPartsWithoutDatalog++;

    // Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR
    lMapIndex = (mPrivate->mTesterHead << 16) | siteNumber;

    if (mPrivate->mPATSites.contains(lMapIndex))
        lPATSite = mPrivate->mPATSites[lMapIndex];

    if (lPATSite == NULL || lPATSite->bPirSeen == false)
    {
        mPrivate->mErrorMessage = "PIR/PRR mismatch for head_num";
        GSLOG(SYSLOG_SEV_ERROR, mPrivate->mErrorMessage.toLatin1().constData());
        return false;
    }

    // Clear the flag now that we've seen the PRR
    lPATSite->bPirSeen = false;

    // This test flow failed PAT limits...then write the new PRR record.
    GS::StdLib::StdfRecordReadInfo lRecordHeader;

    lRecordHeader.iRecordType       = 5;
    lRecordHeader.iRecordSubType    = 20;

    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
    mPrivate->mOutputFile.WriteByte(mPrivate->mTesterHead);     // Head
    mPrivate->mOutputFile.WriteByte(lSite);                     // Site

    switch(mPrivate->mInputFile.GetStdfVersion())
    {
    default :
        break;
    case GEX_STDFV4:
        // Offset to seek from the HardBin to the DieX field
        lSeekDieXY = 2;
        // Saves current READ offset: part_flg flag part: pass/fail info)
        lReadOffsetPartFlag = mPrivate->mOutputFile.GetWriteRecordPos();
        // part_flag
        mPrivate->mInputFile.ReadByte(&lPartFlag);
        // For now, keep default PAss/Fail flag (changed later in this code).
        mPrivate->mOutputFile.WriteByte(lPartFlag);

        if(lPartFlag & 8)
            lOriginalGoodPart = false;
        else
            lOriginalGoodPart = true;
        break;
    }

    // NUM_TEST
    mPrivate->mInputFile.ReadWord(&lIntData);
    mPrivate->mOutputFile.WriteWord(lIntData);

    // Hard Bin
    mPrivate->mInputFile.ReadWord(&lHardBin);
    lPartHardBin = lHardBin;

    // If good part and Wafermap surface control enabled (GDBN or Reticle or Clustering...), check it!
    CPatDieCoordinates * lBadBin = NULL;

    if(mPrivate->IsLastPass())
    {
        // Get DieX, DieY position
        // Saves current READ offset: pointing softbin
        lRecordReadOffset = mPrivate->mInputFile.GetReadRecordPos();

        if(mPrivate->mInputFile.SetReadRecordPos(lRecordReadOffset + lSeekDieXY) == GS::StdLib::Stdf::NoError)
        {
            mPrivate->mInputFile.ReadWord(&lDieX);	// X_COOR
            if(mPrivate->mInputFile.ReadWord(&lDieY) == GS::StdLib::Stdf::NoError)
            {
                // Convert Die coordinates from unsigned to signed
                if(lDieX >= 32768)
                    lDieX -= 65536;

                if(lDieY >= 32768)
                    lDieY -= 65536;
            }
        }

        if(mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList->Contains(lHardBin) &&
           (mPrivate->mContext->GetRecipeOptions().mGDBNRules.count() > 0 || mPrivate->mContext->GetRecipeOptions().GetReticleEnabled() ||
            mPrivate->mContext->GetRecipeOptions().IsNNREnabled() || mPrivate->mContext->GetRecipeOptions().mClusteringPotato ||
            mPrivate->mContext->GetRecipeOptions().mIsIDDQ_Delta_enabled ||
            (mPrivate->mContext->GetRecipeOptions().GetExclusionZoneEnabled() && mPrivate->mContext->GetRecipeOptions().lfCompositeExclusionZoneYieldThreshold > 0.0)))
        {
            if(lDieX != -32768 && lDieY != -32768)
            {
                QString lKey = QString::number(lDieX) + "." + QString::number(lDieY);

                // Checlo for GDBN rule exception
                if(mPrivate->mContext->mGDBNOutliers.contains(lKey))
                {
                    CPatDieCoordinates lOutlier = mPrivate->mContext->mGDBNOutliers.value(lKey);

                    // This a GDBN rejected good die
                    lSoftBin    = lOutlier.mPatSBin;    // This is a PASS die that we must fail because of too many bad neighbours
                    lHardBin    = lOutlier.mPatHBin;    // This is a PASS die that we must fail because of too many bad neighbours
                    lFailType   = lOutlier.mFailType;   // Exception type
                    lPATFailFamily          = "GDBN";
                    lWaferSurfaceException  = true;	// Flag binning overloading
                }

                // Check for NNR rule exception
                if(!lWaferSurfaceException)
                {
                    if(mPrivate->mContext->mNNROutliers.contains(lKey))
                    {
                        CPatDieCoordinates lOutlier = mPrivate->mContext->mNNROutliers.value(lKey);

                        // This a NNR rejected good die
                        lSoftBin    = lOutlier.mPatSBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lHardBin    = lOutlier.mPatHBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lFailType   = lOutlier.mFailType;   // Exception type
                        lPATFailFamily          = "NNR";
                        lWaferSurfaceException  = true;	// Flag binning overloading
                    }
                }

                // Check for IDDQ-Delta rule exception
                if(!lWaferSurfaceException)
                {
                    if(mPrivate->mContext->mIDDQOutliers.contains(lKey))
                    {
                        CPatDieCoordinates lOutlier = mPrivate->mContext->mIDDQOutliers.value(lKey);

                        // This a IDDQ-Delta rejected good die
                        lSoftBin    = lOutlier.mPatSBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lHardBin    = lOutlier.mPatHBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lFailType   = lOutlier.mFailType;   // Exception type
                        lPATFailFamily          = "IDDQ-Delta";
                        lWaferSurfaceException  = true;	// Flag binning overloading
                    }
                }

                // Check for Reticle rule exception
                if(!lWaferSurfaceException)
                {
                    if(mPrivate->mContext->mReticleOutliers.contains(lKey))
                    {
                        CPatDieCoordinates lOutlier = mPrivate->mContext->mReticleOutliers.value(lKey);

                        // This a Reticle rejected good die
                        lSoftBin    = lOutlier.mPatSBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lHardBin    = lOutlier.mPatHBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lFailType   = lOutlier.mFailType;   // Exception type
                        lPATFailFamily          = "Reticle";
                        lWaferSurfaceException  = true;	// Flag binning overloading
                    }
                }

                // Check for Clustering 'potato' rule exception
                if(!lWaferSurfaceException)
                {
                    if(mPrivate->mContext->mClusteringOutliers.contains(lKey))
                    {
                        CPatDieCoordinates lOutlier = mPrivate->mContext->mClusteringOutliers.value(lKey);

                        // This a Cluster rejected good die
                        lSoftBin    = lOutlier.mPatSBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lHardBin    = lOutlier.mPatHBin;    // This is a PASS die that we must fail because of too many bad neighbours
                        lFailType   = lOutlier.mFailType;   // Exception type
                        lPATFailFamily          = "Cluster";
                        lWaferSurfaceException  = true;		// Flag binning overloading
                    }
                }

                // Check for ZPAT rule exception
                if(!lWaferSurfaceException)
                {
                    if(mPrivate->mContext->mZPATOutliers.contains(lKey))
                    {
                        // This a ZPAT rejected good die
                        lSoftBin    = mPrivate->mContext->GetRecipeOptions().iCompositeZone_SBin;	// This is a PASS die that we must fail: Z-PAT failure
                        lHardBin    = mPrivate->mContext->GetRecipeOptions().iCompositeZone_HBin;	// This is a PASS die that we must fail: Z-PAT failure
                        lFailType   = mPrivate->mContext->mZPATOutliers.value(lKey).mFailType;	// Exception type
                        lPATFailFamily          = "ZPAT";
                        lWaferSurfaceException  = true;	// Flag binning overloading
                    }
                }
            }
        }

        // Make sure we're at the right STDF location to read the next field.
        mPrivate->mInputFile.SetReadRecordPos(lRecordReadOffset);
    }

    // If we have a wafer binning overload (eg: GDBN, Reticle, Clustering rule)
    if(lWaferSurfaceException)
    {
        // New Hard bin & Soft Bin!
        mPrivate->mOutputFile.WriteWord(lHardBin);
        mPrivate->mOutputFile.WriteWord(lSoftBin);

        // Update Hard bin summary
        mPrivate->mContext->IncrementPATHardBins(lHardBin, lFailType);

        // Update Soft bin summary
        mPrivate->mContext->IncrementPATSoftBins(lSoftBin, lFailType);

        // Rewind just after the Hardbin & Softbin fields
        mPrivate->mInputFile.SetReadRecordPos(lRecordReadOffset + 2);

        // Flag this is a outlier part and as Failed
        lIsOutlierPart  = true;
    }
    else
    {
        bool lMVPatException = false;

        if(mPrivate->IsLastPass() && lDieX != -32768 && lDieY != -32768)
        {
            if (mPrivate->mContext->IsMVOutlier(GS::Gex::WaferCoordinate(lDieX, lDieY)))
            {
                QString lKey = QString::number(lDieX) + "." + QString::number(lDieY);

                GS::Gex::PATMVOutlier lMVOutlier = mPrivate->mContext->GetMVOutliers().value(lKey);

                // New Hard bin & Soft Bin!
                mPrivate->mOutputFile.WriteWord(lMVOutlier.GetHardBin());
                mPrivate->mOutputFile.WriteWord(lMVOutlier.GetSoftBin());

                // Update Hard bin summary
                mPrivate->mContext->IncrementPATHardBins(lMVOutlier.GetHardBin(), GEX_TPAT_BINTYPE_MVPAT);

                // Update Soft bin summary
                mPrivate->mContext->IncrementPATSoftBins(lMVOutlier.GetSoftBin(), GEX_TPAT_BINTYPE_MVPAT);

                // Rewind just after the Hardbin & Softbin fields
                mPrivate->mInputFile.SetReadRecordPos(lRecordReadOffset + 2);

                // Flag this is a outlier part and as Failed
                lIsOutlierPart  = true;
                lMVPatException = true;

                // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
                mPrivate->mContext->OverloadRefWafermap(lDieX, lDieY, lMVOutlier.GetSoftBin(), lMVOutlier.GetHardBin());
            }
        }

        // No MV PAT failure for this die, check for PPAT failure
        if (lMVPatException == false)
        {
            // Fail parts that have at least the minimum number of outliers required
            if(lPATSite->lOutliersFound < mPrivate->mContext->GetRecipeOptions().iMinimumOutliersPerPart)
                mPrivate->mOutputFile.WriteWord(lHardBin);
            else
            {
                // Fail part: either it was Good bin, or we have the option to overwrite any bin !
                if(mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList->Contains(lHardBin) ||
                   (mPrivate->mContext->GetRecipeOptions().bScanGoodPartsOnly == false))
                {
                    // Will save this part info as PAT-Bined part
                    lSavePatBinInfo = true;

                    // Only done during last pass...
                    if(mPrivate->IsLastPass())
                    {
                        // Clear variable to avoid saving it twice (during 1st pass, and last pass)
                        if(mPrivate->mPassCurrent != 1)
                            lSavePatBinInfo = false;	// if multi-pass mode, then only save Parts failing PAT during last pass!

                        // Overload HardBin
                        mPrivate->mOutputFile.WriteWord(lPATSite->lPatBin);

                        // Keep track of total Parts failing in this PatBin.
                        mPrivate->mContext->IncrementPATHardBins(lPATSite->lPatBin, lPATSite->bFailType);

                        // Flag this is a outlier part
                        lIsOutlierPart = true;
                    }
                    else
                        // Keep original failing bin
                        mPrivate->mOutputFile.WriteWord(lHardBin);
                }
                else
                    // Keep original failing bin
                    mPrivate->mOutputFile.WriteWord(lHardBin);
            }

            // Soft Bin
            mPrivate->mInputFile.ReadWord(&lSoftBin);
            lPartSoftBin = lSoftBin;

            // Fail parts that have at least the minimum number of outliers required
            if(lPATSite->lOutliersFound < mPrivate->mContext->GetRecipeOptions().iMinimumOutliersPerPart)
                mPrivate->mOutputFile.WriteWord(lSoftBin);
            else
            {

                if(mPrivate->IsLastPass() &&
                   (mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList->Contains(lPartHardBin) ||
                    (mPrivate->mContext->GetRecipeOptions().bScanGoodPartsOnly == false)))
                {
                    mPrivate->mOutputFile.WriteWord(lPATSite->lPatBin);

                    // Keep track of total Parts failing in this PatBin.
                    mPrivate->mContext->IncrementPATSoftBins(lPATSite->lPatBin,
                                                   lPATSite->bFailType);

                    // Will save this part info as PAT-Bined part
                    lSavePatBinInfo = true;

                    // Flag this is a outlier part
                    lIsOutlierPart = true;
                }
                else
                    // Keep original failing bin
                    mPrivate->mOutputFile.WriteWord(lSoftBin);
            }
        }
    }

    switch(mPrivate->mInputFile.GetStdfVersion())
    {
    default :
        break;

    case GEX_STDFV4:

        // Saves current READ offset: part_flg flag part: pass/fail info)
        lRecordReadOffset = mPrivate->mOutputFile.GetWriteRecordPos();
        // seek to: part_flag
        mPrivate->mOutputFile.SetWriteRecordPos(lReadOffsetPartFlag);

        if(lIsOutlierPart)
            lPartFlag |= 8;	// Flags that part failed
        mPrivate->mOutputFile.WriteByte(lPartFlag);

        // Seek to X_COOR field
        mPrivate->mOutputFile.SetWriteRecordPos(lRecordReadOffset);

        break;
    }

    // X_COOR
    if(mPrivate->mInputFile.ReadWord(&lDieX) == GS::StdLib::Stdf::NoError)
        mPrivate->mOutputFile.WriteWord(lDieX);

    // Y_COOR
    if(mPrivate->mInputFile.ReadWord(&lDieY) == GS::StdLib::Stdf::NoError)
        mPrivate->mOutputFile.WriteWord(lDieY);

    // Convert Die coordinates from unsigned to signed
    if(lDieX >= 32768)
        lDieX -= 65536;

    if(lDieY >= 32768)
        lDieY -= 65536;

    switch(mPrivate->mInputFile.GetStdfVersion())
    {
    default :
            break;
        case GEX_STDFV4:
            // TEST_T
        {
            long lData;
            if(mPrivate->mInputFile.ReadDword(&lData) == GS::StdLib::Stdf::NoError)
                mPrivate->mOutputFile.WriteDword(lData);
            break;
        }
    }

    // PART_ID
    QString lStringPartID;
    long    lNumericPartID;
    long    lInternalPartID;
    bool    lIsNumerical = false;
    if(mPrivate->mInputFile.ReadString(lTmpString) == GS::StdLib::Stdf::NoError)
    {
        mPrivate->mOutputFile.WriteString(lTmpString);
        lStringPartID = lTmpString;
    }
    else
    {
        mPrivate->mOutputFile.WriteString(lTmpString);

        // No PartID field, then take part# instead (run # in data file)
        lStringPartID = QString::number(mPrivate->mRunID);
    }

    // Case 5903 [HT]: At final test, we need to create virtual X,Y coordinates to be able to manage retest on the
    // strip map
    lNumericPartID = lStringPartID.toLong(&lIsNumerical);

    if (mPrivate->mMapPartID.contains(lStringPartID))
      lInternalPartID = mPrivate->mMapPartID.value(lStringPartID);
    else
    {
      lInternalPartID = mPrivate->mCurrentInternalPartID++;

      if (lStringPartID.isEmpty() == false)
        mPrivate->mMapPartID.insert(lStringPartID, lInternalPartID);
    }

    // Update partID list into results list (if PartID list is enabled)
    if(lIsNumerical == false)
      lNumericPartID = lInternalPartID;	// If no PartID defined, then force it to run# instead.

    if (lStringPartID.isEmpty())
      lStringPartID = QString::number(lNumericPartID);

    // Invalid or No Die location available...if we have to create a StripMap, then build location!
    if(mPrivate->mContext->m_AllSitesMap_Sbin.bStripMap && ((lDieX == -32768) || (lDieY == -32768)))
    {
        lDieX = lNumericPartID % mPrivate->mContext->m_AllSitesMap_Sbin.SizeX;
        lDieY = mPrivate->mContext->m_AllSitesMap_Sbin.iHighDieY - (lNumericPartID / mPrivate->mContext->m_AllSitesMap_Sbin.SizeX);
    }

    // PART_TXT
    mPrivate->mInputFile.ReadString(lTmpString);

    QString lPartTxt = lTmpString;

    // If STDF to hold PAT details, add PAT failure type in PartTXT!
    if(mPrivate->mSettings.bEmbedStdfReport && lIsOutlierPart)
    {
        lPartTxt += " gexPatFail: ";

        if(!lWaferSurfaceException)
        {
            // PPAT
            CPatFailingTest     lFailTest;
            CPatDefinition *    lPatDef    = NULL;

            lPartTxt += lPATFailFamily;

            // Get Rule type (only if one failure in flow)
            if(lPATSite->cOutlierList.count() > 0)
            {
                lFailTest   = lPATSite->cOutlierList.first();
                lPatDef     = mPrivate->mContext->GetPatDefinition(lFailTest.mTestNumber, lFailTest.mPinIndex,
                                                         lFailTest.mTestName);
                lPartTxt    += " : " +
                               (lPatDef ? QString(gexRuleSetItemsGUI[lPatDef->mOutlierRule]) : "");
            }
        }
        else
            lPartTxt += lPATFailFamily + " : ";

        if(lBadBin)
            lPartTxt += lBadBin->mRuleName;

        // Ensure line is not exceeding STDF specs!
        lPartTxt.truncate(255);

        // New PART_TXT
        mPrivate->mOutputFile.WriteString(lPartTxt.toLatin1().data());
    }
    else
        mPrivate->mOutputFile.WriteString(lTmpString);

    // PART_FIX
    if(mPrivate->mInputFile.ReadString(lTmpString) == GS::StdLib::Stdf::NoError)
        mPrivate->mOutputFile.WriteString(lTmpString);

    // If last pass: Write record to disk
    if(mPrivate->IsLastPass())
    {
        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write PRR record";
            return false;
        }

        lWrittenRecord = true;
    }

    if(lSavePatBinInfo)
    {
        // If no die coordinates (final test), then map to zero die location...
        // but PAT-Man is not for final test anyway!
        if(lDieX == -32768 && lDieY == -32768)
            lDieX = lDieY = 0;

        // Save list of outliers in this run so the report can detail them.
        CPatOutlierPart * lOutlierPart = new CPatOutlierPart;
        lOutlierPart->cOutlierList  = lPATSite->cOutlierList;
        lOutlierPart->strPartID     = lStringPartID;
        lOutlierPart->iDieX         = lDieX;
        lOutlierPart->iDieY         = lDieY;
        lOutlierPart->lRunID        = mPrivate->mRunID;
        lOutlierPart->iSite         = siteNumber;
        lOutlierPart->iPatHBin      = lPATSite->lPatBin;
        lOutlierPart->iPatSBin      = lPATSite->lPatBin;
        lOutlierPart->iOrgSoftbin   = lPartSoftBin;
        lOutlierPart->iOrgHardbin   = lPartHardBin;
        lOutlierPart->lfPatScore    = lOutlierPart->getDevicePatScore();	// Compute PAT criticity score, and save it
        lOutlierPart->mRetestIndex  = 0;

        // Remove previous instance: can occur if a wafer is forced to be retested on good bins.
        mPrivate->mContext->CleanPATDieAlarms(lDieX, lDieY, mPrivate->IsLastPass());

        // append this new failing part to our list of outlier parts (structure used for creating the report outlier table)
        mPrivate->mContext->m_lstOutlierParts.append(lOutlierPart);

        // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
        mPrivate->mContext->OverloadRefWafermap(lDieX, lDieY, lOutlierPart->iPatSBin, lOutlierPart->iPatHBin);

    }
    else
    {
        // If Die PASS, ensure to delete all previous failures detected for this die (eg: in case of retests)
        if(lIsOutlierPart == false)
        {
            if (mPrivate->mContext->CleanPATDieAlarms(lDieX, lDieY, mPrivate->IsLastPass()))
                // ensure we overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
                mPrivate->mContext->OverloadRefWafermap(lDieX, lDieY, lPartSoftBin, lPartHardBin);
        }
    }

    // Update Binning 'P' / 'F' type
    mPrivate->mFile->setBinningType(true, lPartSoftBin, lOriginalGoodPart);
    mPrivate->mFile->setBinningType(false, lPartHardBin, lOriginalGoodPart);

    lPATSite->lPatBin           = -1;
    lPATSite->lOutliersFound    = 0;
    lPATSite->bFailType         = 0;
    lPATSite->cOutlierList.clear();	// Empty list of failing tests in this run.

    return true;
}

bool PATStdfUpdate::ProcessPTR(bool& lWrittenRecord)
{   
    char                lAlarmId[STDF_CN_STRING_MAX+1];
    char                lTestText[STDF_CN_STRING_MAX+1];
    char                lTmpString[STDF_CN_STRING_MAX+1];
    unsigned int        lMapIndex;
    BYTE                lByteData;
    BYTE                lHead;
    BYTE                lSite;
    BYTE                lOptFlag;
    BYTE                lParamFlag;
    BYTE                lTestFlag;
    bool                lCreateNewPTR = false;	// Set to 'true' if need to write a new PTR (with new pass/fail flag and limits)
    float               lResult;
    float               lFloatData;
    long                lTestNumber;
    long                lLongData;
    long                lPos;
    CTest  *            lTest       = NULL;
    CPatSite *          lPATSite    = NULL;
    CPatDefinition *    lPatDef     = NULL;

    mPrivate->mInputFile.ReadDword(&lLongData);         // test number
    lTestNumber = (unsigned int) lLongData;

    mPrivate->mInputFile.ReadByte(&lHead);              // Head
    mPrivate->mInputFile.ReadByte(&lSite);              // test site

    GQTL_STDF::Stdf_PTR_V4 ptr;
    ptr.SetHEAD_NUM(lHead);
    ptr.SetSITE_NUM(lSite);

    unsigned short siteNumber = GQTL_STDF::HeadAndSiteNumberDecipher::GetSiteNumberIn(mPrivate->mInputFile, ptr);

    lHead = GQTL_STDF::HeadAndSiteNumberDecipher::GetHeadNumberIn(mPrivate->mInputFile, ptr);

    lPos = mPrivate->mInputFile.GetReadRecordPos();		// Save offset to 'test_flg' flag

    mPrivate->mInputFile.ReadByte(&lTestFlag);			// test_flg
    mPrivate->mInputFile.ReadByte(&lParamFlag);			// parm_flg
    mPrivate->mInputFile.ReadFloat(&lResult);           // test result
    mPrivate->mInputFile.ReadString(lTestText);         // test_txt
    mPrivate->mInputFile.ReadString(lAlarmId);			// alrm_id

    lOptFlag = 0xFF;
    mPrivate->mInputFile.ReadByte(&lOptFlag);// != GS::StdLib::Stdf::NoError)	// opt_flg

    // Keep track of total tests in run
    mPrivate->mTestPerSite[siteNumber]++;

    // Check if limits defined: if no, need to overload with PAT limits
    if(((lOptFlag & 0x10) == 0) || ((lOptFlag & 0x20) == 0))
        lCreateNewPTR = true;

    // Check if test result is valid
    if((lCreateNewPTR == false) && ((lTestFlag & 02) || (lTestFlag & 020)))
        return true;	// Invalid result: Keep original record (only because no limits defined!)

    // Check if this PTR is within a valid PIR/PRR block.
    lMapIndex = (lHead << 16) | siteNumber;	// Compute index for Head&Site...as in multi-head/sites, we have to save multiple results before the PRR

    if(mPrivate->mPATSites.contains(lMapIndex))
        lPATSite = mPrivate->mPATSites.value(lMapIndex);

    if (mPrivate->mSTDFCompliancy.compare("stringent") == 0)
    {
        if (lPATSite == NULL || lPATSite->bPirSeen == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, "PIR/PTR mismatch for head_num, this PTR will be ignored");
            return true;
        }
    }

    if (mPrivate->mFile && mPrivate->mContext->GetSiteList().indexOf(siteNumber) >= 0)
    {
        bool lOutlier = false;

        // Returns pointer to correct cell. If cell doesn't exist ; its created. Test# mapping enabled
        if(mPrivate->mFile->FindTestCell(lTestNumber, GEX_PTEST, &lTest) !=1)
            return true;	// Keep original record

        if(lTest->ldSamplesValidExecs >= mPrivate->mContext->GetRecipeOptions().iMinimumSamples)
        {
            // Get the Pat definition for this test
            lPatDef = mPrivate->mContext->GetPatDefinition(lTest->lTestNumber,
                                                           lTest->lPinmapIndex,
                                                           lTest->strTestName);

            lOutlier = CheckParameterValue(lPatDef, siteNumber, *lTest, lResult,
                                           lPATSite, lWrittenRecord);

            if (!lWrittenRecord && mPrivate->IsLastPass())
            {
                // Check if test result is already fail (if so don't look for a outlier)
                if((lTestFlag & 0300) == 0200)
                    lOutlier = false;	// Test is fail, so ignore if it is also an outlier!

                // Test failing PAT limits. Flag it!
                if(lOutlier)
                {
                    lTestFlag &= 077;	// Clear Bit7 (PASS flag)
                    lTestFlag |= 0200;	// Tells 'PASS/FAIL' flag is valid

                    // Overwrite PASS/FAIL flag in original record if no new record (with new limits) needed
                    if(lCreateNewPTR == false)
                    {
                        long	lCurrentPos = mPrivate->mInputFile.GetReadRecordPos();		// Save current offset position in read buffer
                        mPrivate->mInputFile.SetReadRecordPos(lPos);						// Force offset to point over 'test_flg' byte
                        mPrivate->mInputFile.OverwriteReadByte(lTestFlag);					// Overwrite test_flg
                        mPrivate->mInputFile.SetReadRecordPos(lCurrentPos);					// Revert read-buffer offset position.
                    }
                }

                if (lPatDef)
                {
                    GS::PAT::DynamicLimits lDynLimits = lPatDef->GetDynamicLimits(siteNumber);

                    // If first outlier instance for this test# (on this site), ensure we write the PAT limits.
                    if(lPatDef->m_NewPTR_PerSite[siteNumber] != 1)
                    {
                        // Force to write PTR with PAT limits
                        lCreateNewPTR = true;
                        lPatDef->m_NewPTR_PerSite[siteNumber] = 1;
                    }

                    // Rewrite PTR with PAT limits.
                    if(lCreateNewPTR)
                    {
                        // Extract PAT limits involved
                        float	lLowLimit;
                        float   lHighLimit;

                        if(lPatDef->m_lFailDynamicBin < 0)
                        {
                            // Dynamic PAT disabled
                            if(lPatDef->m_lFailStaticBin < 0)
                            {
                                // PAT fully disabled, keep standard limits
                                lLowLimit   = lTest->GetCurrentLimitItem()->lfLowLimit;
                                lHighLimit  = lTest->GetCurrentLimitItem()->lfHighLimit;
                            }
                            else
                            {
                                // Only STATIC PAT is enabled for this test
                                lLowLimit  = lPatDef->m_lfLowStaticLimit;
                                lHighLimit = lPatDef->m_lfHighStaticLimit;
                            }
                        }
                        else
                        {
                            // Dynamic PAT is enabled
                            lLowLimit  = gex_min(lDynLimits.mLowDynamicLimit1[lPatDef->m_iOutlierLimitsSet],
                                                 lDynLimits.mLowDynamicLimit2[lPatDef->m_iOutlierLimitsSet]);
                            lHighLimit = gex_max(lDynLimits.mHighDynamicLimit1[lPatDef->m_iOutlierLimitsSet],
                                                 lDynLimits.mHighDynamicLimit2[lPatDef->m_iOutlierLimitsSet]);
                        }

                        GS::StdLib::StdfRecordReadInfo lRecordHeader;

                        lRecordHeader.iRecordType       = 15;
                        lRecordHeader.iRecordSubType    = 10;

                        // Ensure 'Valid Test limits' flag is set
                        lOptFlag &= 0xf;	// Clear top 4 bits = valid limits.

                        // Check if must include original test limits...
                        if(mPrivate->mSettings.bReportStdfOriginalLimits)
                            lOptFlag &= ~(014);	// ensure relevant flags in right place (clear BIT2 and 3)

                        // Check if LL or HL is infinite
                        if(lLowLimit <= -3.4e+38F || lLowLimit >= 3.4e+38F)
                            lOptFlag |= 0100; // bit 6 set = no Low limit
                        if(lHighLimit >= 3.4e+38F || lHighLimit <= -3.4e+38F)
                            lOptFlag |= 0200; // bit 7 set = no High limit

                        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                        mPrivate->mOutputFile.WriteDword(lTestNumber);          // Test#
                        mPrivate->mOutputFile.WriteByte(lHead);                 // Head
                        mPrivate->mOutputFile.WriteByte(lSite);                 // Site
                        mPrivate->mOutputFile.WriteByte(lTestFlag);				// Test_flg
                        mPrivate->mOutputFile.WriteByte(lParamFlag);			// param_flg
                        mPrivate->mOutputFile.WriteFloat(lResult);              // test_result
                        mPrivate->mOutputFile.WriteString(lTestText);			// test_txt
                        mPrivate->mOutputFile.WriteString(lAlarmId);			// alrm_id

                        CTest* lTest;
                        mPrivate->mFile->FindTestCell(lTestNumber, -1, &lTest);


                        // RES_SCAL
                        if(mPrivate->mInputFile.ReadByte(&lByteData) != GS::StdLib::Stdf::NoError)
                        {
                            if (lTest)
                            {
                                lByteData = lTest->res_scal;
                                lOptFlag = lOptFlag & 0xFE;
                            }
                            else
                                lByteData = 0;
                        }
                        if(lByteData > 20)
                            lByteData = 0;	// Just in case invalid scale factor found in original file
                        mPrivate->mOutputFile.WriteByte(lOptFlag);				// opt_flag
                        mPrivate->mOutputFile.WriteByte(lByteData);
                        // LLM_SCAL
                        if(mPrivate->mInputFile.ReadByte(&lByteData) != GS::StdLib::Stdf::NoError)
                        {
                            if (lTest)
                            {
                                lByteData = lTest->llm_scal;
                            }
                            else
                                lByteData = 0;
                        }
                        if(lByteData > 20)
                            lByteData = 0;	// Just in case invalid scale factor found in original file
                        mPrivate->mOutputFile.WriteByte(lByteData);
                        // HLM_SCAL
                        if(mPrivate->mInputFile.ReadByte(&lByteData) != GS::StdLib::Stdf::NoError)
                        {
                            if (lTest)
                                lByteData = lTest->hlm_scal;
                            else
                                lByteData = 0;
                        }
                        if(lByteData > 20)
                            lByteData = 0;	// Just in case invalid scale factor found in original file
                        mPrivate->mOutputFile.WriteByte(lByteData);
                        // LO_LIMIT
                        mPrivate->mInputFile.ReadFloat(&lFloatData);
                        mPrivate->mOutputFile.WriteFloat(lLowLimit);
                        // HI_LIMIT
                        mPrivate->mInputFile.ReadFloat(&lFloatData);
                        mPrivate->mOutputFile.WriteFloat(lHighLimit);
                        // UNITS
                        mPrivate->mInputFile.ReadString(lTmpString);
                        mPrivate->mOutputFile.WriteString(lTmpString);


                        // C_RESFMT,xxxFMT
                        mPrivate->mInputFile.ReadString(lTmpString);
                        mPrivate->mOutputFile.WriteString(lTmpString);	// ANSI C result format string
                        mPrivate->mInputFile.ReadString(lTmpString);
                        mPrivate->mOutputFile.WriteString(lTmpString);	// ANSI C low limit format string
                        mPrivate->mInputFile.ReadString(lTmpString);
                        mPrivate->mOutputFile.WriteString(lTmpString);	// ANSI C high limit format string


                        // Write original test limits if option enabled.
                        if(mPrivate->mSettings.bReportStdfOriginalLimits)
                        {
                            // LO_SPEC = Original Test Limits.
                            mPrivate->mOutputFile.WriteFloat(lTest->GetCurrentLimitItem()->lfLowLimit);
                            // HI_SPEC = Original Test Limits.
                            mPrivate->mOutputFile.WriteFloat(lTest->GetCurrentLimitItem()->lfHighLimit);
                        }
                        else
                        {
                            // -- rewrite lo_spec
                            mPrivate->mInputFile.ReadFloat(&lFloatData);
                            mPrivate->mOutputFile.WriteFloat(lFloatData);
                            // -- rewrite hi_spec
                            mPrivate->mInputFile.ReadFloat(&lFloatData);
                            mPrivate->mOutputFile.WriteFloat(lFloatData);
                        }


                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = "*PAT Error* Failed to write PTR record";
                            return false;
                        }

                        // Do not write original PTR record
                        lWrittenRecord = true;
                    }
                }
            }
        }
    }

    return true;
}

bool PATStdfUpdate::WriteATR()
{
    GS::StdLib::StdfRecordReadInfo lRecordHeader;

    // ATR record
    lRecordHeader.iRecordType       = 0;
    lRecordHeader.iRecordSubType    = 20;

    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
    mPrivate->mOutputFile.WriteDword(time(NULL));		// MOD_TIME: current time & date.
    mPrivate->mOutputFile.WriteString(Engine::GetInstance().Get("AppFullName").toString()
                                      .toLatin1().data());	// ATR string.

    // Write record to disk
    return (mPrivate->mOutputFile.WriteRecord() == GS::StdLib::Stdf::NoError);
}

bool PATStdfUpdate::WriteFAR()
{
    GS::StdLib::StdfRecordReadInfo lRecordHeader;
    lRecordHeader.iRecordType       = 0;
    lRecordHeader.iRecordSubType    = 10;

    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
    mPrivate->mOutputFile.WriteByte(mPrivate->mInputFile.GetStdfCpuType());
    mPrivate->mOutputFile.WriteByte(4);							// STDF V4

    return (mPrivate->mOutputFile.WriteRecord() == GS::StdLib::Stdf::NoError);
}

bool PATStdfUpdate::WritePATBinRecords()
{
    if (mPrivate->IsLastPass() && mPrivate->mBRCreated == false)
    {
        if (WritePATSBR() == false)
            return false;

        if (WritePATHBR() == false)
            return false;

        mPrivate->mBRCreated = true;
    }

    return true;
}

///////////////////////////////////////////////////////////
// Return total SoftBin records for a given bin#
// Note: this function handles dies-retest so to only count last occurance.
///////////////////////////////////////////////////////////
long PATStdfUpdate::GetTotalBinCount(bool isSoftBin, int lBin, int lSite)
{
    // Parameters for die X, Y coordinates and tes tre
    CTest * lTestBin   = NULL;
    CTest * lTestDieX  = NULL;
    CTest * lTestDieY  = NULL;

    if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX, GEX_PTEST, &lTestDieX, true, false) != 1)
        return 0;

    if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY, GEX_PTEST, &lTestDieY, true, false) != 1)
        return 0;

    // Create site filter if resquested
    PATPartFilter lPATPartFilter;
    if (lSite >= 0)
    {
        CTest * lTestSite  = NULL;

        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE, GEX_PTEST, &lTestSite, true, false) == 1)
        {
           lPATPartFilter.AddFilterElement(PATPartFilterElement(lTestSite, PATPartFilterElement::InFilter,
                                                                QtLib::Range(QString::number(lSite))));
        }
    }

    // Get handle to Bin results.
    if(isSoftBin)
    {
        // Looking at SoftBin
        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN, GEX_PTEST, &lTestBin, true, false) != 1)
            return 0;
    }
    else
    {
        // Looking at HardBin
        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN, GEX_PTEST, &lTestBin, true, false) != 1)
            return 0;
    }

    // Check if No die info available
    if((lTestDieX->m_testResult.count() == 0) || (lTestDieY->m_testResult.count() == 0))
        return 0;

    // Count total occurance of given binning (only count latest retest)
    QMap<QString,int>   lWafermapBin;
    QString             lDieLocation;
    long                lOffset;

    int lSamplesExecs = gex_min(lTestDieX->m_testResult.count(), lTestDieY->m_testResult.count());
    lSamplesExecs = gex_min(lSamplesExecs, lTestBin->m_testResult.count());

    for(lOffset = 0; lOffset < lSamplesExecs; lOffset++)
    {
        // Build die location key
        lDieLocation = QString::number(lTestDieX->m_testResult.resultAt(lOffset)) + "-" + QString::number(lTestDieY->m_testResult.resultAt(lOffset));

        // Check if we've reached a binning of interest
        if(lPATPartFilter.IsFiltered(lOffset) && lBin == (int) lTestBin->m_testResult.resultAt(lOffset))
            lWafermapBin[lDieLocation] = 1;		// Flag this die has a binning we want to count
        else
            lWafermapBin.remove(lDieLocation);	// Ensure we do NOT count dies with a bin# we don't want to count.
    }

    // Return total occurances of given Bin
    return lWafermapBin.count();
}

bool PATStdfUpdate::WriteBinRecords(int lBinType)
{
    if(mPrivate->IsLastPass())
    {
        GS::StdLib::StdfRecordReadInfo  lRecordHeader;
        const GS::QtLib::Range *        lBinRange   = NULL;

        // Summary record: SBR/HBR
        lRecordHeader.iRecordType = 1;

        switch(lBinType)
        {
            case BINTYPE_SOFT:
                lRecordHeader.iRecordSubType = 50;	// SBR
                lBinRange = mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList;
                break;

            case BINTYPE_HARD:
                lRecordHeader.iRecordSubType = 40;	// HBR
                lBinRange = mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList;
                break;

            default:
                return false;
                break;
        }

        // Create list of binnings found in ALL sites
        CBinning *              lBins   = NULL;
        QMap <int, CBinning *>  lFullBins;

        mPrivate->mContext->cMapTotalGoodParts[255]     = 0;	// Merged sites
        mPrivate->mContext->cMapTotalFailPatParts[255]  = 0;	// Merged sites
        mPrivate->mContext->cMapTotalFailParts[255]     = 0;	// Merged sites

        for(int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
        {
            // Reset Good/Fail totals for this site
            mPrivate->mContext->cMapTotalGoodParts[mPrivate->mContext->GetSiteList().at(lIdx)]       = 0;
            mPrivate->mContext->cMapTotalFailPatParts[mPrivate->mContext->GetSiteList().at(lIdx)]    = 0;
            mPrivate->mContext->cMapTotalFailParts[mPrivate->mContext->GetSiteList().at(lIdx)]       = 0;
        }

        // Handle to bin list
        if (lBinType == BINTYPE_SOFT)
            lBins = mPrivate->mContext->GetSTDFSoftBins();
        else if (lBinType == BINTYPE_HARD)
            lBins = mPrivate->mContext->GetSTDFHardBins();
        else
            lBins = NULL;

        // See all Bin# not already added to the bin list...
        while(lBins)
        {
            // Add this binning to the list
            if(lFullBins.contains(lBins->iBinValue) == false)
                lFullBins[lBins->iBinValue] = lBins;

            // Check next binning entry
            lBins = lBins->ptNextBin;
        };

        // Dump binning: one per testing site
        int		lBinNumber      = 0;
        long	lCount          = 0;
        long    lBinCount       = 0;
        long    lPatBinCount    = 0;
        long    lMergedBinCount = 0;
        int		lSiteNumber     = 0;
        long    lHeadNumber     = 0;

        QMap <int, CBinning *>::Iterator itBinList;
        for (itBinList = lFullBins.begin(); itBinList != lFullBins.end(); ++itBinList)
        {
            // Get Bin# of record to create.
            lBinNumber = (*itBinList)->iBinValue;

            lMergedBinCount = 0;
            for(int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
            {
                // Get testing site
                if(mPrivate->mContext->GetSiteList().at(lIdx) < 0)
                    lSiteNumber = lHeadNumber = 255;
                else
                {
                    // Head# as read in STDF input record (eg: PIR/WIR/PIR/PRR)
                    lHeadNumber = mPrivate->mTesterHead;
                    lSiteNumber = mPrivate->mContext->GetSiteList().at(lIdx);
                }

                // Count total entries for given site and bin
                lBinCount = GetTotalBinCount((lBinType == BINTYPE_SOFT), lBinNumber, lSiteNumber);

                // DPAT bins
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_DPAT,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    = lCount;
                lBinCount       -= lCount;

                // Check how many NNR to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_NNR,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;

                // Check how many IDDQ-Delta to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_IDDQ_Delta,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;

                // Check how many GDBN (bad neighbors) to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_GDBN,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;

                // Check how many Reticle bins to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_Reticle,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;


                // Check how many Clustering 'potato'  bins to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_Clustering,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;

                // Check how many Clustering 'potato'  bins to remove as well for this testing site...
                lCount          = mPrivate->mContext->getOutliersCount(CPatInfo::Outlier_ZPAT,
                                                             lSiteNumber,
                                                             (lBinType == BINTYPE_SOFT),
                                                             lBinNumber);
                lPatBinCount    += lCount;
                lBinCount       -= lCount;

                // If this bin number is consider as a good bin, count them as good parts
                // Otherwise count them as fail parts
                if(lBinRange->Contains(lBinNumber))
                {
                    // Keep track of total Good parts for this site
                    mPrivate->mContext->cMapTotalGoodParts[lSiteNumber] += lBinCount;
                    // Merged sites
                    mPrivate->mContext->cMapTotalGoodParts[255]         += lBinCount;
                }
                else
                {
                    // Keep track of total FAIL parts for this site
                    mPrivate->mContext->cMapTotalFailParts[lSiteNumber] += lBinCount;
                    // Merged sites
                    mPrivate->mContext->cMapTotalFailParts[255]         += lBinCount;
                }

                if (lPatBinCount > 0)
                {
                    // Keep track of total Good parts Failing PAT for this site
                    mPrivate->mContext->cMapTotalFailPatParts[lSiteNumber]  += lPatBinCount;
                    // Merged sites
                    mPrivate->mContext->cMapTotalFailPatParts[255]          += lPatBinCount;
                }

                // Keep track of total bin for all sites merged
                lMergedBinCount += lBinCount;

                // Write STDF record (SBR or HBR)
                switch(mPrivate->mInputFile.GetStdfVersion())
                {
                default :
                        break;

                    case GEX_STDFV4:
                        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                        mPrivate->mOutputFile.WriteByte(lHeadNumber);	// Head (255 = all sites)
                        mPrivate->mOutputFile.WriteByte(lSiteNumber);	// Site #
                        mPrivate->mOutputFile.WriteWord(lBinNumber);		// Bin#
                        mPrivate->mOutputFile.WriteDword(lBinCount);		// Bin count
                        mPrivate->mOutputFile.WriteByte((*itBinList)->cPassFail);                            // Pass/Fail info.
                        mPrivate->mOutputFile.WriteString((*itBinList)->strBinName.toLatin1().constData());	// Bin name

                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = QString("*PAT Error* Failed to write %1 record")
                                                      .arg((lBinType == BINTYPE_SOFT) ? "SBR": "HBR")
                                                      .toLatin1().constData();
                            return false;
                        }
                        break;
                }
            }

            // If multi-sites STDF V4, create one merged-sites record
            if(mPrivate->mContext->GetSiteList().count() > 1)
            {
                // Write STDF record (SBR or HBR)
                switch(mPrivate->mInputFile.GetStdfVersion())
                {
                default :
                        break;

                    case GEX_STDFV4:
                        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                        mPrivate->mOutputFile.WriteByte(255);	// Head (255 = all sites)
                        mPrivate->mOutputFile.WriteByte(0);		// Site #
                        mPrivate->mOutputFile.WriteWord(lBinNumber);			// Bin#
                        mPrivate->mOutputFile.WriteDword(lMergedBinCount);	// Bin count
                        mPrivate->mOutputFile.WriteByte((*itBinList)->cPassFail);                           // Pass/Fail info.
                        mPrivate->mOutputFile.WriteString((*itBinList)->strBinName.toLatin1().constData());	// Bin name

                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = QString("*PAT Error* Failed to write %1 record")
                                                      .arg((lBinType == BINTYPE_SOFT) ? "SBR": "HBR")
                                                      .toLatin1().constData();
                            return false;
                        }
                        break;
                }
            }
        }
    }

    return true;
}

bool PATStdfUpdate::WritePATHBR()
{
    // Write SBRs of Pat Bins
    CPatBin             lPatBin;
    long                lBinCount       = 0;
    long                lMergedBinCount = 0;
    int                 lSiteNumber;
    int                 lHeadNumber;
    int                 lOutlierType;

    if(mPrivate->mContext->GetPATHardBins().count())
    {
        // HBR records
        GS::StdLib::StdfRecordReadInfo lRecordHeader;

        lRecordHeader.iRecordType       = 1;
        lRecordHeader.iRecordSubType    = 40;

        for (tdPATBins::ConstIterator it = mPrivate->mContext->GetPATHardBins().begin();
             it != mPrivate->mContext->GetPATHardBins().end();
             ++it )
        {
            lPatBin = *it;

            // Report failures for EACH testing site.
            lMergedBinCount = 0;
            for(int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
            {
                // Count total entries for given
                lOutlierType    = mPrivate->mContext->getOutlierType(lPatBin.iBin);
                lBinCount       = mPrivate->mContext->getOutliersCount(lOutlierType,
                                                                       mPrivate->mContext->GetSiteList().at(lIdx),
                                                                       false, -1, lPatBin.iBin);
                lMergedBinCount += lBinCount;

                // Get testing site
                if(mPrivate->mContext->GetSiteList().at(lIdx) < 0)
                {
                    lSiteNumber = 255;
                    lHeadNumber = 255;
                }
                else
                {
                    lHeadNumber = mPrivate->mTesterHead;	// Head# as read in STDF input record (eg: PIR/WIR/PIR/PRR)
                    lSiteNumber = mPrivate->mContext->GetSiteList().at(lIdx);
                }

                mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                mPrivate->mOutputFile.WriteByte(lHeadNumber);       // Head: All sites.
                mPrivate->mOutputFile.WriteByte(lSiteNumber);       // Site: irrelevant as Head set to '255'
                mPrivate->mOutputFile.WriteWord(lPatBin.iBin);      // Bin#
                mPrivate->mOutputFile.WriteDword(lBinCount);		// Bin count
                mPrivate->mOutputFile.WriteByte('F');				// Pass/Fail info.
                mPrivate->mOutputFile.WriteString(PAT::GetOutlierBinName(lPatBin.bFailType).toLatin1().constData());	// Bin name

                // Write record to disk
                if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                {
                    mPrivate->mErrorMessage = "*PAT Error* Failed to write HBR record";
                    return false;
                }
            }

            // If multi-sites STDF V4, create one merged-sites record
            if(mPrivate->mContext->GetSiteList().count() > 1)
            {
                // Write STDF record: HBR
                switch(mPrivate->mInputFile.GetStdfVersion())
                {
                default :
                    break;

                  case GEX_STDFV4:
                    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                    mPrivate->mOutputFile.WriteByte(255);               // Head (255 = all sites)
                    mPrivate->mOutputFile.WriteByte(0);                 // Site #
                    mPrivate->mOutputFile.WriteWord(lPatBin.iBin);		// Bin#
                    mPrivate->mOutputFile.WriteDword(lMergedBinCount);	// Bin count
                    mPrivate->mOutputFile.WriteByte('F');				// Pass/Fail info.
                    mPrivate->mOutputFile.WriteString(PAT::GetOutlierBinName(lPatBin.bFailType).toLatin1().constData());	// Bin name

                    // Write record to disk
                    if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                    {
                        mPrivate->mErrorMessage = "*PAT Error* Failed to write HBR record";
                        return false;
                    }
                    break;
                }
            }
        }
    }

    return true;
}

bool PATStdfUpdate::WritePATRecords()
{

    // Build the Outlier Report pages into a QString object (from gexReport class)
    gexReport->BuildPP_OutlierRemovalReport(mPrivate->mSettings.strOutputReportFormat);

    // Write Outlier report into STDF.DTR rrecords
    gexReport->WritePP_OutlierRemovalReport(&mPrivate->mOutputFile, mPrivate->mSettings);

    // Write DTR holding list of Tests under PAT
    if (WritePATTestList() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write PAT Test list in DTR records");
        return false;
    }

    // Write TSR: ignore original ones (as could be per testing site), generate our own updated TSR,
    // for all merged sites.
    if (WriteTSR() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write TSR records");
        return false;
    }

    if (WritePATBinRecords() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write PAT bin records");
        return false;
    }

    if (WriteBinRecords(BINTYPE_SOFT) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write SBR records");
        return false;
    }

    if (WriteBinRecords(BINTYPE_HARD) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write HBR records");
        return false;
    }

    // Write WCR...unless already found
    if (WriteWCR() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write WCR record");
        return false;
    }

    // Write PCR...unless already found & updated from source file
    if (WritePCR() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to write PCR record");
        return false;
    }

    return true;
}

bool PATStdfUpdate::WritePATSBR()
{
    // Write SBRs of Pat Bins
    CPatBin             lPatBin;
    long                lBinCount       = 0;
    long                lMergedBinCount = 0;
    int                 lSiteNumber;
    int                 lHeadNumber;
    int                 lOutlierType;

    if(mPrivate->mContext->GetPATSoftBins().count())
    {
        // SBR records
        GS::StdLib::StdfRecordReadInfo lRecordHeader;

        lRecordHeader.iRecordType       = 1;
        lRecordHeader.iRecordSubType    = 50;

        for (tdPATBins::ConstIterator it = mPrivate->mContext->GetPATSoftBins().begin();
             it != mPrivate->mContext->GetPATSoftBins().end();
             ++it )
        {
            lPatBin = *it;

            // Report failures for EACH testing site.
            lMergedBinCount = 0;
            for(int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
            {
                // Count total entries for given
                lOutlierType    = mPrivate->mContext->getOutlierType(lPatBin.iBin);
                lBinCount       = mPrivate->mContext->getOutliersCount(lOutlierType,
                                                                       mPrivate->mContext->GetSiteList().at(lIdx),
                                                                       true, -1, lPatBin.iBin);
                lMergedBinCount += lBinCount;

                // Get testing site
                if(mPrivate->mContext->GetSiteList().at(lIdx) < 0)
                {
                    lSiteNumber = 255;
                    lHeadNumber = 255;
                }
                else
                {
                    lHeadNumber = mPrivate->mTesterHead;	// Head# as read in STDF input record (eg: PIR/WIR/PIR/PRR)
                    lSiteNumber = mPrivate->mContext->GetSiteList().at(lIdx);
                }

                mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                mPrivate->mOutputFile.WriteByte(lHeadNumber);       // Head: All sites.
                mPrivate->mOutputFile.WriteByte(lSiteNumber);       // Site: irrelevant as Head set to '255'
                mPrivate->mOutputFile.WriteWord(lPatBin.iBin);      // Bin#
                mPrivate->mOutputFile.WriteDword(lBinCount);        // Bin count
                mPrivate->mOutputFile.WriteByte('F');               // Pass/Fail info.
                mPrivate->mOutputFile.WriteString(PAT::GetOutlierBinName(lPatBin.bFailType)
                                                  .toLatin1().constData());	// Bin name

                // Write record to disk
                if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                {
                    mPrivate->mErrorMessage = "*PAT Error* Failed to write SBR record";
                    return false;
                }
            }


            // If multi-sites STDF V4, create one merged-sites record
            if(mPrivate->mContext->GetSiteList().count() > 1)
            {
                // Write STDF record: SBR
                switch(mPrivate->mInputFile.GetStdfVersion())
                {
                default :
                        break;

                    case GEX_STDFV4:
                        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                        mPrivate->mOutputFile.WriteByte(255);                           // Head (255 = all sites)
                        mPrivate->mOutputFile.WriteByte(0);                             // Site #
                        mPrivate->mOutputFile.WriteWord(lPatBin.iBin);                  // Bin#
                        mPrivate->mOutputFile.WriteDword(lMergedBinCount);              // Bin count
                        mPrivate->mOutputFile.WriteByte('F');	// Pass/Fail info.
                        mPrivate->mOutputFile.WriteString(PAT::GetOutlierBinName(lPatBin.bFailType).toLatin1().constData());	// Bin name

                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = "*PAT Error* Failed to write SBR record";
                            return false;
                        }

                        break;
                }
            }
        }
    }

    return true;
}

bool PATStdfUpdate::WritePATTestList()
{
    // Write DTRs (note: ensure maximum of 230 characters per DTR)
    bool                            lIgnoreTest;
    QString                         lDTRLine;
    CPatDefinition *                lPatDef = NULL;
    QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;

    // Record header
    GS::StdLib::StdfRecordReadInfo   lRecordHeader;

    lRecordHeader.iRecordType       = 50;
    lRecordHeader.iRecordSubType    = 30;	// DTR

    for(itPATDefinifion = mPrivate->mContext->GetUnivariateRules().begin();
        itPATDefinifion != mPrivate->mContext->GetUnivariateRules().end(); ++itPATDefinifion)
    {
        lPatDef = *itPATDefinifion;

        // Ensure DTR starts with prefix string
        if(lDTRLine.isEmpty())
            lDTRLine = "<cmd> PatTestList ";

        // Only include tests that are Dynamic-PAT enabled
        lIgnoreTest = false;
        if(lPatDef == NULL)
            lIgnoreTest = true;
        else if((lPatDef->m_lFailStaticBin < 0) &&
                (lPatDef->m_lFailDynamicBin < 0 ||
                 lPatDef->mOutlierRule == GEX_TPAT_RULETYPE_IGNOREID))
            lIgnoreTest = true;

        // Valild PAT test, add its Test# to the DTR string Write DTR prefix string (identifies list of Tests in PAT recipe)
        if(lIgnoreTest == false)
        {
            // Add test # to DTR string
            lDTRLine += QString::number(lPatDef->m_lTestNumber) + ",";

            // If DTR string long enough, dump to disk!
            if(lDTRLine.length() > 230)
            {
                // Write String into STDF.DTR record
                mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                mPrivate->mOutputFile.WriteString(lDTRLine.toLatin1().constData());

                // Write record to disk
                if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                {
                    mPrivate->mErrorMessage = "*PAT Error* Failed to write DTR record";
                    return false;
                }

                // Reset DTR buffer
                lDTRLine = "";
            }
        }
    }

    // Flush DTR if buffer not dumped to disk.
    if(lDTRLine.isEmpty() == false)
    {
        // Write String into STDF.DTR record
        mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
        mPrivate->mOutputFile.WriteString(lDTRLine.toLatin1().constData());

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write DTR record";
            return false;
        }
    }

    return true;
}

bool PATStdfUpdate::WritePCR()
{
    if(mPrivate->IsLastPass() && mPrivate->mPCRCreated == false)
    {
        long    lTotalParts = 0;
        int     lSiteNumber = -1;

        // Write one PCR per site.
        GS::StdLib::StdfRecordReadInfo lRecordHeader;
        lRecordHeader.iRecordType       = 1;
        lRecordHeader.iRecordSubType    = 30;

        for (int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
        {
            lSiteNumber = mPrivate->mContext->GetSiteList().at(lIdx);

            // Read original PCR record
            switch(mPrivate->mInputFile.GetStdfVersion())
            {
            default :
                    break;

                case GEX_STDFV4:	// PCR STDF V4
                    // Write new PCR
                    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                    mPrivate->mOutputFile.WriteByte(0);						// Head: 255=Merged sites info
                    mPrivate->mOutputFile.WriteByte(lSiteNumber);           // test site

                    lTotalParts = mPrivate->mContext->cMapTotalGoodParts[lSiteNumber] +
                                  mPrivate->mContext->cMapTotalFailPatParts[lSiteNumber] +
                                  mPrivate->mContext->cMapTotalFailParts[lSiteNumber];
                    // PART_CNT
                    mPrivate->mOutputFile.WriteDword(lTotalParts);
                    // RTST_CNT
                    mPrivate->mOutputFile.WriteDword(0);
                    // ABRT_CNT
                    mPrivate->mOutputFile.WriteDword(0);
                    // Write new Good die count.
                    lTotalParts = mPrivate->mContext->cMapTotalGoodParts[lSiteNumber];
                    mPrivate->mOutputFile.WriteDword(lTotalParts);	// GOOD_CNT

                    // Write record to disk
                    if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                    {
                        mPrivate->mErrorMessage = "*PAT Error* Failed to write WIR record";
                        return false;
                    }
                    break;
            }
        }

        // If multi-sites, write merged PCR
        if(mPrivate->mContext->GetSiteList().count() > 1)
        {
            // Write new PCR
            mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
            mPrivate->mOutputFile.WriteByte(255);	// Head: 255=Merged sites info
            mPrivate->mOutputFile.WriteByte(0);		// test site

            lTotalParts = mPrivate->mContext->cMapTotalGoodParts[255] + mPrivate->mContext->cMapTotalFailPatParts[255] +
                          mPrivate->mContext->cMapTotalFailParts[255];
            mPrivate->mOutputFile.WriteDword(lTotalParts);		// PART_CNT
            mPrivate->mOutputFile.WriteDword(0);		// RTST_CNT
            mPrivate->mOutputFile.WriteDword(0);		// ABRT_CNT

            lTotalParts = mPrivate->mContext->cMapTotalGoodParts[255];
            mPrivate->mOutputFile.WriteDword(lTotalParts);	// GOOD_CNT

            // Write record to disk
            if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
            {
                mPrivate->mErrorMessage = "*PAT Error* Failed to write WIR record";
                return false;
            }
        }
    }

    return true;
}

bool PATStdfUpdate::WriteTSR()
{
    // Only STDF V4 supported for TSR
    if (mPrivate->mInputFile.GetStdfVersion() == GEX_STDFV4)
    {
        CPatOutlierPart *					lOutlierPart = NULL;
        QList<CPatFailingTest>::iterator	itPart;
        int                                 lOutlierCount;
        long                                lTmpCount;

        // Enumerate all tests in list.
        int     lSiteNumber;
        long	lMergedExecCount    = 0;
        long	lMergedFailCount    = 0;
        CTest * lMasterTest         = mPrivate->mFile->ptTestList;
        CTest * lTestSite           = NULL;
        CGexStats       lStatsEngine;
        PATPartFilter   lPartFilter;

        if (mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_TESTING_SITE, GEX_PTEST, &lTestSite, true, false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find Quantix SITE parameter.";
            return false;
        }

        // TSR header definition
        GS::StdLib::StdfRecordReadInfo lRecordHeader;

        lRecordHeader.iRecordType       = 10;
        lRecordHeader.iRecordSubType    = 30;

        while(lMasterTest)
        {
            if (lMasterTest->bTestType != 'F')
            {
                lMergedExecCount = 0;
                lMergedFailCount = 0;

                // All run removed during data/PAT processing
                for (int lRun = 0; lRun < lMasterTest->m_testResult.count(); ++lRun)
                    lMasterTest->m_testResult.validateResultAt(lRun);

                for (int lIdx = 0; lIdx < mPrivate->mContext->GetSiteList().count(); ++lIdx)
                {
                    // Get site# for this group.
                    lSiteNumber = mPrivate->mContext->GetSiteList().at(lIdx);

                    //compute stats for master test.
                    lPartFilter.RemoveAllFilters();

                    if (lSiteNumber >= 0)
                    {
                        lPartFilter.AddFilterElement(PATPartFilterElement(lTestSite,
                                                                          PATPartFilterElement::InFilter,
                                                                          QtLib::Range(QString::number(lSiteNumber))));
                    }
                    else
                        lSiteNumber = 0;

                    // Recompute some statistics
                    lStatsEngine.ComputeLowLevelTestStatistics(lMasterTest, ScalingPower(lMasterTest->res_scal), &lPartFilter);
                    lStatsEngine.ComputeBasicTestStatistics(lMasterTest, true, &lPartFilter);

                    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                    mPrivate->mOutputFile.WriteByte(1);                                 // head_num
                    mPrivate->mOutputFile.WriteByte(lSiteNumber);                       // site_num
                    mPrivate->mOutputFile.WriteByte(lMasterTest->bTestType);            // test type
                    mPrivate->mOutputFile.WriteDword(lMasterTest->lTestNumber);         // test number
                    mPrivate->mOutputFile.WriteDword(lMasterTest->ldSamplesValidExecs);	// exec count

                    // Keep track of total executions for all merged sites
                    lMergedExecCount += lMasterTest->ldSamplesValidExecs;

                    // Scan all outlier parts
                    lOutlierCount = 0;	// To keep track of total outlier fail count for the test
                    tdIterPatOutlierParts	itOutlierParts(mPrivate->mContext->m_lstOutlierParts);

                    while(itOutlierParts.hasNext())
                    {
                        lOutlierPart = itOutlierParts.next();

                        for (itPart = lOutlierPart->cOutlierList.begin();
                             itPart != lOutlierPart->cOutlierList.end();
                             ++itPart )
                        {
                            // Check if matching site#, or currentmly flushing all sites combined.
                            if((*itPart).mTestNumber == lMasterTest->lTestNumber &&
                               (*itPart).mSite == lSiteNumber)
                                lOutlierCount++;
                        }
                    };

                    // Write new fail count (original fail count + outlier count)
                    lTmpCount = lMasterTest->GetCurrentLimitItem()->ldFailCount + lOutlierCount;
                    // Keep track of total failures for all merged sites
                    mPrivate->mOutputFile.WriteDword(lTmpCount);
                    lMergedFailCount += lTmpCount;
                    // Alarm count
                    mPrivate->mOutputFile.WriteDword(4294967295lu);
                    // Test name
                    mPrivate->mOutputFile.WriteString(lMasterTest->strTestName.toLatin1().constData());

                    if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                    {
                        mPrivate->mErrorMessage = "*PAT Error* Failed to write TSR record";
                        return false;
                    }
                }

                // If multiple sites, add merged-site TSR.
                if(mPrivate->mContext->GetSiteList().count() > 1)
                {
                    mPrivate->mOutputFile.WriteHeader(&lRecordHeader);
                    mPrivate->mOutputFile.WriteByte(255);                    // head_num = 255 (All sites)
                    mPrivate->mOutputFile.WriteByte(0);                      // site_num
                    mPrivate->mOutputFile.WriteByte(lMasterTest->bTestType);       // test type
                    mPrivate->mOutputFile.WriteDword(lMasterTest->lTestNumber);    // test number
                    mPrivate->mOutputFile.WriteDword(lMergedExecCount);		// exec count for ALL sites
                    mPrivate->mOutputFile.WriteDword(lMergedFailCount);		// Fail count (all sites merged)
                    mPrivate->mOutputFile.WriteDword(4294967295lu);			// Alarm count
                    // Test name
                    mPrivate->mOutputFile.WriteString(lMasterTest->strTestName.toLatin1().constData());

                    if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                    {
                        mPrivate->mErrorMessage = "*PAT Error* Failed to write TSR record";
                        return false;
                    }
                }
            }

            // Next test
            lMasterTest = lMasterTest->GetNextTest();
        };
    }

    return true;
}

bool PATStdfUpdate::WriteWCR()
{
    // Only create WCR if it doesn"t already exist
    if(mPrivate->mWCRFound == false)
    {
        // WCR
        GS::StdLib::StdfRecordReadInfo lRecordHeader;

        switch(mPrivate->mInputFile.GetStdfVersion())
        {
        default :
            break;

            case GEX_STDFV4:

                // Write new WCR
                lRecordHeader.iRecordType       = 2;
                lRecordHeader.iRecordSubType    = 30;
                mPrivate->mOutputFile.WriteHeader(&lRecordHeader);

                mPrivate->mOutputFile.WriteFloat(0);            // WAFR_SIZ
                mPrivate->mOutputFile.WriteFloat(0);            // DIE_HT
                mPrivate->mOutputFile.WriteFloat(0);            // DIE_WID
                mPrivate->mOutputFile.WriteByte(0);             // WF_UNITS
                mPrivate->mOutputFile.WriteByte(' ');           // WF_FLAT
                mPrivate->mOutputFile.WriteWord((WORD)-32768);	// CENTER_X
                mPrivate->mOutputFile.WriteWord((WORD)-32768);	// CENTER_Y
                mPrivate->mOutputFile.WriteByte(' ');           // POS_X
                mPrivate->mOutputFile.WriteByte(' ');           // POS_Y
                break;
        }

        // Write record to disk
        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
        {
            mPrivate->mErrorMessage = "*PAT Error* Failed to write WCR record";
            return false;
        }
    }

    return true;
}

bool PATStdfUpdate::WriteExternalMapFailures()
{
    if (mPrivate->mSettings.mOutputIncludeExtFails == true && mPrivate->mExtFailuresAdded == false)
    {
        // Ensure to remove all runs of excluded dies.
        // Remove Runs that do not belong to the GOOD dies list
        int                 lProberBin      = GEX_WAFMAP_EMPTY_CELL;
        int                 lProberIdx      = -1;
        int                 lXCoord         = GEX_WAFMAP_INVALID_COORD;
        int                 lYCoord         = GEX_WAFMAP_INVALID_COORD;
        int                 lRunCount       = 0;
        CBinning *          lBinning        = NULL;
        CBinning *          lSiteBinning    = NULL;
        CTest *             lTestDieX       = NULL;
        CTest *             lTestDieY       = NULL;
        CTest *             lTestSBin       = NULL;
        CTest *             lTestHBin       = NULL;

        // Get handle to Die X,Y info
        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST, &lTestDieX,true,false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on die X coordinates";
            return false;
        }

        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST, &lTestDieY,true,false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on die Y coordinates";
            return false;
        }

        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST, &lTestSBin,true,false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on Soft Bin";
            return false;
        }

        if(mPrivate->mFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST, &lTestHBin,true,false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on Hard Bin";
            return false;
        }

        // Scan all parts tested.
        lRunCount = qMin(lTestDieX->m_testResult.count(), lTestDieY->m_testResult.count());

        for(int lIdx = 0;lIdx < lRunCount; ++lIdx)
        {
            // Look for the die in the prober map if result is valid for this run
            if (lTestDieX->m_testResult.isValidResultAt(lIdx) &&
                lTestDieY->m_testResult.isValidResultAt(lIdx))
            {
                lXCoord   = static_cast<int>(lTestDieX->m_testResult.resultAt(lIdx));
                lYCoord   = static_cast<int>(lTestDieY->m_testResult.resultAt(lIdx));

                if (mPrivate->mContext->m_ProberMap.indexFromCoord(lProberIdx, lXCoord, lYCoord))
                {
                    lProberBin  = mPrivate->mContext->m_ProberMap.getWafMap()[lProberIdx].getBin();

                    if (lProberBin != GEX_WAFMAP_EMPTY_CELL)
                        lBinning    = mPrivate->mContext->GetExternalBins()->Find(lProberBin);
                    else
                        lBinning    = NULL;

                    if (lBinning && lBinning->cPassFail == 'F')
                    {
                        GS::StdLib::StdfRecordReadInfo lPIRRecordHeader;
                        GS::StdLib::StdfRecordReadInfo lPRRRecordHeader;

                        switch(mPrivate->mInputFile.GetStdfVersion())
                        {
                            case GEX_STDFV4:

                                // Set up PIR header
                                lPIRRecordHeader.iRecordType       = 5;
                                lPIRRecordHeader.iRecordSubType    = 10;

                                // Set up PRR header
                                lPRRRecordHeader.iRecordType       = 5;
                                lPRRRecordHeader.iRecordSubType    = 20;
                                break;


                        default:
                                break;
                        }

                        // Write PIR record
                        mPrivate->mOutputFile.WriteHeader(&lPIRRecordHeader);

                        mPrivate->mOutputFile.WriteByte(1);             // HEAD_NUM
                        mPrivate->mOutputFile.WriteByte(1);             // SITE_NUM

                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = "*PAT Error* Failed to write PIR record";
                            return false;
                        }

                        // Write PRR record
                        mPrivate->mOutputFile.WriteHeader(&lPRRRecordHeader);

                        mPrivate->mOutputFile.WriteByte(1);             // HEAD_NUM
                        mPrivate->mOutputFile.WriteByte(1);             // SITE_NUM
                        mPrivate->mOutputFile.WriteByte(8);             // PART_FLG
                        mPrivate->mOutputFile.WriteWord((WORD)0);       // NUM_TEST
                        mPrivate->mOutputFile.WriteWord(lProberBin);    // HARD_BIN
                        mPrivate->mOutputFile.WriteWord(lProberBin);    // SOFT_BIN
                        mPrivate->mOutputFile.WriteWord(lXCoord);       // X_COORD
                        mPrivate->mOutputFile.WriteWord(lYCoord);       // Y_COORD
                        mPrivate->mOutputFile.WriteDword(0);            // TEST_T
                        mPrivate->mOutputFile.WriteString("");          // PART_ID
                        mPrivate->mOutputFile.WriteString("");          // PART_TXT
                        mPrivate->mOutputFile.WriteString("");          // PART_FIX

                        // Write record to disk
                        if (mPrivate->mOutputFile.WriteRecord() != GS::StdLib::Stdf::NoError)
                        {
                            mPrivate->mErrorMessage = "*PAT Error* Failed to write PRR record";
                            return false;
                        }

                        // Update Soft bin list for this site
                        CBinning * lSoftBins = mPrivate->mFile->m_ParentGroup->cMergedData.ptMergedSoftBinList;
                        lSiteBinning = lSoftBins->Find(lProberBin);

                        if (lSiteBinning)
                            lSiteBinning->ldTotalCount++;
                        else
                            lSoftBins->Append(lProberBin, 'F', 1, "External Failed Bin");

                        // Update Hard bin list for this site
                        CBinning * lHardBins = mPrivate->mFile->m_ParentGroup->cMergedData.ptMergedHardBinList;
                        lSiteBinning = lHardBins->Find(lProberBin);

                        if (lSiteBinning)
                            lSiteBinning->ldTotalCount++;
                        else
                            lHardBins->Append(lProberBin, 'F', 1, "External Failed Bin");

                        // Update Hard and Soft bin parameters
                        lTestSBin->m_testResult.pushResultAt(lIdx, lProberBin, true);
                        lTestHBin->m_testResult.pushResultAt(lIdx, lProberBin, true);

                        if (mPrivate->mContext->CleanPATDieAlarms(lXCoord,lYCoord, true))
                            mPrivate->mContext->OverloadRefWafermap(lXCoord, lYCoord, lProberBin, lProberBin);
                    }
                }
            }
        }

        mPrivate->mExtFailuresAdded = true;
    }

    return true;
}

bool PATStdfUpdate::WriteOutlierRemovalReport()
{
    // Build the Outlier Report pages into a QString object (from gexReport class)
    gexReport->BuildPP_OutlierRemovalReport(mPrivate->mSettings.strOutputReportFormat);

    // Write Outlier report into STDF.DTR rrecords
    gexReport->WritePP_OutlierRemovalReport(&mPrivate->mOutputFile, mPrivate->mSettings);

    return true;
}

}   // namespace Gex
}   // namespace GS
#endif
