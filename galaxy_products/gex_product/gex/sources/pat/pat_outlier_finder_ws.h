#ifdef GCORE15334
#ifndef PAT_OUTLIER_FINDER_WS_H
#define PAT_OUTLIER_FINDER_WS_H

#include "pat_outlier_finder.h"

class CWaferMap;

namespace GS
{
namespace Gex
{

class PATOutlierFinderWSPrivate;

class PATOutlierFinderWS : public PATOutlierFinder
{
public:

    PATOutlierFinderWS(CPatInfo * lContext, QObject * parent = NULL);
    virtual ~PATOutlierFinderWS();

    bool    AnalyzeWaferSurface();
    bool    AnalyzeWaferSurfaceZPAT(CWaferMap * lWaferMap);
    bool    ComputeMVPATOutliers();

protected:

    PATOutlierFinderWS(PATOutlierFinderWSPrivate & lPrivateData, CPatInfo * lContext, QObject *parent = 0);

    CTest * FindTestCell(CPatDefinition * lPatDef);
    bool	UpdateDatasetPointer(int lSite);

private:

    Q_DISABLE_COPY(PATOutlierFinderWS)
    Q_DECLARE_PRIVATE_D(mPrivate, PATOutlierFinderWS)
};

}
}

#endif // PAT_OUTLIER_FINDER_WS_H
#endif
