#ifdef GCORE15334

#ifndef PAT_OUTLIER_FINDER_FT_H
#define PAT_OUTLIER_FINDER_FT_H

#include "pat_outlier_finder.h"

namespace GS
{
namespace Gex
{

class PATOutlierFinderFTPrivate;
class SiteTestResults;

class PATOutlierFinderFT : public PATOutlierFinder
{
public:

    PATOutlierFinderFT(CPatInfo * lContext,
                       QMap<int, GS::Gex::SiteTestResults *> *lFTAllSites,
                       QObject * parent = NULL);
    virtual ~PATOutlierFinderFT();

    bool    ComputeTestStats(const QList<int>& lSites);
    bool    ComputeMultiSiteDynamicLimits(const QList<int>& lSites, const QList<int>& lSitesFrom);

protected:

    PATOutlierFinderFT(PATOutlierFinderFTPrivate & lPrivateData, QMap<int, GS::Gex::SiteTestResults *> *lFTAllSites,
                       CPatInfo * lContext, QObject *parent = 0);

    CTest * FindTestCell(CPatDefinition * lPatDef);
    bool	UpdateDatasetPointer(int lSite);

private:

    Q_DISABLE_COPY(PATOutlierFinderFT)
    Q_DECLARE_PRIVATE_D(mPrivate, PATOutlierFinderFT)
};

}
}

#endif // PAT_OUTLIER_FINDER_FT_H
#endif
