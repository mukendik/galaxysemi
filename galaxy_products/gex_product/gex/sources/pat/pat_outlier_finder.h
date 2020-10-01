#ifdef GCORE15334

#ifndef PAT_OUTLIER_FINDER_H
#define PAT_OUTLIER_FINDER_H

#include <QStringList>

class CPatInfo;
class CPatDefinition;
class CTest;

namespace GS
{

namespace PAT
{
class DynamicLimits;
}

namespace Gex
{
class PATOutlierFinderPrivate;
class GsQaDump;

class PATOutlierFinder : public QObject
{
public:

    virtual ~PATOutlierFinder();

    bool    ComputePatLimits(const QList<int>& Sites, const bool ComputeSPAT);

protected:

    PATOutlierFinder(PATOutlierFinderPrivate & lPrivateData, QObject * parent = NULL);

    virtual CTest * FindTestCell(CPatDefinition * lPatDef) = 0;
    virtual bool	UpdateDatasetPointer(int lSite) = 0;

    bool    ApplySamplesFiltering(CPatDefinition * lPatDef, CTest * lTestCell);
    void    CheckSPATSpecLimits(CPatDefinition * lPatDef, const CTest *lTestCell);
    bool	ComputePatLimits(CPatDefinition * lPatDef, int lSite, GsQaDump & lQaDump, const bool ComputeSPAT);
    bool    ComputeDynamicLimits(CPatDefinition * lPatDef, CTest * lTestCell, int lSite, GsQaDump & lQaDump);
    double  FindOutlierValue(const CTest * lTest, const GS::PAT::DynamicLimits &lDynLimits) const;
    double  GetRelevantCpk(const CTest * lTest, int lOutliersKept) const;
    void    SetAlgorithmValues(int lAlgorithm, const CTest *lTestCell,
                               GS::PAT::DynamicLimits &lDynLimits) const;
    bool    UpdateDynamicLimits(CPatDefinition *lPatDef, CTest *lTestCell,
                                double lExponent, GS::PAT::DynamicLimits &lDynLimits, bool forceNewRule=false) const;
    void	UpdateForceLimitsPctLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                       GS::PAT::DynamicLimits &lDynLimits) const;
    void	UpdateLimitPctLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                 GS::PAT::DynamicLimits &lDynLimits) const;
    void	UpdateQ1Q3IQRSigmaLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                     GS::PAT::DynamicLimits &lDynLimits) const;
    void	UpdateRangeLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                              GS::PAT::DynamicLimits &lDynLimits) const;
    void	UpdateRobustSigmaLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                    GS::PAT::DynamicLimits &lDynLimits) const;
    void	UpdateSigmaLimits(CPatDefinition *lPatDef, const CTest *lTestCell,
                              GS::PAT::DynamicLimits &lDynLimits) const;
    bool	UpdateSmartLimits(CPatDefinition *lPatDef, CTest * lTestCell,
                              GS::PAT::DynamicLimits &lDynLimits, bool forceNewRule=false) const;

    int		ComputeAutoDetectSmartLimits(CPatDefinition * lPatDef, CTest *lTestCell,
                                         GS::PAT::DynamicLimits & lDynLimits) const;
    void	ComputeGaussianSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                       GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeTailedGaussianSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                             GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeDoubleTailedGaussianSmartLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                                   GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeLogNormalSmartLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                        GS::PAT::DynamicLimits & lDynLimits) const;
    void	ComputeBiModalSmartLimits(CPatDefinition * lPatDef, const CTest * lTestCell,
                                      GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeMultiModalSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                         GS::PAT::DynamicLimits & lDynLimits) const;
    void	ComputeClampedSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                      GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeDoubleClampedSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                            GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeCategorySmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                       GS::PAT::DynamicLimits &lDynLimits) const;
    void	ComputeUnknownSmartLimits(CPatDefinition * lPatDef, const CTest *lTestCell,
                                      GS::PAT::DynamicLimits &lDynLimits) const;

private:

    Q_DISABLE_COPY(PATOutlierFinder)
    Q_DECLARE_PRIVATE_D(mPrivate, PATOutlierFinder)

protected:

    PATOutlierFinderPrivate * const    mPrivate;
};

}
}

#endif // PAT_OUTLIER_FINDER_H
#endif
