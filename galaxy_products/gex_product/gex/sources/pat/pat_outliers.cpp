#include "pat_outliers.h"
#include "pat_defines.h"

namespace GS
{
namespace Gex
{

PATMVOutlier::PATMVOutlier()
    : mRunID(-1), mSite(-1), mSBin(-1), mHBin(-1), mOrigHBin(-1), mOrigSBin(-1)
{
}

PATMVOutlier::PATMVOutlier(const PATMVOutlier &other)
{
    *this = other;
}

PATMVOutlier::~PATMVOutlier()
{
}

PATMVOutlier &PATMVOutlier::operator =(const PATMVOutlier &other)
{
    if (this != &other)
    {
        mPartID         = other.mPartID;
        mRunID          = other.mRunID;
        mCoord          = other.mCoord;
        mSite           = other.mSite;
        mSBin           = other.mSBin;
        mHBin           = other.mHBin;
        mOrigHBin       = other.mOrigHBin;
        mOrigSBin       = other.mOrigSBin;
        mFailingRules   = other.mFailingRules;
    }

    return *this;
}

void PATMVOutlier::AddFailingRule(const QString &lName, double lZScore, int lSeverity)
{
    AddFailingRule(PATMVFailingRule(lName, lZScore, lSeverity));
}

void PATMVOutlier::AddFailingRule(const PATMVFailingRule &lFailingRule)
{
    mFailingRules.append(lFailingRule);
}

const QString &PATMVOutlier::GetPartID() const
{
    return mPartID;
}

long PATMVOutlier::GetRunID() const
{
    return mRunID;
}

const WaferCoordinate &PATMVOutlier::GetCoordinate() const
{
    return mCoord;
}

int PATMVOutlier::GetSite() const
{
    return mSite;
}

int PATMVOutlier::GetSoftBin() const
{
    return mSBin;
}

int PATMVOutlier::GetHardBin() const
{
    return mHBin;
}

int PATMVOutlier::GetOriginalHardBin() const
{
    return mOrigHBin;
}

int PATMVOutlier::GetOriginalSoftBin() const
{
    return mOrigSBin;
}

const QList<PATMVFailingRule> &PATMVOutlier::GetFailingRules() const
{
    return mFailingRules;
}

void PATMVOutlier::SetPartID(const QString &lPartID)
{
    mPartID = lPartID;
}

void PATMVOutlier::SetRunID(long lRunID)
{
    mRunID = lRunID;
}

void PATMVOutlier::SetCoordinate(const WaferCoordinate &lCoordinate)
{
    mCoord = lCoordinate;
}

void PATMVOutlier::SetSite(int lSite)
{
    mSite = lSite;
}

void PATMVOutlier::SetSoftBin(int lSoftBin)
{
    mSBin = lSoftBin;
}

void PATMVOutlier::SetHardBin(int lHardBin)
{
    mHBin = lHardBin;
}

void PATMVOutlier::SetOriginalSoftBin(int lOriginalSoftBin)
{
    mOrigSBin = lOriginalSoftBin;
}

void PATMVOutlier::SetOriginalHardBin(int lOriginalHardBin)
{
    mOrigHBin = lOriginalHardBin;
}

PATMVFailingRule::PATMVFailingRule()
    : mZScore(0), mSeverity(GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR)
{

}

PATMVFailingRule::PATMVFailingRule(const QString &lName, double lZScore,
                                   int lSeverity)
    : mRuleName(lName), mZScore(lZScore), mSeverity(lSeverity)
{

}

PATMVFailingRule::PATMVFailingRule(const PATMVFailingRule &other)
{
    *this = other;
}

PATMVFailingRule::~PATMVFailingRule()
{

}

PATMVFailingRule &PATMVFailingRule::operator=(const PATMVFailingRule &other)
{
    if (this != &other)
    {
        mRuleName   = other.mRuleName;
        mZScore     = other.mZScore;
        mSeverity   = other.mSeverity;
    }

    return *this;
}

const QString &PATMVFailingRule::GetRuleName() const
{
    return mRuleName;
}

double PATMVFailingRule::GetZScore() const
{
    return mZScore;
}

int PATMVFailingRule::GetSeverity() const
{
    return mSeverity;
}

void PATMVFailingRule::SetRuleName(const QString &lName)
{
    mRuleName = lName;
}

void PATMVFailingRule::SetZScore(double lScore)
{
    mZScore = lScore;
}

void PATMVFailingRule::SetSeverity(int lSeverity)
{
    mSeverity = lSeverity;
}

PATMVRuleSummary::PATMVRuleSummary()
    : mNearOultiers(0), mMediumOutliers(0), mFarOutliers(0)
{

}

PATMVRuleSummary::PATMVRuleSummary(const QString &lRuleName)
    : mRuleName(lRuleName), mNearOultiers(0), mMediumOutliers(0), mFarOutliers(0)
{

}

PATMVRuleSummary::PATMVRuleSummary(const PATMVRuleSummary &other)
{
    *this = other;
}

PATMVRuleSummary::~PATMVRuleSummary()
{

}

PATMVRuleSummary &PATMVRuleSummary::operator=(const PATMVRuleSummary &other)
{
    if (this != &other)
    {
        mRuleName       = other.mRuleName;
        mNearOultiers   = other.mNearOultiers;
        mMediumOutliers = other.mMediumOutliers;
        mFarOutliers    = other.mFarOutliers;
    }

    return *this;
}

bool PATMVRuleSummary::operator<(const PATMVRuleSummary &other) const
{
    return GetSeverityScore() > other.GetSeverityScore();
}

void PATMVRuleSummary::AddNearOutlier(int lCount)
{
    mNearOultiers += lCount;
}

void PATMVRuleSummary::AddMediumOutlier(int lCount)
{
    mMediumOutliers += lCount;
}

void PATMVRuleSummary::AddFarOutlier(int lCount)
{
    mFarOutliers += lCount;
}

const QString &PATMVRuleSummary::GetRuleName() const
{
    return mRuleName;
}

int PATMVRuleSummary::GetSeverityScore() const
{
    int lSeverity = 0;

    lSeverity += 2 * mNearOultiers;
    lSeverity += 4 * mMediumOutliers;
    lSeverity += 6 * mFarOutliers;

    return lSeverity;
}

int PATMVRuleSummary::GetOutlierCount() const
{
    return (mNearOultiers + mMediumOutliers + mFarOutliers);
}

int PATMVRuleSummary::GetNearOutlierCount() const
{
    return mNearOultiers;
}

int PATMVRuleSummary::GetMediumOutlierCount() const
{
    return mMediumOutliers;
}

int PATMVRuleSummary::GetFarOutlierCount() const
{
    return mFarOutliers;
}

void PATMVRuleSummary::SetRuleName(const QString &lRuleName)
{
    mRuleName = lRuleName;
}

}   // End of namespace Gex
}   // End of namespace GS

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CPatSite::CPatSite()
{
    bPirSeen=false;			// 'true' if this site is currently datalogged
    lPatBin=-1;				// Set to PAT failing Bin if a test failes its PAT limits during the PIR/PRR sequence.
    lOutliersFound=0;
}

///////////////////////////////////////////////////////////
// Computes PAT criticity score based on all outliers in the
// part
///////////////////////////////////////////////////////////
double CPatOutlierPart::getDevicePatScore(void)
{
    QList<CPatFailingTest>::iterator itOutlierTests;
    CPatFailingTest cFailTest;

    double	lfPatScore = 0;
    for(itOutlierTests = cOutlierList.begin(); itOutlierTests != cOutlierList.end(); ++itOutlierTests )
    {
        cFailTest = *itOutlierTests;

        switch(cFailTest.mFailureMode)
        {
            case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_NEAR:		// Near outlier: increment score by 1.0
                lfPatScore += 1.0;
                break;
            case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_MEDIUM:	// Medium outlier: increment score by 2.0
                lfPatScore += 2.0;
                break;
            case GEX_TPAT_OUTLIER_SEVERITY_LIMIT_FAR:		// Far outlier: increment score by 4.0
                lfPatScore += 4.0;
                break;
        }
    }

    // Return PAT score (the higher the more critical the outlier part)
    return lfPatScore;
}

///////////////////////////////////////////////////////////
// < Operator definition
///////////////////////////////////////////////////////////
bool CPatOutlierPart::operator<(const CPatOutlierPart& other) const
{
    return lfPatScore < other.lfPatScore;
}
