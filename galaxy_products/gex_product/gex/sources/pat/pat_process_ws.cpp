#ifdef GCORE15334
#include "cbinning.h"
#include "stdf.h"
#include "stdfparse.h"
#include "engine.h"
#include "gex_pat_processing.h"
#include "gex_report.h"
#include "gqtl_log.h"
#include "import_kla_inf_layers.h"
#include "import_prober_tel.h"
#include "import_semi_e142_xml.h"
#include "importWoburnSECSIIMap.h"
#include "importMexicaliMap.h"
#include "message.h"
#include "pat_definition.h"
#include "pat_process_ws.h"
#include "pat_process_ws_private.h"
#include "pat_recipe_io.h"
#include "pat_report_ws.h"
#include "pat_rules.h"
#include "pat_stdf_updater.h"
#include "product_info.h"
#include "temporary_files_manager.h"
#include "tb_merge_retest.h"
#include "wafer_export.h"
#include "csl/csl_engine.h"
#include "pat_outlier_finder_ws.h"
#include "parserFactory.h"
#include "parserAbstract.h"
#include "merge_maps.h"
#include "stdf_content_utils.h"
#include "map_alignment.h"



extern CGexReport *     gexReport;
extern GexScriptEngine *pGexScriptEngine;

extern QString          ConvertToScriptString(const QString &strFile);

//#define MV_PAT_TEST

namespace GS
{
namespace Gex
{

class WaferCompositePat
{
public:
    WaferCompositePat();			// Constructors
    ~WaferCompositePat();			// Destructor

    CWaferMap                   mExclusionZone;		// Holds binnary Wafermap (rejected dies from exclusion zone)
    QMap<unsigned, CWaferMap*>	mWafers;			// Holds list of wafer maps in lot (used for 3D neigbhorhood)
};

PATProcessWS::PATProcessWS(CPatInfo * context)
    : mPrivate(new PATProcessWSPrivate(context))
{

}

PATProcessWS::PATProcessWS(const PATProcessing &lSettings)
    : mPrivate(new PATProcessWSPrivate(NULL))
{
    mPrivate->mSettings = lSettings;
}

PATProcessWS::~PATProcessWS()
{
    if (mPrivate)
    {
        delete mPrivate;
        mPrivate = NULL;
    }
}

bool PATProcessWS::CreateSTDFDataFile(const PATProcessing &lSettings, QString &lSTDFDataFile,
                                    QString& lErrorMessage)
{
    PATProcessWS lPATProcessWS(lSettings);
    QString    lExternalMap;

    if (lPATProcessWS.CreateSTDFDataFile(lSTDFDataFile, lExternalMap) == false)
    {
        lErrorMessage = lPATProcessWS.GetErrorMessage();
        return false;
    }

    return true;
}

bool PATProcessWS::Execute(PATProcessing &lSettings, const QString& lDataFile /*= QString()*/)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          "Starting WS PAT Processing");

    PAT_PERF_RUN
    PAT_PERF_BENCH

    // Clean up before starting.
    mPrivate->mErrorMessage.clear();
    mPrivate->mInclusionDies.clear();
    mPrivate->mStartTime = QDateTime::currentDateTime();
    mPrivate->mSettings  = lSettings;

    // Save 'process Start' timestamp
    mPrivate->mSettings.dtProcessStartTime = QDateTime::currentDateTime();

    // Clear any previous PAT analysis results or info array.
    if (mPrivate->mContext == NULL)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Context NULL");
        mPrivate->mErrorMessage = "No context initialized for the pat processing";
        return false;
    }

    // Check environment variable: include original limits in PAT STDF?
    QByteArray lEnv = qgetenv(PATMAN_STDF_ORG_LIMITS);
    if(lEnv.isNull() == false)
        mPrivate->mSettings.bReportStdfOriginalLimits = (lEnv.toInt() == 1) ? true: false;

    // Keep list of data files to merge (for backup purpose)
    mPrivate->mContext->strDataSources    = mPrivate->mSettings.strSources;
    // Set the data files to use for PAT processing if any
    mPrivate->mContext->strDataFile       = lDataFile;

    // Prepare dataset
    if (PrepareData() == false)
    {
        // Ensure to delete intermediate Prober Map STDF file created.
        Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);
        return false;
    }

    // Performs all required pre-processing actions
    if (PreProcessing() == false)
    {
        // Ensure to delete intermediate Prober Map STDF file created.
        Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);
        return false;
    }

    // Process PAT
    if (Processing() == false)
    {
        // Ensure to delete intermediate Prober Map STDF file created.
        Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);
        return false;
    }

    // Performs all required post-processing actions
    if (PostProcessing() == false)
    {
        // Ensure to delete intermediate Prober Map STDF file created.
        Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);
        return false;
    }

    // Ensure to delete intermediate Prober Map STDF file created.
    Engine::GetInstance().GetTempFilesManager().removeAll(TemporaryFile::BasicCheck, true);

    // Update Settings object given in arguments as it can be updated by the PAT process
    lSettings = mPrivate->mSettings;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "WS PAT successfully done");

    return true;
}

const QString &PATProcessWS::GetErrorMessage() const
{
    return mPrivate->mErrorMessage;
}

const PAT::ExternalMapDetails &PATProcessWS::GetExternalMapDetails() const
{
    return mPrivate->mExternalMapDetails;
}

bool PATProcessWS::ApplyInclusionDiesList()
{
    // If valid list of dies to keep, process it
    if(mPrivate->mInclusionDies.count())
    {
        // Ensure to remove all runs of excluded dies.
        // Remove Runs that do not belong to the GOOD dies list
        QString             lKey;
        CTest *             ptDieX      = NULL;
        CTest *             ptDieY      = NULL;
        CTest *             ptTestCell  = NULL;
        CGexGroupOfFiles *  pGroup      = NULL;
        CGexFileInGroup *   pFile       = NULL;

        int         lDieX;
        int         lDieY;
        int         lRunCount = 0;

        // Iterator on Groups list
        QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

        while(itGroupsList.hasNext())
        {
            pGroup	= itGroupsList.next();
            pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

            // Get handle to Die X,Y info
            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST,&ptDieX,true,false) != 1)
            {
                mPrivate->mErrorMessage = "Unable to find parameters on die X coordinates";
                return false;
            }

            if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST,&ptDieY,true,false) != 1)
            {
                mPrivate->mErrorMessage = "Unable to find parameters on die Y coordinates";
                return false;
            }

            // Scan all parts tested.
            lRunCount = qMin(ptDieX->m_testResult.count(), ptDieY->m_testResult.count());

            for(int lIdx = 0;lIdx < lRunCount; ++lIdx)
            {
                lDieX   = static_cast<int>(ptDieX->m_testResult.resultAt(lIdx));
                lDieY   = static_cast<int>(ptDieY->m_testResult.resultAt(lIdx));
                lKey    = QString::number(lDieX) + "." + QString::number(lDieY);

                if(mPrivate->mInclusionDies.contains(lKey) == false)
                {
                    // This die is NOT part of the Wafermap dies to keep: so delete run samples.
                    ptTestCell = pFile->ptTestList;
                    while(ptTestCell)
                    {
                        if((ptTestCell->bTestType != '-') && ptTestCell->m_testResult.count() > 0)
                        {
                            // GCORE-6821 HTH
                            // Those dies must be deleted, not invalidated.
                            // We don't want them to come back in the process after DPAT limit computation where
                            // we re-include all invalidated dies
//                            ptTestCell->m_testResult.invalidateResultAt(lIdx);
                            ptTestCell->m_testResult.deleteResultAt(lIdx);
                        }

                        // Move to next test sample in run.
                        ptTestCell = ptTestCell->GetNextTest();
                    };
                }
            }
        };
    }

    return true;
}

void PATProcessWS::CheckAxisDirection()
{
    // Force STDF positive X direction if requested
    if (mPrivate->mSettings.mStdfXDirection == PATProcessing::ePositiveForceLeft)
    {
        mPrivate->mContext->m_AllSitesMap_Sbin.SetPosXDirection(false);
        mPrivate->mContext->m_AllSitesMap_Hbin.SetPosXDirection(false);
        mPrivate->mContext->m_Stdf_SbinMap.SetPosXDirection(false);
        mPrivate->mContext->m_Stdf_HbinMap.SetPosXDirection(false);
    }
    else if (mPrivate->mSettings.mStdfXDirection == PATProcessing::ePositiveForceRight)
    {
        mPrivate->mContext->m_AllSitesMap_Sbin.SetPosXDirection(true);
        mPrivate->mContext->m_AllSitesMap_Hbin.SetPosXDirection(true);
        mPrivate->mContext->m_Stdf_SbinMap.SetPosXDirection(true);
        mPrivate->mContext->m_Stdf_HbinMap.SetPosXDirection(true);
    }
    else if (toupper(mPrivate->mContext->m_AllSitesMap_Sbin.cPos_X) != 'L' &&
                toupper(mPrivate->mContext->m_AllSitesMap_Sbin.cPos_X) != 'R')
    {
        if (mPrivate->mSettings.mStdfXDirection == PATProcessing::ePositiveRight)
        {
            mPrivate->mContext->m_AllSitesMap_Sbin.SetPosXDirection(true);
            mPrivate->mContext->m_AllSitesMap_Hbin.SetPosXDirection(true);
            mPrivate->mContext->m_Stdf_SbinMap.SetPosXDirection(true);
            mPrivate->mContext->m_Stdf_HbinMap.SetPosXDirection(true);
        }
        else if (mPrivate->mSettings.mStdfXDirection == PATProcessing::ePositiveLeft)
        {
            mPrivate->mContext->m_AllSitesMap_Sbin.SetPosXDirection(false);
            mPrivate->mContext->m_AllSitesMap_Hbin.SetPosXDirection(false);
            mPrivate->mContext->m_Stdf_SbinMap.SetPosXDirection(false);
            mPrivate->mContext->m_Stdf_HbinMap.SetPosXDirection(false);
        }
    }

    // Force STDF positive Y direction if requested
    if (mPrivate->mSettings.mStdfYDirection == PATProcessing::ePositiveForceUp)
    {
        mPrivate->mContext->m_AllSitesMap_Sbin.SetPosYDirection(false);
        mPrivate->mContext->m_AllSitesMap_Hbin.SetPosYDirection(false);
        mPrivate->mContext->m_Stdf_SbinMap.SetPosYDirection(false);
        mPrivate->mContext->m_Stdf_HbinMap.SetPosYDirection(false);
    }
    else if (mPrivate->mSettings.mStdfYDirection == PATProcessing::ePositiveForceDown)
    {
        mPrivate->mContext->m_AllSitesMap_Sbin.SetPosYDirection(true);
        mPrivate->mContext->m_AllSitesMap_Hbin.SetPosYDirection(true);
        mPrivate->mContext->m_Stdf_SbinMap.SetPosYDirection(true);
        mPrivate->mContext->m_Stdf_HbinMap.SetPosYDirection(true);
    }
    else if (toupper(mPrivate->mContext->m_AllSitesMap_Sbin.cPos_Y) != 'U' &&
                toupper(mPrivate->mContext->m_AllSitesMap_Sbin.cPos_Y) != 'D')
    {
        if (mPrivate->mSettings.mStdfYDirection == PATProcessing::ePositiveDown)
        {
            mPrivate->mContext->m_AllSitesMap_Sbin.SetPosYDirection(true);
            mPrivate->mContext->m_AllSitesMap_Hbin.SetPosYDirection(true);
            mPrivate->mContext->m_Stdf_SbinMap.SetPosYDirection(true);
            mPrivate->mContext->m_Stdf_HbinMap.SetPosYDirection(true);
        }
        else if (mPrivate->mSettings.mStdfYDirection ==PATProcessing::ePositiveUp)
        {
            mPrivate->mContext->m_AllSitesMap_Sbin.SetPosYDirection(false);
            mPrivate->mContext->m_AllSitesMap_Hbin.SetPosYDirection(false);
            mPrivate->mContext->m_Stdf_SbinMap.SetPosYDirection(false);
            mPrivate->mContext->m_Stdf_HbinMap.SetPosYDirection(false);
        }
    }

    // Force External positive X direction if requested
    if (mPrivate->mSettings.mExternalXDirection == PATProcessing::ePositiveForceLeft)
        mPrivate->mContext->m_ProberMap.SetPosXDirection(false);
    else if (mPrivate->mSettings.mExternalXDirection == PATProcessing::ePositiveForceRight)
        mPrivate->mContext->m_ProberMap.SetPosXDirection(true);
    else if (toupper(mPrivate->mContext->m_ProberMap.cPos_X) != 'L' &&
                toupper(mPrivate->mContext->m_ProberMap.cPos_X) != 'R')
    {
        if (mPrivate->mSettings.mExternalXDirection == PATProcessing::ePositiveRight)
            mPrivate->mContext->m_ProberMap.SetPosXDirection(true);
        else if (mPrivate->mSettings.mExternalXDirection == PATProcessing::ePositiveLeft)
            mPrivate->mContext->m_ProberMap.SetPosXDirection(false);
    }

    // Force STDF positive Y direction if requested
    if (mPrivate->mSettings.mExternalYDirection == PATProcessing::ePositiveForceUp)
        mPrivate->mContext->m_ProberMap.SetPosYDirection(false);
    else if (mPrivate->mSettings.mExternalYDirection == PATProcessing::ePositiveForceDown)
        mPrivate->mContext->m_ProberMap.SetPosYDirection(true);
    else if (toupper(mPrivate->mContext->m_ProberMap.cPos_Y) != 'U' &&
                toupper(mPrivate->mContext->m_ProberMap.cPos_Y) != 'D')
    {
        if (mPrivate->mSettings.mExternalYDirection == PATProcessing::ePositiveDown)
            mPrivate->mContext->m_ProberMap.SetPosYDirection(true);
        else if (mPrivate->mSettings.mExternalYDirection ==PATProcessing::ePositiveUp)
            mPrivate->mContext->m_ProberMap.SetPosYDirection(false);
    }
}

bool PATProcessWS::CheckReferenceDieLocation()
{
    // External map Reference die location defined in the PAT Processing keys
    if (mPrivate->mSettings.mExternalRefLocation.isEmpty() == false)
    {
        if (mPrivate->mContext->m_ProberMap.ComputeReferenceDieLocation(mPrivate->mSettings.mExternalRefLocation) == false)
        {
            mPrivate->mErrorMessage = "*GTF Error* Unable to assign reference die location to the external map";
            return false;
        }
    }

    // STDF Reference die location defined in the PAT Processing keys
    if (mPrivate->mSettings.mStdfRefLocation.isEmpty() == false)
    {
        if (mPrivate->mContext->m_AllSitesMap_Hbin.ComputeReferenceDieLocation(mPrivate->mSettings.mStdfRefLocation) == false)
        {
            mPrivate->mErrorMessage = "*GTF Error* Unable to assign reference die location to the input data file";
            return false;
        }

        QList<WaferCoordinate> referenceDies = mPrivate->mContext->m_AllSitesMap_Hbin.GetReferenceDieLocation();

        mPrivate->mContext->m_AllSitesMap_Sbin.SetReferenceDieLocation(referenceDies);
        mPrivate->mContext->m_Stdf_SbinMap.SetReferenceDieLocation(referenceDies);
        mPrivate->mContext->m_Stdf_HbinMap.SetReferenceDieLocation(referenceDies);
    }

    // Check that the number of die used as reference die is the same in both STDF map and external map.
    // If it doesnt' match, reject the PAT processing
    if (mPrivate->mContext->m_AllSitesMap_Hbin.GetReferenceDieLocation().count() !=
            mPrivate->mContext->m_ProberMap.GetReferenceDieLocation().count())
    {
        QString rootCause = QString("Reference die location count doesn't macth between STDF and External map (%1 vs %2")
                            .arg(mPrivate->mContext->m_AllSitesMap_Hbin.GetReferenceDieLocation().count())
                            .arg(mPrivate->mContext->m_ProberMap.GetReferenceDieLocation().count());

        GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());

        mPrivate->mErrorMessage = "*GTF Error* " + rootCause;

        return false;
    }

    return true;
}

bool PATProcessWS::CheckWafermapAlignment()
{
    // Perform maps alignement if requested
    if (mPrivate->mSettings.mCoordSystemAlignment.compare("STDF", Qt::CaseInsensitive) == 0 ||
        mPrivate->mSettings.mCoordSystemAlignment.compare("External", Qt::CaseInsensitive) == 0)
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Coordinate system use for alignment: %1")
              .arg(mPrivate->mSettings.mCoordSystemAlignment).toLatin1().data());

        WaferTransform lTransform;
        GS::Gex::MapAlignment lAlignment;

        if (mPrivate->mSettings.mOnOppositeAxisDir.compare("Flip", Qt::CaseInsensitive) == 0)
            lAlignment.SetOppositeAxisDirAction(MapAlignment::Reflect);
        else if (mPrivate->mSettings.mOnOppositeAxisDir.compare("Reject", Qt::CaseInsensitive) == 0)
            lAlignment.SetOppositeAxisDirAction(MapAlignment::Reject);
        else if (mPrivate->mSettings.mOnOppositeAxisDir.compare("NoAction", Qt::CaseInsensitive) &&
                 mPrivate->mSettings.mOnOppositeAxisDir.isEmpty() == false)
        {
            mPrivate->mErrorMessage = QString("Opposed Axis Direction action is invalid: %1.")
                    .arg(mPrivate->mSettings.mOnOppositeAxisDir);

            GSLOG(SYSLOG_SEV_ERROR, mPrivate->mErrorMessage.toLatin1().constData());
            return false;
        }

        if (lAlignment.PerformMapAlignment(mPrivate->mContext->m_AllSitesMap_Sbin,
                                                       mPrivate->mContext->m_ProberMap,
                                                       lTransform) == false)
        {
            mPrivate->mErrorMessage = lAlignment.GetErrorMessage();
            return false;
        }

        mPrivate->mContext->SetExternalTransformation(lTransform);

        CWaferMap::Alignment alignError;
        CWaferMap::Alignment lAlignCheck = CWaferMap::AlignBoundaryBins | CWaferMap::AlignGeometry |
                                           CWaferMap::AlignRefLocation;

        if (mPrivate->mSettings.mGeometryCheck.compare("Disable") == 0)
            lAlignCheck &= ~CWaferMap::AlignGeometry;

        alignError = mPrivate->mContext->m_ProberMap.CompareAlignment(mPrivate->mContext->m_AllSitesMap_Sbin,
                                                            lAlignCheck, mPrivate->mSettings.mAutoAlignBoundaryBins);

        // The alignment is not correct
        if (alignError != CWaferMap::AlignNone)
        {
            QString rootCause;

            switch(alignError)
            {
                case CWaferMap::AlignGeometry       :
                    rootCause = "Map geometries don't match.";
                    break;

                case CWaferMap::AlignAxisDirection  :
                    rootCause = "Axis directions don't match.";
                    break;

                case CWaferMap::AlignOrientation    :
                    rootCause = "Map orientations don't match.";
                    break;

                case CWaferMap::AlignRefLocation    :
                    rootCause = "Reference die locations don't match.";
                    break;

                case CWaferMap::AlignBoundaryBins   :
                    rootCause = "Boundary bins overlapped.";
                    break;

                default:
                    rootCause = "Unknown map alignment error.";
                    break;
            }

            mPrivate->mErrorMessage = "*PAT Error* Map alignment failed: " + rootCause;
            return false;
        }
    }
    else if (mPrivate->mSettings.mCoordSystemAlignment.isEmpty() == false)
    {
        QString rootCause = QString("Coordinate System alignment invalid: %1.")
                            .arg(mPrivate->mSettings.mCoordSystemAlignment);

        GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());

        mPrivate->mErrorMessage = "*GTF Error* " + rootCause;

        return false;
    }

    return true;
}

void PATProcessWS::CheckWafermapOrientation()
{
    // Force STDF orientation if requested
    char lStdfOrientation = mPrivate->mContext->m_AllSitesMap_Sbin.cWaferFlat_Stdf;

    if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationForceDown)
        lStdfOrientation = 'D';
    else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationForceLeft)
        lStdfOrientation = 'L';
    else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationForceUp)
        lStdfOrientation = 'U';
    else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationForceRight)
        lStdfOrientation = 'R';
    else if (toupper(lStdfOrientation) != 'U' && toupper(lStdfOrientation) != 'R' &&
             toupper(lStdfOrientation) != 'D' && toupper(lStdfOrientation) != 'L')
    {
        if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationDown)
            lStdfOrientation = 'D';
        else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationLeft)
            lStdfOrientation = 'L';
        else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationUp)
            lStdfOrientation = 'U';
        else if (mPrivate->mSettings.mStdfOrientation == PATProcessing::WOrientationRight)
            lStdfOrientation = 'R';
        else
            lStdfOrientation = mPrivate->mContext->m_AllSitesMap_Sbin.cWaferFlat_Active;
    }

    mPrivate->mContext->m_AllSitesMap_Sbin.cWaferFlat_Active  = lStdfOrientation;
    mPrivate->mContext->m_AllSitesMap_Hbin.cWaferFlat_Active  = lStdfOrientation;
    mPrivate->mContext->m_Stdf_SbinMap.cWaferFlat_Active      = lStdfOrientation;
    mPrivate->mContext->m_Stdf_HbinMap.cWaferFlat_Active      = lStdfOrientation;

    // Force External orientation if requested
    char lExternalOrientation = mPrivate->mContext->m_ProberMap.cWaferFlat_Stdf;

    if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationForceDown)
        lExternalOrientation = 'D';
    else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationForceLeft)
        lExternalOrientation = 'L';
    else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationForceUp)
        lExternalOrientation = 'U';
    else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationForceRight)
        lExternalOrientation = 'R';
    else if (toupper(lExternalOrientation) != 'U' && toupper(lExternalOrientation) != 'R' &&
             toupper(lExternalOrientation) != 'D' && toupper(lExternalOrientation) != 'L')
    {
        if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationDown)
            lExternalOrientation = 'D';
        else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationLeft)
            lExternalOrientation = 'L';
        else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationUp)
            lExternalOrientation = 'U';
        else if (mPrivate->mSettings.mExternalOrientation == PATProcessing::WOrientationRight)
            lExternalOrientation = 'R';
        else
            lExternalOrientation = mPrivate->mContext->m_ProberMap.cWaferFlat_Active;
    }

    mPrivate->mContext->m_ProberMap.cWaferFlat_Active  = lExternalOrientation;
}

/* [HTH] This code has to be removed
    Since we are automatically merging external maps and stdf map in memory,
  this check is not needed anymore
bool PATProcessWS::CheckWafermapConfiguration(const QStringList& lInputSTDF,
                                            const QString& lExternalMap)
{
    if(lInputSTDF.isEmpty())
        return false;

    QStringList lAllSTDFFiles = lInputSTDF;

    if (lExternalMap.isEmpty() == false)
        lAllSTDFFiles.append(lExternalMap);

    CWaferMap       lRefWafer;
    CWaferMap       lCurrentWafer;
    QString         lOptionWaferConfig = ReportOptions.GetOption("wafer", "wafermap_orien_check").toString();
    QtLib::Range    lAlignBoundaryBins;

    // Get the configuration for the 1st wafer which will be used as a reference
    bool    lWCRFound   = false;
    int     lIdxRef     = 0;

    for (lIdxRef = 0; lIdxRef <lAllSTDFFiles.count() && lWCRFound == false; ++lIdxRef)
    {
        if (ReadWafermapConfiguration(lAllSTDFFiles.at(lIdxRef), lRefWafer, lWCRFound) == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, mPrivate->mErrorMessage.toLatin1().constData());
            return false;
        }
    }

    // Loop over all other wafers, and check that the configuration is the same as for the
    // reference wafer
    for (int lIdx = lIdxRef; lIdx < lAllSTDFFiles.count(); ++lIdx)
    {
        if (ReadWafermapConfiguration(lAllSTDFFiles.at(lIdx), lCurrentWafer, lWCRFound) == false)
        {
            GSLOG(SYSLOG_SEV_WARNING, mPrivate->mErrorMessage.toLatin1().constData());
            return false;
        }

        if (lWCRFound)
        {
            CWaferMap::Alignment alignError;

            alignError = lRefWafer.CompareAlignment(lCurrentWafer,
                                                    (CWaferMap::AlignAxisDirection |
                                                     CWaferMap::AlignOrientation),
                                                    lAlignBoundaryBins);

            // The alignment is not correct
            if (alignError != CWaferMap::AlignNone)
            {
                QString rootCause;

                switch(alignError)
                {
                    case CWaferMap::AlignGeometry       :
                        rootCause = "Map geometries don't match";
                        break;

                    case CWaferMap::AlignAxisDirection  :
                        rootCause = "Axis directions don't match";
                        break;

                    case CWaferMap::AlignOrientation    :
                        rootCause = "Map orientations don't match";
                        break;

                    case CWaferMap::AlignRefLocation    :
                        rootCause = "Reference die locations don't match";
                        break;

                    case CWaferMap::AlignBoundaryBins   :
                        rootCause = "Boundary bins overlapped";
                        break;

                    default:
                        rootCause = "Unknown map alignment error";
                        break;
                }

                rootCause += QString(" between files %1 and %2.").arg(lAllSTDFFiles.at(lIdxRef))
                             .arg(lAllSTDFFiles.at(lIdx));

                if(lOptionWaferConfig == "error")
                {
                    mPrivate->mErrorMessage = "*PAT Error* Map Configurations differ: " + rootCause;
                    return false;
                }
                else
                {
                    Message::critical("", rootCause);
                    return true;
                }
            }
        }
    }

    return true;
}
*/

bool PATProcessWS::ConvertToSTDF(const QString &lFileName, QString& lSTDFFileName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          "Converting input file to STDF (if needed)");

    PAT_PERF_BENCH

    // Build output name and extension
    GS::Gex::ConvertToSTDF  StdfConvert;
    bool            lFileCreated    = false;
    int             lStatus         = false;

    lSTDFFileName   = Engine::GetInstance().Get("TempFolder").toString() + QDir::separator() +
                      QFileInfo(lFileName).fileName() + GEX_TEMPORARY_STDF;
    lStatus         = StdfConvert.Convert(false, true, false, true, lFileName, lSTDFFileName,
                                          "", lFileCreated, mPrivate->mErrorMessage);

    if(lStatus == GS::Gex::ConvertToSTDF::eConvertError)
        return false;

    // Check if Input file was already in STDF format!
    if(lFileCreated == false)
        lSTDFFileName = lFileName;
    else
        // Add file to list of temporary files to erase on Examinator exit...
        Engine::GetInstance().GetTempFilesManager().addFile(lSTDFFileName, TemporaryFile::BasicCheck);


    return true;
}

bool PATProcessWS::CreateSTDFDataFile(QString &lSTDFDataFile, QString& lExternalMap)
{
    QStringList lInputSources;

    // Convert all input file into STDF format
    for (int lIdx = 0; lIdx < mPrivate->mSettings.strSources.count(); ++lIdx)
    {
        QString lConvertedFile;

        if (ConvertToSTDF(mPrivate->mSettings.strSources.at(lIdx), lConvertedFile) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to convert STDF file file %1")
                  .arg(mPrivate->mSettings.strSources.at(lIdx)).toLatin1().constData());
            return false;
        }

        lInputSources.append(lConvertedFile);
    }

    if (mPrivate->mSettings.strOptionalSource.isEmpty() == false)
    {
        QString lOptionalSource  = mPrivate->mSettings.strOptionalSource.section(" ", 0, 0);

        if (ConvertToSTDF(lOptionalSource, lExternalMap) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR,
                  QString("Failed to convert external map file %1")
                  .arg(lOptionalSource).toLatin1().constData());
            return false;
        }
    }

    /* TO BE REMOVED
    if (CheckWafermapConfiguration(lInputSources, lExternalMap) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Wafermap configuration failed: %1")
              .arg(mPrivate->mErrorMessage).toLatin1().data());
        return false;
    }
    */


    // Merge Input STDF files into a single one if needed
    if (MergeInputSTDFFiles(lInputSources, lSTDFDataFile) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to merge STDF input files.");
        return false;
    }
    return true;
}

bool PATProcessWS::ExcludeIrrelevantDies()
{
    if (ApplyInclusionDiesList() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to apply inclusion dies list");
        return false;
    }

    // Scan each map of each flow...
    for(int lIdx = 0; lIdx < mPrivate->mContext->GetRecipeOptions().strFlowNames.count(); ++lIdx)
    {
        // Remove 'UglyDies' from STDF SBIN map
        if(mPrivate->mContext->GetRecipeOptions().strFlowKillSTDF_SBIN.size() > lIdx)
            RemoveDiesInMap(mPrivate->mContext->m_AllSitesMap_Sbin,
                            mPrivate->mContext->GetRecipeOptions().strFlowKillSTDF_SBIN[lIdx]);

        // Remove 'UglyDies' from STDF HBIN map
        if(mPrivate->mContext->GetRecipeOptions().strFlowKillSTDF_HBIN.size() > lIdx)
            RemoveDiesInMap(mPrivate->mContext->m_AllSitesMap_Hbin,
                            mPrivate->mContext->GetRecipeOptions().strFlowKillSTDF_HBIN[lIdx]);

            // Remove 'UglyDies' from MAP BIN map
        if(mPrivate->mContext->GetRecipeOptions().strFlowKillMAP_BIN.size() > lIdx)
            RemoveDiesInMap(mPrivate->mContext->m_ProberMap,
                            mPrivate->mContext->GetRecipeOptions().strFlowKillMAP_BIN[lIdx]);
    }

    return true;
}

bool PATProcessWS::FillInclusionDies()
{
    // Expecting one or parameters : <original file> <new file>
    if(mPrivate->mSettings.strOptionalSource.isEmpty() == true)
        return true;	// No input wafermap defined, no more work to do here!

    if (mPrivate->mContext->m_ProberMap.SizeX == 0 ||
            mPrivate->mContext->m_ProberMap.SizeY == 0)
    {
        mPrivate->mErrorMessage = "\t> *Error* No External map loaded";
        return false;
    }

    if (mPrivate->mContext->GetExternalBins() == NULL)
    {
        mPrivate->mErrorMessage = "\t> *Error* No External bins list";
        return false;
    }

    int         lDieCount = mPrivate->mContext->m_ProberMap.SizeX * mPrivate->mContext->m_ProberMap.SizeY;
    int         lCoordX   = GEX_WAFMAP_INVALID_COORD;
    int         lCoordY   = GEX_WAFMAP_INVALID_COORD;
    int         lBin      = GEX_WAFMAP_EMPTY_CELL;
    QString     lKey;
    CBinning *  pBinInfo  = NULL;

    for (int idx = 0; idx < lDieCount; ++idx)
    {
        // Get the bin of the die
        lBin = mPrivate->mContext->m_ProberMap.getWafMap()[idx].getBin();

        // Die has been tested, so check the bin status
        if (lBin != GEX_WAFMAP_EMPTY_CELL)
        {
            pBinInfo = mPrivate->mContext->GetExternalBins();

            while (pBinInfo && pBinInfo->iBinValue != lBin)
                pBinInfo = pBinInfo->ptNextBin;

            if (pBinInfo && pBinInfo->iBinValue == lBin)
            {
                if (pBinInfo->cPassFail == 'P' ||
                   (mPrivate->mContext->GetRecipeOptions().iPatLimitsFromBin == GEX_TPAT_BUILDLIMITS_ALLBINS))
                {
                    if (mPrivate->mContext->m_ProberMap.coordFromIndex(idx, lCoordX, lCoordY))
                    {
                        lKey = QString::number(lCoordX) + "." + QString::number(lCoordY);

                        // Save valid die location in our list
                        if(mPrivate->mInclusionDies.contains(lKey) == false)
                        {
                            // Holds the  Die X,Y position
                            mPrivate->mInclusionDies.insert(lKey, WaferCoordinate(lCoordX, lCoordY));
                        }
                    }
                }
            }
            else
            {
                mPrivate->mErrorMessage = QString("\t> *Error* Bin %1 not found in the external bins").arg(lBin);
                return false;
            }
        }
    }

    return true;
}

bool PATProcessWS::FilterExternalMap(const QString &lMapFile, const QString &lFilteredMap,
                                   int lCPUType, int lBin, QString lBinName) const
{
    GQTL_STDF::StdfParse	lSTDFMapFile;
    GQTL_STDF::StdfParse	lSTDFFilteredMapFile;
    int                         lStatus;
    int                         lRecordType;

    if(lSTDFMapFile.Open(lMapFile.toLatin1().constData(), STDF_READ) == false)
    {
        mPrivate->mErrorMessage = QString("*PAT Error* Failed to open STDF file %1").arg(lMapFile);
        return false;
    }

    if(lSTDFFilteredMapFile.Open(lFilteredMap.toLatin1().constData(), STDF_WRITE, lCPUType) == false)
    {
        mPrivate->mErrorMessage = QString("*PAT Error* Failed to create STDF file %1").arg(lMapFile);
        return false;
    }

    QList<int>  lSBinFailList;
    QList<int>  lHBinFailList;
    QList<int>  lAllowedRecord;
    int         lPRRCount   = 0;
    bool        lWrittenSBR = false;
    bool        lWrittenHBR = false;
    int         lHead       = 0;
    int         lSite       = 0;

    // HTH TO CHECK: Could not work as expected in case of interlaced PIR/PRR records
    // HTH TO CHECK: Should the bin be computer per head/site pair?
    GQTL_STDF::Stdf_Record *    lV4Record    = NULL;
    GQTL_STDF::Stdf_PIR_V4 *       lPIRRecord   = NULL;

    lAllowedRecord << GQTL_STDF::Stdf_Record::Rec_FAR << GQTL_STDF::Stdf_Record::Rec_MIR
                   << GQTL_STDF::Stdf_Record::Rec_WIR << GQTL_STDF::Stdf_Record::Rec_PIR
                   << GQTL_STDF::Stdf_Record::Rec_PRR << GQTL_STDF::Stdf_Record::Rec_WRR
                   << GQTL_STDF::Stdf_Record::Rec_SBR << GQTL_STDF::Stdf_Record::Rec_HBR
                   << GQTL_STDF::Stdf_Record::Rec_MRR;


    // Read one record from STDF file.
    lStatus = lSTDFMapFile.LoadNextRecord(&lRecordType);

    // Read file...and look for PRR that list the siteID
    while (lStatus == GS::StdLib::Stdf::NoError)
    {
        // Check the record is contained into the allowed record list
        if(lAllowedRecord.contains(lRecordType))
        {
            lV4Record = GQTL_STDF::CStdfRecordFactory::recordInstance(lRecordType);

            if(lV4Record && lSTDFMapFile.ReadRecord(lV4Record))
            {
                if (lRecordType == GQTL_STDF::Stdf_Record::Rec_PIR)
                    lPIRRecord = (GQTL_STDF::Stdf_PIR_V4 *) lV4Record;
                else if(lRecordType == GQTL_STDF::Stdf_Record::Rec_PRR)
                {
                    GQTL_STDF::Stdf_PRR_V4 * lPRRRecord = (GQTL_STDF::Stdf_PRR_V4 *) lV4Record;

                    if((lPRRRecord->m_b1PART_FLG & 0x10) == 0 && lPRRRecord->m_b1PART_FLG & 0x08)
                    {
                        //FAIL PRR replace IT BIN
                        lSBinFailList.append(lPRRRecord->m_u2SOFT_BIN);
                        lHBinFailList.append(lPRRRecord->m_u2HARD_BIN);
                        lPRRRecord->m_u2HARD_BIN = lBin;
                        lPRRRecord->m_u2SOFT_BIN = lBin;
                        lHead = lPRRRecord->m_u1HEAD_NUM;
                        lSite = lPRRRecord->m_u1SITE_NUM;

                        if(lPIRRecord)
                            lSTDFFilteredMapFile.WriteRecord(lPIRRecord);

                        lSTDFFilteredMapFile.WriteRecord(lPRRRecord);
                        lPRRCount++;
                    }
                }
                else if(lRecordType == GQTL_STDF::Stdf_Record::Rec_SBR)
                {
                    if(!lWrittenSBR)
                    {
                        //Write the first one and ignore the rest
                        GQTL_STDF::Stdf_SBR_V4 * lSBRRecord = (GQTL_STDF::Stdf_SBR_V4 *) lV4Record;

                        lSBRRecord->m_u1HEAD_NUM = lHead;
                        lSBRRecord->m_u1SITE_NUM = lSite;
                        lSBRRecord->m_u2SBIN_NUM = lBin;
                        lSBRRecord->m_u4SBIN_CNT = lPRRCount;
                        lSBRRecord->m_c1SBIN_PF  = 'F';
                        lSBRRecord->m_cnSBIN_NAM = lBinName;

                        lSTDFFilteredMapFile.WriteRecord(lSBRRecord);
                        lWrittenSBR = true;
                    }

                } else if(lRecordType == GQTL_STDF::Stdf_Record::Rec_HBR)
                {
                    if(!lWrittenHBR)
                    {
                        //Write the first one and ignore the rest
                        GQTL_STDF::Stdf_HBR_V4 * lHBRRecord = (GQTL_STDF::Stdf_HBR_V4*) lV4Record;

                        lHBRRecord->m_u1HEAD_NUM = lHead;
                        lHBRRecord->m_u1SITE_NUM = lSite;
                        lHBRRecord->m_u2HBIN_NUM = lBin;
                        lHBRRecord->m_u4HBIN_CNT = lPRRCount;
                        lHBRRecord->m_c1HBIN_PF  = 'F';
                        lHBRRecord->m_cnHBIN_NAM = lBinName;

                        lSTDFFilteredMapFile.WriteRecord(lHBRRecord);
                        lWrittenHBR = true;
                    }
                }
                else
                    lSTDFFilteredMapFile.WriteRecord(lV4Record);
            }

        }

        lStatus = lSTDFMapFile.LoadNextRecord(&lRecordType);
    };

    // Add file to list of temporary files to erase on Examinator exit...
    Engine::GetInstance().GetTempFilesManager().addFile(lFilteredMap, TemporaryFile::BasicCheck);

    return true;
}



bool PATProcessWS::GenerateOutputMaps()
{
    GSLOG(SYSLOG_SEV_NOTICE, "[Begin] output maps generation");

    PAT_PERF_BENCH

    CGexGroupOfFiles *	lGroup	= gexReport->getGroupsList().isEmpty() ?
                                      NULL : gexReport->getGroupsList().first();;

    if (lGroup == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    CGexFileInGroup * lFile   = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();
    if (lFile == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    // Overload Product name (enables to overload the Product name in the exported Wafermap INF file
    // with the one specified in the trigger file)
    if(!mPrivate->mSettings.m_strProductName.isEmpty())
        mPrivate->mSINFInfo.mDeviceName = mPrivate->mSettings.m_strProductName;

    // If Wafermap file to create, do it now!
    QString     lOutputWafermapPath;
    QString     lWaferFullFileName;
    int         lFormat;

    QMap<QString, QPair<PATProcessing::MapBinType, QString> >::const_iterator itBegin;
    QMap<QString, QPair<PATProcessing::MapBinType, QString> >::const_iterator itEnd;

    itBegin  = mPrivate->mSettings.mOutputFormat.constBegin();
    itEnd    = mPrivate->mSettings.mOutputFormat.constEnd();

    // Set Status to no error
    while (itBegin != itEnd)
    {
        if (WaferExport::IsSupportedOutputFormat(itBegin.key(), lFormat))
        {
            switch (itBegin.value().first)
            {
                case PATProcessing::MapHardBin:
                    // Reload Hard-Bin map
                    lFile->getWaferMapData() = mPrivate->mContext->m_AllSitesMap_Hbin;
                    break;

                case PATProcessing::MapSoftBin:
                    // Reload Soft-Bin map
                    lFile->getWaferMapData() = mPrivate->mContext->m_AllSitesMap_Sbin;
                    break;

                default:
                    if(mPrivate->mSettings.bOutputMapIsSoftBin == true)
                    {
                        // Reload Soft-Bin map
                        lFile->getWaferMapData() = mPrivate->mContext->m_AllSitesMap_Sbin;
                    }
                    else
                    {
                        // Reload Hard-Bin map
                        lFile->getWaferMapData() = mPrivate->mContext->m_AllSitesMap_Hbin;
                    }
                    break;
            }

            if (mPrivate->mSettings.mCoordSystemAlignment.compare("External", Qt::CaseInsensitive) == 0)
            {
                if (lFile->getWaferMapData().Transform(mPrivate->mContext->GetExternalTransformation().Inverted()) == false)
                {
                    QString lRootCause = "Failed to transform back the external map.";
                    mPrivate->mErrorMessage = "*PAT Error* " + lRootCause;
                    GSLOG(SYSLOG_SEV_ERROR, lRootCause.toLatin1().constData());
                    return false;
                }
            }

            if(itBegin.value().second.isEmpty())
            {
                QFileInfo lSTDFFileInfo(mPrivate->mContext->GetOutputDataFilename());
                lOutputWafermapPath = lSTDFFileInfo.absolutePath(); // Get path to STDF file created
            }
            else
                lOutputWafermapPath = itBegin.value().second;	// Custom wafermap output folder.

            WaferExport lWaferExport;

            lWaferExport.SetCustomer(mPrivate->mSettings.strCustomerName);
            lWaferExport.SetSupplier(mPrivate->mSettings.strSupplierName);
            lWaferExport.SetRotateWafer(mPrivate->mSettings.bRotateWafer);
            lWaferExport.SetXAxisDirection(mPrivate->mSettings.mOutputMapXDirection);
            lWaferExport.SetYAxisDirection(mPrivate->mSettings.mOutputMapYDirection);
            lWaferExport.SetOrientation(mPrivate->mSettings.mOutputMapNotch);
            lWaferExport.SetSINFInfo(mPrivate->mSINFInfo);
            lWaferExport.SetOriginalFiles(mPrivate->mSettings.strSources);

            // 1463
            // gcore-1463: moved just above generation of output file name to avoid a level 4 GSLOG, knowing this name
            // is not used for the Olympus format, which generates its own name from within the JS
            if (lFormat==WaferExport::outputOlympusAL2000)
            {
                GSLOG(SYSLOG_SEV_INFORMATIONAL, "Creating Olympus output map...");

                QString lRes=lWaferExport.CreateOlympusAL2kOutputMap(mPrivate->mSettings, lFile);
                if (!lRes.startsWith("ok"))
                {
                    QString lRootCause = QString("Failed to create output map %1: %2").arg(itBegin.key()).arg(lRes);
                    mPrivate->mErrorMessage="*PAT Error* "+lRootCause;
                    return false;
                }
                ++itBegin;
                continue;
            }

            // Full file path to create
            QFileInfo lFileInfo(lOutputWafermapPath);
            if(lFileInfo.isDir() == false)
                lWaferFullFileName = lOutputWafermapPath;
            else
                lWaferFullFileName = lOutputWafermapPath + "/" +
                                     lWaferExport.GenerateOutputName((WaferExport::Output) lFormat);
            if(mPrivate->mContext)
                mPrivate->mContext->addToGeneratedFilesList(lWaferFullFileName);

            GSLOG(SYSLOG_SEV_INFORMATIONAL,
                  QString("[Create] output map %1").arg(lWaferFullFileName).toLatin1().constData());

            if (lWaferExport.CreateOutputMap((WaferExport::Output)lFormat,
                                             lWaferFullFileName) == false)
            {
                QString lRootCause = QString("Failed to create output map %1: %2")
                                     .arg(itBegin.key()).arg(lWaferExport.GetErrorMessage());

                mPrivate->mErrorMessage = "*PAT Error* " + lRootCause;
                GSLOG(SYSLOG_SEV_WARNING, lRootCause.toLatin1().constData());

                return false;
            }
        }

        ++itBegin;
    }

    GSLOG(SYSLOG_SEV_NOTICE, "[End] output maps generation");

    return true;
}

bool PATProcessWS::ImportCompositeWaferLotRules()
{
    // If no Composite file defined:return!
    if(mPrivate->mSettings.strCompositeFile.isEmpty())
        return true;

    // If Composite file defined, check it exists
    if(QFile::exists(mPrivate->mSettings.strCompositeFile) == false)
    {
        mPrivate->mErrorMessage = "*Error* Failed to import Composite Wafer-lot info from file:\n  " +
                                  mPrivate->mSettings.strCompositeFile;
        return false;
    }

    // Open composite WaferLot result file  file
    QFile f(mPrivate->mSettings.strCompositeFile);
    if(!f.open(QIODevice::ReadOnly))
    {
        // Failed Opening Composite  file
        mPrivate->mErrorMessage = "*Error* Failed to read from Composite Wafer-lot file:\n  " +
                                  mPrivate->mSettings.strCompositeFile;
        return false;
    }

    // Assign file I/O stream
    QTextStream hCompositeFile(&f);

    QString strString = hCompositeFile.readLine();
    if(strString != "<composite_pat>")
    {
        mPrivate->mErrorMessage = "*Error* Corrupted Composite Wafer-lot file:\n  " +
                                  mPrivate->mSettings.strCompositeFile;
        return false;
    }

    // Seek to '<Exclusion_mask>' section
    do
    {
        // Read next line from file
        strString = hCompositeFile.readLine();

        if(strString.startsWith("<Exclusion_mask>", Qt::CaseInsensitive))
            break;
    }
    while(hCompositeFile.atEnd() == false);

    //  Read Exclusion zone mask
    int                 iWaferID;
    CWaferMap           lCompositeMap;

    if(lCompositeMap.loadFromCompositefile(hCompositeFile, iWaferID) == true)
    {
        // Exclusion zone exists: Create list of dies to keep based on exclusion list just loaded
        QString             lKey;
        int                 lCoordX;
        int                 lCoordY;
        int                 iBinCode;
        CPatDieCoordinates  lGoodBin;
        int                 iLine;
        int                 iCol;

        for(iLine=0;iLine < lCompositeMap.SizeY;iLine++)
        {
            for(iCol=0;iCol < lCompositeMap.SizeX;iCol++)
            {
                // Compute die location
                lCoordX = lCompositeMap.iLowDieX + iCol;
                lCoordY = lCompositeMap.iLowDieY + iLine;

                iBinCode    = lCompositeMap.getWafMap()[(iCol+(iLine * lCompositeMap.SizeX))].getBin();
                lKey        = QString::number(lCoordX) + "." + QString::number(lCoordY);

                // BinCode=1 means Keep die, 0=ignore die
                if(iBinCode)
                {
                    // Save valid die location in our list
                    if(mPrivate->mInclusionDies.contains(lKey) == false)
                    {
                        // Holds the  Die X,Y position
                        mPrivate->mInclusionDies.insert(lKey, WaferCoordinate(lCoordX, lCoordY));
                    }
                }
                else
                {
                    // Check if this is a good die cell, but to FAIL because of Z-Pat
                    if(mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList->Contains(mPrivate->mContext->m_AllSitesMap_Hbin.binValue(lCoordX, lCoordY, CWaferMap::LastTest)))
                    {
                        // This die has a low-yield on the stacked wafer...we must reject any good die at this location!
                        if (RemoveStdfRunCompositePAT(lCoordX, lCoordY) == false)
                        {
                            GSLOG(SYSLOG_SEV_ERROR,
                                  QString("Failed to remove samples using composite map: " +
                                          mPrivate->mErrorMessage).toLatin1().constData());
                            return false;
                        }

                        mPrivate->mContext->OverloadRefWafermap(lCoordX, lCoordY,
                                                      mPrivate->mContext->GetRecipeOptions().iCompositeZone_SBin,
                                                      mPrivate->mContext->GetRecipeOptions().iCompositeZone_HBin);

                        // Save valid die location in our list
                        if(mPrivate->mContext->mZPATOutliers.contains(lKey) == false)
                        {
                            // Die location details
                            lGoodBin.mDieX          = lCoordX;
                            lGoodBin.mDieY          = lCoordY;
                            lGoodBin.mSite          = -1;	// Site unkown

                            // For Good die at low-yield location, overload master maps bin (Force to Fail die on wafermap! So next rules can inherit & process this failure)
                            lGoodBin.mFailType      = GEX_TPAT_BINTYPE_ZPAT;
                            lGoodBin.mRuleName      = "ZPAT";
                            lGoodBin.mPatHBin       = mPrivate->mContext->GetRecipeOptions().iCompositeZone_HBin;
                            lGoodBin.mPatSBin       = mPrivate->mContext->GetRecipeOptions().iCompositeZone_SBin;
                            lGoodBin.mOrigHBin      = mPrivate->mContext->GetOriginalBin(false, lCoordX, lCoordY);
                            lGoodBin.mOrigSBin      = mPrivate->mContext->GetOriginalBin(true, lCoordX, lCoordY);
                            lGoodBin.mPartId        = gexReport->getDiePartId(-1, lCoordX, lCoordY);

                            mPrivate->mContext->mZPATOutliers.insert(lKey, lGoodBin);	// Holds the  Die X,Y position
                        }
                    }
                }
            }
        }

        PATOutlierFinderWS lWSEngine(mPrivate->mContext);
        if (lWSEngine.AnalyzeWaferSurfaceZPAT(&lCompositeMap) == false)
        {
            f.close();
            return false;
        }

        // Remove all good dies that failed GDBN rule!
        tdIterGPATOutliers itGDBN(mPrivate->mContext->mGDBNOutliers);
        while(itGDBN.hasNext())
        {
            itGDBN.next();

            if (RemoveStdfRunCompositePAT(itGDBN.value().mDieX, itGDBN.value().mDieY) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Failed to remove samples using composite map: " +
                              mPrivate->mErrorMessage).toLatin1().constData());
                return false;
            }
        }

        // Remove all good dies that failed Reticle rule!
        tdIterGPATOutliers itReticle(mPrivate->mContext->mReticleOutliers);
        while (itReticle.hasNext())
        {
            itReticle.next();

            if (RemoveStdfRunCompositePAT(itReticle.value().mDieX, itReticle.value().mDieY) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Failed to remove samples using composite map: " +
                              mPrivate->mErrorMessage).toLatin1().constData());
                return false;
            }
        }

        // Remove all good dies that failed Clustering rule!
        tdIterGPATOutliers itClustering(mPrivate->mContext->mClusteringOutliers);
        while (itClustering.hasNext())
        {
            itClustering.next();

            if (RemoveStdfRunCompositePAT(itClustering.value().mDieX, itClustering.value().mDieY) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Failed to remove samples using composite map: " +
                              mPrivate->mErrorMessage).toLatin1().constData());
                return false;
            }
        }
    }

    f.close();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// Z-PAT composite use only; Remove all tests in a given DieX,DieY run
/////////////////////////////////////////////////////////////////////////////
bool PATProcessWS::RemoveStdfRunCompositePAT(int lCoordX, int lCoordY)
{
    CGexGroupOfFiles *	pGroup      = NULL;
    CGexFileInGroup	 *	pFile       = NULL;
    CTest *             ptDieX      = NULL;
    CTest *             ptDieY      = NULL;
    CTest *             ptHardBin   = NULL;
    CTest *             ptSoftBin   = NULL;
    int                 lHardBin;
    int                 lSamplesExecs = 0;

    QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());

    while(itGroupsList.hasNext())
    {
        pGroup	= itGroupsList.next();
        pFile	= (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();

        // Get handle to Die X,Y info
        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEX,GEX_PTEST, &ptDieX, true, false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on die X coordinates";
            return false;
        }

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_DIEY,GEX_PTEST, &ptDieY, true, false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on die Y coordinates";
            return false;
        }

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_SBIN,GEX_PTEST, &ptSoftBin, true, false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on Soft Bin";
            return false;
        }

        if(pFile->FindTestCell(GEX_TESTNBR_OFFSET_EXT_HBIN,GEX_PTEST, &ptHardBin, true, false) != 1)
        {
            mPrivate->mErrorMessage = "Unable to find parameters on Hard Bin";
            return false;
        }

        lSamplesExecs   = gex_min(ptDieX->m_testResult.count(), ptDieY->m_testResult.count());
        lSamplesExecs   = gex_min(lSamplesExecs, ptHardBin->m_testResult.count());
        lSamplesExecs   = gex_min(lSamplesExecs, ptSoftBin->m_testResult.count());

        // Scan all parts tested.
        for (int lIdx = 0; lIdx < lSamplesExecs; ++lIdx)
        {
            lHardBin  = (int) ptHardBin->m_testResult.resultAt(lIdx);

            // If this Bin is good, force to fail it (because stacked wafer yield for this location is low)
            if((mPrivate->mContext->GetRecipeOptions().pGoodHardBinsList->Contains(lHardBin) == true) &&
               ((int)ptDieX->m_testResult.resultAt(lIdx) == lCoordX) &&
               ((int)ptDieY->m_testResult.resultAt(lIdx) == lCoordY))
            {
                ptSoftBin->m_testResult.forceResultAt(lIdx, mPrivate->mContext->GetRecipeOptions().iCompositeZone_HBin);
                ptHardBin->m_testResult.forceResultAt(lIdx, mPrivate->mContext->GetRecipeOptions().iCompositeZone_SBin);
            }
        }
    };

    return true;
}

bool PATProcessWS::LoadExternalMap(const QString &lFileName)
{
    PAT_PERF_BENCH

    // If no file specified, quietly return
    if(lFileName.isEmpty())
        return true;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Loading External Input map: %1").arg(lFileName).toLatin1().constData());


    QString lMapFile = lFileName;

    // If file specified, check if exists
    if(QFile::exists(lMapFile) == false)
    {
        // The map name has two paths as we need to update it; two parameters
        // then: <original file> <new file>
        lMapFile = lMapFile.section(' ',0,0);

        // If original file doesn't exists, fail processing
        if(QFile::exists(lMapFile) == false)
        {
            mPrivate->mErrorMessage = "Failed to open input file.\nFile missing? : " + lMapFile;
            return false;
        }
    }

    // Have the Prober file analysed...
    FILE *              hFile       = NULL;
    QString             lScriptFile = Engine::GetInstance().GetAssistantScript();
    CGexGroupOfFiles *  pGroup      = NULL;
    CGexFileInGroup *   pFile       = NULL;

    // Create script that will read data file so we save the global wafermap.
    hFile = fopen(lScriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        mPrivate->mErrorMessage = "Failed to create script file : " + lScriptFile;
        return false;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        mPrivate->mErrorMessage = "Can't write option section";
        return false;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // Write all groups (one per site)
    lMapFile = ConvertToScriptString(lMapFile);	// Make sure any '\' in string is doubled.

    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_1');\n");
    fprintf(hFile,
            "  gexFile(group_id,'insert','%s','all','all',' ','');\n\n",
            lMapFile.toLatin1().constData());
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");

    // Avoids editor to be launched if output options is set to Word or Spreadsheet CSV!
    fprintf(hFile,"  gexOptions('output','format','html');\n");
    // Choose test merge option based on recipe test key option
    switch (mPrivate->mContext->GetRecipeOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }
    // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");
    // Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
    // Disable outlier removal so to make sure all values are taken into account.
    fprintf(hFile,"  gexOptions('dataprocessing','data_cleaning_mode','none');\n");
    // Ensure FULL wafermap is created in any case, non matter the filtering of parts
    fprintf(hFile,"  gexOptions('wafer','visual_options','all_parts');\n");
    // Use 'auto' mode for positive X direction
    fprintf(hFile,"  gexOptions('wafer','positive_x','auto');\n");
    // Use 'auto' mode for positive Y direction
    fprintf(hFile,"  gexOptions('wafer','positive_y','auto');\n");
    // Use 'From data file if specified, else detect' mode for flat/notch orientation
    fprintf(hFile,"  gexOptions('wafer','notch_location','file_or_detected');\n");
    // Use wafer map as binning source information
    fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
    // Disable STDF.FTR processing (for PAT speed improvement since FTR are not used in PAT algorihtms)
    fprintf(hFile,"  gexOptions('dataprocessing', 'functional_tests', 'disabled');\n");

    fprintf(hFile,"  SetProcessData();\n");

    // Only data analysis, no report created!
    fprintf(hFile,"  gexOptions('report','build','false');\n");

    fprintf(hFile,"  gexBuildReport('hide','0');\n");	// Do NOT show report tab
    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    if (CSLEngine::GetInstance().RunScript(lScriptFile).IsFailed())
    {
        mPrivate->mErrorMessage = "Failed to convert external map: " + lMapFile;
        return false;
    }

    // Get handle to dataset just analyzed
    pGroup = gexReport->getGroupsList().isEmpty()?NULL:gexReport->getGroupsList().first();
    if(pGroup == NULL)
        return false;
    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
        return false;

    // Load Wafermap arra   y with SoftBins
    gexReport->FillWaferMap(pGroup, pFile, NULL, GEX_WAFMAP_HARDBIN, true);

    // Get wafermap and save it into PAT structure for later use...
    mPrivate->mContext->m_ProberMap   = pFile->getWaferMapData();

    if (mPrivate->mSettings.mExternalFailOverload > 0)
    {
        int         lBinProber;
        CBinning *  lExternalBinInfo = NULL;

        for (int lIdxProber = 0; lIdxProber < mPrivate->mContext->m_ProberMap.SizeX * mPrivate->mContext->m_ProberMap.SizeY;
             ++lIdxProber)
        {
            lBinProber          = mPrivate->mContext->m_ProberMap.getWafMap()[lIdxProber].getBin();
            lExternalBinInfo    = pFile->m_ParentGroup->cMergedData.ptMergedHardBinList->Find(lBinProber);

            if (lBinProber != GEX_WAFMAP_EMPTY_CELL)
            {
                if (lExternalBinInfo == NULL)
                {
                    QString rootCause = QString("Bin %1 from external map not found in the external bins list").arg(lBinProber);
                    GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());

                    mPrivate->mErrorMessage = "*PAT Error* External map bins overload failed: " +
                                              rootCause;

                    return false;
                }
                else
                    if (lExternalBinInfo->cPassFail == 'F')
                        mPrivate->mContext->m_ProberMap.getWafMap()[lIdxProber].setBin(mPrivate->mSettings.mExternalFailOverload);
            }
        }

        CBinning *  lBinInfo    = NULL;
        CBinning    lFailBinInfo;

        lExternalBinInfo            = pFile->m_ParentGroup->cMergedData.ptMergedHardBinList;
        lFailBinInfo.iBinValue      = mPrivate->mSettings.mExternalFailOverload;
        lFailBinInfo.cPassFail      = 'F';
        lFailBinInfo.ldTotalCount   = 0;
        lFailBinInfo.strBinName     = "External Failed Bin";

        while (lExternalBinInfo)
        {
            if (lExternalBinInfo->cPassFail == 'P')
            {
                if (lBinInfo)
                    lBinInfo->Append(lExternalBinInfo->iBinValue, lExternalBinInfo->cPassFail,
                                     lExternalBinInfo->ldTotalCount, lExternalBinInfo->strBinName);
                else
                {
                    lBinInfo = new CBinning;

                    lBinInfo->cPassFail     = lExternalBinInfo->cPassFail;
                    lBinInfo->iBinValue     = lExternalBinInfo->iBinValue;
                    lBinInfo->ldTotalCount  = lExternalBinInfo->ldTotalCount;
                    lBinInfo->strBinName    = lExternalBinInfo->strBinName;
                }
            }
            else
                lFailBinInfo.ldTotalCount += lExternalBinInfo->ldTotalCount;

            lExternalBinInfo = lExternalBinInfo->ptNextBin;
        }

        if (lFailBinInfo.ldTotalCount > 0)
        {
            if (lBinInfo)
                lBinInfo->Append(lFailBinInfo.iBinValue, lFailBinInfo.cPassFail,
                                 lFailBinInfo.ldTotalCount, lFailBinInfo.strBinName);
            else
            {
                lBinInfo = new CBinning;

                lBinInfo->cPassFail     = lFailBinInfo.cPassFail;
                lBinInfo->iBinValue     = lFailBinInfo.iBinValue;
                lBinInfo->ldTotalCount  = lFailBinInfo.ldTotalCount;
                lBinInfo->strBinName    = lFailBinInfo.strBinName;
            }
        }

        mPrivate->mContext->SetExternalBins(lBinInfo);
    }
    else
        mPrivate->mContext->SetExternalBins(pFile->m_ParentGroup->cMergedData.ptMergedHardBinList->Clone());

    return true;
}

bool PATProcessWS::MergeInputSTDFFiles(const QStringList &lInputFiles, QString &lMergedInput)
{
    if (lInputFiles.count() == 0)
    {
        mPrivate->mErrorMessage = "No source files defined.";
        return false;
    }
    else if(lInputFiles.count() > 1)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              "Merging STDF input files");

        GexTbMergeRetest lMergeRetest(false);

        // Build output file name to use when merging files.
        QFileInfo   lSourceFileInfo(lInputFiles.first());
        QFileInfo   lOutputFolderInfo;

        if (mPrivate->mSettings.mOutputFolderSTDF.isEmpty())
            lOutputFolderInfo.setFile(lInputFiles.first());
        else
            lOutputFolderInfo.setFile(mPrivate->mSettings.mOutputFolderSTDF);

        if (lOutputFolderInfo.isDir())
            lMergedInput  = lOutputFolderInfo.absoluteFilePath();
        else
            lMergedInput  = lOutputFolderInfo.absolutePath();

        lMergedInput  += QDir::separator();
        lMergedInput  += lSourceFileInfo.baseName();
        lMergedInput  += "_merged.stdf";

        // Remove temporary merged STDF file if already exists..
        Engine::RemoveFileFromDisk(lMergedInput);

        // Merge files (all samples data, ignore summary sections and HBR/SBR)
        int	lStatus = lMergeRetest.MergeSamplesFiles(lInputFiles,
                                                     lMergedInput, true, true);


        // Check Status and display message accordingly.
        if(lStatus != GexTbMergeRetest::NoError)
        {
            mPrivate->mErrorMessage = lMergeRetest.GetErrorMessage();
            return false;
        }

        Engine::GetInstance().GetTempFilesManager().addFile(lMergedInput, TemporaryFile::BasicCheck);
    }
    else
        lMergedInput = lInputFiles.first();

    // 1463 : let s keep a trace of this central file name
    mPrivate->mSettings.setProperty("InputDataFile", lMergedInput);

    return true;
}

bool PATProcessWS::PrepareData()
{
    QString     lExternalMap;

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Preparing PAT data : load recipe %1").arg(mPrivate->mSettings.strRecipeFile)
          .toLatin1().constData());

    // Save configuration file name
    mPrivate->mContext->SetRecipeFilename(mPrivate->mSettings.strRecipeFile);
    mPrivate->mContext->SetReticleStepInfo(mPrivate->mSettings.mReticleStepInfo);

    // Load configuration file
    if (LoadRecipeFile() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to load recipe file.");
        return false;
    }

    // No data file specified, so use information coming from the settings
    if (mPrivate->mContext->strDataFile.isEmpty())
    {
        if (CreateSTDFDataFile(mPrivate->mContext->strDataFile, lExternalMap) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to create merged data file.");
            return false;
        }
    }
    else
    {
        if (mPrivate->mSettings.strOptionalSource.isEmpty() == false)
        {
            QString lOptionalSource  = mPrivate->mSettings.strOptionalSource.section(" ", 0, 0);

            if (ConvertToSTDF(lOptionalSource, lExternalMap) == false)
            {
                GSLOG(SYSLOG_SEV_ERROR,
                      QString("Failed to convert external map file %1")
                      .arg(lOptionalSource).toLatin1().constData());
                return false;
            }
        }
    }

    // Set the name of the STDF data file to use for PAT processing
    mPrivate->mContext->SetSTDFFilename(mPrivate->mContext->strDataFile);

    if (LoadExternalMap(lExternalMap) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to load external map file.");
        return false;
    }

    if (LoadSTDF(mPrivate->mContext->GetSTDFFilename()) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to load STDF maps.");
        return false;
    }

    // Check if we have the reticle information
    if (mPrivate->mContext->GetReticleStepInformation() == NULL &&
        mPrivate->mSettings.mReticleStepInfo.compare("none", Qt::CaseInsensitive) &&
        mPrivate->mSettings.mReticleStepInfo.isEmpty() == false)
    {
        if (mPrivate->mSettings.mReticleStepInfo.compare("map_file", Qt::CaseInsensitive) == 0)
            mPrivate->mErrorMessage = "No Reticle Step Information found in Map file";
        else if (mPrivate->mSettings.mReticleStepInfo.compare("input_file", Qt::CaseInsensitive) == 0)
            mPrivate->mErrorMessage = "No Reticle Step Information found in Input file";
        else
            mPrivate->mErrorMessage = "No Reticle Step Information found.";

        return false;
    }

    // Check if the file has enough data
    CGexGroupOfFiles *	lGroup	= gexReport->getGroupsList().isEmpty() ?
                                      NULL : gexReport->getGroupsList().first();;

    if (lGroup == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    CGexFileInGroup *	lFile   = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if (lFile == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    int     lPartsLimit = (int)(mPrivate->mSettings.fValidWaferThreshold_GrossDieRatio *
                                (float)mPrivate->mSettings.iGrossDiePerWafer);

    if(lFile->getWaferMapData().iTotalPhysicalDies < lPartsLimit)
    {
        // Not enough parts in wafer
        if(mPrivate->mSettings.bDeleteIncompleteWafers)
        {
            // Delete list of files used as 'input data source'
            for(QStringList::Iterator it = mPrivate->mSettings.strSources.begin();
                 it != mPrivate->mSettings.strSources.end(); ++it )
            {
                // Get file name to remove.
                Gex::Engine::GetInstance().RemoveFileFromDisk(*it);	// Delete file if exists
            }
        }

        mPrivate->mErrorMessage = "*GTF Error* Not enough parts in wafer: " +
                                  QString::number(lFile->getWaferMapData().iTotalPhysicalDies);
        mPrivate->mErrorMessage += " < " + QString::number(lPartsLimit);

        return false;
    }

    return true;
}

bool PATProcessWS::PreProcessing()
{
    if (CheckReferenceDieLocation() == false)
        return false;

    // Check whether we have to force positive X and/or Y direction
    CheckAxisDirection();

    // Check whether we have to force the wafermap orientation
    CheckWafermapOrientation();

    // Perform maps alignment if requested
    if (CheckWafermapAlignment() == false)
        return false;

    // Perform maps merge if requested
    if (mPrivate->mSettings.mMergeMaps)
    {
        GSLOG(SYSLOG_SEV_NOTICE,
              QString("Merging STDF and External maps using bin# %1 as missing STDF die")
              .arg(mPrivate->mSettings.mMergeBinGoodMapMissingSTDF).toLatin1().constData());

        if (mPrivate->mSettings.mMergeBinRuleJSHook.isEmpty() == false)
        {
            GSLOG(SYSLOG_SEV_NOTICE,
                  QString("Merging STDF and External maps using %1 hook to merge binning")
                  .arg(mPrivate->mSettings.mMergeBinRuleJSHook).toLatin1().constData());
        }

        // Merge the soft bin maps
        GS::Gex::MergeMaps lMergeMaps;
        QList<CWaferMap*> lWaferMaps;
        CWaferMap lOutputMap;
        lWaferMaps.append(&mPrivate->mContext->m_AllSitesMap_Sbin);
        lWaferMaps.append(&mPrivate->mContext->m_ProberMap);
        QList<CBinning *>  lBinsMaps;
        CBinning* lOutputBin = (mPrivate->mContext->GetSoftBins())->Clone();
        lBinsMaps.append(mPrivate->mContext->GetSoftBins());
        lBinsMaps.append(mPrivate->mContext->GetExternalBins());
        if (lMergeMaps.MergeWaferMaps(lWaferMaps,
                                      lBinsMaps,
                                      mPrivate->mSettings.mMergeBinRuleJSHook,
                                      mPrivate->mErrorMessage,
                                      lOutputMap,
                                      lOutputBin,
                                      mPrivate->mSettings.mMergeBinGoodMapMissingSTDF) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to merge sbin map with the external map");
            return false;
        }
        mPrivate->mContext->m_AllSitesMap_Sbin = lOutputMap;
        mPrivate->mContext->SetSoftBins(lOutputBin);
        if (lOutputBin)
        {
            CBinning * lTmpBin = NULL;
            while(lOutputBin != NULL)
            {
              lTmpBin = lOutputBin->ptNextBin;
              delete lOutputBin;
              lOutputBin = lTmpBin;
            };
            lOutputBin = NULL;
        }

        // Merge the hard bin maps
        lWaferMaps.clear();
        lWaferMaps.append(&mPrivate->mContext->m_AllSitesMap_Hbin);
        lWaferMaps.append(&mPrivate->mContext->m_ProberMap);
        lBinsMaps.clear();
        lBinsMaps.append(mPrivate->mContext->GetHardBins());
        lBinsMaps.append(mPrivate->mContext->GetExternalBins());
        lOutputBin = (mPrivate->mContext->GetSoftBins())->Clone();

        if (lMergeMaps.MergeWaferMaps(lWaferMaps,
                                      lBinsMaps,
                                      mPrivate->mSettings.mMergeBinRuleJSHook,
                                      mPrivate->mErrorMessage,
                                      lOutputMap,
                                      lOutputBin,
                                      mPrivate->mSettings.mMergeBinGoodMapMissingSTDF) == false)
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to merge soft bin map with the external map");
            return false;
        }
        mPrivate->mContext->m_AllSitesMap_Hbin = lOutputMap;
        mPrivate->mContext->SetHardBins(lOutputBin);
        if (lOutputBin)
        {
            CBinning * lTmpBin = NULL;
            while(lOutputBin != NULL)
            {
              lTmpBin = lOutputBin->ptNextBin;
              delete lOutputBin;
              lOutputBin = lTmpBin;
            };
            lOutputBin = NULL;
        }
    }

    // Fill inclusion dies list from the External Map
    if (FillInclusionDies() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to load inclusion dies list");
        return false;
    }

    // Apply Composite maps
    if (ImportCompositeWaferLotRules() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to process composite map inclusion dies list");
        return false;
    }

    // Exclude irrelevant dies depending on different situation
    if (ExcludeIrrelevantDies() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to exclude irrelevant dies");
        return false;
    }

    return true;
}

bool PATProcessWS::Processing()
{
    PAT_PERF_BENCH

    QFileInfo   lFileInfo(mPrivate->mContext->GetSTDFFilename());
    QString     lOutputSTDFFile;
    bool        lCustomOutput = false;

    if(mPrivate->mSettings.mOutputFolderSTDF.isEmpty())
    {
        // Output file is input file name with _patman.std extension!
        lOutputSTDFFile = lFileInfo.absolutePath() + QDir::separator() +
                          lFileInfo.completeBaseName() + "_patman";
    }
    else
    {
        // Output path defined, then simply build file name...unless full name specified (instead of only a folder)
        QFileInfo lFolder(mPrivate->mSettings.mOutputFolderSTDF);
        if(lFolder.isDir() == false &&
           (lFolder.suffix().compare("stdf", Qt::CaseInsensitive) ||
            lFolder.suffix().compare("std", Qt::CaseInsensitive)))
        {
            lCustomOutput = true;
        }

        lOutputSTDFFile = mPrivate->mSettings.mOutputFolderSTDF;

        if(lCustomOutput == false)
        {
            if(lOutputSTDFFile.endsWith("/") == false)
                lOutputSTDFFile += "/";

            lOutputSTDFFile += lFileInfo.completeBaseName() + "_patman";
        }
    }

    // If no custom output name defined, build it, ensure extension is correct.
    if(lCustomOutput == false)
    {
        if(lFileInfo.suffix().compare("stdf", Qt::CaseInsensitive) == 0)
            lOutputSTDFFile += ".stdf";
        else
            lOutputSTDFFile += ".std";
    }

#ifdef MV_PAT_TEST
    PATMultiVariateRule lMVRule;
    lMVRule.AddTestData(PATMultiVariateRule::MVTestData("X__Voq(0mT)", 580, -1));
    lMVRule.AddTestData(PATMultiVariateRule::MVTestData("NB__Uout@0mT", 4325, -1));
    lMVRule.SetBin(250);
    lMVRule.SetEnabled(true);
    lMVRule.SetName("My Test rule");
    lMVRule.SetOutlierDistanceMode(PAT::Near);

    QList<PATMultiVariateRule> lRules;
    lRules << lMVRule;

    mPrivate->mContext->cOptionsPat.SetMVPATEnabled(true);
    mPrivate->mContext->SetMVRules(lRules);
    mPrivate->mContext->cOptionsPat.SetMVPATDistance(PAT::Near, 6);
    mPrivate->mContext->cOptionsPat.SetMVPATReportCorrCharts(true);
    mPrivate->mContext->cOptionsPat.SetMVPATReportPairs(PAT::ConsecutivePairs);
    mPrivate->mContext->cOptionsPat.SetMVPATReportPCAProjection(true);
#endif // MV_PAT_TEST

    PATStdfUpdate lSTDFUpdate;

    lSTDFUpdate.SetPATSettings(mPrivate->mSettings);

    if(mPrivate->mContext)
        mPrivate->mContext->addToGeneratedFilesList(lOutputSTDFFile);

    if (lSTDFUpdate.Execute(mPrivate->mContext->GetSTDFFilename(), lOutputSTDFFile) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Failed to PAT process %1").arg(mPrivate->mContext->GetSTDFFilename())
              .toLatin1().constData());

        mPrivate->mErrorMessage = lSTDFUpdate.GetErrorMessage();

        return false;
    }

    mPrivate->mContext->SetOutputDataFilename(lOutputSTDFFile);

    return true;
}

bool PATProcessWS::PostProcessing()
{
    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : entering PostProcessing...");

    // Consolidate PAT statistics (accurate failing die count, etc...)
    if (mPrivate->mContext->ConsolidatePATStatistics() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to consolidate PAT Statistics");

        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : Consolidation PAT stats done.");

    // 1463
    mPrivate->mSettings.Set("TotalGoodAfterPAT", mPrivate->mContext->GetTotalGoodPartsPostPAT());
    mPrivate->mSettings.Set("TotalPATFails", mPrivate->mContext->GetTotalPATFailingParts());


    // Save 'process finished' timestamp
    mPrivate->mSettings.dtProcessEndTime = QDateTime::currentDateTime();

    if (UpdateExternalMapFile(mPrivate->mSettings.strOptionalSource,
                              mPrivate->mSettings.strOptionalOutput) == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to Update External Map file");

        // Delete STDF file created...as failed creating wafmap files
        QFile::remove(mPrivate->mContext->GetOutputDataFilename());
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : External map updated.");

    if (GenerateOutputMaps() == false)
    {
        GSLOG(SYSLOG_SEV_ERROR, "Failed to generate output map");
        return false;
    }

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : Output map generated.");

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : exiting PostProcessing...");

    return true;
}

bool PATProcessWS::LoadRecipeFile()
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("Loading Recipe File: %1").arg(mPrivate->mSettings.strRecipeFile).toLatin1().constData());

    PAT_PERF_BENCH

    // If reipce file doesn't exist, fail processing
    if(QFile::exists(mPrivate->mSettings.strRecipeFile) == false)
    {
        mPrivate->mErrorMessage = "Recipe file not found: " + mPrivate->mSettings.strRecipeFile;
        return false;
    }

    QSharedPointer<GS::Gex::PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(mPrivate->mSettings.strRecipeFile));
    if (lRecipeIO.isNull())
    {
        GSLOG(SYSLOG_SEV_DEBUG,
              QString("Error to load the file %1").arg(mPrivate->mSettings.strRecipeFile).toLatin1().constData());

        mPrivate->mErrorMessage = QString("Invalid recipe file format: %1").arg(mPrivate->mSettings.strRecipeFile);
        return false;
    }
    lRecipeIO->SetRecipeName(mPrivate->mSettings.strRecipeFile);

    if (lRecipeIO->Read(mPrivate->mContext->GetRecipe()) == false)
    {
        mPrivate->mErrorMessage = lRecipeIO->GetErrorMessage();
        return false;
    }

    if (mPrivate->mContext->GetRecipeOptions().GetRecipeType() != GS::Gex::PAT::RecipeWaferSort)
    {
        mPrivate->mErrorMessage = "*PAT Error* Wrong Recipe type";
        return false;
    }

    // If Custom PAT Library to be used, then load plugin now!
    if(mPrivate->mContext->GetRecipeOptions().bCustomPatLib)
    {
        // Load PAT Library...
        if(!mPrivate->mExternalPAT.LoadPlugin(mPrivate->mContext->GetRecipeOptions().strCustomPatLibFile))
        {
            // Failed loading library.
            mPrivate->mErrorMessage = "*PAT Error* Failed loading recipe; ";
            mPrivate->mErrorMessage += "missing PAT library: ";
            mPrivate->mErrorMessage += mPrivate->mContext->GetRecipeOptions().strCustomPatLibFile;
            return false;
        }
    }

//    if (OverloadRecipeFile() == false)
//    {
//        GSLOG(SYSLOG_SEV_WARNING,
//              QString("Failed to load Overlaoad Recipe file: %1").arg(mPrivate->mSettings.strRecipeOverloadFile).toLatin1().data());
//        return false;
//    }

    // Now that recipe is loaded, check if we have STDF output overloading options
    // Check STDF Output Overload option (eg: from ST Production recipe)
    switch(mPrivate->mContext->GetRecipeOptions().iSTDF_Output)
    {
        // STDF output mode is default, change it!
        case GEX_PAT_STDF_OUTPUT_DEFAULT:
            switch(mPrivate->mSettings.mOutputDatalogFormat)
            {
                case GEX_TPAT_DLG_OUTPUT_NONE:
                    mPrivate->mContext->GetRecipeOptions().iSTDF_Output = GEX_PAT_STDF_OUTPUT_DISABLED;
                    break;
                case GEX_TPAT_DLG_OUTPUT_STDF:
                    mPrivate->mContext->GetRecipeOptions().iSTDF_Output = GEX_PAT_STDF_OUTPUT_FULL;
                    break;
                case GEX_TPAT_DLG_OUTPUT_ATDF:
                    mPrivate->mContext->GetRecipeOptions().iSTDF_Output = GEX_PAT_STDF_OUTPUT_FULL;
                    break;
            }
            break;

        case GEX_PAT_STDF_OUTPUT_DISABLED:	// STDF output disabled
            mPrivate->mSettings.mOutputDatalogFormat = GEX_TPAT_DLG_OUTPUT_NONE;
            break;

        case GEX_PAT_STDF_OUTPUT_WAFERMAP:	// STDF output: light mode, only wafermap included
            mPrivate->mSettings.mOutputDatalogFormat = GEX_TPAT_DLG_OUTPUT_STDF; // WARNING: Will need to be changed when Wafermap-only STDF file implemented
            break;

        case GEX_PAT_STDF_OUTPUT_FULL:	// STDF output: full mode (parametric + wafermap)
            mPrivate->mSettings.mOutputDatalogFormat = GEX_TPAT_DLG_OUTPUT_STDF;
            break;
    }

    return true;
}

bool PATProcessWS::LoadSTDF(const QString &lFileName)
{
    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          "Loading data from STDF Input file");

    PAT_PERF_BENCH

    // Reset ignore MRR flag
    CGexGroupOfFiles::resetIgnoreAllMRR();

    // Have the STDF file analysed...
    FILE *              hFile   = NULL;
    CGexGroupOfFiles *  pGroup  = NULL;
    CGexFileInGroup *   pFile   = NULL;
    QString             lScriptFile = Engine::GetInstance().GetAssistantScript();

    // Create script that will read data file + compute all statistics (but NO report created)
    hFile = fopen(lScriptFile.toLatin1().constData(),"w");

    if(hFile == NULL)
    {
        mPrivate->mErrorMessage = "  > Failed to create script file: " +
                                  lScriptFile;
        return false;
    }

    // Creates 'SetOptions' section
    if(!ReportOptions.WriteOptionSectionToFile(hFile))
    {
        mPrivate->mErrorMessage = "Error : can't write option section";
        return false;
    }

    // Creates 'SetProcessData' section
    fprintf(hFile,"SetProcessData()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  var group_id;\n");
    fprintf(hFile,"  gexGroup('reset','all');\n");

    // Make sure any '\' in string is doubled.
    QString lSTDFScriptFile = ConvertToScriptString(lFileName);

    QList<int> lSites;
    // If All sites are merged together, set the site number to -1
    if (mPrivate->mContext->GetRecipeOptions().GetAllSitesMerged())
    {
        lSites.append(-1);
    }
    else
    {
        // Split file over each site
        QString lSTDFCompliancy = ReportOptions.GetOption("dataprocessing", "stdf_compliancy").toString();
        bool    lValidSiteOnly  = false;

        if (lSTDFCompliancy == "stringent")
            lValidSiteOnly = true;

        // Read all valid sites from data file
        std::vector<int> lSite;
        if (GS::Gex::StdfContentUtils::GetSites((std::string)(lFileName.toLatin1().constData()), lSite, lValidSiteOnly) == false)
        {
            mPrivate->mErrorMessage = "Error : Cannot extract site from input file";
            return false;
        }

        lSites = QList<int>::fromVector(QVector<int>::fromStdVector(lSite));
    }

    // Add the list of site to process to the PAT context
    mPrivate->mContext->SetSiteList(lSites);

    fprintf(hFile,"  group_id = gexGroup('insert','DataSet_all');\n");

    // Only consider last test instance for each die location.
    fprintf(hFile,
            "  gexFile(group_id,'insert','%s','All','last_instance',' ','');\n\n",
            lSTDFScriptFile.toLatin1().constData());
    fprintf(hFile,"}\n\n");

    // Creates 'main' section
    fprintf(hFile,"main()\n");
    fprintf(hFile,"{\n");
    fprintf(hFile,"  SetOptions();\n");
    // Avoids editor to be launched if output options is set to Word or Spreadsheet CSV!
    fprintf(hFile,"  gexOptions('output','format','html');\n");
    // Choose test merge option based on recipe test key option
    switch (mPrivate->mContext->GetRecipeOptions().mTestKey)
    {
        case GEX_TBPAT_KEY_TESTNUMBER:
            // Merge tests with same test number (even if test name is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge');\n");
            break;

        case GEX_TBPAT_KEY_TESTNAME:
            // Merge tests with same test name (even if test number is different)
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'merge_name');\n");
            break;

        case GEX_TBPAT_KEY_TESTMIX:
            // Merge tests with same test number and test name
            fprintf(hFile,"  gexOptions('dataprocessing', 'duplicate_test', 'no_merge');\n");
            break;

        default:
            // Use default option from options tab
            break;
    }
    // Always compute Quartiles & Skew & Kurtosis.
    fprintf(hFile,"  gexOptions('speed','adv_stats','always');\n");
    // Force to compute statistics from samples, ignore summary.
    fprintf(hFile,"  gexOptions('statistics','computation','samples_only');\n");
    // Default: keep test values as is (no scaling)
    fprintf(hFile,"  gexOptions('dataprocessing', 'scaling', 'normalized');\n");
    // Disable outlier removal so to make sure all values are taken into account.
    fprintf(hFile,"  gexOptions('dataprocessing', 'data_cleaning_mode','none');\n");
    // Ensure FULL wafermap is created in any case, non matter the filtering of parts (ensures good processing over Bad Neigbhoors)
    fprintf(hFile,"  gexOptions('wafer','visual_options','all_parts');\n");
    // Use 'From data file if specified, else detect' mode for flat/notch orientation
    fprintf(hFile,"  gexOptions('wafer','notch_location','file_or_detected');\n");
    // Use wafer map as binning source information
    fprintf(hFile,"  gexOptions('binning','computation','wafer_map');\n");
    // Disable STDF.FTR processing (for PAT speed improvement since FTR are not used in PAT algorihtms)
    fprintf(hFile,"  gexOptions('dataprocessing', 'functional_tests', 'disabled');\n");
    // Force Part identification to use xy coordinates
    fprintf(hFile,"  gexOptions('dataprocessing', 'part_identification', 'xy');\n");

    fprintf(hFile,"  SetProcessData();\n");
    fprintf(hFile,"  gexOptions('report','build','false');\n");	// Only data analysis, no report created!

    fprintf(hFile,"  gexBuildReport('hide','0');\n");	// Do NOT show report tab

    fprintf(hFile,"}\n\n");
    fclose(hFile);

    // Execute script.
    if (CSLEngine::GetInstance().RunScript(lScriptFile).IsFailed())
    {
        mPrivate->mErrorMessage = "  > Failed to read stdf file split by site: " +
                                  lFileName;
        return false;
    }

    // Get handle to dataset just analyzed
    pGroup = gexReport->getGroupsList().isEmpty()? NULL : gexReport->getGroupsList().first();
    if(pGroup == NULL)
    {
        mPrivate->mErrorMessage = "  > No data loaded from file";
        return false;
    }

    if (pGroup->pFilesList.count() > 1)
    {
        mPrivate->mErrorMessage = QString("Error: %1 wafers found in the input STDF file %2.")
                                  .arg(pGroup->pFilesList.count()).arg(lFileName);
        return false;
    }

    pFile = (pGroup->pFilesList.isEmpty()) ? NULL : pGroup->pFilesList.first();
    if(pFile == NULL)
    {
        mPrivate->mErrorMessage = "  > No data loaded from file";
        return false;
    }

    // Load Wafermap arra   y with SoftBins
    gexReport->FillWaferMap(pGroup, pFile, NULL, GEX_WAFMAP_SOFTBIN, true);

    // Get wafermap and save it into PAT structure for later use...
    // Note: This does NOT include the PAT bins yet, it is ONLY the merged wafermap of all sites BEFORE PAT.
    mPrivate->mContext->m_AllSitesMap_Sbin    = pFile->getWaferMapData();
    mPrivate->mContext->m_Stdf_SbinMap        = pFile->getWaferMapData();

    // Load Wafermap array with HardBins
    gexReport->FillWaferMap(pGroup, pFile, NULL, GEX_WAFMAP_HARDBIN, true);

    // Save Hardbin wafermap data
    mPrivate->mContext->m_AllSitesMap_Hbin    = pFile->getWaferMapData();
    mPrivate->mContext->m_Stdf_HbinMap        = pFile->getWaferMapData();

    // Save Hard and soft binning list for STDF map.
    mPrivate->mContext->SetSTDFSoftBins(pGroup->cMergedData.ptMergedSoftBinList);
    mPrivate->mContext->SetSTDFHardBins(pGroup->cMergedData.ptMergedHardBinList);

    CGexGroupOfFiles::resetIgnoreAllMRR();

    return true;
}

/*
bool PATProcessWS::OverloadRecipeFile()
{
    if (mPrivate->mSettings.strRecipeOverloadFile.isEmpty() == false)
    {
        // Load overload recipe
        CPROD_Recipe lOverloadRecipe;
        if(lOverloadRecipe.loadRecipe(mPrivate->mSettings.strRecipeOverloadFile,
                                      mPrivate->mErrorMessage) == false)
            return false;

        // Overload Parametric Bin (SPAT, DPAT,NNR,IDDQ-Delta)
        long lValue = lOverloadRecipe.m_iPPAT_Bin;
        if(lValue)
        {
            mPrivate->mContext->cOptionsPat.iFailStatic_HBin  = lValue;
            mPrivate->mContext->cOptionsPat.iFailDynamic_HBin = lValue;
            mPrivate->mContext->cOptionsPat.iNNR_HBin         = lValue;
            mPrivate->mContext->cOptionsPat.iIDDQ_Delta_HBin  = lValue;

            // Overload binning for each Parametric test with a custom PAT setting
            QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
            CPatDefinition	*                           ptPatDef = NULL;

            for(itPATDefinifion = mPrivate->mContext->mPATDefinitions.begin();
                itPATDefinifion != mPrivate->mContext->mPATDefinitions.end(); ++itPATDefinifion)
            {
                ptPatDef = *itPATDefinifion;

                // If test is SPAT enabled, overload its binning!
                if(ptPatDef->m_lFailStaticBin >= 0)
                    ptPatDef->m_lFailStaticBin = lValue;

                // If test is DPAT enabled, overload its binning!
                if(ptPatDef->m_lFailDynamicBin >= 0)
                    ptPatDef->m_lFailDynamicBin = lValue;
            }
        }

        // Overload Geographic Bin (GDBN,Clustering,Reticle)
        lValue = lOverloadRecipe.m_iGPAT_Bin;
        if(lValue >= 0)
        {
            mPrivate->mContext->cOptionsPat.iGDBN_PatHBin         = lValue;
            mPrivate->mContext->cOptionsPat.iReticleHBin          = lValue;
            mPrivate->mContext->cOptionsPat.iClusteringPotatoHBin = lValue;
        }

        // Overload Z-PAT Bin
        lValue = lOverloadRecipe.m_iZPAT_Bin;
        if(lValue >= 0)
        {
            // Overload Bin only for rules enabled!
            // Zpat enabled
            if(mPrivate->mContext->cOptionsPat.lfCompositeExclusionZoneYieldThreshold > 0)
                mPrivate->mContext->cOptionsPat.iCompositeZone_HBin  = lValue;
            if(mPrivate->mContext->cOptionsPat.bMergeEtestStdf)
                mPrivate->mContext->cOptionsPat.iCompositeEtestStdf_HBin = lValue;
        }

        // Flow: for now ONLY consider first flow defined.
        mPrivate->mContext->cOptionsPat.cWorkingFlow = lOverloadRecipe.m_cFlows.first();
        switch(mPrivate->mContext->cOptionsPat.cWorkingFlow.m_iDataSources)
        {
            case FLOW_SOURCE_MAP:
                // This recipe only involves SEMI maps...
                break;
            case FLOW_SOURCE_STDF:
                // This recipe only works on STDF
                break;
            case FLOW_SOURCE_MAP_STDF:
                // This recipe used MAP + STDF files
                break;
        }

        // STDF Flow: For now, support single flow only.
        mPrivate->mContext->cOptionsPat.iUpdateMap_FlowNameID = 0;

        // Check Flows used in GDBN rules
        QList <CGDBN_Rule *>::iterator itGDBN;
        CGDBN_Rule * ptGdbnRule;
        int iRuleID=0;
        for(itGDBN = mPrivate->mContext->cOptionsPat.cGDBN_Rule.begin(); itGDBN != mPrivate->mContext->cOptionsPat.cGDBN_Rule.end(); ++itGDBN, iRuleID++)
        {
            ptGdbnRule = *itGDBN;
            ptGdbnRule->iGDBN_WafermapSource = ptGdbnRule->iGDBN_WafermapSource % 3;
        }

        // Check Flows used in Cluster rules
        QList <CClusterPotatoRule *>::iterator itClusterPotato;
        CClusterPotatoRule *ptClusterRule;
        iRuleID=0;
        for(itClusterPotato = mPrivate->mContext->cOptionsPat.cClusterPotatoRule.begin(); itClusterPotato != mPrivate->mContext->cOptionsPat.cClusterPotatoRule.end(); ++itClusterPotato, iRuleID++)
        {
            ptClusterRule = *itClusterPotato;
            ptClusterRule->iWaferSource = ptClusterRule->iWaferSource % 3;
        }

        // Check Flows used in Reticle
        mPrivate->mContext->cOptionsPat.iReticle_WafermapSource = mPrivate->mContext->cOptionsPat.iReticle_WafermapSource % 3;
    }

    // Success
    return true;
}
*/
void PATProcessWS::RemoveDiesInMap(CWaferMap &map, const QString &bins)
{
    if(bins.isEmpty() == false)
    {
        // Create BinList
        QtLib::Range lRange(bins);

        // Scan wafermap, and 'Kill' relevant dies
        for(int lIdx = 0; lIdx < map.SizeX * map.SizeY; ++lIdx)
        {
            if(lRange.Contains(map.getWafMap()[lIdx].getBin()))
                map.getWafMap()[lIdx].setBin(GEX_WAFMAP_EMPTY_CELL);	// Kill die!
        }
    }
}

bool PATProcessWS::ReadWafermapConfiguration(const QString &lSTDFFile, CWaferMap &lWafer,
                                           bool &lWCRFound)
{
    GQTL_STDF::StdfParse    lSTDFHandle;
    int                         lRecordType;
    int                         lStatus;

    lWCRFound   = false;
    lStatus     = lSTDFHandle.Open(lSTDFFile.toLatin1().constData(), STDF_READ);

    if(lStatus == false)
    {
        mPrivate->mErrorMessage = QString("*PAT Error* Failed to read file file %1.").arg(lSTDFFile);

        return false;
    }

    lStatus = lSTDFHandle.LoadNextRecord(&lRecordType);

    while(lStatus ==  GQTL_STDF::StdfParse::NoError ||
          lStatus == GQTL_STDF::StdfParse::UnexpectedRecord)
    {
        if (lRecordType == GQTL_STDF::Stdf_Record::Rec_WCR)
        {
            GQTL_STDF::Stdf_WCR_V4 * lWCRRecord = new GQTL_STDF::Stdf_WCR_V4();

            if(lSTDFHandle.ReadRecord(lWCRRecord))
            {
                lWafer.bWaferUnits      = lWCRRecord->m_u1WF_UNITS;		// WCR.WF_UNITS
                lWafer.cWaferFlat_Stdf  = lWCRRecord->m_c1WF_FLAT;		// WCR.WF_FLAT
                lWafer.cPos_X           = lWCRRecord->m_c1POS_X;		// WCR.POS_X
                lWafer.cPos_Y           = lWCRRecord->m_c1POS_Y;		// WCR.POS_Y

                // WCR.WAFR_SIZ
                lWafer.SetDiameter(lWCRRecord->m_r4WAFR_SIZ);
                // WCR.DIE_HT
                lWafer.SetDieHeight(lWCRRecord->m_r4DIE_HT);
                // WCR.DIE_WID
                lWafer.SetDieWidth(lWCRRecord->m_r4DIE_WID);
                // WCR.CENTER_X and WCR.CENTER_Y
                lWafer.SetCenterDie(GS::Gex::WaferCoordinate(lWCRRecord->m_i2CENTER_X,
                                                             lWCRRecord->m_i2CENTER_Y));

                // Set active flat with the stdf flat information
                lWafer.cWaferFlat_Active = lWafer.cWaferFlat_Stdf;

                // Set X and Y direction as stated in the STDF file.
                if (lWafer.cPos_X == 'L')
                    lWafer.SetPosXDirection(false);  // DO mirror along X line
                else
                    lWafer.SetPosXDirection(true);  // No mirror along X line

                if (lWafer.cPos_Y == 'U')
                    lWafer.SetPosYDirection(false);  // DO mirror along Y line
                else
                    lWafer.SetPosYDirection(true);  // No mirror along Y line

                lWCRFound = true;
                return true;
            }
        }

        // Read one record from STDF file
        lStatus = lSTDFHandle.LoadNextRecord(&lRecordType);
    }

    return true;
}

bool PATProcessWS::StackWafermaps()
{
    CGexGroupOfFiles *	lGroup	= gexReport->getGroupsList().isEmpty() ?
                                      NULL : gexReport->getGroupsList().first();;

    if (lGroup == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    CGexFileInGroup *	lFile   = (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    if (lFile == NULL)
    {
        mPrivate->mErrorMessage = "*PAT Error* No datasets created";
        return false;
    }

    // Get copy of combined wafermap (includes PAT binning).
    CWaferMap lWafer = mPrivate->mContext->m_AllSitesMap_Sbin;

    // Merged soft(or hard) bins of all groups into group#1 + merge parts in all goups (used for 3D wafermap!)
    // Iterator on Groups list
    QListIterator<CGexGroupOfFiles*> itGroupsList(gexReport->getGroupsList());
    itGroupsList.toFront();

    CGexGroupOfFiles *	lGroup1				= itGroupsList.next();
    CGexFileInGroup  *	lFile1				= (lGroup1->pFilesList.isEmpty()) ?
                                                  NULL : lGroup1->pFilesList.first();
    CBinning *			ptNewBinItem		= NULL;
    CBinning *			ptMasterBinList		= NULL;
    CBinning *			ptMasterBinListTail	= NULL;
    CBinning *			ptBinList			= NULL;
    bool				lBinAlreadyExists;
    int                 lBin;

    while(itGroupsList.hasNext())
    {
        // Handle to file in group
        lGroup	= itGroupsList.next();
        lFile	= (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

        // Keep track of total bins
        if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
        {
            lGroup1->cMergedData.lTotalHardBins += lGroup->cMergedData.lTotalHardBins;
            ptBinList = lGroup->cMergedData.ptMergedHardBinList; // Points first HARD binning structure
        }
        else
        {
            lGroup1->cMergedData.lTotalSoftBins += lGroup->cMergedData.lTotalSoftBins;
            ptBinList= lGroup->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure
        }

        while(ptBinList)
        {
            // Check if each bin list entry exists in global list (group#1)
            lBinAlreadyExists = false;

            if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
            {
                ptMasterBinList= lGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            }
            else
            {
                ptMasterBinList= lGroup1->cMergedData.ptMergedHardBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            }

            while(ptMasterBinList)
            {
                // Check if this bin is already in the master list
                if(ptMasterBinList->iBinValue == ptBinList->iBinValue)
                {
                    lBinAlreadyExists = true;
                    break;
                }

                // Move to next Bin# in master list
                ptMasterBinListTail = ptMasterBinList;
                ptMasterBinList     = ptMasterBinList->ptNextBin;
            };

            // If Bin# was not in master list, add it.
            if(lBinAlreadyExists == false)
            {
                // Add Bin cell to the list (append to the end)
                ptNewBinItem            = new CBinning;
                ptNewBinItem->iBinValue = ptBinList->iBinValue;
                ptNewBinItem->cPassFail = ptBinList->cPassFail;
                ptNewBinItem->strBinName= ptBinList->strBinName;
                ptNewBinItem->ptNextBin = NULL;

                if(ptMasterBinListTail == NULL)
                {
                    if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
                        lGroup1->cMergedData.ptMergedHardBinList = ptNewBinItem;
                    else
                        lGroup1->cMergedData.ptMergedSoftBinList = ptNewBinItem;
                }
                else
                    ptMasterBinListTail->ptNextBin = ptNewBinItem;
            }

            // Move to next Bin# in current group
            ptBinList = ptBinList->ptNextBin;
        };

        // Merge total part count into Group#1 (collapse)
        lFile1->getPcrDatas().lPartCount += lFile->getPcrDatas().lPartCount;
    };

    // Reset die count
    if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
        ptMasterBinList= lGroup1->cMergedData.ptMergedHardBinList; // Points first HARD binning structure in group#1 (master bin list to update)
    else
        ptMasterBinList= lGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)

    while(ptMasterBinList)
    {
        // Clear all die counts
        ptMasterBinList->ldTotalCount = 0;
        ptMasterBinList = ptMasterBinList->ptNextBin;
    };

    // handle to first group!
    lGroup	= gexReport->getGroupsList().isEmpty() ? NULL : gexReport->getGroupsList().first();
    lFile	= (lGroup->pFilesList.isEmpty()) ? NULL : lGroup->pFilesList.first();

    // count all bins in merged wafermap produced...
    lWafer.iTotalPhysicalDies = 0;

    if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
        lGroup1->cMergedData.lTotalHardBins = 0;
    else
        lGroup1->cMergedData.lTotalSoftBins = 0;

    for(int lIdx = 0; lIdx < lWafer.SizeX * lWafer.SizeY; ++lIdx)
    {
        // Update Die count (in combined group1 wafermap) for given binning
        lBin = lWafer.getWafMap()[lIdx].getBin();

        if(lBin != GEX_WAFMAP_EMPTY_CELL)
        {
            // Keep track of die count
            lWafer.iTotalPhysicalDies++;

            // Update relevant bin count.
            if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
                ptMasterBinList= lGroup1->cMergedData.ptMergedHardBinList; // Points first HARD binning structure in group#1 (master bin list to update)
            else
                ptMasterBinList= lGroup1->cMergedData.ptMergedSoftBinList; // Points first SOFT binning structure in group#1 (master bin list to update)
            while(ptMasterBinList)
            {
                if(ptMasterBinList->iBinValue == lBin)
                {
                    ptMasterBinList->ldTotalCount++;
                    if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
                        lGroup1->cMergedData.lTotalHardBins++;
                    else
                        lGroup1->cMergedData.lTotalSoftBins++;
                    break;
                }
                // Next Bin structure
                ptMasterBinList = ptMasterBinList->ptNextBin;
            };
        }
    }

    // Duplicate Wafermap array
    int	lWaferArraysize = lWafer.SizeX*lWafer.SizeY;

    delete [] lFile->getWaferMapData().getWafMap();

    lFile->getWaferMapData().setWaferMap(CWafMapArray::allocate(lWaferArraysize, lWafer.getWafMap()));
    lFile->getWaferMapData().allocCellTestCounter(lWaferArraysize, lWafer.getCellTestCounter());

    lFile->getWaferMapData().iLowDieX   = lWafer.iLowDieX;
    lFile->getWaferMapData().iHighDieX  = lWafer.iHighDieX;
    lFile->getWaferMapData().iLowDieY   = lWafer.iLowDieY;
    lFile->getWaferMapData().iHighDieY  = lWafer.iHighDieY;
    lFile->getWaferMapData().SizeX      = lWafer.SizeX;
    lFile->getWaferMapData().SizeY      = lWafer.SizeY;                         // Wafermap size (cells in X, and Y)
    lFile->getWaferMapData().iTotalPhysicalDies = lWafer.iTotalPhysicalDies;	// Total number of physical dies tested on the wafer (dosn't counts retests)

    // Update total parts tested
    if(mPrivate->mContext->GetRecipeOptions().iReport_WafermapType)
        lGroup->cMergedData.lTotalHardBins = lGroup1->cMergedData.lTotalHardBins;
    else
        lGroup->cMergedData.lTotalSoftBins = lGroup1->cMergedData.lTotalSoftBins;

    return true;
}

bool PATProcessWS::UpdateExternalMapFile(const QString &inputExternalMap, QString &outputExternalMap)
{
    PAT_PERF_BENCH

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : entering UpdateExternalMapFile");

    QString lInputFile  = inputExternalMap.section(" ", 0, 0);
    QString lOption     = inputExternalMap.section(" ", 1, 1);

    mPrivate->mSINFInfo.Clear();
    mPrivate->mExternalMapDetails.Clear();

    GSLOG(SYSLOG_SEV_DEBUG, "---PAT : private sinf and external map infos cleared.");

    if (lInputFile.isEmpty())
    {
        GSLOG(SYSLOG_SEV_DEBUG, "---PAT : input file empty, exiting UpdateExternalMapFile");
        return true;
    }

    // If original file doesn't exists, fail processing
    if(QFile::exists(lInputFile) == false)
    {
        mPrivate->mErrorMessage = "\t> *Error* Failed to open input file.\nFile missing? : " +
                                  lInputFile;

        GSLOG(SYSLOG_SEV_DEBUG, "---PAT : input file doesn't exist, exiting UpdateExternalMapFile");

        return false;
    }

    // If lOption is empty, then only one parameter specified;
    // means new file created will have '.new' extension
    if(lOption.isEmpty())
    {
        outputExternalMap = lInputFile + ".new";
    }
    else if (lOption.startsWith("no_update", Qt::CaseInsensitive) == false)
    {
        // We have two parameters then: <original file> <new file>
        outputExternalMap = lOption;

        // If new name and old name are same name, then first rename original name to .old
        if(lInputFile.compare(outputExternalMap, Qt::CaseInsensitive) == 0)
        {
            QString lOldInputFile = lInputFile + ".old";

            if (QFile::rename(lInputFile, lOldInputFile) == false)
            {
                mPrivate->mErrorMessage = QString("\t> *Error* Failed to rename input file %1 into %2")
                                          .arg(lInputFile).arg(lOldInputFile);

                GSLOG(SYSLOG_SEV_DEBUG, "---PAT : failed input file rename, exiting UpdateExternalMapFile");

                return false;
            }

            lInputFile = lOldInputFile;
        }
    }
    else
        outputExternalMap.clear();

    if(!outputExternalMap.isEmpty())
    {
        if(mPrivate->mContext)
        {
            mPrivate->mContext->addToGeneratedFilesList(outputExternalMap);
            GSLOG(SYSLOG_SEV_DEBUG, "---PAT : added outptu external map as generated files list");
        }
    }

    // Supported format for update are
    //  - KLA/INF
    //  - Semi E142
    //  - TELP8
    //  - Sky NP CSV
    // We must keep legacy behavior, if external map is not one of our supported format to be updated,
    // just ignore it.
    if (CGKlaInfLayerstoSTDF::IsCompatible(lInputFile.toLatin1().constData()))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "---PAT : KlaInf map updated, exiting UpdateExternalMapFile");
        return UpdateKLAINFExternalMap(lInputFile, outputExternalMap);
    }
    else if (CGSemiE142XmltoSTDF::IsCompatible(lInputFile.toLatin1().constData()))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "---PAT : E142 map updated, exiting UpdateExternalMapFile");
        return UpdateE142ExternalMap(lInputFile, outputExternalMap);
    }
    else if (CGProberTelThousandToSTDF::IsCompatible(lInputFile.toLatin1().constData()) ||
             CGProberTelMillionToSTDF::IsCompatible(lInputFile.toLatin1().constData()))
    {
        GSLOG(SYSLOG_SEV_DEBUG, "---PAT : TELP8 map updated, exiting UpdateExternalMapFile");
        return UpdateTELP8ExternalMap(lInputFile, outputExternalMap);
    }
    else
    {
        GS::Parser::ParserFactory * lFactory = GS::Parser::ParserFactory::GetInstance();

        if (lFactory)
        {
            QSharedPointer<GS::Parser::ParserAbstract> lParser(lFactory->CreateParser(lInputFile.toLatin1().constData()));
            if (lParser.isNull() == false)
            {
                GS::Parser::ParserType lParserType = lParser->GetType();
                switch(lParserType)
                {
                    case GS::Parser::typeSkyNPCSV:          return UpdateSkyNPExternalMap(lInputFile, outputExternalMap);
                    case GS::Parser::typeWoburnSECSIIMap:   return UpdateExternalMap<GS::Parser::ParserDef<GS::Parser::typeWoburnSECSIIMap> >   (lInputFile, outputExternalMap);
                    case GS::Parser::typeMexicaliMap:       return UpdateExternalMap<GS::Parser::ParserDef<GS::Parser::typeMexicaliMap> >       (lInputFile, outputExternalMap);


                    default:break;
                }
            }
            GSLOG(SYSLOG_SEV_DEBUG, "---PAT : external map updated using parser factory");
        }
        else
        {
            mPrivate->mErrorMessage = "Failed to instantiate Parser Factory.";
            GSLOG(SYSLOG_SEV_ERROR, "Failed to instantiate Parser Factory for external map update.");
            return false;
        }
    }

    GSLOG(SYSLOG_SEV_INFORMATIONAL,
          QString("%1 is not a supported format for external map update.").toLatin1().constData());

    return true;
}

template <typename ParserDef>
bool PATProcessWS::UpdateExternalMap( const QString &inputExternalMap, const QString &outputExternalMap)
{
    //-- If not output file name defined, don't do anything.
    if (outputExternalMap.isEmpty())
        return true;

    if (mPrivate->mContext == 0)
    {
        mPrivate->mErrorMessage = "Failed to retrieve any PAT processing information.";
        return false;
    }

    QFile lInputFile(inputExternalMap);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        mPrivate->mErrorMessage = "Failed to open input file.\nProtection issue? : " +
                                  inputExternalMap;
        return false;
    }

    if(ParserDef::Parser::IsCompatible(inputExternalMap) == false)
    {
        mPrivate->mErrorMessage = "\t> *Error* The file is not a Mexicali file:" + inputExternalMap;
        return false;
    }

    //-- Write updated wafermap to output file
    //-- 1: Create new output Sky NP file
    QFile lOutputFile(outputExternalMap);
    if(!lOutputFile.open(QIODevice::WriteOnly))
    {
        mPrivate->mErrorMessage = "*Error* Failed to create updated Mexicali.\nProtection issue?";
        return false;
    }

    //-- Assign file I/O stream
    QTextStream lInputStream(&lInputFile);

    QStringList lOutputStrings;
    ParserDef::Parser::CopyHeader(lInputStream, lOutputStrings);

    QString lLine;
    CBinning* lExternalBinInfo  = 0, *lSTDFBinInfo=0;
    QChar lCharBin;
    int lDieX = 0, lDieY = 0;
    int iWaferRows              = 0;
    int lSTDFBin                = GEX_WAFMAP_EMPTY_CELL;
    int lIdx                    = 0;
    int lBin                    = 0;
    bool lOriginSet             = false;
    int xOffset                 = 0;
    int yOffset                 = 0;

    while (lInputStream.atEnd() == false)
    {
        lLine   = lInputStream.readLine();

        if (lLine.isEmpty() == false )
        {
            lDieY =iWaferRows;

            bool XReadReverse   = ParserDef::Parser::IsReverseXRead();
            int iWaferColumns   = 0;
            int iIndex          = 0;
            int iEnd            = lLine.length();
            int iIncrement      = 1;
            if(XReadReverse)
            {
                iIndex      = lLine.length() - 1;
                iEnd        = -1;
                iIncrement  = -1;
            }

            while(true)
            {

                if(iIndex == iEnd)
                    break;

                lDieX = iWaferColumns;
                lCharBin = lLine.at(iIndex);

                // -- is this a die
                if(ParserDef::Parser::IsInTheCoordinateSystem(lCharBin) == false)
                {
                    iIndex = iIndex + iIncrement;
                    ++iWaferColumns;
                    continue;
                }

                // -- set the origin. In some case it can be  the first die and not the 0, 0
                if(lOriginSet == false)
                {
                   // mPrivate->mContext->GetExternalTransformation().SetOrigin(WaferCoordinate(lDieX, lDieY));

                    ParserDef::Parser::SetOffsetOrigin(xOffset, yOffset, lDieX, lDieY);
                    lOriginSet = true;
                }

                lDieX = lDieX - xOffset;
                lDieY = lDieY - yOffset;

                // -- is this a die that can be updated
                if(ParserDef::Parser::IsPatValidDie(lCharBin))
                {
                    if (lCharBin == '0')
                        lBin = 0;
                    else
                        lBin = 1;

                    // Get STDF die value
                    WaferCoordinate wCoord(lDieX, lDieY);

                    // Get the transformed coordinates in case of map alignment
                    wCoord = mPrivate->mContext->GetExternalTransformation().Map(wCoord);

                    // --  Get the External bin info class
                    lExternalBinInfo = mPrivate->mContext->GetExternalBins()->Find(lBin);

                    // If not found, return an error
                    if (lExternalBinInfo == NULL)
                    {
                        mPrivate->mErrorMessage = QString("Failed to update Woburn SECSII map: Unable to retrieve bin value (%1) from external map bin list")
                                                  .arg(lBin);
                        return false;
                    }

                    // --  Get the STDF bin info class
                    // Check if valid STDF die location
                    lSTDFBin = GEX_WAFMAP_EMPTY_CELL;
                    // -- get the index of the die "lIdx" according to the X, Y
                    if(mPrivate->mContext->m_Stdf_HbinMap.indexFromCoord(lIdx, wCoord.GetX(), wCoord.GetY()))
                    {
                        // Die within STDF wafermap area (but maybe die not tested!), get PAT-Man value!...offset X & Y to start on wafer corner
                        lSTDFBin = mPrivate->mContext->m_Stdf_HbinMap.getWafMap()[lIdx].getBin();
                         ++mPrivate->mExternalMapDetails.mTotalMatchingMapDiesLoc;
                    }

                    lSTDFBinInfo = mPrivate->mContext->GetSTDFHardBins()->Find(lSTDFBin);

                    if (lSTDFBin != GEX_WAFMAP_EMPTY_CELL && lSTDFBinInfo == 0)
                    {
                        mPrivate->mErrorMessage = QString("Failed to update Woburn SECSII map: Unable to retrieve Soft bin value (%1) from stdf map soft bin list")
                                                  .arg(lSTDFBin);
                        return false;
                    }

                    // Keep some informations about external map compared to stdf map
                    mPrivate->mExternalMapDetails.mTotalDies++;

                    if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL)
                    {
                        ++mPrivate->mExternalMapDetails.mGoodPartsMapSTDFMissing;
                    }

                    // Good die in external map 'P', fail die in stdf 'F' or
                    // Fail die in external map 'P', good die in stdf 'F'
                    if(lSTDFBinInfo && lExternalBinInfo->cPassFail != lSTDFBinInfo->cPassFail )
                    {
                    ++mPrivate->mExternalMapDetails.mTotalMismatchingMapPassBin;
                    }

                    // Get pat failure details if any
                    if (mPrivate->mContext->isDieOutlier(wCoord.GetX(), wCoord.GetY(), lBin))
                    {
                        // Good die in external map, PAT outlier
                        if (lExternalBinInfo->cPassFail == 'P')
                            ++mPrivate->mExternalMapDetails.mGoodPartsMapDPATFailures;

                        // Update the bin fields and the failed parameters field
                        lLine[iIndex] = '0';
                    }
                    else if (mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0 || mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                    {
                        // If external die is pass, check some gtf keys.
                      if (lExternalBinInfo->cPassFail == 'P')
                        {
                            // Untested STDF die
                            if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL)
                            {
                                if (mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                                {
                                    lLine[iIndex] = QString::number(mPrivate->mSettings.iGoodMapOverload_MissingStdf)[0];
                                }
                            }
                            else if (mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                            {
                                // Check if STDF bin doesn't belong to 'good bins' list
                                if(mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(lSTDFBin) == false)
                                {
                                    lLine[iIndex] = QString::number(mPrivate->mSettings.iGoodMapOverload_FailStdf)[0];
                                }
                            }
                        }
                    }
                }

                iIndex = iIndex + iIncrement;
                ++iWaferColumns;
            }
            ++iWaferRows;
            lOutputStrings << lLine;
        }
        else
            lOutputStrings << lLine;
    }

    ParserDef::Parser::UpdateMapData(lOutputStrings,
         QString::number(mPrivate->mExternalMapDetails.mTotalDies
                         - mPrivate->mExternalMapDetails.mGoodPartsMapDPATFailures));

    QTextStream lOutputStream(&lOutputFile);
    QStringList::iterator lIterB(lOutputStrings.begin()), lIterE(lOutputStrings.end());
    for(;lIterB != lIterE; ++lIterB )
    {
        lOutputStream<< *lIterB << endl;
    }

    // Close the file
    lOutputFile.close();
    lInputFile.close();


    return true;
}

bool PATProcessWS::UpdateE142ExternalMap(const QString &inputExternalMap, const QString &outputExternalMap)
{
    // If not output file name defined, don't do anything.
    if (outputExternalMap.isEmpty())
        return true;

    // Open KLA / INF file
    QFile lInputFile(inputExternalMap);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening KLA / INF file
        mPrivate->mErrorMessage = "\t> *Error* Failed to open input file.\nProtection issue? : " +
                                  inputExternalMap;
        return false;
    }

    // Assign file I/O stream
    QTextStream lInputStream(&lInputFile);
    QStringList	lE142File;
    QString     lSection;
    QString     lLine;

    // Needs to handle possible empty lines at the beginning
    do
    {
        lLine = lInputStream.readLine();
        lE142File += lLine;
    } while (lLine.trimmed().isEmpty() && lInputStream.atEnd() == false);

    if(lLine.contains("<?xml", Qt::CaseInsensitive) == false)
    {
        // Failed Opening KLA / INF file
        mPrivate->mErrorMessage = "\t> *Error* The file is not e Semi E142 file:" + inputExternalMap;
        return false;
    }

    // Needs to handle possible empty lines between <?xml..... ?> and <MadData ....> lines
    do
    {
        lLine = lInputStream.readLine();
        lE142File += lLine;
    } while (lLine.trimmed().isEmpty() && lInputStream.atEnd() == false);

    if(lLine.contains("urn:semi-org:xsd.E142", Qt::CaseInsensitive) == false)
    {
        // Failed Opening KLA / INF file
        mPrivate->mErrorMessage = "\t> *Error* The file is not e Semi E142 file:" + inputExternalMap;
        return false;
    }

    // Extract Recipe name & Version
    lSection = QFileInfo(mPrivate->mSettings.strRecipeFile).baseName();	// eg: 'R7105XXL_Version1_patman_config'

    QString lRecipeName     = lSection.section('_',0,0);  	// eg: 'R7105XXL'
    QString lRecipeVersion  = lSection.section('_',1,1).section("Version",1);;  // eg: 'Version1'

    // Read all file, and overload needed fields/data
    CPatFailureDeviceDetails cDevice;

    QString lDie;
    QString lNullBin;
    int     iIndex;
    int     iLineOffset;
    int     iBinCode;
    int     iBinCount;
    int     iStart;
    int     iEnd;
    int     iBinWidth   = 2;
    char    lOrgPosX    = 'L';
    char    lOrgPosY    = 'U';
    char    lAxisPosX   = 'R';
    char    lAxisPosY   = 'D';
    int     lOriginX    = -32768;
    int     lOriginY    = -32768;
    int     lUpperLeftX = -32768;
    int     lUpperLeftY = -32768;
    int     lStepX      = 1;
    int     lStepY      = 1;
    int     lRows       = 0;
    int     lCurrentRow = 0;
    int     lCurrentCol = 0;
    int     lSTDFBin    = -1;
    int     lDieIndex   = -1;
    int     lBinIndent  = 0;
    bool    lOk;

    QStringList     lBinAttributeOrdering;
    QMap<int,int>   lBinList;
    QDateTime       lDateTime;

    // Pass1: extact Bin list, count + wafermap
    // Pass2: update map to disk
    for(int lPassIdx = 1; lPassIdx <= 2; ++lPassIdx)
    {
        // Reset variables + rewinf input map file
        lCurrentRow = 0;
        lE142File.clear();
        lInputStream.seek(0);

        do
        {
            // Read next line from file
            lLine = lInputStream.readLine();

            // Update 'CreateDate'
            iIndex = lLine.indexOf("<CreateDate",0,Qt::CaseInsensitive);
            if(iIndex >= 0)
            {
                // Overwrite CreateDate
                lDateTime = mPrivate->mSettings.dtProcessStartTime;
                lLine = lLine.left(iIndex) + "<CreateDate>" + lDateTime.toString("yyyyMMddhhmmss") + "</CreateDate>";
                goto save_line;
            }

            // Update 'LastModified'
            iIndex = lLine.indexOf("<LastModified",0,Qt::CaseInsensitive);
            if(iIndex >= 0)
            {
                // Overwrite 'LastModified'
                lDateTime = QDateTime::currentDateTime();
                lLine = lLine.left(iIndex) + "<LastModified>" + lDateTime.toString("yyyyMMddhhmmss") + "</LastModified>";
                goto save_line;
            }

            if (lPassIdx == 1 && lLine.contains("<SubstrateMap", Qt::CaseInsensitive))
            {
                QString tmpString;
                iStart = lLine.indexOf("OriginLocation", 0, Qt::CaseInsensitive);

                if (iStart >= 0)
                {
                    iStart  = lLine.indexOf("\"", iStart);
                    iEnd    = lLine.indexOf("\"", iStart+1);

                    tmpString = lLine.mid(iStart+1, iEnd - iStart - 1);

                    if(tmpString.startsWith("UPPER",Qt::CaseInsensitive))
                        lOrgPosY = 'U';
                    else if(tmpString.startsWith("LOWER",Qt::CaseInsensitive))
                        lOrgPosY = 'D';
                    else if(tmpString.startsWith("CENTER",Qt::CaseInsensitive))
                        lOrgPosY = 'C';

                    if(tmpString.endsWith("LEFT",Qt::CaseInsensitive))
                        lOrgPosX = 'L';
                    else if(tmpString.endsWith("RIGHT",Qt::CaseInsensitive))
                        lOrgPosX = 'R';
                    else if(tmpString.startsWith("CENTER",Qt::CaseInsensitive))
                        lOrgPosX = 'C';
                }

                iStart = lLine.indexOf("AxisDirection", 0, Qt::CaseInsensitive);

                if (iStart >= 0)
                {
                    iStart  = lLine.indexOf("\"", iStart);
                    iEnd    = lLine.indexOf("\"", iStart+1);

                    tmpString = lLine.mid(iStart+1, iEnd - iStart - 1);

                    // the invers as ORIGINLOCATION
                    if(tmpString.startsWith("DOWN",Qt::CaseInsensitive))
                        lAxisPosY = 'D';
                    else if(tmpString.startsWith("UP",Qt::CaseInsensitive))
                        lAxisPosY = 'U';

                    if(tmpString.endsWith("RIGHT",Qt::CaseInsensitive))
                        lAxisPosX = 'R';
                    else if(tmpString.endsWith("LEFT",Qt::CaseInsensitive))
                        lAxisPosX = 'L';
                }

                goto save_line;
            }

            // Find Wafermap left corner coordinates
            if(lPassIdx == 1 &&
               lLine.contains("<ReferenceDevice",Qt::CaseInsensitive) &&
               lLine.contains("\"OriginLocation\"",Qt::CaseInsensitive))
            {
                // Write this line to the output map
                lE142File += lLine;

                // Read next line from file
                lLine = lInputStream.readLine();
                if(lLine.contains("<Coordinates",Qt::CaseInsensitive) == false)
                {
                    mPrivate->mErrorMessage = "\t> *Error* Failed to find coordinates in the map file";
                    return false;
                }
                //eg: <Coordinates X="21" Y="22"/>
                lOriginX = lLine.section('"',1,1).toInt(&lOk);			// LowX
                if(!lOk)
                {
                    mPrivate->mErrorMessage = "\t> *Error* Failed to get X coordinate in the map file";
                    return false;
                }
                lOriginY = lLine.section('"',3,3).toInt(&lOk);			// LowY
                if(!lOk)
                {
                    mPrivate->mErrorMessage = "\t> *Error* Failed to get Y coordinate in the map file";
                    return false;
                }

                goto save_line;
            }

            // Are we in the wafermap die size definition?. eg: '<BinCodeMap BinType="HexaDecimal" NullBin="5E">'
            if(lLine.contains("NullBin",Qt::CaseInsensitive))
            {
                if (lPassIdx == 1)
                {
                    // Extract NullBin string
                    lNullBin = lLine.section('"',3,3);
                    iBinWidth = lNullBin.length();
                }
                goto save_line;
            }

            // Are we in the Alias section?
            iIndex = lLine.indexOf("<AliasId",0,Qt::CaseInsensitive);
            if(iIndex >= 0)
            {
                // If Prober name, overload with PAT-Man name
                if(lLine.contains("ProberName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"ProberName\" Value=\"PAT-Man\"/>";
                else if(lLine.contains("TesterName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"TesterName\" Value=\"\"/>";
                else if(lLine.contains("ProbeCardName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"ProbeCardName\" Value=\"\"/>";
                else if(lLine.contains("LoadBoardName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"LoadBoardName\" Value=\"\"/>";
                else if(lLine.contains("OperatorName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"OperatorName\" Value=\"\"/>";
                else if(lLine.contains("TestProgName",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"TestProgName\" Value=\"" + lRecipeName + "\"/>";
                else if(lLine.contains("TestProgVer",Qt::CaseInsensitive))
                    lLine = lLine.left(iIndex) + "<AliasId Type=\"TestProgVer\" Value=\"" + lRecipeVersion + "\"/>";

                goto save_line;
            }

            // Update 'GoodDevices'
            iIndex = lLine.indexOf("<GoodDevices>",0,Qt::CaseInsensitive);
            if(iIndex >= 0)
            {
                // Overload <GoodDevices> count
                lSection = lLine.section('>',1,1);
                lSection = lSection.section('<',0,0);

                iBinCode = lSection.toInt(&lOk);
                if(lOk)
                {
                    // Update total good parts in OUTPUT map (not in STDF map)
                    mPrivate->mContext->lTotalGoodAfterPAT_OutputMap = iBinCode -
                                                             mPrivate->mContext->GetPPATPartCount() -
                                                             mPrivate->mContext->GetGPATPartCount();

                    // Update XML string.
                    lLine = lLine.left(iIndex) + "<GoodDevices>" +
                            QString::number(mPrivate->mContext->lTotalGoodAfterPAT_OutputMap) +
                            "</GoodDevices>";
                }

                goto save_line;
            }

            // If binning section, overload bin count per binning (taking into account the PAT bins.
            if((lPassIdx == 2) && (lLine.indexOf("<BinDefinition", 0, Qt::CaseInsensitive) >= 0))
            {
                lBinIndent = lLine.indexOf("<BinDefinition", 0, Qt::CaseInsensitive);

                // Extract Bin#. Eg '<BinDefinition BinCode="01" BinCount="52" BinQuality="Pass" BinDescription="GOOD DIE CODE 1" />'
                if(lLine.contains("BinCode",Qt::CaseInsensitive))
                {
                    if (lBinAttributeOrdering.isEmpty())
                    {
                        QString lBinLine = lLine;

                        lBinLine.remove("<BinDefinition ");
                        lBinLine.remove("/>");
                        lBinLine = lBinLine.trimmed();

                        lBinAttributeOrdering = lBinLine.split(QRegExp("=\\s*\\\"([^\\\"]+)\\\"\\s*"), QString::SkipEmptyParts);
                    }

                    // Update BinDefinition processing
                    iBinCode    = lLine.section('"',1,1).toInt(&lOk,16);	// Hex Bin#
                    iBinCount   = lBinList[iBinCode];
                    lSection    = QString::number(iBinCount);

                    // Remove original Bin count
                    iStart  = lLine.indexOf("BinCount",0);
                    iStart  = lLine.indexOf("\"",iStart);
                    iEnd    = lLine.indexOf("\"",iStart+1);
                    lLine.remove(iStart+1,iEnd-iStart-1);

                    // Insert new bin count
                    lLine.insert(iStart+1, lSection);

                    // Remove this Binning entry
                    lBinList.remove(iBinCode);
                }


                goto save_line;
            }

            // End of binning section
            if((lPassIdx == 2) && (lLine.indexOf("</BinDefinitions>",0,Qt::CaseInsensitive) >= 0))
            {
                // Add PAT binning definitions*
                QString lNewLine;
                QString lAttribute;
                QMap<int, int>::const_iterator it = lBinList.constBegin();
                while (it != lBinList.constEnd())
                {
                    // eg: '            <BinDefinition BinCode="01" BinCount="52" BinQuality="Pass" BinDescription="GOOD DIE CODE 1" />'
                    lNewLine = QString().rightJustified(lBinIndent, ' ');
                    lNewLine += "<BinDefinition ";

                    for (int lIdx = 0; lIdx < lBinAttributeOrdering.count(); ++lIdx)
                    {
                        lAttribute = lBinAttributeOrdering.at(lIdx);

                        if (lAttribute == "BinCode")
                        {
                            lNewLine += "BinCode=\"" +
                                        QString::number(it.key(),16).toUpper().rightJustified(iBinWidth, '0')
                                        + "\" ";
                        }
                        else if (lAttribute == "BinCount")
                        {
                            lNewLine += "BinCount=\"" + QString::number(it.value()) + "\" ";
                        }
                        else if (lAttribute == "BinDescription")
                        {
                            lNewLine += "BinDescription=";

                            if (it.key() == mPrivate->mSettings.iGoodMapOverload_MissingStdf)
                                lNewLine += "\"STDF Missing\" ";
                            else
                                lNewLine += "\"PAT BIN " + QString::number(it.key()) + "\" ";
                        }
                        else if (lAttribute == "BinQuality")
                        {
                            lNewLine += "BinQuality=\"Fail\" ";
                        }
                        else if (lAttribute == "Pick")
                        {
                            lNewLine += "Pick=\"False\" ";
                        }
                    }

                    lNewLine += "/>";

                    lE142File += lNewLine;
                    ++it;
                }

                // Write end of binning bloc
                lE142File += lLine;

                // clear buffer
                lLine = "";

                goto save_line;
            }

            // If wafermap, toggle PAT dies.
            iLineOffset = lLine.indexOf("<BinCode>",0,Qt::CaseInsensitive);
            if(iLineOffset >= 0)
            {
                if (mPrivate->mSettings.iGoodMapOverload_MissingStdf > 0)
                {
                    QString lMissingSTDF;

                    lMissingSTDF.sprintf("%X", mPrivate->mSettings.iGoodMapOverload_MissingStdf);

                    if (lMissingSTDF.length() > iBinWidth)
                    {
                        QString rootCause = QString("GoodMapOverload_MissingSTDF bin too high (0x%1). Hexadecimal size is %2 character.")
                                            .arg(lMissingSTDF).arg(iBinWidth);

                        GSLOG(SYSLOG_SEV_ERROR, rootCause.toLatin1().constData());

                        mPrivate->mErrorMessage = "\t> *Error* " + rootCause;
                        return false;
                    }
                }

                if (lRows == 0)
                {
                    QString line;
                    qint64  pos = lInputStream.pos();

                    ++lRows;

                    do
                    {
                        line = lInputStream.readLine();

                        if (line.indexOf("<BinCode>", 0, Qt::CaseInsensitive) >= 0)
                            ++lRows;

                    }while (lInputStream.atEnd() == false);

                    lInputStream.seek(pos);
                }

                // Seek offset to first die character in string
                iLineOffset = lLine.indexOf(">",iLineOffset,Qt::CaseInsensitive)+1;

                // Extract Hex bin string
                lSection = lLine.section("<BinCode>",1);	// Remove prefix XML code
                lSection = lSection.section("<",0,0);		// Remove suffix XML code

                // Wafer coordinate
                GS::Gex::WaferCoordinate wCoord;

                // Compute UpperLeft coordinate and X/Y step
                if (lUpperLeftX == -32768 && lUpperLeftY == -32768)
                {
                    int lTmpOriginX = 0;
                    int lTmpOriginY = 0;

                    if(lAxisPosX == 'L')
                        lStepX = -1;

                    if(lAxisPosY == 'U')
                        lStepY = -1;

                    int lColumns = lSection.length() / iBinWidth;

                    if(lOrgPosX == 'C')
                    {
                        if(lAxisPosX == 'R')
                            lTmpOriginX = lUpperLeftX = (lColumns/2) - lColumns;
                        else
                            lTmpOriginX = lUpperLeftX = (lColumns/2);

                        if(lAxisPosY == 'D')
                            lTmpOriginY = lUpperLeftY = (lRows/2) - lRows;
                        else
                            lTmpOriginY = lUpperLeftY = (lRows/2);
                    }
                    else
                    {
                        if(lAxisPosX == 'L')
                        {
                            // Axis to Left
                            // iXStep < 0
                            // If the OriginLocation is at the Right of the Wafer
                            // start to Col else to 0
                            if(lOrgPosX != lAxisPosX)
                                lUpperLeftX = lColumns - 1;
                        }
                        else
                        {
                            // Axis to Right
                            // iXStep > 0
                            if(lOrgPosX == lAxisPosX)
                                lUpperLeftX = -lColumns + 1;
                        }

                        if(lAxisPosY == 'U')
                        {
                            if(lOrgPosY != lAxisPosY)
                                lUpperLeftY = lRows - 1;
                        }
                        else
                        {
                            if(lOrgPosY == lAxisPosY)
                                lUpperLeftY = -lRows + 1;
                        }

                        if(lOrgPosX == 'R')
                            lTmpOriginX = lColumns - 1;
                        if(lOrgPosY == 'D')
                            lTmpOriginY = lRows - 1;
                    }

                    if(lOriginX != -32768 && lOriginY != -32768)
                    {
                        lUpperLeftX = (lTmpOriginX + lOriginX);
                        lUpperLeftY = (lTmpOriginY + lOriginY);
                    }
                }

                // Set the current col
                lCurrentCol = 0;

                // Loop over all dies in line
                for(iIndex = 0; iIndex < lSection.length(); iIndex += iBinWidth)
                {
                    // Compute die position
                    wCoord.SetX(lCurrentCol + lUpperLeftX);
                    wCoord.SetY(lCurrentRow + lUpperLeftY);

                    // Get transformed map coordinate (in case of map alignment)
                    wCoord = mPrivate->mContext->GetExternalTransformation().Map(wCoord);

                    // Get PAT binning for this die (if any)
                    mPrivate->mContext->GetPatFailureDeviceDetails(wCoord.GetX(), wCoord.GetY(), cDevice);

                    iBinCode = cDevice.iPatHBin;	// =-1 if this die location is NOT a pat failure

                    // Get STDF bin
                    // Die within STDF wafermap area (but maybe die not tested!), get PAT-Man value!...offset X & Y to start on wafer corner
                    if (mPrivate->mContext->m_Stdf_SbinMap.indexFromCoord(lDieIndex, wCoord.GetX(), wCoord.GetY()))
                        lSTDFBin    = mPrivate->mContext->m_Stdf_SbinMap.getWafMap()[lDieIndex].getBin();
                    else
                        lSTDFBin    = GEX_WAFMAP_EMPTY_CELL;

                    // Wafermap:Die processing
                    if(lPassIdx == 1)
                    {
                        // Pass1: If this is NOT a PAT failure, get its bin
                        lDie = lSection.mid(iIndex,iBinWidth);	// Hex bin string

                        // Parts has been tested (not null bin)
                        if(lNullBin.isEmpty() || lDie.compare(lNullBin, Qt::CaseInsensitive))
                        {
                            if(iBinCode < 0)
                            {
                                iBinCode = lDie.toInt(&lOk,16);			// Hex Bin#

                                if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL &&
                                    mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                                {
                                    CBinning * pBin = mPrivate->mContext->GetExternalBins()->Find(iBinCode);

                                    if (pBin && pBin->cPassFail == 'P')
                                        iBinCode = mPrivate->mSettings.iGoodMapOverload_MissingStdf;
                                }
                            }

                            // Update BinCount for this Bin#
                            if(lBinList.contains(iBinCode))
                                lBinList[iBinCode]++;		// Bin entry already exists, update its count
                            else
                                lBinList[iBinCode]=1;		// New bin entry, set its count
                        }
                    }
                    else
                    {
                        // Pass2: Update BinMap if this die is a PAT failure
                        lDie = lSection.mid(iIndex,iBinWidth);	// Hex bin string
                        if(iBinCode >= 0)
                        {
                            // Write PAT Hex bin#
                            lDie.sprintf("%X",iBinCode);
                            lDie = lDie.rightJustified(iBinWidth,'0');
                        }
                        else if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL &&
                                 mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                        {
                            CBinning * pBin = mPrivate->mContext->GetExternalBins()->Find(lDie.toInt(&lOk, 16));

                            if (pBin && pBin->cPassFail == 'P')
                            {
                                // Write PAT Hex bin#
                                lDie.sprintf("%X", mPrivate->mSettings.iGoodMapOverload_MissingStdf);
                                lDie = lDie.rightJustified(iBinWidth,'0');
                            }
                        }

                        // Update die Bin value
                        lLine.replace(iLineOffset + iIndex,iBinWidth,lDie);
                    }

                    lCurrentCol += lStepX;
                }

                // Pass2: write update map line
                if(lPassIdx == 2)
                    lE142File += lLine;

                // Reset buffer
                lLine = "";

                // Keep track of wafer line#
                lCurrentRow += lStepY;
            }

            // Save line
save_line:
            if((lPassIdx == 2) && (lLine.isEmpty() == false))
                lE142File += lLine;
        }
        while(lInputStream.atEnd() == false);
    }

    lInputFile.close();

    // Write updated wafermap to output file
    // 1: Create new output KLA / INF file
    QFile lOutputFile(outputExternalMap);
    if(!lOutputFile.open(QIODevice::WriteOnly))
    {
        // Failed Opening new SEMI-E142 file
        mPrivate->mErrorMessage = "\t> *Error* Failed to create updated SEMI-E142.\nProtection issue?";
        return false;
    }

    // Assign file I/O stream
    QTextStream lOutputStream(&lOutputFile);

    // 2: Write complete file
    for(iIndex = 0; iIndex < lE142File.count(); ++iIndex)
        lOutputStream << lE142File.at(iIndex) << endl;

    lOutputFile.close();

    return true;
}

bool PATProcessWS::UpdateKLAINFExternalMap(const QString &inputExternalMap, const QString &outputExternalMap)
{
    // Open KLA / INF file
    QFile lInputFile(inputExternalMap);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening KLA / INF file
        mPrivate->mErrorMessage = "\t> *Error* Failed to open input file.\nProtection issue? : " +
                                  inputExternalMap;
        return false;
    }

    // Assign file I/O stream
    QTextStream lInputStream(&lInputFile);

    // Check if first line is the correct KLA INF header...
    long	lLineIdx    = 0;					// Index to line read from KLA file
    long	lTotalLines = 0;					// Total lines in KLA file

    long	lStart_LastSmWaferPass=0;			// Index to beginning of last 'SmWaferPass' section
    long	lStart_SmWaferPass_iBinCode=0;		// 'iBinCode' section in last 'SmWaferPass' section
    long	lStart_SmWaferPass_iBinCodeLast=0;	// 'iBinCodeLast' section in last 'SmWaferPass' section
    long	lEnd_SmWaferPass_iBinCodeLast=0;	// First 'NlLayer' bloc header  just AFTER lStart_SmWaferPass_iBinCodeLast

    long	lStart_LastMdMapResult=0;			// 'MdMapResult' starting section
    long	lStart_MdMapResult_iBinCode=0;		// 'iBinCode' section in last 'MdMapResult' section
    long	lStart_MdMapResult_iBinCodeLast=0;	// 'iBinCodeLast' section in last 'MdMapResult' section
    long	lEnd_MdMapResult_iBinCodeLast=0;	// end of 'iBinCodeLast' section in last 'MdMapResult' section

    QStringList	lKLAINFFile;
    QString     lSection;
    QString     lLine = lInputStream.readLine();

    if(lLine != "SmWaferFlow")
    {
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF first line is not 'SmWaferFlow' section.";
        return false;	// This file is not a KLA/INF file!
    }

    // Load full file into a QStrinList object
    do
    {
        // Check if pointing line: 'SESSION_RESULTS:PROCESSED '
        // If so, double the 'PROCESSED' field as we're going to add a second layer!
        if(lLine.indexOf("SESSION_RESULTS:PROCESSED") >= 0)
            lLine += "PROCESSED ";

        // Save file line in buffer
        lKLAINFFile += lLine;

        // Update line index.
        ++lLineIdx;

        // Read next line from file
        lLine = lInputStream.readLine();
    }
    while(lInputStream.atEnd() == false);

    // Save last file line in buffer
    lKLAINFFile += lLine;

    lInputFile.close();

    // Total lines in KLA / Inf file
    lTotalLines = lLineIdx;

    // 1: Find last 'MdMapResult' section
    lLineIdx = 0;
    QStringList::Iterator it;
    for (it = lKLAINFFile.begin(); it != lKLAINFFile.end(); ++it )
    {
        // Remove spaces
        if((*it).trimmed() == "MdMapResult")
        {
            // Detect  the last 'MdMapResult'
            lStart_LastMdMapResult = lLineIdx;
        }

        // Update line index
        lLineIdx++;
    }

    // 2: Find last 'SmWaferPass' BEFORE the last 'MdMapResult'
    lLineIdx = 0;
    for (it = lKLAINFFile.begin(); it != lKLAINFFile.end(); ++it )
    {
        if((*it).trimmed() == "SmWaferPass")
        {
            // Detect the last 'SmWaferPass' BEFORE the last 'MdMapResult'
            if(lLineIdx < lStart_LastMdMapResult)
                lStart_LastSmWaferPass = lLineIdx;
        }

        // Update line index
        lLineIdx++;
    }

    // 3: Find the 'iBinCode' and 'iBinCodeLast' of section 'SmWaferPass'
    for (lLineIdx = lStart_LastSmWaferPass; lLineIdx < lStart_LastMdMapResult; ++lLineIdx)
    {
        lLine = lKLAINFFile.at(lLineIdx).trimmed();

        if(lLine == "strTag:iBinCode")
            lStart_SmWaferPass_iBinCode = lLineIdx;	// Parsing 'SmWaferPass' section
        else if(lLine == "strTag:iBinCodeLast")
            lStart_SmWaferPass_iBinCodeLast = lLineIdx;	// Parsing 'SmWaferPass' section

        // Find first 'NlLayer' after the last 'iBinCodeLast'
        if(lStart_SmWaferPass_iBinCodeLast && (!lEnd_SmWaferPass_iBinCodeLast) && lLine == "NlLayer")
            lEnd_SmWaferPass_iBinCodeLast = lLineIdx;
    }

    // 4: Find the 'iBinCode' and 'iBinCodeLast' of section 'MdMapResult'
    for (lLineIdx = lStart_LastMdMapResult; lLineIdx < lTotalLines;  lLineIdx++)
    {
        lLine = lKLAINFFile.at(lLineIdx).trimmed();

        if(lLine == "strTag:iBinCode")
            lStart_MdMapResult_iBinCode = lLineIdx;	// Parsing 'SmWaferPass' section
        else if(lLine == "strTag:iBinCodeLast")
            lStart_MdMapResult_iBinCodeLast = lLineIdx;	// Parsing 'SmWaferPass' section

        if(lStart_MdMapResult_iBinCodeLast && !lEnd_MdMapResult_iBinCodeLast && lLine == "NlLayer")
        {
            // We've reached the end of the wafermap define in last iBinCodeLast of section 'MdMapResult'
            lEnd_MdMapResult_iBinCodeLast = lLineIdx;
            break;
        }
    }

    // 5: Extract some fields that may be required if generating a SINF file
    bool                    lRowRdc = false;
    bool                    lColRdc = false;
    CGKlaInfLayersBinmap    lMapBins; // Case 6531
    QList<int>              lGoodBins;

    for (lLineIdx = 0; lLineIdx < lTotalLines;  lLineIdx++)
    {
        lLine = lKLAINFFile.at(lLineIdx).trimmed();

        if(lLine.startsWith("WPT_ID:"))
            // Check for Device name
            mPrivate->mSINFInfo.mDeviceName = lLine.section(':',1);
        else if(lLine.startsWith("LOT:"))
            // Check for Lot#
            mPrivate->mSINFInfo.mLot = lLine.section(':',1);
        else if(lLine.startsWith("ROT:"))
            // Check for Wafermap FLAT orientation
            mPrivate->mSINFInfo.mFlatOrientation = lLine.section(':',1).toLong();
        else if(lLine.startsWith("ID:"))
            // Check for Wafer#
            mPrivate->mSINFInfo.mWaferID = lLine.section(':',1);
        else if(lLine.startsWith("strTag:PSBN"))
        {
            // Check for list of GOOD bins
            // If a 'StBinTable' with PSBN tag is found, read binning info
            // P0 case 6531: updated code to read this section (buggy) and
            // keep binning info in map
            bool    lEndOfSection   = false;
            int     lBinNumber      = 0;

            // Skip line 'strKeyword:PSBN'
            ++lLineIdx;

            // HTH TO CHECK --> lLineIdx could be greater than lTotalLines which could lead to crash
            while(!lEndOfSection)
            {
                lSection = lKLAINFFile.at(lLineIdx).trimmed();
                if(lSection.indexOf("}")>=0)
                {
                    lEndOfSection   = true;
                    lSection        = lSection.section('}',0);
                }
                if(lSection.indexOf("{")>=0)
                {
                    lEndOfSection   = true;
                    lSection        = lSection.section('{',0);
                }
                if(lSection.startsWith("ListData:"))
                {
                    // Parse line '	ListData:01100000000000000000000000000000'
                    lSection = lSection.section(':', 1);

                    for(int lIdx = 0; lIdx < lSection.length(); ++lIdx)
                    {
                        lMapBins[lBinNumber].nBinNb = lBinNumber;
                        lMapBins[lBinNumber].bPass = (lSection.at(lIdx) == QChar('1'));

                        // Case 6571: add good bins in a list instead of
                        // directly using m_cSinfInfo.strBCEQ, as we don't
                        // know which formatting to use yet
                        if(lMapBins[lBinNumber].bPass)
                            lGoodBins.append(lBinNumber);

                        ++lBinNumber;
                    }
                }

                // Move to next line
                ++lLineIdx;
            }
        }

        // Extract reference die coordinates
        if(lLineIdx > lStart_LastMdMapResult)
        {
            if(lLine.startsWith("iRowRdc:"))
            {
                lSection = lLine.section(':',1);
                mPrivate->mSINFInfo.mRowRdc = lSection.toLong(&lRowRdc);
            }
            else if(lLine.startsWith("iColRdc:"))
            {
                lSection = lLine.section(':',1);
                mPrivate->mSINFInfo.mColRdc = lSection.toLong(&lColRdc);
            }
            if(lLine.startsWith("dDieWidth:"))
            {
                lSection = lLine.section(':',1);
                mPrivate->mSINFInfo.mDieSizeX = lSection.toDouble();
            }
            else if(lLine.startsWith("dDieHeight:"))
            {
                lSection = lLine.section(':',1);
                mPrivate->mSINFInfo.mDieSizeY = lSection.toDouble();
            }
        }
    }

    // File loaded, check if compliant
    if(!lStart_LastSmWaferPass)
    {
        // Error: not even one strucutre 'SmWaferPass' in KLA file!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF file missing 'SmWaferPass' section.";
        return false;
    }
    if(!lStart_SmWaferPass_iBinCode && !lStart_SmWaferPass_iBinCodeLast)
    {
        // Error: not even one strucutre 'SmWaferPass' in KLA file!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF file missing 'SmWaferPass: iBinCode' and 'SmWaferPass: iBinCodeLast' sections.";
        return false;
    }
    if(!lStart_LastMdMapResult)
    {
        // Error: not even one strucutre 'MdMapResult' in KLA file!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF file missing 'MdMapResult' section.";
        return false;
    }
    if(!lStart_MdMapResult_iBinCode)
    {
        // Error: not even one strucutre 'MdMapResult' in KLA file!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF file missing 'MdMapResult: iBinCode' section.";
        return false;
    }
    if(!lStart_MdMapResult_iBinCodeLast)
    {
        // Error: not even one strucutre 'MdMapResult' in KLA file!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF file missing 'MdMapResult: iBinCodeLast' section.";
        return false;
    }

    // Detect wafermap iRowMin, iColMin et iBase (eg: Hexa binnings)
    int     lIdx;
    long	iRowMin = 0;
    long    iColMin = 0;
    long    iBinningBase        = 0;
    long    iFieldSize          = 0;
    bool	bRowMin             = false;
    bool    bRowCol             = false;
    bool    bBinningBase        = false;
    bool    bFieldSize          = false;
    bool	bNlFormatSection    = false;
    QString	strNlFormat         = "";

    for(lLineIdx = lStart_MdMapResult_iBinCodeLast; lLineIdx < lEnd_MdMapResult_iBinCodeLast; ++lLineIdx)
    {
        lLine       = lKLAINFFile.at(lLineIdx);
        lSection    = lKLAINFFile.at(lLineIdx).trimmed();

        if(lSection.startsWith("NlFormat"))
        {
            // Flag to tell record the 'NlFormat' section!
            bNlFormatSection = true;
        }

        if(bNlFormatSection)
        {
            strNlFormat += lLine + "\n";
            if(lSection.startsWith("}"))
                bNlFormatSection = false;	// Stop saving 'NlFromat' section when the structure ends.
        }

        if(lSection.startsWith("iRowMin:"))
        {
            lSection    = lSection.section(':',1);
            iRowMin     = lSection.toLong(&bRowMin);
        }
        else if(lSection.startsWith("iColMin:"))
        {
            lSection    = lSection.section(':',1);
            iColMin     = lSection.toLong(&bRowCol);
        }
        else if(lSection.startsWith("iBase:"))
        {
            lSection        = lSection.section(':',1);
            iBinningBase    = lSection.toLong(&bBinningBase);
        }
        else if(lSection.startsWith("sField:"))
        {
            lSection    = lSection.section(':',1);
            iFieldSize  = lSection.toLong(&bFieldSize);
        }
    }
    // Check if succesful
    if(!bRowMin || !bRowCol || !bBinningBase || !bFieldSize || !lRowRdc || !lColRdc)
    {
        // Error: failed to identify KLA Wafermap size & coding type!
        mPrivate->mErrorMessage = "\t> *Error* KLA/INF wafer info missing. Failed to extract wafer size, and binning coding base and size.";
        return false;
    }

    // Compute die reference location (only used if generating a SINF file, and updating a KLA/INF
    mPrivate->mSINFInfo.mRefPX = 1 + mPrivate->mSINFInfo.mColRdc - iColMin;
    mPrivate->mSINFInfo.mRefPY = 1 + mPrivate->mSINFInfo.mRowRdc - iRowMin;

    // Create new Wafermap...
    QStringList	strDies;
    int         iDiesOnLine;        // Number of dies on a wafer row line.
    QChar       cChar;
    int         iBinCode;
    int         iCol;
    int         iRow = iRowMin;
    char        szFormatting[10];   // Used for formatting ASCII Bin output
    char        szBinResult[10];    // Used for coding Bin result in given base and with leading 0 if needed

    // Create formatting string used in 'sprinf' function
    switch(iBinningBase)
    {
        case 16:
            sprintf(szFormatting,"%%0%ldX", iFieldSize);	// Eg. "%02X"
            break;
        case 7:
            sprintf(szFormatting,"%%0%ldo", iFieldSize);	// Eg. "%02o"
            break;
        default:
            sprintf(szFormatting,"%%%ldd", iFieldSize);	// Eg. "%2d"
    }

    // Case 6571: write strBCEQ string with correct formatting
    for(int lIdx = 0; lIdx < lGoodBins.size();  ++lIdx)
    {
        sprintf(szBinResult,szFormatting,lGoodBins.at(lIdx));

        if(lIdx == 0)
            mPrivate->mSINFInfo.mBCEQ = QString(szBinResult);
        else
            mPrivate->mSINFInfo.mBCEQ += QString(" ") + QString(szBinResult);
    }

    // Detect the X,Y offset to apply in case the KLA wafermap includes fully empty lines and columns (top, left margins)
    int	iOffsetX        = 10000;
    int	iOffsetY        = 10000;
    int	iRowDataLine    = 0;
    int	iTotalDieInfoInLine;	// Used to detect a full empty line in the wafer (and ignore it)

    for(lLineIdx = lStart_MdMapResult_iBinCodeLast; lLineIdx < lEnd_MdMapResult_iBinCodeLast; ++lLineIdx)
    {
        lLine       = lKLAINFFile.at(lLineIdx);
        lSection    = lKLAINFFile.at(lLineIdx).trimmed();

        if(lSection.startsWith("RowData:"))
        {
            iTotalDieInfoInLine = 0;

            // Extract Bin data (info to the right of 'RowData:')
            // E.g: "RowData:__ __ __ __ __ __ __ __ __ __ __ __ __ @@ @@ @@ @@ 0D@ 01 05 @@ @@ @@ @@ @@ @@ @@ __ __ __ ____ __ __ __"
            lLine       = lLine.section(':', 1);
            strDies     = lLine.split(' ', QString::SkipEmptyParts);
            iDiesOnLine = strDies.count();

            // Processing a wafer line.
            for(iCol = 0; iCol < iDiesOnLine; iCol++)
            {
                // Detect first column with valid die info
                cChar = strDies[iCol][0];
                if((cChar.isDigit() == true) || (cChar >= 'A' && cChar <= 'F'))
                {
                    // Valid die info
                    iOffsetX = gex_min(iOffsetX, iCol);
                    iTotalDieInfoInLine++;
                }
            }

            if(iTotalDieInfoInLine)
                iOffsetY = gex_min(iOffsetY, iRowDataLine);

            // Increment RawData line#
            iRowDataLine++;
        }
    }

    // Save Input INF Wafer space area
    mPrivate->mSINFInfo.mWaferAndPaddingRows = iRowDataLine;
    mPrivate->mSINFInfo.mWaferAndPaddingCols = strDies.count();

    // Reset wafermap row#.
    CPatFailureDeviceDetails cDevice;
    bool        bGood_die_before_PAT;
    int         iPatBin;
    int         iStdfBin;
    int         iKlaBin;
    bool        bValidKlaBin;
    bool        bKlaBinIsPass;

    iRow = 0;
    for(lLineIdx = lStart_MdMapResult_iBinCodeLast; lLineIdx < lEnd_MdMapResult_iBinCodeLast; ++lLineIdx)
    {
        lLine       = lKLAINFFile.at(lLineIdx);
        lSection    = lKLAINFFile.at(lLineIdx).trimmed();

        if(lSection.startsWith("RowData:"))
        {
            // Extract Bin data (info to the right of 'RowData:')
            // E.g: "RowData:__ __ __ __ __ __ __ __ __ __ __ __ __ @@ @@ @@ @@ 0D@ 01 05 @@ @@ @@ @@ @@ @@ @@ __ __ __ ____ __ __ __"
            lLine       = lLine.section(':',1);
            strDies     = lLine.split(' ', QString::SkipEmptyParts);
            iDiesOnLine = strDies.count();

            // Processing a wafer line.
            for(iCol = 0; iCol < iDiesOnLine; iCol++)
            {
                // If start of line, then insert keyword header
                if(!iCol)
                    mPrivate->mSINFInfo.mNewWafermap += KLA_ROW_DATA_TAB_STRING; // Add string: "\t    RowData:";

                // Get original Hexadecimal binning in the KLA/INF file
                iKlaBin = strDies[iCol].toInt(&bValidKlaBin, iBinningBase);
                if(bValidKlaBin == false)
                {
                    mPrivate->mSINFInfo.mNewWafermap += strDies[iCol];
                }
                else
                {
                    // Check if KLA Bin is PASS
                    // P0 case 6531: decide P/F based on binning QMap or Bin=1 if no entry
                    bKlaBinIsPass = lMapBins.isGoodBin(iKlaBin);

                    // Keep track of total valid dies in KLA/MAP file
                    mPrivate->mExternalMapDetails.mTotalDies++;

                    // Keep track of total bins in the map Before PAT
                    if(mPrivate->mExternalMapDetails.mBinCountBeforePAT.contains(iKlaBin))
                        mPrivate->mExternalMapDetails.mBinCountBeforePAT[iKlaBin] = mPrivate->mExternalMapDetails.mBinCountBeforePAT[iKlaBin] + 1;
                    else
                        mPrivate->mExternalMapDetails.mBinCountBeforePAT[iKlaBin] = 1;

                    // Compute KLA/INF die location
                    WaferCoordinate wCoord(iColMin + iCol, iRowMin + iRow);

                    wCoord = mPrivate->mContext->GetExternalTransformation().Map(wCoord);

                    // Check if valid STDF die location
                    if(mPrivate->mContext->m_Stdf_SbinMap.indexFromCoord(lIdx, wCoord.GetX(), wCoord.GetY()) == false)
                    {
                        // Die not tested in STDF!
                        iStdfBin = iBinCode = iPatBin = GEX_WAFMAP_EMPTY_CELL;
                        bGood_die_before_PAT = false;
                    }
                    else
                    {
                        // Check if this die is a DPAT failrue
                        mPrivate->mContext->GetPatFailureDeviceDetails(wCoord.GetX(), wCoord.GetY(), cDevice);

                        iPatBin = cDevice.iPatSBin;

                        // Die within STDF wafermap area (but maybe die not tested!), get PAT-Man value!...offset X & Y to start on wafer corner
                        iStdfBin = iBinCode = mPrivate->mContext->m_Stdf_SbinMap.getWafMap()[lIdx].getBin();

                        if(cDevice.iPatSBin >= 0)
                            bGood_die_before_PAT = true;	// This die is a PAT failure, so was a good die!
                        else
                        {
                            // Check if STDF bin belongs to 'good bins' list
                            if(mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin))
                                bGood_die_before_PAT = true;
                            else
                                bGood_die_before_PAT = false;
                        }
                    }

                    // New ADI specs. (Oct-2008)
                    switch(mPrivate->mContext->GetRecipeOptions().iPatLimitsFromBin)
                    {
                        case GEX_TPAT_BUILDLIMITS_ALLBINS:
                        case GEX_TPAT_BUILDLIMITS_LISTSOFTBINS:
                        case GEX_TPAT_BUILDLIMITS_LISTHARDBINS:
                        case GEX_TPAT_BUILDLIMITS_EXCEPTSOFTBINS:
                        case GEX_TPAT_BUILDLIMITS_EXCEPTHARDBINS:
                        default:
                            // KLA = GOOD, STDF (before PAT) = GOOD
                            if((bKlaBinIsPass) && (bGood_die_before_PAT))
                            {
                                // KLA=STDF=Good
                                if(iPatBin >= 0)
                                    iBinCode = iPatBin;	// STDF PAT failure
                                else
                                    iBinCode = iKlaBin;	// Not PAT failure
                            }
                            if((bKlaBinIsPass) && (bGood_die_before_PAT == false) &&
                               (iStdfBin != GEX_WAFMAP_EMPTY_CELL))
                            {
                                // KLA=Good, STDF fail => Overlay rule
                                if(mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                                    iBinCode = mPrivate->mSettings.iGoodMapOverload_FailStdf;
                                else
                                    iBinCode = 0x1F;
                            }
                            if((!bKlaBinIsPass) && (bGood_die_before_PAT))
                            {
                                // KLA=Fail, STDF (before pat) = Good
                                iBinCode = iKlaBin;
                            }
                            if((!bKlaBinIsPass) && (bGood_die_before_PAT == false) &&
                               (iStdfBin != GEX_WAFMAP_EMPTY_CELL))
                            {
                                // KLA=Fail, STDF (before pat) Fail
                                iBinCode = iKlaBin;
                            }
                            if(iStdfBin == GEX_WAFMAP_EMPTY_CELL)
                            {
                                // KLA=Good/Fail, STDF Missing
                                if(mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                                    iBinCode = mPrivate->mSettings.iGoodMapOverload_MissingStdf;
                                else
                                    iBinCode = 0x1F;
                            }
                            break;

                        case GEX_TPAT_BUILDLIMITS_GOODHARDBINS:
                        case GEX_TPAT_BUILDLIMITS_GOODSOFTBINS:
                            // Using GOOD bins only to compute PAT limits, so keep KLA origina bin if STDF binning is missing
                            if((bKlaBinIsPass) && (bGood_die_before_PAT))
                            {
                                // KLA=STDF (before pat) =Good
                                if(iPatBin >= 0)
                                    iBinCode = iPatBin;	// STDF PAT failure
                                else
                                    iBinCode = iKlaBin;	// Not PAT failure
                            }
                            if((bKlaBinIsPass) && (bGood_die_before_PAT == false) &&
                               (iStdfBin != GEX_WAFMAP_EMPTY_CELL))
                            {
                                // KLA=Good, STDF (before pat) fail => Overlay rule
                                if(mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                                    iBinCode = mPrivate->mSettings.iGoodMapOverload_FailStdf;
                                else
                                    iBinCode = 0x1F;
                            }
                            if((!bKlaBinIsPass) && (bGood_die_before_PAT))
                            {
                                // KLA=Fail, STDF (before pat) Good
                                iBinCode = iKlaBin;
                            }
                            if((!bKlaBinIsPass) && (bGood_die_before_PAT == false) &&
                               (iStdfBin != GEX_WAFMAP_EMPTY_CELL))
                            {
                                // KLA=Fail, STDF (before pat) Fail
                                iBinCode = iKlaBin;
                            }
                            if(iStdfBin == GEX_WAFMAP_EMPTY_CELL)
                            {
                                // KLA = Fail
                                if(!bKlaBinIsPass)
                                    iBinCode = iKlaBin;
                                else
                                    // KLA=Good...custom FAIL bin defined?
                                    if(mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                                        iBinCode = mPrivate->mSettings.iGoodMapOverload_MissingStdf;
                                    else
                                        iBinCode = 0x1F;
                            }
                            break;
                    }

                    // Check if Die tested in STDF
                    if(iBinCode < 0)
                    {
                        // Die not tested in STDF!
                        mPrivate->mSINFInfo.mNewWafermap += "1F";				// Die not tested in STDF!...exclude die.
                    }
                    else
                    {
                        // Write ASCII binning using relevant coding base
                        sprintf(szBinResult, szFormatting, iBinCode);
                        mPrivate->mSINFInfo.mNewWafermap += szBinResult;

                        // Keep track of matching die locations between STDF and KLA MAP
                        mPrivate->mExternalMapDetails.mTotalMatchingMapDiesLoc++;

                        // number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
                        if(bKlaBinIsPass && (iPatBin >= 0))
                            mPrivate->mExternalMapDetails.mGoodPartsMapDPATFailures++;

                        // number of good die from the input wafer map that do not have matching STDF data.
                        if(bKlaBinIsPass &&
                           mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin) == false)
                            mPrivate->mExternalMapDetails.mGoodPartsMapSTDFMissing++;

                        // keep track of binning mismatch
                        if(iStdfBin != iKlaBin)
                        {
                            if(bKlaBinIsPass)
                            {
                                // KLA=Pass, STDF = Fail
                                if (mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin) == false)
                                    mPrivate->mExternalMapDetails.mTotalMismatchingMapPassBin++;
                            }
                            else
                            {
                                // KLA=Fail, STDF = Pass
                                if(mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin))
                                    mPrivate->mExternalMapDetails.mTotalMismatchingMapFailBin++;
                            }
                        }
                    }

                    // Keep track of total bins in the map After PAT
                    if(iBinCode < 0)
                        iBinCode = 0x1F;
                    if(mPrivate->mExternalMapDetails.mBinCountAfterPAT.contains(iBinCode))
                        mPrivate->mExternalMapDetails.mBinCountAfterPAT[iBinCode] = mPrivate->mExternalMapDetails.mBinCountAfterPAT[iBinCode] + 1;
                    else
                        mPrivate->mExternalMapDetails.mBinCountAfterPAT[iBinCode] = 1;
                }

                // Insert space between binning results
                mPrivate->mSINFInfo.mNewWafermap += " ";
            }

            // Insert CR-LF between lines
            mPrivate->mSINFInfo.mNewWafermap += "\n";

            iRow++;
        }
    }

    mPrivate->mExternalMapDetails.mComputed = true;

    // Check if need to create output file
    if(outputExternalMap.isEmpty() == false)
    {
        ////////// Create new KLA file
        // 1: Create new output KLA / INF file
        QFile lNewKLAFile(outputExternalMap);

        if(lNewKLAFile.open(QIODevice::WriteOnly) == false)
        {
            // Failed Opening new KLA / INF file
            mPrivate->mErrorMessage = "\t> *Error* Failed to create updated KLA/INF.\nProtection issue?";
            return false;
        }

        // Assign file I/O stream
        QTextStream hNewKlaInfFile(&lNewKLAFile);

        // 2: Write header section as-is ('SmWaferFlow') section, up-to line prior to last 'SmWaferPass'
        for(lLineIdx = 0; lLineIdx < lStart_LastMdMapResult; ++lLineIdx)
            hNewKlaInfFile << lKLAINFFile.at(lLineIdx) << endl;

        // 3: Add new 'SmWaferPass' section holding PAT-Man new wafer iBinCode and iBinCodeLast layers (if no 'iBinCode' section, recah 'iBinCodeLast' instead)
        long lMaxFileLine = (lStart_SmWaferPass_iBinCode) ? lStart_SmWaferPass_iBinCode: lStart_SmWaferPass_iBinCodeLast;
        for(lLineIdx = lStart_LastSmWaferPass; lLineIdx < lMaxFileLine; ++lLineIdx)
        {
            // Copy header section until 'NlLayer' of 'MdMapResult' sub-block found
            lLine       = lKLAINFFile.at(lLineIdx);
            lSection    = lKLAINFFile.at(lLineIdx).trimmed();

            if(lSection.indexOf("NlLayer") >= 0)
                break;	// We've copied all the header, so exit loop now.

            // Replace PASS_ID:xxxx with 'PASS_ID:2'
            if(lSection.indexOf("PASS_ID:") >= 0)
            {
                int iIndex = lLine.indexOf(':');
                lLine.truncate(iIndex);
                lLine += ":2";
            }

            // Replace SESSION_NUM:xxxx with 'SESSION_NUM:2'
            if(lSection.indexOf("SESSION_NUM:") >= 0)
            {
                int iIndex = lLine.indexOf(':');
                lLine.truncate(iIndex);
                lLine += ":2";
            }

            // Replace OPID:xxxx with 'OPID:Galaxy'
            if(lSection.indexOf("OPID:") >= 0)
            {
                int iIndex = lLine.indexOf(':');
                lLine.truncate(iIndex);
                lLine += ":Galaxy";
            }

            // Replace TST_SW_RV:xxxx with 'TST_SW_RV:<galaxy revision>'
            if(lSection.indexOf("TST_SW_RV:") >= 0)
            {
                int iIndex = lLine.indexOf(':');
                lLine.truncate(iIndex+1);
                lLine += Engine::GetInstance().Get("AppFullName").toString();
            }

            // Replace TST_PRG_SW_NM:xxxx with 'TST_PRG_SW_NM:<Galaxy recipe name>'
            if(lSection.indexOf("TST_PRG_SW_NM:") >= 0)
            {
                int iIndex = lLine.indexOf(':');
                lLine.truncate(iIndex+1);
                lLine += mPrivate->mContext->GetRecipeFilename();
            }

            // Write line to file.
            hNewKlaInfFile << lLine << endl;
        }

        // Add new PAT-Man 'iBinCode' section
        hNewKlaInfFile << "\t\tNlLayer" << endl;
        hNewKlaInfFile << "\t\t{" << endl;
        hNewKlaInfFile << "\t\tstrTag:iBinCode" << endl;
        hNewKlaInfFile << "\t\tstrKeyword:iBinCode" << endl;
        hNewKlaInfFile << "\t\tiRowMin:" << QString::number(iRowMin)  << endl;
        hNewKlaInfFile << "\t\tiColMin:" << QString::number(iColMin)  << endl;
        hNewKlaInfFile << mPrivate->mSINFInfo.mNewWafermap;	// new-line in included in wafermap buffer
        hNewKlaInfFile << strNlFormat;
        hNewKlaInfFile << "\t\t}" << endl;	// close 'NlLayer'

        hNewKlaInfFile << "\t\tNlLayer" << endl;
        hNewKlaInfFile << "\t\t{" << endl;
        hNewKlaInfFile << "\t\tstrTag:iBinCodeLast" << endl;
        hNewKlaInfFile << "\t\tstrKeyword:iBinCodeLast" << endl;
        hNewKlaInfFile << "\t\tiRowMin:" << QString::number(iRowMin)  << endl;
        hNewKlaInfFile << "\t\tiColMin:" << QString::number(iColMin)  << endl;
        hNewKlaInfFile << mPrivate->mSINFInfo.mNewWafermap;	// new-line in included in wafermap buffer
        hNewKlaInfFile << strNlFormat;
        hNewKlaInfFile << "\t\t}" << endl;	// close 'NlLayer'

        // Add ALL codes following layer just after the 'iBinCodeLast' and before 'MdMapResult
        if(lEnd_SmWaferPass_iBinCodeLast)
        {
            // In case blocs of 'NiLayer' exist after last 'iBinCodeLast'
            for(lLineIdx = lEnd_SmWaferPass_iBinCodeLast; lLineIdx < lStart_LastMdMapResult; ++lLineIdx)
            {
                // Write line to file.
                hNewKlaInfFile << lKLAINFFile.at(lLineIdx) << endl;
            }
        }

        // Below line removed: per Analog request Jan.2008
        // hNewKlaInfFile << "\t}" << endl;	// close 'SmWaferPass'

        // 4: MdMapResult: replace last iBinCode and iBinCodeLast sections.
        for(lLineIdx = lStart_LastMdMapResult; lLineIdx < lStart_MdMapResult_iBinCode; ++lLineIdx)
        {
            // Write line to file.
            hNewKlaInfFile << lKLAINFFile.at(lLineIdx) << endl;
        }

        // Lines MUST start with "\t + 4 spaces" (because of ADI perl script!)
        hNewKlaInfFile << "\t    strTag:iBinCode" << endl;
        hNewKlaInfFile << "\t    strKeyword:iBinCode" << endl;
        hNewKlaInfFile << "\t    iRowMin:" << QString::number(iRowMin)  << endl;
        hNewKlaInfFile << "\t    iColMin:" << QString::number(iColMin)  << endl;
        hNewKlaInfFile << mPrivate->mSINFInfo.mNewWafermap;	// new-line in included in wafermap buffer
        hNewKlaInfFile << strNlFormat;
        hNewKlaInfFile << "\t\t}" << endl;	// close 'NlLayer'

        hNewKlaInfFile << "\t\tNlLayer" << endl;
        hNewKlaInfFile << "\t\t{" << endl;
        hNewKlaInfFile << "\t    strTag:iBinCodeLast" << endl;
        hNewKlaInfFile << "\t    strKeyword:iBinCodeLast" << endl;
        hNewKlaInfFile << "\t    iRowMin:" << QString::number(iRowMin)  << endl;
        hNewKlaInfFile << "\t    iColMin:" << QString::number(iColMin)  << endl;
        hNewKlaInfFile << mPrivate->mSINFInfo.mNewWafermap;	// new-line in included in wafermap buffer
        hNewKlaInfFile << strNlFormat;
        hNewKlaInfFile << "\t}" << endl;	// close 'NlLayer'

        // Find first line after the end of the iBinCodeLast bloc in 'mdMapResult'
        for(lLineIdx = lStart_MdMapResult_iBinCodeLast+1; lLineIdx <= lTotalLines; ++lLineIdx)
        {
            if(lKLAINFFile.at(lLineIdx).trimmed().startsWith("NlLayer"))
                break;	// We've copied all the header, so exit loop now.
        }

        // Copy the rest of the file...
        while(lLineIdx <= lTotalLines)
        {
            hNewKlaInfFile << lKLAINFFile.at(lLineIdx) << endl;
            ++lLineIdx;
        }

        // Close file
        lNewKLAFile.close();
    }

    return true;
}

bool PATProcessWS::UpdateTELP8ExternalMap(const QString &inputExternalMap,
                                        const QString &outputExternalMap)
{
    // Open TEL-P8 file
    char	szString[256];
    FILE *  lHandleInput    = NULL;
    FILE *  lHandleOutput   = NULL;

    // Original file (to update)
    lHandleInput = fopen(inputExternalMap.toLatin1().constData(), "rb");
    if(lHandleInput == NULL)
    {
        // Failed Opening TEL-P8 file
        mPrivate->mErrorMessage = "\t> *Error* Failed to open input file.\nProtection issue? : " +
                                  inputExternalMap;
        return false;
    }

    // Check if No need to create output file
    if(outputExternalMap.isEmpty())
    {
        lHandleOutput = NULL;
    }
    else
    {
        // New file (to create)
        lHandleOutput = fopen(outputExternalMap.toLatin1().constData(), "wb");
        if(lHandleOutput == NULL)
        {
            // Failed Opening TEL-P8 file
            mPrivate->mErrorMessage = "\t> *Error* Failed to update optional input file.\nProtection issue?";
            fclose(lHandleInput);
            return false;
        }
    }

    // Check if OLD P8 format or NEW million-die format
    QString             strString;
    bool                bFlag;
    bool                bNewP8_Format=false;
    long                lPassTotal  = 0;
    long                lFailTotal  = 0;
    long                lTestTotal  = 0;
    unsigned char       uLsb;
    unsigned char       uMsb;
    unsigned char       uValue;
    int                 iRecords;
    int                 iDiesInPreviousRow=-1;
    int                 iDiesInRow;
    int                 iPatBin;
    int                 iStdfBin;
    unsigned long       lP8_EncodedBin;	// Binning in P8 encoded format
    unsigned long       lP8Bin;			// P8 Binning decoded (0=Bin0,1=Bin1,...N=BinN)
    int                 iDieX;
    int                 iDieY;	// Die coordinates
    int                 lIdx;
    CPatFailureDeviceDetails cDevice;

    // Ensure end-of-line character is set in buffer.
    szString[11] = 0;
    if (fread(szString, sizeof(char), 11, lHandleInput) != 11)
    {
        return false;
    }
    strString   = szString;
    lPassTotal  = strString.toLong(&bFlag);

    if(bFlag == false)
        goto process_p8_file;	// This file is NOT a new P8 format!

    if (fread(szString, sizeof(char), 11, lHandleInput) != 11)
    {
        return false;
    }

    strString   = szString;
    lFailTotal  = strString.toLong(&bFlag);

    if(bFlag == false)
        goto process_p8_file;	// This file is NOT a new P8 format!

    if (fread(szString, sizeof(char), 11, lHandleInput) != 11)
    {
        return false;
    }

    strString   = szString;
    lTestTotal  = strString.toLong(&bFlag);

    if(bFlag == false)
        goto process_p8_file;	// This file is NOT a new P8 format!

    // This is a 'Million-die', new P8 format!
    bNewP8_Format = true;

process_p8_file:

    // Rewind input file
    rewind(lHandleInput);

    if(bNewP8_Format)
    {
        // Total Pass
        if (fread(szString, sizeof(char), 11, lHandleInput) != 11)
        {  // Skip data
            return false;
        }
        sprintf(szString,"%011ld", lPassTotal - mPrivate->mContext->m_lstOutlierParts.count());
        if(lHandleOutput)
            if (fwrite(szString, 11, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Total Fail
        if (fread(szString, sizeof(char), 11, lHandleInput) != 11)
        {  // Skip data
            return false;
        }
        sprintf(szString,"%011ld", lFailTotal + mPrivate->mContext->m_lstOutlierParts.count());
        if(lHandleOutput)
            if (fwrite(szString, 11, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Total tested
        if (fread(szString, sizeof(char), 11, lHandleInput) != 11) { // Skip data
            return false;
        }
        sprintf(szString,"%011ld",lTestTotal);
        if(lHandleOutput)
            if (fwrite(szString, 11, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        if (fread(szString, sizeof(char), 12, lHandleInput) != 12) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 12, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer ending time (BCD 12 bytes)
        if (fread(szString, sizeof(char), 12, lHandleInput) != 12) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 12, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer-ID (36 characters)
        if (fread(szString, sizeof(char), 36, lHandleInput) != 36) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 36, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer# (2 chars.)
        if (fread(szString, sizeof(char), 2, lHandleInput) != 2) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 2, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Cassette# (1 char)
        if (fread(szString, sizeof(char), 1, lHandleInput) != 1) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 1, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Slot# (2 chars)
        if (fread(szString, sizeof(char), 2, lHandleInput) != 2) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 2, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Test count (1 char)
        if (fread(szString, sizeof(char), 1, lHandleInput) != 1) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 1, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Map Coordinate System+BIN type + distance info + address info
        if (fread(szString, sizeof(char), 58, lHandleInput) != 58) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 58, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Number of records (2 bytes).
        if (fscanf(lHandleInput, "%c%c", &uLsb, &uMsb) != 2) {
            return false;
        }
        uLsb &= 0xff;
        uMsb &= 0xff;
        iRecords = ((int)uMsb << 8) + (int)uLsb;
        if(lHandleOutput)
            fprintf(lHandleOutput,"%c%c",uLsb,uMsb);
    }
    else
    {
        // Lot-ID (25 characters)
        if (fread(szString, sizeof(char), 25, lHandleInput) != 25) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 25, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer# (2 chars.)
        if (fread(szString, sizeof(char), 2, lHandleInput) != 2) {
            return false;
        }
        szString[2] = 0;
        strString = szString;
        strString.toLong(&bFlag);
        if(strString.isEmpty() == false && bFlag == false)
            goto invalid_P8;	// Error: expecting a number
        if(lHandleOutput)
            if (fwrite(szString, 2, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Cassette# (1 char)
        if (fread(szString, sizeof(char), 1, lHandleInput) != 1) {
            return false;
        }
        szString[1] = 0;
        strString = szString;
        strString.toLong(&bFlag);
        if(strString.isEmpty() == false && bFlag == false)
            goto invalid_P8;	// Error: expecting a number
        if(lHandleOutput)
            if (fwrite(szString, 1, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Slot# (2 chars)
        if (fread(szString, sizeof(char), 2, lHandleInput) != 2) {
            return false;
        }
        szString[2] = 0;
        strString = szString;
        strString.toLong(&bFlag);
        if(strString.isEmpty() == false && bFlag == false)
            goto invalid_P8;	// Error: expecting a number
        if(lHandleOutput)
            if (fwrite(szString, 2, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Test count (1 char)
        if (fread(szString, sizeof(char), 1, lHandleInput) != 1) {
            return false;
        }
        szString[2] = 0;
        strString = szString;
        strString.toLong(&bFlag);
        if(strString.isEmpty() == false && bFlag == false)
            goto invalid_P8;	// Error: expecting a number
        if(lHandleOutput)
            if (fwrite(szString, 1, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Total pass (2 bytes): LSB, MSB
        int	iCount;
        if (fscanf(lHandleInput, "%c%c", &uLsb, &uMsb) != 2) {
            return false;
        }
        uLsb &= 0xff;
        uMsb &= 0xff;
        iCount = ((int)uMsb << 8) + (int)uLsb;
        // Remove PAT failures
        iCount -= mPrivate->mContext->m_lstOutlierParts.count();
        if(iCount < 0) iCount = 0;	// Check underflow in case original 'total pass' wasn't accurate
        // Compute new MSB & LSB
        uLsb = iCount & 0xff;
        uMsb = iCount >> 8;
        if(lHandleOutput)
            fprintf(lHandleOutput,"%c%c",uLsb,uMsb);

        // Total fail (2 bytes): LSB, MSB
        if (fscanf(lHandleInput, "%c%c", &uLsb, &uMsb) != 2) {
            return false;
        }
        uLsb &= 0xff;
        uMsb &= 0xff;
        iCount = ((int)uMsb << 8) + (int)uLsb;
        // Add PAT failures
        iCount += mPrivate->mContext->m_lstOutlierParts.count();
        // Compute new MSB & LSB
        uLsb = iCount & 0xff;
        uMsb = iCount >> 8;
        if(lHandleOutput)
            fprintf(lHandleOutput,"%c%c",uLsb,uMsb);

        // Total tests (2 bytes): LSB, MSB
        if (fread(szString, sizeof(char), 2, lHandleInput) != 2) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 2, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer starting time (BCD 12 bytes)
        // eg: 0 5 1 2 1 2  0 9 0 4 0 2
        //     Y Y M M D D  H H M M S S
        if (fread(szString, sizeof(char), 12, lHandleInput) != 12) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 12, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Wafer ending time (BCD 12 bytes)
        if (fread(szString, sizeof(char), 12, lHandleInput) != 12) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 12, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }

        // Number of records (1 byte). May suffer overflow if more than 255 records are listed!
        if (fread(szString, sizeof(char), 1, lHandleInput) != 1) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 1, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }
        iRecords = szString[0];

        // X,Y Distance from origin (4 bytes)
        if (fread(szString, sizeof(char), 4, lHandleInput) != 4) {
            return false;
        }
        if(lHandleOutput)
            if (fwrite(szString, 4, sizeof(char), lHandleOutput) != sizeof(char)) {
                return false;
            }
    }

    // Reset buffer handling input optional map mismatch details...
    mPrivate->mSINFInfo.Clear();

    // Loop until end of file (so we don't care if the number of records specified is accurate!
    while(1)
    {
        // First address of record (X,Y)
        if(fscanf(lHandleInput,"%c%c",&uLsb,&uMsb) != 2)
            break;	// reached end of file.
        if(lHandleOutput)
            fprintf(lHandleOutput,"%c%c",uLsb,uMsb);
        uLsb &= 0xff;
        uMsb &= 0xff;
        iDieX = ((int)uMsb << 8) + (int)uLsb;
        if(fscanf(lHandleInput,"%c%c",&uLsb,&uMsb) != 2)
            break;	// reached end of file.
        if(lHandleOutput)
            fprintf(lHandleOutput,"%c%c",uLsb,uMsb);
        uLsb &= 0xff;
        uMsb &= 0xff;
        iDieY = ((int)uMsb << 8) + (int)uLsb;

        // Number of dies in record: 1 byte value (old P8), or 2 bytes (million-die new P8 format)
        if(bNewP8_Format)
        {
            if (fscanf(lHandleInput, "%c%c", &uLsb, &uMsb) != 2) {
                break;
            }
            uLsb &= 0xff;
            uMsb &= 0xff;
            iDiesInRow = ((int)uMsb << 8) + (int)uLsb;
            if(lHandleOutput)
                fprintf(lHandleOutput,"%c%c",uLsb,uMsb);
        }
        else
        {
            if(fscanf(lHandleInput,"%c",&uValue) != 1)
                break;	// reached end of file.
            if(lHandleOutput)
                fprintf(lHandleOutput,"%c",uValue);
            iDiesInRow = uValue & 0xff;

            // Check if die count in row is abnormaly low...
            if(iDiesInPreviousRow > 50  && (iDiesInRow < iDiesInPreviousRow/3))
                iDiesInRow += 0x100;	// Fix overflow error in original data file (because only one byte to hold #)
        }

        // Keep track of total dies in previous row.
        iDiesInPreviousRow = iDiesInRow;

        while(iDiesInRow)
        {
            // Get die info
            if(bNewP8_Format)
            {
                unsigned char uChar0,uChar1,uChar2,uChar3;	// 32bits for Bin# (only one bit set for a bin# ranging in 0-31)
                if (fscanf(lHandleInput, "%c%c%c%c%c",
                           &uChar0, &uChar1, &uChar2, &uChar3, &uLsb) != 5) {
                    break;
                }
                // 1->Bin0, 2->Bin1, 4->Bin2, 8->Bin3, ...N->Bin(2^N)-1
                lP8Bin = lP8_EncodedBin = ((unsigned long) uChar0) + (((unsigned long) uChar1) << 8) + (((unsigned long) uChar2) << 16) + (((unsigned long) uChar3) << 24);

                // Convert encoded bin to a bin#:
                if(lP8_EncodedBin)
                    lP8Bin = (unsigned long)((double)log((double)lP8_EncodedBin)/log(2.0));
            }
            else
            {
                if (fscanf(lHandleInput, "%c%c", &uValue, &uLsb) != 2) {
                    break;
                }
                uLsb &= 0xff;
                lP8_EncodedBin = uValue & 0xff;

                // Decode encoded binning.
                if(lP8_EncodedBin >= '1' && lP8_EncodedBin <= '9')
                    lP8Bin = lP8_EncodedBin - '1';	// Bin mapped to ascii equivalent: '1'->Bin0,...'9'->Bin8
                else
                    if(lP8_EncodedBin >= 'A' && lP8_EncodedBin <= 'U')
                        lP8Bin = lP8_EncodedBin - 'A' + 9;	// Bin mapped to ascii character 'A' -> Bin9, 'B' -> Bin10 , etc...'U' -> Bin29
                    else
                        if(lP8_EncodedBin >= 'W' && lP8_EncodedBin <= 'X')
                            lP8Bin = lP8_EncodedBin - 'W' + 30;	// Bin mapped to ascii character 'W'->Bin30, 'X' -> Bin31
                        else
                            lP8Bin = 31;	// Any higher binning sticks to highest valid P8 binning.
            }

            // Get STDF die value
            WaferCoordinate wCoord(iDieX, iDieY);

            wCoord = mPrivate->mContext->GetExternalTransformation().Map(wCoord);

            if (mPrivate->mContext->m_Stdf_SbinMap.indexFromCoord(lIdx, wCoord.GetX(), wCoord.GetY()))
                iStdfBin = mPrivate->mContext->m_Stdf_SbinMap.getWafMap()[lIdx].getBin();
            else
                iStdfBin = GEX_WAFMAP_EMPTY_CELL;

            // Check if valid die (PAss or Fail tested die).
            if((uLsb & 0xB0) == 0)
            {
                mPrivate->mExternalMapDetails.mTotalDies++;	// Keep track of total dies in P8 file.
                // Keep track of matching dies in P8 and STDF file.
                if(iStdfBin != GEX_WAFMAP_EMPTY_CELL)
                    mPrivate->mExternalMapDetails.mTotalMatchingMapDiesLoc++;
            }

            // Get PAT binning in case
            mPrivate->mContext->GetPatFailureDeviceDetails(wCoord.GetX(), wCoord.GetY(), cDevice);

            iPatBin = cDevice.iPatHBin;

            // Leave all dies as-is if not 'PASS + Test' flags set.
            if((uLsb & 0xB1) == 0)
            {
                // Valid PASS + TEST (no inking, no Skip): check if this die failed the PAT algorithms?

                // Overload binning with PAT binning....usin below decision matrix:
                //  STDF   P8    Result (output P8)
                // ----- -----  ----------------------
                //   1     1      Outlier PAT rule: 1 or PAT bin (if failure detected)
                //   n/a   1      DPAT fail
                //  *F*    1      DPAT Fail
                //   1    *F*     Fail to P8 bin
                //   n/a  *F*     Fail to P8 bin
                //  *F*   *F*     Fail to P8 bin

                // P8 Pass bin.
                if(lP8Bin == 1)
                {
                    // number of good die from the input wafer map that become bad die (dpat failures) in the output wafer map.
                    if (iPatBin >= 0)
                        mPrivate->mExternalMapDetails.mGoodPartsMapDPATFailures++;

                    // keep track of binning mismatch
                    if(mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin) == false)
                    {
                        if (iStdfBin == GEX_WAFMAP_EMPTY_CELL)
                            // number of good die from the input wafer map that do not have matching STDF data.
                            mPrivate->mExternalMapDetails.mGoodPartsMapSTDFMissing++;
                        else
                            // P8=Pass, STDF = Fail
                            mPrivate->mExternalMapDetails.mTotalMismatchingMapPassBin++;

                        if(cDevice.iPatRules & (GEX_TPAT_BINTYPE_NNR | GEX_TPAT_BINTYPE_IDDQ_DELTA | GEX_TPAT_BINTYPE_BADNEIGHBORS | GEX_TPAT_BINTYPE_RETICLE | GEX_TPAT_BINTYPE_BADCLUSTER))
                            lP8Bin = iPatBin;	// Good die rejected from rule: bad neigbhorhood/Reticle/Clustering
                        else
                        {
                            lP8Bin = mPrivate->mContext->GetRecipeOptions().iFailDynamic_SBin;  // STDF bin is FAIL, then force a DPAT bin...unless overload bins defined.

                            // If P8=1, STDF=F (but not PAT failure) and overload bin defined, use it!
                            if((iPatBin < 0) && (iStdfBin != GEX_WAFMAP_EMPTY_CELL) &&
                               mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                                lP8Bin = mPrivate->mSettings.iGoodMapOverload_FailStdf;
                            else
                                // If P8=1, STDF=Missing and overload bin defined, use it!
                                if((iPatBin < 0) && (iStdfBin == GEX_WAFMAP_EMPTY_CELL) &&
                                   mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                                    lP8Bin = mPrivate->mSettings.iGoodMapOverload_MissingStdf;
                        }
                    }
                    else
                    {
                        // P8 = STDF = Bin1, Then overload binning with PAT bin (if any)
                        if(iPatBin >= 0)
                            lP8Bin = iPatBin;	// DPAT failure.
                    }

                    // If P8 bin has been overloaded, set FAIL flag
                    if(lP8Bin != 1)
                        uLsb |= 0x1;	// Set 'FAIL' flag
                }

                // Map this bin# to a valid P8 Bin#
                if(bNewP8_Format)
                {
                    // Million die P8 format
                    if(lP8Bin > 31) lP8Bin = 31;
                    lP8_EncodedBin = 1 << lP8Bin;	// Set bit matching bin#.
                }
                else
                {
                    // Old P8 Format
                    if(lP8Bin <= 8)
                        lP8_EncodedBin = lP8Bin + '1';	// Bin mapped to ascii equivalent: Bin0 -> '1',...Bin8->'9'
                    else
                        if(lP8Bin >= 9 && lP8Bin <= 29)
                            lP8_EncodedBin = lP8Bin + 'A' - 9;	// Bin mapped to ascii character Bin9 -> 'A', Bin10 -> 'B', etc...Bin29 -> 'U'
                        else
                            if(lP8Bin >= 30 && lP8Bin <= 31)
                                lP8_EncodedBin = lP8Bin + 'W' - 30;	// Bin mapped to ascii character Bin30 -> 'W', Bin31 -> 'X'
                            else
                                lP8_EncodedBin = 'X';	// Any higher binning sticks to highest valid P8 binning.
                }
            }
            else if((uLsb & 0xB1) == 1)
            {
                // Valid FAIL + TEST (no inking, no Skip).
                // Check P8 vs STDF mismatch
                if(iStdfBin != GEX_WAFMAP_EMPTY_CELL && mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(iStdfBin))
                    mPrivate->mExternalMapDetails.mTotalMismatchingMapFailBin++; // P8=Fail, STDF = Pass
            }

            // See if overload Flags and/or Die binning
            if(lHandleOutput)
            {
                if(bNewP8_Format)
                {

                    fprintf(lHandleOutput, "%c%c%c%c%c",
                            (unsigned char)( lP8_EncodedBin        & 0xff),
                            (unsigned char)((lP8_EncodedBin >>  8) & 0xff),
                            (unsigned char)((lP8_EncodedBin >> 16) & 0xff),
                            (unsigned char)((lP8_EncodedBin >> 24) & 0xff),
                            uLsb);
                }
                else
                {
                    uMsb = lP8_EncodedBin & 0xff;
                    fprintf(lHandleOutput,"%c%c",uMsb,uLsb);
                }
            }

            // Keep track of Die position (X,Y)
            iDieX++;

            // Keep track of dies left in this record
            iDiesInRow--;
        };

        // Keep track of records left to process
        iRecords--;
    };

    mPrivate->mExternalMapDetails.mComputed = true;

    // Close files.
    fclose(lHandleInput);
    if(lHandleOutput)
        fclose(lHandleOutput);

    // TEL-P8 file updated.
    return true;

    // Invalid/ Not a TEL-P8 file
invalid_P8:

    mPrivate->mErrorMessage = "\t> *Error* Unknown/Corrupted input map file.\n";
    fclose(lHandleInput);

    return false;
}

bool PATProcessWS::UpdateSkyNPExternalMap(const QString &inputExternalMap, const QString &outputExternalMap)
{
    // If not output file name defined, don't do anything.
    if (outputExternalMap.isEmpty())
        return true;

    if (mPrivate->mContext == NULL)
    {
        mPrivate->mErrorMessage = "Failed to retrieve any PAT processing information.";
        return false;
    }

    // Open Sky NP file
    QFile lInputFile(inputExternalMap);
    if(!lInputFile.open(QIODevice::ReadOnly))
    {
        // Failed Opening Sky NP file
        mPrivate->mErrorMessage = "Failed to open input file.\nProtection issue? : " +
                                  inputExternalMap;
        return false;
    }

    // Assign file I/O stream
    QTextStream lInputStream(&lInputFile);

    QString     lLine   = lInputStream.readLine();
    QStringList lCells  = lLine.split(",",QString::KeepEmptyParts);

    if (lCells.count() < 17 ||
        lCells[0].contains("#Test ID", Qt::CaseInsensitive) == false ||
        lCells[1].contains("Test Level", Qt::CaseInsensitive) == false ||
        lCells[2].contains("Mask", Qt::CaseInsensitive) == false ||
        lCells[3].contains("Lot", Qt::CaseInsensitive) == false ||
        lCells[4].contains("Operator", Qt::CaseInsensitive) == false)
    {
        // Unexpected header format
        mPrivate->mErrorMessage = "*Error* The file is not e Sky NP csv file (header doesn't match):" + inputExternalMap;
        return false;
    }

    int lSBinPos        = lCells.indexOf(QRegExp("(SW_BIN)", Qt::CaseInsensitive));
    int lHBinPos        = lCells.indexOf(QRegExp("(BIN)", Qt::CaseInsensitive));
    int lFailedParamPos = lCells.indexOf(QRegExp("(Failed_Parameters)", Qt::CaseInsensitive));
    int lProberXPos     = lCells.indexOf(QRegExp("(Prober_X)", Qt::CaseInsensitive));
    int lProberYPos     = lCells.indexOf(QRegExp("(Prober_Y;|Prober_Y)", Qt::CaseInsensitive));

    if (lSBinPos == -1)
    {
        mPrivate->mErrorMessage = QString("Soft bin column [SW_BIN] not found");
        return false;
    }
    else if (lHBinPos == -1)
    {
        mPrivate->mErrorMessage = QString("Hard bin column [BIN] not found");
        return false;
    }
    else if (lFailedParamPos == -1)
    {
        mPrivate->mErrorMessage = QString("Failed parameters column [Failed_Parameters] not found");
        return false;
    }
    else if (lProberXPos == -1)
    {
        mPrivate->mErrorMessage = QString("Prober X column [Prober_X] not found");
        return false;
    }
    else if (lProberYPos == -1)
    {
        mPrivate->mErrorMessage = QString("Prober Y column [Prober_Y or Prober_Y;] not found");
        return false;
    }

    // Write updated wafermap to output file
    // 1: Create new output Sky NP file
    QFile lOutputFile(outputExternalMap);
    if(!lOutputFile.open(QIODevice::WriteOnly))
    {
        // Failed Opening new Sky NP file
        mPrivate->mErrorMessage = "*Error* Failed to create updated Sky NP.\nProtection issue?";
        return false;
    }

    // Assign file I/O stream
    QTextStream lOutputStream(&lOutputFile);
    int         lLineCount  = 0;
    int         lDieX       = -1;
    int         lDieY       = -1;
    int         lHBin       = -1;
    int         lSBin       = -1;
    bool        lOk         = false;
    int         lRequiredFields = 0;
    int         lSTDFBin        = GEX_WAFMAP_EMPTY_CELL;
    int         lIdx            = -1;
    QString     lFailedParameters;
    CBinning *  lExternalBinInfo    = NULL;
    CBinning *  lSTDFBinInfo        = NULL;

    // Compute the number of fields each line should have.
    lRequiredFields = qMax(lSBin, lHBin);
    lRequiredFields = qMax(lRequiredFields, lFailedParamPos);
    lRequiredFields = qMax(lRequiredFields, lProberXPos);
    lRequiredFields = qMax(lRequiredFields, lProberYPos);

    // Write the header line
    lOutputStream << lLine << endl;

    while (lInputStream.atEnd() == false)
    {
        lLine   = lInputStream.readLine();
        ++lLineCount;

        if (lLine.isEmpty() == false || lLine.simplified().split(",", QString::SkipEmptyParts).size() != 0)
        {
            lCells  = lLine.split(",", QString::KeepEmptyParts);

            // If a line corresponds to a Wafer Part, then analyze it. Otherwise, keep it as read.
            if (lCells.at(1).simplified().compare("wafer", Qt::CaseInsensitive) == 0)
            {
                if (lCells.size() < lRequiredFields)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: too few fields (%1) on line %2")
                                              .arg(lCells.size()).arg(lLineCount);

                    return false;
                }

                lDieX = lCells.at(lProberXPos).toInt(&lOk);

                if (lOk == false)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Prober X value (%1) is not a number at line %2")
                                              .arg(lCells.at(lProberXPos)).arg(lLineCount);
                    return false;
                }

                lDieY = lCells.at(lProberYPos).section(";", 0, 0).toInt(&lOk);
                if (lOk == false)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Prober Y value (%1) is not a number at line %2")
                                              .arg(lCells.at(lProberYPos)).arg(lLineCount);
                    return false;
                }

                lHBin = lCells.at(lHBinPos).toInt(&lOk);
                if (lOk == false)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Hard bin  value (%1) is not a number at line %2")
                                              .arg(lCells.at(lHBinPos)).arg(lLineCount);
                    return false;
                }

                lSBin = lCells.at(lSBinPos).toInt(&lOk);
                if (lOk == false)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Soft bin  value (%1) is not a number at line %2")
                                              .arg(lCells.at(lSBinPos)).arg(lLineCount);
                    return false;
                }

                // Get STDF die value
                WaferCoordinate wCoord(lDieX, lDieY);

                // Get the transformed coordinates in case of map alignment
                wCoord = mPrivate->mContext->GetExternalTransformation().Map(wCoord);

                // Check if valid STDF die location
                if(mPrivate->mContext->m_Stdf_HbinMap.indexFromCoord(lIdx, wCoord.GetX(), wCoord.GetY()) == false)
                    lSTDFBin = GEX_WAFMAP_EMPTY_CELL;
                else
                    // Die within STDF wafermap area (but maybe die not tested!), get PAT-Man value!...offset X & Y to start on wafer corner
                    lSTDFBin = mPrivate->mContext->m_Stdf_HbinMap.getWafMap()[lIdx].getBin();

                // Get the External bin info class
                lExternalBinInfo = mPrivate->mContext->GetExternalBins()->Find(lHBin);

                // If not found, return an error
                if (lExternalBinInfo == NULL)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Unable to retrieve bin value (%1) from external map bin list")
                                              .arg(lHBin);
                    return false;
                }

                // Get the STDF bin info class
                lSTDFBinInfo = mPrivate->mContext->GetSTDFHardBins()->Find(lSTDFBin);

                if (lSTDFBin != GEX_WAFMAP_EMPTY_CELL && lSTDFBinInfo == NULL)
                {
                    mPrivate->mErrorMessage = QString("Failed to update Sky NP map: Unable to retrieve Soft bin value (%1) from stdf map soft bin list")
                                              .arg(lSTDFBin);
                    return false;
                }

                // Keep some informations about external map compared to stdf map
                // Keep track of total valid dies in KLA/MAP file
                mPrivate->mExternalMapDetails.mTotalDies++;

                if (lSTDFBin != GEX_WAFMAP_EMPTY_CELL)
                    ++mPrivate->mExternalMapDetails.mTotalMatchingMapDiesLoc;

                // Good die in external map, fail die in stdf
                if (lExternalBinInfo->cPassFail == 'P')
                {
                    if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL)
                        ++mPrivate->mExternalMapDetails.mGoodPartsMapSTDFMissing;
                    else if (lSTDFBinInfo && lSTDFBinInfo->cPassFail == 'F')
                        ++mPrivate->mExternalMapDetails.mTotalMismatchingMapPassBin;
                }

                // Fail die in external map, good die in stdf
                if (lExternalBinInfo->cPassFail == 'F' && lSTDFBinInfo && lSTDFBinInfo->cPassFail == 'P')
                    ++mPrivate->mExternalMapDetails.mTotalMismatchingMapPassBin;

                // Get pat failure details if any
                if (mPrivate->mContext->isDieOutlier(lDieX, lDieY, lSBin))
                {
                    // Good die in external map, PAT outlier
                    if (lExternalBinInfo->cPassFail == 'P')
                        ++mPrivate->mExternalMapDetails.mGoodPartsMapDPATFailures;

                    lFailedParameters = "PAT Failure: ";

                    if (mPrivate->mContext->isDieFailure_PPAT(lDieX, lDieY, lSBin, lHBin) >= 0)
                    {
                        CPatOutlierPart * lOutlierPart = mPrivate->mContext->FindPPATOutlierPart(lDieX, lDieY);

                        if (lOutlierPart)
                        {
                            if (lOutlierPart->cOutlierList.count())
                            {
                                lFailedParameters += QString::number(lOutlierPart->cOutlierList.at(0).mTestNumber);

                                if (lOutlierPart->cOutlierList.at(0).mPinIndex >= 0)
                                    lFailedParameters += "." + QString::number(lOutlierPart->cOutlierList.at(0).mPinIndex);

                                lFailedParameters += " - ";
                                lFailedParameters += lOutlierPart->cOutlierList.at(0).mTestName;
                            }
                            else
                                lFailedParameters += "Unknown parameter";
                        }
                        else
                        {
                            mPrivate->mErrorMessage = QString("No PPAT outlier information retrieved for part at coordinates (%1,%2).")
                                                      .arg(lDieX).arg(lDieY);
                            return false;
                        }
                    }
                    else if (mPrivate->mContext->isDieFailure_GPAT(lDieX, lDieY, lSBin, lHBin) >= 0)
                    {
                        lFailedParameters += "GPAT";
                    }
                    else if (mPrivate->mContext->isDieFailure_MVPAT(lDieX, lDieY, lSBin, lHBin) >= 0)
                    {
                        lFailedParameters += "MVPAT";
                    }

                    // Update the bin fields and the failed parameters field
                    lCells[lSBinPos] = QString::number(lSBin);
                    lCells[lHBinPos] = QString::number(lHBin);
                    lCells[lFailedParamPos] = lFailedParameters;
                }
                else if (mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0 || mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                {
                    // If external die is pass, check some gtf keys.
                    if (lExternalBinInfo->cPassFail == 'P')
                    {
                        // Untested STDF die
                        if (lSTDFBin == GEX_WAFMAP_EMPTY_CELL)
                        {
                            if (mPrivate->mSettings.iGoodMapOverload_MissingStdf >= 0)
                            {
                                lCells[lSBinPos] = QString::number(mPrivate->mSettings.iGoodMapOverload_MissingStdf);
                                lCells[lHBinPos] = QString::number(mPrivate->mSettings.iGoodMapOverload_MissingStdf);
                            }
                        }
                        else if (mPrivate->mSettings.iGoodMapOverload_FailStdf >= 0)
                        {
                            // Check if STDF bin doesn't belong to 'good bins' list
                            if(mPrivate->mContext->GetRecipeOptions().pGoodSoftBinsList->Contains(lSTDFBin) == false)
                            {
                                lCells[lSBinPos] = QString::number(mPrivate->mSettings.iGoodMapOverload_FailStdf);
                                lCells[lHBinPos] = QString::number(mPrivate->mSettings.iGoodMapOverload_FailStdf);
                            }
                        }
                    }
                }

                lOutputStream << lCells.join(",") << endl;
            }
            else
                lOutputStream << lLine << endl;
        }
        else
            lOutputStream << lLine << endl;
    }

    // Close the file
    lOutputFile.close();
    lInputFile.close();

    return true;
}

}   // namespace Gex
}   // namespace GS
#endif
