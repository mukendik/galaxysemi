#include "pat_gdbn_engine.h"
#include "pat_gdbn_abstract_algorithm.h"
#include "pat_gdbn_weighting_algo.h"
#include "pat_gdbn_squeeze_algo.h"
#include "pat_gdbn_generic_baddies.h"
#include "pat_rules.h"
#include "pat_engine.h"
#include "gex_report.h"

extern CGexReport *		gexReport;				// report_build.cpp: Handle to report class

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnEngine - Class to evaluate if a die is a GDBN
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnEngine::PatGdbnEngine() : m_pGdbnAlgorithm(NULL), m_pWafermap(NULL), m_pGoodBinList(NULL), m_nWaferSize(0)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnEngine::~PatGdbnEngine()
{
    if (m_pGdbnAlgorithm)
    {
        delete m_pGdbnAlgorithm;
        m_pGdbnAlgorithm = NULL;
    }
}

///////////////////////////////////////////////////////////
// Propeties
///////////////////////////////////////////////////////////

void PatGdbnEngine::setAlgorithm(PatGdbnAbstractAlgorithm * pAlgorithm)
{
    if (m_pGdbnAlgorithm)
    {
        delete m_pGdbnAlgorithm;
        m_pGdbnAlgorithm = NULL;
    }

    m_pGdbnAlgorithm = pAlgorithm;
}

void PatGdbnEngine::setAlgorithm(const CGDBN_Rule& gdbnRule)
{
    PatGdbnAbstractAlgorithm * pAlgorithm = NULL;

    switch(gdbnRule.mAlgorithm)
    {
        case GEX_TPAT_GDBN_ALGO_WEIGHT  :   {
                                                PatGdbnWeightingAlgo * pWAlgo = new PatGdbnWeightingAlgo();
                                                pWAlgo->setEdgeDieWeighting(gdbnRule.mEdgeDieWeighting);
                                                pWAlgo->setEdgeDieWeightingScale(gdbnRule.mEdgeDieWeightingScale);
                                                pWAlgo->setMinimumWeighting(gdbnRule.mMinimumWeighting);
                                                pWAlgo->setAdjacentWeighting(gdbnRule.mAdjWeightLst);
                                                pWAlgo->setDiagonalWeighting(gdbnRule.mDiagWeightLst);

                                                pAlgorithm = pWAlgo;
                                            }
                                            break;

        case GEX_TPAT_GDBN_ALGO_SQUEEZE :   {
                                                PatGdbnSqueezeAlgo * pSAlgo = new PatGdbnSqueezeAlgo();
                                                pSAlgo->setClusterSize(gdbnRule.mClusterSize);
                                                pSAlgo->setFailEdgeDies(gdbnRule.mFailWaferEdges);
                                                pSAlgo->setMinimumFailCount(gdbnRule.mFailCount);

                                                pAlgorithm = pSAlgo;
                                            }
                                            break;

        default                         :   break;
    }

    if (pAlgorithm)
    {
        pAlgorithm->SetBadDies(new PatGdbnGenericBadDies(gdbnRule.mBadBinList));
        pAlgorithm->SetEdgeDieType(gdbnRule.mEdgeDieType);
        pAlgorithm->SetRuleName(gdbnRule.mRuleName);
        pAlgorithm->SetSoftBin(gdbnRule.mSoftBin);
        pAlgorithm->SetHardBin(gdbnRule.mHardBin);

        // Check if Mask exists (if so, load it)
        if(gdbnRule.mMaskName.isEmpty() == false)
        {
            CMask_Rule * ptMask = GS::Gex::PATEngine::GetInstance().GetContext()->GetMaskDefinition(gdbnRule.mMaskName);
            if(ptMask)
            {
                pAlgorithm->SetRingRadius(ptMask->mRadius);

                switch(ptMask->mWorkingArea)
                {
                    case 0  :	// Outer ring
                    default :   pAlgorithm->SetRingArea(CWaferMap::RingFromEdge);
                                break;

                    case 1  :	// Inner ring
                                pAlgorithm->SetRingArea(CWaferMap::RingFromCenter);
                                break;
                }
            }
        }

    }

    setAlgorithm(pAlgorithm);
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

bool PatGdbnEngine::init(const CWaferMap * pWafermap,
                         const GS::QtLib::Range * pGoodBinList)
{
    if (m_pGdbnAlgorithm && pWafermap && pGoodBinList)
    {
        m_pWafermap         = pWafermap;
        m_pGoodBinList      = pGoodBinList;
        m_nWaferSize        = m_pWafermap->SizeX * m_pWafermap->SizeY;

        return true;
    }

    return false;
}

bool PatGdbnEngine::excludeDie(int dieIndex) const
{
    if (m_pGdbnAlgorithm && m_pWafermap)
        return m_pGdbnAlgorithm->excludeDie(m_pWafermap, dieIndex);
    else
        return false;
}

bool PatGdbnEngine::processWafer(const CWaferMap * pWafermap,
                                 const GS::QtLib::Range * pGoodBinList,
                                 QHash<QString, CPatDieCoordinates> &lGDBNOutliers)
{
    if (init(pWafermap, pGoodBinList))
    {
        CPatDieCoordinates  lGPATOutlier;
        QString             lKey;
        CPatInfo *          lPatInfo = GS::Gex::PATEngine::GetInstance().GetContext();

        if (lPatInfo == NULL)
            return false;

        for(int nIndex=0; nIndex < m_nWaferSize; nIndex++)
        {
            if (((CWaferMap *)m_pWafermap)->getWafMap()[nIndex].getBin() >= 0 &&
                m_pGoodBinList->Contains(((CWaferMap *)m_pWafermap)->getWafMap()[nIndex].getBin()))
            {
                int nCoordX;
                int nCoordY;
                int	iBin;

                // Compute coordinates from index
                m_pWafermap->coordFromIndex(nIndex, nCoordX, nCoordY);

                // If die has been already set as an outlier, do nothing
                if (lPatInfo->isDieOutlier(nCoordX, nCoordY, iBin) == false && excludeDie(nIndex))
                {
                    // This good bin passes the conditions to be failed (enough bad neighbors, and squeezed between failures)
                    lKey = QString::number(nCoordX) + "." + QString::number(nCoordY);

                    lGPATOutlier.mDieX          = nCoordX;
                    lGPATOutlier.mDieY          = nCoordY;
                    lGPATOutlier.mSite          = gexReport->getDieTestingSite(-1,0,nCoordX,nCoordY);	// Testing site
                    lGPATOutlier.mFailType      = GEX_TPAT_BINTYPE_BADNEIGHBORS;
                    lGPATOutlier.mRuleName      = m_pGdbnAlgorithm->GetRuleName();
                    lGPATOutlier.mPatHBin       = m_pGdbnAlgorithm->GetHardBin();
                    lGPATOutlier.mPatSBin       = m_pGdbnAlgorithm->GetSoftBin();
                    lGPATOutlier.mOrigHBin      = lPatInfo->GetOriginalBin(false, nCoordX, nCoordY);
                    lGPATOutlier.mOrigSBin      = lPatInfo->GetOriginalBin(true, nCoordX, nCoordY);
                    lGPATOutlier.mPartId        = gexReport->getDiePartId(-1, nCoordX, nCoordY);

                    // Save PAT definition in our list
                    lGDBNOutliers.insert(lKey, lGPATOutlier);
                }
            }
        }

        return true;
    }

    return false;
}


