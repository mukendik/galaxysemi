#ifndef WAFERMAP_PARAMETRIC
#define WAFERMAP_PARAMETRIC

#include <QList>

namespace GS
{
    namespace QtLib
    {
        class Range;
    }
}

class CGexGroupOfFiles;
class CGexFileInGroup;
class CTest;

//////////////////////////////////////////////////////
// Parametric WaferMap class
//////////////////////////////////////////////////////
struct CParametricWafMapArray
{
    double	lfValue;	// A value of 'GEX_C_DOUBLE_NAN' means no result for this die location
};

// Structure passed to CParametricWaferMap::isNRR()
class	CParametricWaferMapNNR
{
public:
    int		iMatrixSize;		// Matrix size for NNR analysis (eg: 9 for 9x9 size)
    int		iAlgorithm;			// Algorithm (eg: GEX_TPAT_NNR_ALGO_LOCAL_SIGMA)
    double	lfN_Factor;			// Algorithm N factor
    int		lf_LA;				// Location Averaging: top X% best dies in cluster
    int		iDieX;				// DieX to analyze
    int		iDieY;				// DieY to analyze
    double	lfValue;			// In case of NNR detected, returns test value.
};

class	CParametricWaferMap
{
public:
    CParametricWaferMap();
    ~CParametricWaferMap();

    bool    Init(int lLowDieX, int lLowDieY, int lHighDieX, int lHighDieY);
    CParametricWaferMap& operator=(const CParametricWaferMap& lOther);

    int     GetLowDieX() const;
    int     GetLowDieY() const;
    int     GetHighDieX() const;
    int     GetHighDieY() const;
    int     GetSizeX() const;
    int     GetSizeY() const;

    // All sites merged (with offseting each site)
    // Data are stored in multiple datasets (one for each site)
    bool	FillWaferMapFromMultipleDatasets(CTest * ptTestCell, GS::QtLib::Range * pGoodBinsList = NULL);

    // All sites merged (with offseting each site)
    // Data are stored in single datasets (List of sites are given)
    bool	FillWaferMapFromSingleDataset(CTest * ptTestCell, QList<int> lSiteList,
                                          GS::QtLib::Range *pGoodBinsList = NULL);

    /* Not used as test result array are aligned by X,Y
    // Fill a wafermap for IDDQ (delta between test pre and post)
    bool	FillIDDQWaferMap(CTest * testCellPre, CTest * testCellPost,
                             GS::QtLib::Range *pGoodBinsList = NULL);
    */

    bool	NNR_GetLA(CParametricWaferMap& cParametricMap, CParametricWaferMapNNR &cNNR, double OriginalValue);
    bool	NNR_BuildMap(CParametricWaferMap& cParametricMap,CParametricWaferMapNNR &cNNR);
    bool	isNNR(CParametricWaferMapNNR &cNNR);

    CParametricWafMapArray	*cParametricWafMap;			// Pointer to wafermap Value results array

    double	mLowLimit;
    double	mHighLimit;

private:

    int		mLowDieX;
    int     mHighDieX;
    int     mLowDieY;
    int     mHighDieY;
    int		mSizeX;
    int     mSizeY;		// Wafermap size (cells in X, and Y)

    void    getNNRClusterDistribution(QVector<double>& validDataPoints,
                                        long& totalValidValues,
                                        long& totalNeededValues,
                                        const CParametricWaferMap& cParametric,
                                        const CParametricWaferMapNNR &cNNR);

#ifdef BOSCH_NNR
    // DEPRECATED
    // Developped in 2011 for BOSCH prospect, but didn't match with their expectation
    QVector<double>     mNNRWholeWafer;
    void    getNNRWholeWaferDistribution(QVector<double>& validDataPoints,
                                        long& totalValidValues,
                                        long& totalNeededValues,
                                        const CParametricWaferMap& cParametric,
                                        const CParametricWaferMapNNR &cNNR);
#endif
};
#endif // WAFERMAP_PARAMETRIC

