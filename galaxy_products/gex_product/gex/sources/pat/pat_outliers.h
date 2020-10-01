#ifndef PAT_OUTLIERS_H
#define PAT_OUTLIERS_H

#include "gstdl_type.h"
#include "wafer_coordinate.h"

#include <QList>
#include <QString>
#include "pat_rules.h"

namespace GS
{
namespace Gex
{

class PATMVRuleSummary
{
public:

    PATMVRuleSummary();
    PATMVRuleSummary(const QString& lRuleName);
    PATMVRuleSummary(const PATMVRuleSummary& other);
    ~PATMVRuleSummary();

    PATMVRuleSummary&   operator=(const PATMVRuleSummary& other);
    bool                operator<(const PATMVRuleSummary& other) const;

    void            AddNearOutlier(int lCount);
    void            AddMediumOutlier(int lCount);
    void            AddFarOutlier(int lCount);

    const QString&  GetRuleName() const;
    int             GetSeverityScore() const;
    int             GetOutlierCount() const;
    int             GetNearOutlierCount() const;
    int             GetMediumOutlierCount() const;
    int             GetFarOutlierCount() const;

    void            SetRuleName(const QString& lRuleName);

private:

    QString     mRuleName;
    int         mNearOultiers;
    int         mMediumOutliers;
    int         mFarOutliers;
};

class PATMVFailingRule
{
public:

    PATMVFailingRule();
    PATMVFailingRule(const QString& lName, double lZScore, int lSeverity);
    PATMVFailingRule(const PATMVFailingRule& other);
    ~PATMVFailingRule();

    PATMVFailingRule& operator=(const PATMVFailingRule& other);

    const QString&          GetRuleName() const;
    double                  GetZScore() const;
    int                     GetSeverity() const;

    void                    SetRuleName(const QString& lName);
    void                    SetZScore(double lScore);
    void                    SetSeverity(int lSeverity);

private:

    QString     mRuleName;
    double      mZScore;
    int         mSeverity;
};

class PATMVOutlier
{
public:

    PATMVOutlier();
    PATMVOutlier(const PATMVOutlier& other);
    ~PATMVOutlier();

    PATMVOutlier& operator=(const PATMVOutlier& other);

    void                    AddFailingRule(const QString& lName,
                                           double lZScore, int lSeverity);
    void                    AddFailingRule(const PATMVFailingRule& lFailingRule);
    const QString&          GetPartID() const;
    long                    GetRunID() const;
    const WaferCoordinate&  GetCoordinate() const;
    int                     GetSite() const;
    int                     GetSoftBin() const;
    int                     GetHardBin() const;
    int                     GetOriginalHardBin() const;
    int                     GetOriginalSoftBin() const;
    const QList<PATMVFailingRule>&  GetFailingRules() const;

    void                    SetPartID(const QString& lPartID);
    void                    SetRunID(long lRunID);
    void                    SetCoordinate(const WaferCoordinate& lCoordinate);
    void                    SetSite(int lSite);
    void                    SetSoftBin(int lSoftBin);
    void                    SetHardBin(int lHardBin);
    void                    SetOriginalSoftBin(int lOriginalSoftBin);
    void                    SetOriginalHardBin(int lOriginalHardBin);

private:

    QString         mPartID;		// PartID (if exists)
    long            mRunID;			// Run#
    WaferCoordinate mCoord;         // Die coordinates
    int             mSite;			// Testing site
    int             mSBin;          // Pat Softbin issued.
    int             mHBin;          // Pat Hardbin issued.
    int             mOrigHBin;      // Original HardBin
    int             mOrigSBin;      // Original SoftBin
    QList<PATMVFailingRule> mFailingRules;
};

}   // End of namespace Gex
}   // End of namespace GS

class CNNR_Rule;

class CPatFailingTest
{
public:
    unsigned long	mTestNumber;
    int             mPinIndex;
    QString         mTestName;
    int             mFailureMode;		// -1 = Static failure, otherwise, Dynamic severity level (GEX_TPAT_OUTLIER_SEVERITY_LIMIT_xxx)
    int             mFailureDirection;	// -1 = Negative, +1 = Positive
    int             mSite;				// Holds failing site#
    double          mValue;			// Holds outlier data point value (same units as samples).
    double          mLowLimit;
    double          mHighLimit;
};

class CPatSite
{
    public:
        CPatSite();
        bool	bPirSeen;			// 'true' if this site is currently datalogged
        long	lPatBin;			// '-1' if this site didn't fail any of the PAT limits during the flow, Pat fail bin value otherwise.
        long	lOutliersFound;		// Keeps track of total outliers in a run.
        BYTE	bFailType;			// Flag to define if this bin is a Static or Dynamic pat failure
        QList<CPatFailingTest> cOutlierList;	// Holds the list of tests failing in the run (list cleared at each run).
};

// Strucutre to hold details about a part found outlier.
class	CPatOutlierPart
{
public:

    QString	strPartID;		// PartID (if exists)
    long	lRunID;			// Run#
    int		iDieX;			// DieX
    int		iDieY;			// DieY
    int		iSite;			// Testing site
    int		iPatSBin;		// Pat Softbin issued.
    int		iPatHBin;		// Pat Hardbin issued.
    int		iOrgSoftbin;	// Original SoftBin for this PAT failed part
    int		iOrgHardbin;	// Original HardBin for this PAT failed part
    // Holds the list of tests failing in the run (list cleared at each run).
    QList<CPatFailingTest> cOutlierList;
    double	lfPatScore;		// PAT criticity score (computed by 'getDevicePatScore' )
    int     mRetestIndex;   // Retest index where the outlier is detected (FT PAT)
    double	getDevicePatScore(void);	// Computes PAT criticity score (based on each outlier)

    bool		operator<(const CPatOutlierPart& other) const;

    static bool lessThan(const CPatOutlierPart * pItem1, const CPatOutlierPart * pItem2);
};

// Class used to keep track of Good dies that need to be failed (because of high bad neigbhours count)
class CPatOutlierNNR
{
public:
    int             mDieX;			// DieX
    int             mDieY;			// DieY
    int             mSite;			// Testing site
    unsigned long	mTestNumber;
    int             mPinmap;
    QString         mTestName;
    double          mValue;			// Holds outlier data point value (same units as samples).
    // NNR rule
    CNNR_Rule	    mNNRRule;
};

// Class used to keep track of Good dies that need to be failed (because of high bad neigbhours count)
class CPatOutlierIDDQ_Delta
{
public:
    int		iDieX;			// DieX
    int		iDieY;			// DieY
    int		iSite;			// Testing site
    unsigned long	lTestNumber1;
    int		lPinmapIndex1;
    unsigned long	lTestNumber2;
    int		lPinmapIndex2;
    double	lfValue;			// Holds outlier data point value (same units as samples).
};

#endif // PAT_OUTLIERS_H
