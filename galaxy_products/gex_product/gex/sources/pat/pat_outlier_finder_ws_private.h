#ifdef GCORE15334
#ifndef PAT_OUTLIER_FINDER_WS_PRIVATE_H
#define PAT_OUTLIER_FINDER_WS_PRIVATE_H

#include "pat_outlier_finder_private.h"

class CGexFileInGroup;
class CGexGroupOfFiles;
class CWaferMap;

namespace GS
{

namespace SE
{
class MVOutliersFinder;
class RMatrix;
class RVector;
class StatsEngine;
}

namespace Gex
{

class PATMultiVariateRule;

class PATOutlierFinderWSPrivate : public PATOutlierFinderPrivate
{
public:

    PATOutlierFinderWSPrivate();
    virtual ~PATOutlierFinderWSPrivate();

    bool    Init(CPatInfo * lContext);

    bool    AnalyzeWaferSurfaceGDBN(CWaferMap * lExternWafermap = NULL,
                                    bool lIgnoreYieldThreshold = false);
    bool    AnalyzeWaferSurfaceIDDQ_Delta();
    bool    AnalyzeWaferSurfaceNNR();
    bool    AnalyzeWaferSurfacePotatoCluster(CWaferMap * lExternWafermap = NULL);
    bool    AnalyzeWaferSurfaceReticle(CWaferMap * lExternWafermap = NULL);

    bool    BuildMVPATCorrelationCharts(const QString &lRuleName, int lIndex,
                                        SE::MVOutliersFinder& lAlgo,
                                        SE::RMatrix& lMatrix,
                                        SE::RVector& lLabels,
                                        const QString &lTitle,
                                        const QStringList &lAxisLabels = QStringList(),
                                        double lChi = 0,
                                        double lSigma = 0);
    bool    ComputeMVPATOutlier(const PATMultiVariateRule& lRule,
                                int lIndex,
                                GS::SE::StatsEngine *statsEngine);

    bool    FillPartFilter(PATPartFilter * lPartFilter, int lSite);
    CTest * FindTestCell(unsigned long testNumber, long pinIndex, const QString& testName);
    int     GetRequiredSamples() const;
    bool    HasRequiredSamples(const CTest &lTestCell) const;
    void    PreComputeDynamicLimits(CTest * lTestCell);
    void    PostComputeDynamicLimits(CTest * lTestCell);
    void    OnHighCPKDetected(CPatDefinition& lPatDef, CTest& lTestCell,
                              GS::PAT::DynamicLimits&	lDynLimits);

    int                 mCurrentSite;	// Hold site# of current call to 'UpdateDatasetPointer', allows faster processing.
    CGexGroupOfFiles *  mGroup;
    CGexFileInGroup *   mFile;
    CTest *             mTestSoftBin;
    CTest *             mTestHardBin;
    CTest *             mTestSite;
};

}
}

#endif // PAT_OUTLIER_FINDER_WS_PRIVATE_H
#endif
