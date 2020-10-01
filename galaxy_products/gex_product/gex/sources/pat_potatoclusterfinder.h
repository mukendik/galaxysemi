#ifndef _PAT_POTATO_CLUSTER_FINDER_
#define _PAT_POTATO_CLUSTER_FINDER_

///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "pat_info.h"
#include "pat_gdbn_abstract_baddies.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QList>
#include <QHash>

///////////////////////////////////////////////////////////////////////////////////
// STL Includes
///////////////////////////////////////////////////////////////////////////////////
#include <list>
#include <vector>



class CWaferMap;
namespace GS
{
  namespace QtLib
  {
    class Range;
  }
}
class PatGdbnWeightingAlgo;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CPatPotatoClusterSettings
//
// Description	:	Class holding potato cluster settings
//
///////////////////////////////////////////////////////////////////////////////////
class CPatPotatoClusterSettings
{
public:

    enum RulesFlag
    {
        NoRule						= 0x00,
        IgnoreHorizontalScratches	= 0x01,
        IgnoreVerticalScratches		= 0x02,
        IgnoreBothScratches			= 0x03,
        IgnoreDiagonalBadDies		= 0x04,
        UseBadNeighboursAlgorithm	= 0x08
    };

    Q_DECLARE_FLAGS(Rules, RulesFlag)

    CPatPotatoClusterSettings(int nSize);
    CPatPotatoClusterSettings(const CPatPotatoClusterSettings& potatoClusterSettings);
    ~CPatPotatoClusterSettings();

    int				size() const											{ return m_nSize; }
    int				outlineWidth() const									{ return m_nOutlineWidth; }
    int				edgeDieType() const										{ return m_nEdgeDieType; }
    int				edgeDieWeighting() const								{ return m_nEdgeDieWeighting; }
    double			edgeDieWeightingScale() const							{ return m_dEdgeDieWeightingScale; }
    int				adjacentWeightingCount() const							{ return m_lstAdjacentWeighting.count(); }
    int				diagonalWeightingCount() const							{ return m_lstDiagonalWeighting.count(); }
    int				adjacentWeighting(int nRing) const;
    int				diagonalWeighting(int nRing) const;
    int				minimumWeighting() const								{ return m_nMinimumWeighting; }
    int				ringRadius() const										{ return m_nRingRadius; }
    CWaferMap::Ring ringArea() const										{ return m_eRingArea; }
    Rules			rules() const											{ return m_eRules; }
    int				failType()												{ return m_uFailType; }
    QString			ruleName()												{ return m_strRuleName; }
    QList<int>      adjacentWeighting()                                     { return m_lstAdjacentWeighting; }
    QList<int>      diagonalWeighting()                                     { return m_lstDiagonalWeighting; }
    int             GetSoftBin() const;
    int             GetHardBin() const;

    void			setOutlineWidth(int nWidth)								{ m_nOutlineWidth			= nWidth; }
    void			setEdgeDieType(int nEdgeDieType)						{ m_nEdgeDieType			= nEdgeDieType; }
    void			setEdgeDieWeighting(int nEdgeDieWeighting)				{ m_nEdgeDieWeighting		= nEdgeDieWeighting; }
    void			setEdgeDieWeightingScale(double dEdgeDieWeightingScale)	{ m_dEdgeDieWeightingScale	= dEdgeDieWeightingScale; }
    void			setAdjacentWeighting(const QList<int>& lstWeight);
    void			setDiagonalWeighting(const QList<int>& lstWeight);
    void			setMinimumWeighting(int nWeight)						{ m_nMinimumWeighting	= nWeight; }
    void			setRingRadius(int nRadius)								{ m_nRingRadius = nRadius; }
    void			setRingArea(CWaferMap::Ring eArea)						{ m_eRingArea = eArea; }
    void			setRules(Rules eRules)									{ m_eRules = eRules; }
    void			setFailType(int uFailType)								{ m_uFailType = uFailType; }
    void			setRuleName(QString strRuleName)						{ m_strRuleName = strRuleName; }
    void            SetSoftBin(int bin);
    void            SetHardBin(int bin);

    bool			ignoreVerticalScratches() const							{ return (m_eRules & IgnoreVerticalScratches); }
    bool			ignoreHorizontalScratches() const						{ return (m_eRules & IgnoreHorizontalScratches); }
    bool			ignoreDiagonalBadDies() const							{ return (m_eRules & IgnoreDiagonalBadDies); }
    bool			useBadNeighboursAlgorithm() const						{ return (m_eRules & UseBadNeighboursAlgorithm); }

    void			enableIgnoreVerticalScratches(bool bEnable);
    void			enableIgnoreHorizontalScratches(bool bEnable);
    void			enableIgnoreDiagonalBadDies(bool bEnable);
    void			enableUseBadNeighboursAlgorithm(bool bEnable);

    CPatPotatoClusterSettings& operator=(const CPatPotatoClusterSettings& potatoClusterSettings);

private:

    int					m_nSize;						// Minimal size to create a potato cluster
    int					m_nOutlineWidth;				// Number of good-die rings to remove around the cluster
    int					m_nEdgeDieType;					// Edge die type to analyze
    int					m_nEdgeDieWeighting;			// Weighting rule for edge die
    double				m_dEdgeDieWeightingScale;		// Weighting scale to apply on edge die
    int					m_nMinimumWeighting;			// Minimum weight to exclude a die
    Rules				m_eRules;						// Holds the rules enbaled
    int					m_nRingRadius;					// Mask Radius for the ring
    CWaferMap::Ring		m_eRingArea;					// Ring area
    QList<int>			m_lstAdjacentWeighting;			// Weight for an adjacent failed die (same line or column)
    QList<int>			m_lstDiagonalWeighting;			// Weight for a diagonal failed die
    int					m_uFailType;					// Failure type.
    QString				m_strRuleName;					// PAT rule name.
    int                 mSoftBin;                       // Holds PAT Soft Bin
    int                 mHardBin;                       // Holds PAT Hard Bin

};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	PatGdbnClusterBadDies
//
// Description	:	Class to determine if a die should be taken into
//                  account in a gdbn algorithm used for clustering
//
///////////////////////////////////////////////////////////////////////////////////
class PatGdbnClusterBadDies : public PatGdbnAbstractBadDies
{
public:

    PatGdbnClusterBadDies(const GS::QtLib::Range * pBadBinCluster,
                          const GS::QtLib::Range *pBadBinGdbn,
                          const std::list<int> clusterBadDies);
    ~PatGdbnClusterBadDies();

    bool    isDieIncluded(int /*bin*/, int /*dieIndex*/) const;

private:

    QList<int>              m_clusterBadDies;
    const GS::QtLib::Range *       m_pBadBinCluster;
    const GS::QtLib::Range *       m_pBadBinGdbn;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CPatPotatoClusterFinder
//
// Description	:	Class to find out potato cluster on a wafermap
//
///////////////////////////////////////////////////////////////////////////////////
class CPatPotatoClusterFinder
{
public:

    CPatPotatoClusterFinder(int nClusterSize);
    CPatPotatoClusterFinder(const CPatPotatoClusterSettings& potatoClusterSettings);
    ~CPatPotatoClusterFinder();

    // Launch the process to find out potato cluster
    bool processWafer(CWaferMap * pWaferMap,
                      GS::QtLib::Range	*ptGoodBinList,
                      GS::QtLib::Range * pBadBinListClusteringPotato,
                      GS::QtLib::Range * pBadBinListInking,
                      tdGPATOutliers * pClusteringExclusions);

protected:

    // initialize the processing
    bool						init(CWaferMap * pWaferMap,
                                     GS::QtLib::Range *ptGoodBinList,
                                     GS::QtLib::Range * pBadBinListClusteringPotato,
                                     GS::QtLib::Range * pBadBinListInking,
                                     tdGPATOutliers * pClusteringExclusions);

    bool						processDie(int nIndex);																										// Process a single die, if the bin of the die is failed, add it into the list of bad die

    void						findOutBadDiesAround(std::list<int>::iterator itBadDie);																	// Looking for bad die around the die processed

    void						applyBadClusterBin();																										// apply the clustering if results matche with the rules
    void						setBadClusterBin(int nIndex, QHash<QString, CPatDieCoordinates> &hashClusterDie);											// set the die at the index as bad die
    bool						verifyBadClusteringRules();																									// check that the cluster found matches with the rules defined
    bool						verifyDieExclusionRules(int nDieIndex);																						// check that the die matches with the exclusion rules defined

private:

    std::list<int>              m_lstBadDies;						// list of bad die found for a cluster
    std::list<int>              m_lstTotalBadDies;					// list of total bad die found for a cluster
    bool *						m_pArrayProcessedDies;				// Bad die already processed

    CWaferMap *					m_pWaferMap;						// wafer map object
    // List of good bins
    GS::QtLib::Range *          m_ptGoodBinList;
    // list of bad binning to identify a cluster
    GS::QtLib::Range *          m_pBadBinListClusteringPotato;
    // list of bad binning inking a cluster
    GS::QtLib::Range *          m_pBadBinListInking;

    tdGPATOutliers *			m_pClusteringExclusions;			// list of good die to pass to bad die
    int							m_nWaferSize;						// wafermap size

    CPatPotatoClusterSettings	m_potatoClusterSettings;			// Holds settings to apply

    PatGdbnWeightingAlgo *      m_pGdbnAlgorithm;                   // Algorithm for gdbn
};

#endif // _PAT_POTATO_CLUSTER_FINDER_
