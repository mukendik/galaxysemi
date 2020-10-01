#include "pat_reticle_engine.h"
#include "pat_reticle_abstract_algorithm.h"
#include "pat_reticle_repeating_pattern.h"
#include "pat_reticle_defectivity_check.h"
#include "pat_reticle_map_legacy.h"
#include "pat_reticle_map_extern.h"
#include "wafermap.h"
#include "pat_info.h"
#include "pat_engine.h"
#include "gex_report.h"
#include "gqtl_log.h"

extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class

namespace GS
{
namespace Gex
{

PatReticleEngine::PatReticleEngine()
    : mWafermap(NULL), mGoodBinList(NULL), mContext(NULL), mWaferSize(0)
{

}

PatReticleEngine::~PatReticleEngine()
{

}

bool PatReticleEngine::processWafer(const CWaferMap *wafermap, const QtLib::Range *goodBinList,
                                    const PATOptionReticle &reticleSettings,
                                    QHash<QString, CPatDieCoordinates> &patOutliers)
{
    // Initialize Reticle Engine
    if (init(wafermap, goodBinList, reticleSettings))
    {
        // Create a reticle map object to iterate over reticles
        QScopedPointer<PatReticleMapAbstract> lReticleMap(createReticleMap());

        if (lReticleMap.isNull())
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to instantiate a Reticle map");
            return false;
        }

        // Check if Mask exists (if so, load it)
        if(reticleSettings.GetReticleMaskName().isEmpty() == false)
        {
            CMask_Rule * lMask = mContext->GetMaskDefinition(reticleSettings.GetReticleMaskName());
            if(lMask)
            {
                lReticleMap->SetMaskWidth(lMask->mRadius);

                switch(lMask->mWorkingArea)
                {
                    case 0:	// Outer ring
                        lReticleMap->SetMaskType(CWaferMap::RingFromEdge);
                        break;
                    case 1:	// Inner ring
                        lReticleMap->SetMaskType(CWaferMap::RingFromCenter);
                        break;
                }
            }
        }

        // Create Reticle algorithm depending on reticle settings
        QScopedPointer<PatReticleAbstractAlgorithm> lAlgo(PatReticleAbstractAlgorithm::Create(reticleSettings));

        if (lAlgo.isNull())
        {
            GSLOG(SYSLOG_SEV_ERROR, "Failed to instantiate a Reticle Algorithm");
            return false;
        }

        // Execute Reticle algorithm
        if (lAlgo->ProcessReticleMap(lReticleMap.data()) == false)
            return false;

        // Update outliers list
        if (inkOut(lAlgo->GetOutliers(), patOutliers) == false)
            return false;

        // Collect information for reporting
         mContext->SetReticleResults(reticleSettings.GetRuleName(), lAlgo->GetReticleResults());

        return true;
    }

    return false;
}

PatReticleMapAbstract *PatReticleEngine::createReticleMap()
{
    PatReticleMapAbstract * lReticleMap = NULL;

    switch(mContext->GetRecipeOptions().GetReticleSizeSource())
    {
        case PATOptionReticle::RETICLE_SIZE_FILE:
            lReticleMap = new PatReticleMapExtern(mWafermap, mContext->GetReticleStepInformation());
            break;

        case PATOptionReticle::RETICLE_SIZE_FIXED:
            lReticleMap = new PatReticleMapLegacy(mWafermap,
                                                  mContext->GetRecipeOptions().GetReticleSizeX(),
                                                  mContext->GetRecipeOptions().GetReticleSizeY());
            break;

        default:
            GSLOG(SYSLOG_SEV_WARNING, "Reticle source undefined in PAT recipe");
            break;
    }

    return lReticleMap;
}

bool PatReticleEngine::init(const CWaferMap *wafermap, const QtLib::Range *goodBinList,
                            const PATOptionReticle& reticleSettings)
{
    mContext = GS::Gex::PATEngine::GetInstance().GetContext();

    if (wafermap && goodBinList && mContext)
    {
        mWafermap           = wafermap;
        mGoodBinList        = goodBinList;
        mReticleSettings    = reticleSettings;
        mWaferSize          = mWafermap->GetSizeX() * mWafermap->GetSizeY();

        return true;
    }

    return false;
}

bool PatReticleEngine::inkOut(const QList<WaferCoordinate>& outlierDie, QHash<QString, CPatDieCoordinates> &patOutliers)
{
    int                 lBin        = GEX_WAFMAP_EMPTY_CELL;
    int                 lArrayIndex = -1;
    WaferCoordinate     lDie;
    CPatDieCoordinates  lOutlierDie;
    QString             lKey;

    for (int lIdx = 0; lIdx < outlierDie.count(); ++lIdx)
    {
        lDie = outlierDie.at(lIdx);

        // Convert the X,Y coordinate to an index in the array
        if (mWafermap->indexFromCoord(lArrayIndex, lDie.GetX(), lDie.GetY()))
        {
            lBin = mWafermap->getWafMapDie(lArrayIndex).getBin();

            // Only good die must be rebinned to PAT Outlier
            if (lBin != GEX_WAFMAP_EMPTY_CELL && mGoodBinList->Contains(lBin))
            {
                // If die has been already set as an outlier, do nothing
                if (mContext->isDieOutlier(lDie.GetX(), lDie.GetY(), lBin) == false)
                {
                    // This good bin passes the conditions to be failed
                    lKey = QString::number(lDie.GetX()) + "." + QString::number(lDie.GetY());

                    lOutlierDie.mDieX          = lDie.GetX();
                    lOutlierDie.mDieY          = lDie.GetY();
                    lOutlierDie.mSite          = gexReport->getDieTestingSite(-1, 0, lDie.GetX(), lDie.GetY());
                    lOutlierDie.mFailType      = GEX_TPAT_BINTYPE_RETICLE;
                    lOutlierDie.mRuleName      = mReticleSettings.GetRuleName();
                    lOutlierDie.mPatHBin       = mReticleSettings.GetReticleHBin();
                    lOutlierDie.mPatSBin       = mReticleSettings.GetReticleSBin();
                    lOutlierDie.mOrigHBin      = mContext->GetOriginalBin(false, lDie.GetX(), lDie.GetY());
                    lOutlierDie.mOrigSBin      = mContext->GetOriginalBin(true, lDie.GetX(), lDie.GetY());
                    lOutlierDie.mPartId        = gexReport->getDiePartId(-1, lDie.GetX(), lDie.GetY());

                    // Save PAT definition in our list
                    patOutliers.insert(lKey, lOutlierDie);
                }
            }
        }
        else
            return false;
    }

    return true;
}

}
}
