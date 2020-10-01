///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "pat_potatoclusterfinder.h"
#include "report_build.h"
#include "report_options.h"
#include "pat_gdbn_weighting_algo.h"
#include "pat_gdbn_generic_baddies.h"
#include "gex_report.h"
#include "pat_engine.h"

///////////////////////////////////////////////////////////////////////////////////
// External object
///////////////////////////////////////////////////////////////////////////////////
extern CReportOptions	ReportOptions;
extern CGexReport *gexReport;				// report_build.cpp: Handle to report class

// For MemoryLeak detection: ALWAYS have to be LAST #include in file
#include "DebugMemory.h"

///////////////////////////////////////////////////////////////////////////////////
// Class PatGdbnClusterBadDies - Class to determine if a die should be taken into
//                               account in a gdbn algorithm used for clustering
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
PatGdbnClusterBadDies::PatGdbnClusterBadDies(
        const GS::QtLib::Range * pBadBinCluster,
        const GS::QtLib::Range * pBadBinGdbn,
        const std::list<int> clusterBadDies)
    : PatGdbnAbstractBadDies(), m_pBadBinCluster(pBadBinCluster), m_pBadBinGdbn(pBadBinGdbn)
{
    m_clusterBadDies    = QList<int>::fromStdList(clusterBadDies);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
PatGdbnClusterBadDies::~PatGdbnClusterBadDies()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CPatPotatoClusterSettings& operator=(const CPatPotatoClusterSettings& potatoClusterSettings)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool PatGdbnClusterBadDies::isDieIncluded(int dieBin, int dieIndex) const
{
    // Here are the conditions to take into account a die for the GDBN
    //    ° Rule 1: The die must have a bin matching with the list of bins used to build a cluster and belongs to a cluster.
    //    ° Rule 2: The die must have a bin matching with the list of bins used to ink and not matching with the list of bins used to build a cluster
    //    ° Rule 3: The die is located on the edge of the wafermap
    //
    // Here is an example
    //    Bins used for building a cluster : 14,22,23
    //    Bins used for inking a die : 14,22,23,180
    //
    //    Die has BIN14 and belongs to a cluster			: matches with the rule 1, so we have to add the weight corresponding
    //    Die has BIN14 and doesn't belong to any cluster	: doesn't match with any rule, so we don't take into account this die in the weighting rule
    //    Die has BIN180									: matches with the second rule, so we have to add the weight corresponding
    //
    if ((m_pBadBinCluster->Contains(dieBin) && m_clusterBadDies.indexOf(dieIndex) != -1) ||
        (m_pBadBinGdbn->Contains(dieBin) && !m_pBadBinCluster->Contains(dieBin)))
            return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Class CPatPotatoClusterSettings - Class holding potato cluster settings
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CPatPotatoClusterSettings::CPatPotatoClusterSettings(int nSize) : m_nSize(nSize)
{
    m_nOutlineWidth				= 1;
    m_nMinimumWeighting			= 0;
    m_nEdgeDieType				= CWaferMap::EdgeDieBoth;
    m_nEdgeDieWeighting			= GEX_TPAT_GPAT_EDGE_SCALE;
    m_dEdgeDieWeightingScale	= 1.0;
    m_eRules					= NoRule;
    m_nRingRadius				= -1;
    m_eRingArea					= CWaferMap::RingFromCenter;
    m_uFailType					= 0;
    mSoftBin                    = -1;
    mHardBin                    = -1;
}

CPatPotatoClusterSettings::CPatPotatoClusterSettings(
        const CPatPotatoClusterSettings& potatoClusterSettings)
{
    *this = potatoClusterSettings;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CPatPotatoClusterSettings::~CPatPotatoClusterSettings()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CPatPotatoClusterSettings& operator=(const CPatPotatoClusterSettings& potatoClusterSettings)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CPatPotatoClusterSettings& CPatPotatoClusterSettings::operator=(const CPatPotatoClusterSettings& potatoClusterSettings)
{
    if (this != &potatoClusterSettings)
    {
        m_nSize						= potatoClusterSettings.m_nSize;
        m_nOutlineWidth				= potatoClusterSettings.m_nOutlineWidth;
        m_lstAdjacentWeighting		= potatoClusterSettings.m_lstAdjacentWeighting;
        m_lstDiagonalWeighting		= potatoClusterSettings.m_lstDiagonalWeighting;
        m_nMinimumWeighting			= potatoClusterSettings.m_nMinimumWeighting;
        m_eRules					= potatoClusterSettings.m_eRules;
        m_nRingRadius				= potatoClusterSettings.m_nRingRadius;
        m_eRingArea					= potatoClusterSettings.m_eRingArea;
        m_nEdgeDieType				= potatoClusterSettings.m_nEdgeDieType;
        m_nEdgeDieWeighting			= potatoClusterSettings.m_nEdgeDieWeighting;
        m_dEdgeDieWeightingScale	= potatoClusterSettings.m_dEdgeDieWeightingScale;
        m_uFailType					= potatoClusterSettings.m_uFailType;
        m_strRuleName				= potatoClusterSettings.m_strRuleName;
        mSoftBin                    = potatoClusterSettings.mSoftBin;
        mHardBin                    = potatoClusterSettings.mHardBin;
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int CPatPotatoClusterSettings::adjacentWeighting(int nRing)
//
// Description	:	Gets the weighting for adjacent die on the nRing
//
///////////////////////////////////////////////////////////////////////////////////
int CPatPotatoClusterSettings::adjacentWeighting(int nRing) const
{
    if (nRing < m_lstAdjacentWeighting.count())
        return m_lstAdjacentWeighting.at(nRing);
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int CPatPotatoClusterSettings::diagonalWeighting(int nRing)
//
// Description	:	Gets the weighting for diagonal die on the nRing
//
///////////////////////////////////////////////////////////////////////////////////
int CPatPotatoClusterSettings::diagonalWeighting(int nRing) const
{
    if (nRing < m_lstDiagonalWeighting.count())
        return m_lstDiagonalWeighting.at(nRing);
    else
        return 0;
}

int CPatPotatoClusterSettings::GetSoftBin() const
{
    return mSoftBin;
}

int CPatPotatoClusterSettings::GetHardBin() const
{
    return mHardBin;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CPatPotatoClusterSettings::setAdjacentWeighting(const QList<int>& lstWeight)
//
// Description	:	Sets the weighting for adjacents die
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::setAdjacentWeighting(const QList<int>& lstWeight)
{
    m_lstAdjacentWeighting = lstWeight;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CPatPotatoClusterSettings::setDiagonalWeighting(const QList<int>& lstWeight)
//
// Description	:	Sets the weighting for diagonals die
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::setDiagonalWeighting(const QList<int>& lstWeight)
{
    m_lstDiagonalWeighting = lstWeight;
}

void CPatPotatoClusterSettings::SetSoftBin(int bin)
{
    mSoftBin = bin;
}

void CPatPotatoClusterSettings::SetHardBin(int bin)
{
    mHardBin = bin;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void enableIgnoreVerticalScratches(bool bEnable)
//
// Description	:	Enable/Disable vertical scratches rule
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::enableIgnoreVerticalScratches(bool bEnable)
{
    if (bEnable)
        m_eRules |= CPatPotatoClusterSettings::IgnoreVerticalScratches;
    else
        m_eRules &= ~(CPatPotatoClusterSettings::IgnoreVerticalScratches);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void enableIgnoreHorizontalScratches(bool bEnable)
//
// Description	:	Enable/Disable horizontal scratches rule
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::enableIgnoreHorizontalScratches(bool bEnable)
{
    if (bEnable)
        m_eRules |= IgnoreHorizontalScratches;
    else
        m_eRules &= ~(IgnoreHorizontalScratches);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void enableIgnoreDiagonalBadDies(bool bEnable)
//
// Description	:	Enable/Disable diagonal bad dies rule
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::enableIgnoreDiagonalBadDies(bool bEnable)
{
    if (bEnable)
        m_eRules |= IgnoreDiagonalBadDies;
    else
        m_eRules &= ~(IgnoreDiagonalBadDies);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void enableUseBadNeighboursAlgorithm(bool bEnable)
//
// Description	:	Enable/Disable to use bad neighbours algorithm
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterSettings::enableUseBadNeighboursAlgorithm(bool bEnable)
{
    if (bEnable)
        m_eRules |= UseBadNeighboursAlgorithm;
    else
        m_eRules &= ~(IgnoreDiagonalBadDies);
}

///////////////////////////////////////////////////////////////////////////////////
// Class CPatPotatoClusterFinder - Class to find out potato cluster on a wafermap
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CPatPotatoClusterFinder::CPatPotatoClusterFinder(int nClusterSize) : m_potatoClusterSettings(nClusterSize)
{
    m_pGdbnAlgorithm                = NULL;
    m_pWaferMap						= NULL;
    m_pBadBinListClusteringPotato	= NULL;
    m_pBadBinListInking				= NULL;
    m_pClusteringExclusions			= NULL;
    m_nWaferSize					= 0;
}

CPatPotatoClusterFinder::CPatPotatoClusterFinder(const CPatPotatoClusterSettings& potatoClusterSettings)
    : m_potatoClusterSettings(potatoClusterSettings)
{
    m_pGdbnAlgorithm                = NULL;
    m_pWaferMap						= NULL;
    m_pBadBinListClusteringPotato	= NULL;
    m_pBadBinListInking				= NULL;
    m_pClusteringExclusions			= NULL;
    m_nWaferSize					= 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CPatPotatoClusterFinder::~CPatPotatoClusterFinder()
{
    if (m_pGdbnAlgorithm)
    {
        delete m_pGdbnAlgorithm;
        m_pGdbnAlgorithm = NULL;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void processWafer(CWaferMap * pWaferMap, CGexRange	*ptGoodBinList,
//                                      CGexRange * pBadBinListClusteringPotato,
//                                      tdGPATOutliers * pClusteringExclusions)
//
// Description	:	Launch the process to find out potato cluster
//
///////////////////////////////////////////////////////////////////////////////////
bool CPatPotatoClusterFinder::processWafer(
        CWaferMap * pWaferMap,
        GS::QtLib::Range	*ptGoodBinList,
        GS::QtLib::Range * pBadBinListClusteringPotato,
        GS::QtLib::Range * pBadBinListInking,
        tdGPATOutliers * pClusteringExclusions)
{
    if (init(pWaferMap, ptGoodBinList, pBadBinListClusteringPotato, pBadBinListInking, pClusteringExclusions))
    {
        for(int nIndex=0; nIndex < m_nWaferSize; nIndex++)
        {
            // Check if we have to process this die
            if (pWaferMap->isDieInsideRing(m_potatoClusterSettings.ringArea(), m_potatoClusterSettings.ringRadius(), nIndex) && processDie(nIndex))
            {
                // Find out bad die around this one
                findOutBadDiesAround(m_lstBadDies.begin());

                // Verify cluster validity
                verifyBadClusteringRules();
            }
        }

        // Apply PAT binning
        applyBadClusterBin();

        delete [] m_pArrayProcessedDies;
        m_pArrayProcessedDies = NULL;

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool init(CWaferMap * pWaferMap, CGexRange	*ptGoodBinList,
//                              CGexRange * pBadBinListClusteringPotato,
//                              CPatDieList * pClusteringExclusions)
//
// Description	:	initialize the processing
//
///////////////////////////////////////////////////////////////////////////////////
bool CPatPotatoClusterFinder::init(CWaferMap * pWaferMap,
                                   GS::QtLib::Range	*ptGoodBinList,
                                   GS::QtLib::Range * pBadBinListClusteringPotato,
                                   GS::QtLib::Range * pBadBinListInking,
                                   tdGPATOutliers * pClusteringExclusions)
{
    if (pWaferMap && ptGoodBinList && pBadBinListClusteringPotato && pClusteringExclusions && pBadBinListInking)
    {
        m_ptGoodBinList					= ptGoodBinList;
        m_pWaferMap						= pWaferMap;
        m_pBadBinListClusteringPotato	= pBadBinListClusteringPotato;
        m_pBadBinListInking				= pBadBinListInking;
        m_pClusteringExclusions			= pClusteringExclusions;
        m_nWaferSize					= m_pWaferMap->SizeX * pWaferMap->SizeY;

        m_pArrayProcessedDies	= new bool[m_nWaferSize];
        memset(m_pArrayProcessedDies, false, sizeof(bool) * (m_nWaferSize));

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool processDie(int nIndex)
//
// Description	:	Process a single die
//					if the bin of the die is failed, add it into the list of bad die
//
///////////////////////////////////////////////////////////////////////////////////
bool CPatPotatoClusterFinder::processDie(int nIndex)
{
    // add die into list of bad dies
    if (m_pWaferMap->getWafMap()[nIndex].getBin() >= 0 &&
        m_pBadBinListClusteringPotato->Contains(m_pWaferMap->getWafMap()[nIndex].getBin()) &&
        m_pArrayProcessedDies[nIndex] == false
        && m_pWaferMap->isDieInsideRing(m_potatoClusterSettings.ringArea(), m_potatoClusterSettings.ringRadius(), nIndex))
    {
        m_lstBadDies.push_back(nIndex);
        m_pArrayProcessedDies[nIndex] = true;

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void findOutBadDiesAround(QList<int>::iterator itBadDie)
//
// Description	:	Looking for bad die around the die processed
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterFinder::findOutBadDiesAround(std::list<int>::iterator itBadDie)
{
    while (itBadDie != m_lstBadDies.end())
    {
        int nIndex = (*itBadDie);

        std::vector<int>			vecIndexDie = m_pWaferMap->aroundDieCoordinate(nIndex, (m_potatoClusterSettings.ignoreDiagonalBadDies()) ? CWaferMap::AdjacentDie : CWaferMap::BothDie);
        std::vector<int>::iterator	itBegin		= vecIndexDie.begin();
        std::vector<int>::iterator	itEnd		= vecIndexDie.end();

        while (itBegin != itEnd)
        {
            processDie(*itBegin);
            itBegin++;
        }

        itBadDie++;
        //findOutBadDiesAround(itBadDie);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void applyBadClusterBin()
//
// Description	:	apply the clustering if results matche with the rules
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterFinder::applyBadClusterBin()
{
    // At least m_nClusterSize
    std::list<int>::iterator            itDieBegin	= m_lstTotalBadDies.begin();
    std::list<int>::iterator            itDieEnd	= m_lstTotalBadDies.end();
    QHash<QString, CPatDieCoordinates>	hashClusterDie;

    if (m_potatoClusterSettings.useBadNeighboursAlgorithm())
    {
        m_pGdbnAlgorithm = new PatGdbnWeightingAlgo;

        m_pGdbnAlgorithm->SetRingArea(m_potatoClusterSettings.ringArea());
        m_pGdbnAlgorithm->SetRingRadius(m_potatoClusterSettings.ringRadius());
        m_pGdbnAlgorithm->SetEdgeDieType(m_potatoClusterSettings.edgeDieType());
        m_pGdbnAlgorithm->setEdgeDieWeighting(m_potatoClusterSettings.edgeDieWeighting());
        m_pGdbnAlgorithm->setEdgeDieWeightingScale(m_potatoClusterSettings.edgeDieWeightingScale());
        m_pGdbnAlgorithm->setMinimumWeighting(m_potatoClusterSettings.minimumWeighting());
        m_pGdbnAlgorithm->setAdjacentWeighting(m_potatoClusterSettings.adjacentWeighting());
        m_pGdbnAlgorithm->setDiagonalWeighting(m_potatoClusterSettings.diagonalWeighting());
        m_pGdbnAlgorithm->SetBadDies(new PatGdbnClusterBadDies(m_pBadBinListClusteringPotato, m_pBadBinListInking, m_lstTotalBadDies));
    }

    while (itDieBegin != itDieEnd)
    {
        int nIndex = (*itDieBegin);

        std::vector<int>			vecIndexDie = m_pWaferMap->aroundDieCoordinate(nIndex, CWaferMap::BothDie, 0, m_potatoClusterSettings.outlineWidth());
        std::vector<int>::iterator	itBegin		= vecIndexDie.begin();
        std::vector<int>::iterator	itEnd		= vecIndexDie.end();

        while (itBegin != itEnd)
        {
            setBadClusterBin(*itBegin, hashClusterDie);
            itBegin++;
        }

        itDieBegin++;
    }

    m_lstTotalBadDies.clear();

    // Fill the exclusion clustering die list with the list of die to ink
    QHashIterator<QString, CPatDieCoordinates>	itClusterDie(hashClusterDie);

    while (itClusterDie.hasNext())
    {
        itClusterDie.next();
        m_pClusteringExclusions->insert(itClusterDie.key(), itClusterDie.value());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void setBadClusterBin(int nIndex, QHash<QString, CPatDieCoordinates>& hashClusterDie)
//
// Description	:	set the die at the index as bad die
//
///////////////////////////////////////////////////////////////////////////////////
void CPatPotatoClusterFinder::setBadClusterBin(int nIndex, QHash<QString,
                                               CPatDieCoordinates>& hashClusterDie)
{
    // add die into list of bad dies
    if (verifyDieExclusionRules(nIndex))
    {
        int                 nCoordX;
        int                 nCoordY;
        int                 iBin;
        CPatDieCoordinates  lGoodBin;
        QString             lKey;
        CPatInfo *          lContext = GS::Gex::PATEngine::GetInstance().GetContext();

        // Compute coordinates from index
        m_pWaferMap->coordFromIndex(nIndex, nCoordX, nCoordY);

        // If die has been already set as an outlier, do nothing
        if (lContext->isDieOutlier(nCoordX, nCoordY, iBin) == false)
        {
            // This good bin passes the conditions to be failed (enough bad neighbors, and squeezed between failures)
            lKey = QString::number(nCoordX) + "." + QString::number(nCoordY);

            lGoodBin.mDieX      = nCoordX;
            lGoodBin.mDieY      = nCoordY;
            lGoodBin.mSite      = gexReport->getDieTestingSite(-1,0,nCoordX,nCoordY);	// Testing site
            lGoodBin.mFailType  = m_potatoClusterSettings.failType();
            lGoodBin.mRuleName  = m_potatoClusterSettings.ruleName();
            lGoodBin.mPatHBin   = m_potatoClusterSettings.GetHardBin();
            lGoodBin.mPatSBin   = m_potatoClusterSettings.GetSoftBin();
            lGoodBin.mOrigHBin  = lContext->GetOriginalBin(false, nCoordX, nCoordY);
            lGoodBin.mOrigSBin  = lContext->GetOriginalBin(true, nCoordX, nCoordY);
            lGoodBin.mPartId    = gexReport->getDiePartId(-1, nCoordX, nCoordY);

            // Save PAT definition in our list
            hashClusterDie.insert(lKey, lGoodBin);	// Holds the Good Die X,Y position to fail
            m_pArrayProcessedDies[nIndex] = true;
        }
    }
}



///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool verifyBadClusteringRules()
//
// Description	:	check that the cluster found matches with the rules defined
//
///////////////////////////////////////////////////////////////////////////////////
bool CPatPotatoClusterFinder::verifyBadClusteringRules()
{
    bool bBadClustering = false;

    if (m_lstBadDies.size() >= (unsigned int) m_potatoClusterSettings.size())
    {
        int nMinX	= m_nWaferSize;
        int nMaxX	= 0;
        int nMinY	= m_nWaferSize;
        int nMaxY	= 0;
        int nCoordX = -1;
        int nCoordY = -1;

        // At least one of the two rules is actrivated and cluster contains more than one die
        if ((m_potatoClusterSettings.rules() & CPatPotatoClusterSettings::IgnoreBothScratches) && m_lstBadDies.size() > 1)
        {
            std::list<int>::iterator itDieBegin	= m_lstBadDies.begin();
            std::list<int>::iterator itDieEnd	= m_lstBadDies.end();

            while (itDieBegin != itDieEnd)
            {
                nCoordX = (*itDieBegin) % m_pWaferMap->SizeX;
                nCoordY = (*itDieBegin) / m_pWaferMap->SizeX;

                // Get the min and max for x coordinates
                if (m_potatoClusterSettings.ignoreVerticalScratches())
                {
                    nMinX = qMin(nMinX, nCoordX);
                    nMaxX = qMax(nMaxX, nCoordX);
                }

                // Get the min and max for y coordinates
                if (m_potatoClusterSettings.ignoreHorizontalScratches())
                {
                    nMinY = qMin(nMinY, nCoordY);
                    nMaxY = qMax(nMaxY, nCoordY);
                }

                itDieBegin++;
            }
        }

        if ( (nMaxY - nMinY) != 0 && (nMaxX - nMinX) != 0)
        {
            // Add the cluster dies to the total bad dies
            m_lstTotalBadDies.merge(m_lstBadDies);
            bBadClustering = true;
        }
    }

    m_lstBadDies.clear();

    return bBadClustering;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool verifyDieExclusionRules(int nDieIndex)
//
// Description	:	check that the die matches with the exclusion rules defined
//
///////////////////////////////////////////////////////////////////////////////////
bool CPatPotatoClusterFinder::verifyDieExclusionRules(int nDieIndex)
{
    bool bExcludeDie			= false;

    // Not a die to exclude
    if (m_pWaferMap->getWafMap()[nDieIndex].getBin() >= 0 &&
        m_pBadBinListClusteringPotato->Contains(m_pWaferMap->getWafMap()[nDieIndex].getBin()) == false &&
        m_ptGoodBinList->Contains(m_pWaferMap->getWafMap()[nDieIndex].getBin()) && m_pArrayProcessedDies[nDieIndex] == false &&
        m_pWaferMap->isDieInsideRing(m_potatoClusterSettings.ringArea(), m_potatoClusterSettings.ringRadius(), nDieIndex))
    {
        // Apply bad neighbours algorithm
        if (m_pGdbnAlgorithm)
            bExcludeDie = m_pGdbnAlgorithm->excludeDie(m_pWaferMap, nDieIndex);
        else
            bExcludeDie = true;
    }

    return bExcludeDie;
}
