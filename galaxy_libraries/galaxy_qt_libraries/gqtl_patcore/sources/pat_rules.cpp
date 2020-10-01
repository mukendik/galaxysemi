#include "pat_rules.h"
#include "pat_defines.h"
#include "gqtl_utils.h"

CNNR_Rule::CNNR_Rule()
{
    mIsEnabled = true;					// Default: rule is enabled!
    mRuleName = "";                     // Holds NNR rule name
    mAlgorithm = GEX_TPAT_NNR_ALGO_LOCAL_SIGMA;// NNR Algorithm (Stringent, Yield safe...)
    mN_Factor = 6.0;					// N factor (eg: 6 for 6*Sigma)
    mClusterSize = 5;					// NNR cluster size (5,7,9,11,13,15)
    mLA = 70.0;                         // NNR Location Averaging: use top X% best dies
    mSoftBin = -1;
    mHardBin = -1;
    mFailBinColor = QColor(255,202,181);
}
QColor CNNR_Rule::GetFailBinColor() const
{
    return mFailBinColor;
}

void CNNR_Rule::SetFailBinColor(const QColor &failBinColor)
{
    mFailBinColor = failBinColor;
}

int CNNR_Rule::GetSoftBin() const
{
    return mSoftBin;
}

int CNNR_Rule::GetHardBin() const
{
    return mHardBin;
}

void CNNR_Rule::SetSoftBin(int softBin)
{
    mSoftBin = softBin;
}

void CNNR_Rule::SetHardBin(int hardBin)
{
    mHardBin = hardBin;
}

double CNNR_Rule::GetLA() const
{
    return mLA;
}

void CNNR_Rule::SetLA(double lA)
{
    mLA = lA;
}

int CNNR_Rule::GetClusterSize() const
{
    return mClusterSize;
}

void CNNR_Rule::SetClusterSize(int clusterSize)
{
    mClusterSize = clusterSize;
}

double CNNR_Rule::GetNFactor() const
{
    return mN_Factor;
}

void CNNR_Rule::SetNFactor(double n_Factor)
{
    mN_Factor = n_Factor;
}

int CNNR_Rule::GetAlgorithm() const
{
    return mAlgorithm;
}

void CNNR_Rule::SetAlgorithm(int algorithm)
{
    mAlgorithm = algorithm;
}

QString CNNR_Rule::GetRuleName() const
{
    return mRuleName;
}

void CNNR_Rule::SetRuleName(const QString &ruleName)
{
    mRuleName = ruleName;
}

bool CNNR_Rule::IsEnabled() const
{
    return mIsEnabled;
}

void CNNR_Rule::SetIsEnabled(bool isEnabled)
{
    mIsEnabled = isEnabled;
}


CIDDQ_Delta_Rule::CIDDQ_Delta_Rule()
{
    mIsEnabled  = true;					// Default: rule is enabled!
    mRuleName   = "My Rule...";			// Holds IDDQ rule name
    mPreStress  = "PreStrs";             // Pre-stress sub-string to find in test names
    mPostStress = "PostStrs";           // Post-stress sub-string to find in test names
    mCaseSensitive = false;             // true if case sensitive string checks.
    mAlgorithm  = GEX_TPAT_IDDQ_DELTA_SIGMA;	// IDDQ Algorithm (N*sigma, IQR,...)
    mN_Factor   = 6.0;					// N factor (eg: 6 for 6*Sigma)
    mSoftBin    = -1;
    mHardBin    = -1;
    mFailBinColor = QColor(255,202,181);
}
bool CIDDQ_Delta_Rule::IsEnabled() const
{
    return mIsEnabled;
}

void CIDDQ_Delta_Rule::SetIsEnabled(bool isEnabled)
{
    mIsEnabled = isEnabled;
}

QString CIDDQ_Delta_Rule::GetPreStress() const
{
    return mPreStress;
}

void CIDDQ_Delta_Rule::SetPreStress(const QString &preStress)
{
    mPreStress = preStress;
}

QString CIDDQ_Delta_Rule::GetRuleName() const
{
    return mRuleName;
}

void CIDDQ_Delta_Rule::SetRuleName(const QString &ruleName)
{
    mRuleName = ruleName;
}

QString CIDDQ_Delta_Rule::GetPostStress() const
{
    return mPostStress;
}

void CIDDQ_Delta_Rule::SetPostStress(const QString &postStress)
{
    mPostStress = postStress;
}
bool CIDDQ_Delta_Rule::GetCaseSensitive() const
{
    return mCaseSensitive;
}

void CIDDQ_Delta_Rule::SetCaseSensitive(bool caseSensitive)
{
    mCaseSensitive = caseSensitive;
}
int CIDDQ_Delta_Rule::GetAlgorithm() const
{
    return mAlgorithm;
}

void CIDDQ_Delta_Rule::SetAlgorithm(int algorithm)
{
    mAlgorithm = algorithm;
}
double CIDDQ_Delta_Rule::GetNFactor() const
{
    return mN_Factor;
}

void CIDDQ_Delta_Rule::SetNFactor(double n_Factor)
{
    mN_Factor = n_Factor;
}

void CIDDQ_Delta_Rule::SetSoftBin(int softBin)
{
    mSoftBin = softBin;
}
int CIDDQ_Delta_Rule::GetSoftBin() const
{
    return mSoftBin;
}

int CIDDQ_Delta_Rule::GetHardBin() const
{
    return mHardBin;
}

void CIDDQ_Delta_Rule::SetHardBin(int hardBin)
{
    mHardBin = hardBin;
}

QColor CIDDQ_Delta_Rule::GetFailBinColor() const
{
    return mFailBinColor;
}

void CIDDQ_Delta_Rule::SetFailBinColor(const QColor &failBinColor)
{
    mFailBinColor = failBinColor;
}


CClusterPotatoRule::CClusterPotatoRule()
{
    mIsEnabled = true;					// Default: rule is enabled!
    // List of Bad bins defining a bad cluster
    mBadBinIdentifyList = new GS::QtLib::Range((char *)"0;2-65535");
    // List of Bad bins inking a bad cluster
    mBadBinInkingList = new GS::QtLib::Range((char *)"0;2-65535");
    mRuleName = "My Rule...";			// Holds clustering rule name
    mMaskName = "-None-";               // No Mask
    mWaferSource = GEX_PAT_WAFMAP_SRC_SOFTBIN;	// Wafermap source: STDF Soft/Hard bin, Prober map.
    mClusterSize = 50;					// Positive number to define minimum number of dies to flag an area as bad cluster, if negative numbern defines a percentage of dies over wafer size instead
    mOutlineWidth = 1;					// Number of good-die rings to remove around the cluster (default is 1)
    mIsLightOutlineEnabled = true;				// true if light outline enabled (weight each die surround).
    mOutlineMatrixSize = 0;				// Default Matrix size 0=3x3; 1=5x5; 2=7x7
    mAdjWeightLst.clear();
    mAdjWeightLst << 2 << 2 << 2;       // Adjacent weight for bad dies
    mDiagWeightLst.clear();
    mDiagWeightLst << 1 << 1 << 1;		// Diagonal weight for bad dies
    mFailWeight = 8;					// Fail weight threshold of bad dies.
    mIgnoreScratchLines = true;			// True if ignore one-die scratch lines
    mIgnoreScratchRows = true;			// True if ignore one-die scratch rows
    mIgnoreDiagonalBadDies = true;		// True if ignore diagonal bad dies when doing cluster identification
    mEdgeDieWeighting = GEX_TPAT_GPAT_EDGE_SCALE;	// Weighting rule: Edge die handling
    mEdgeDieWeightingScale = 1.1;

    /// TODO -> CWaferMap::EdgeDieBoth
    mEdgeDieType = 3;
    mSoftBin = -1;
    mHardBin = -1;
    mFailBinColor = QColor(255,0,255);
}

CClusterPotatoRule::~CClusterPotatoRule()
{
    if(mBadBinIdentifyList != NULL)
    {
        delete mBadBinIdentifyList;
        mBadBinIdentifyList = NULL;
    }
    if(mBadBinInkingList != NULL)
    {
        delete mBadBinInkingList;
        mBadBinInkingList = NULL;
    }
}


CClusterPotatoRule::CClusterPotatoRule(const CClusterPotatoRule &rule)
    : QObject(rule.parent()), mBadBinIdentifyList(NULL), mBadBinInkingList(NULL)
{
    *this = rule;
}

CClusterPotatoRule& CClusterPotatoRule::operator=(const CClusterPotatoRule& rule)
{
    if (this != &rule)
    {
        mIsEnabled = rule.mIsEnabled;
        mRuleName = rule.mRuleName;
        mMaskName = rule.mMaskName;

        if (rule.mBadBinIdentifyList)
        {
            if (mBadBinIdentifyList)
                delete mBadBinIdentifyList;
            mBadBinIdentifyList = new GS::QtLib::Range(rule.mBadBinIdentifyList->GetRangeList());
        }
        else
        {
            if (mBadBinIdentifyList)
                delete mBadBinIdentifyList;
            mBadBinIdentifyList = new GS::QtLib::Range((char*) "0;2-65535");
        }

        if (rule.mBadBinInkingList)
        {
            if (mBadBinInkingList)
                delete mBadBinInkingList;
            mBadBinInkingList = new GS::QtLib::Range(rule.mBadBinInkingList->GetRangeList());
        }
        else
        {
            if (mBadBinInkingList)
                delete mBadBinInkingList;
            mBadBinInkingList = new GS::QtLib::Range((char*) "0;2-65535");
        }

        mWaferSource = rule.mWaferSource;
        mClusterSize = rule.mClusterSize;
        mOutlineWidth = rule.mOutlineWidth;
        mIsLightOutlineEnabled = rule.mIsLightOutlineEnabled;
        mOutlineMatrixSize = rule.mOutlineMatrixSize;
        mAdjWeightLst = rule.mAdjWeightLst;
        mDiagWeightLst = rule.mDiagWeightLst;
        mFailWeight = rule.mFailWeight;
        mIgnoreScratchLines = rule.mIgnoreScratchLines;
        mIgnoreScratchRows = rule.mIgnoreScratchRows;
        mIgnoreDiagonalBadDies = rule.mIgnoreDiagonalBadDies;

        mEdgeDieType = rule.mEdgeDieType;
        mEdgeDieWeighting = rule.mEdgeDieWeighting;
        mEdgeDieWeightingScale = rule.mEdgeDieWeightingScale;
        mSoftBin = rule.mSoftBin;
        mHardBin = rule.mHardBin;
        mFailBinColor = rule.mFailBinColor;
    }

    return *this;
}


CGDBN_Rule::CGDBN_Rule()
{
    mIsEnabled = true;                              // Default: rule is enabled!
    mRuleName = "Rule...";
    mMaskName = "-None-";                           // No Mask
    mBadBinList = new GS::QtLib::Range("0,2-65535");
    mAlgorithm = GEX_TPAT_GDBN_ALGO_SQUEEZE;        // Squeeze algorithm
    mYieldThreshold = 50.0;
    mClusterSize = 3;                               // Matrix 3x3
    mFailCount = 2;                                 // 2 bad dies in neighborhood to fail a good die.
    mFailWaferEdges = false;                        // 'true' if to fail wafer edges with bad neighbors.
    mWafermapSource = GEX_PAT_WAFMAP_SRC_SOFTBIN;	// GDBN wafermap source : STDF Soft/Hardbin, Prober map
    mAdjWeightLst.append(2);                        // Weight for adjacent failing dies
    mDiagWeightLst.append(1);                       // Weight for diagonal failing dies
    mMinimumWeighting = 8;                          // Minimum surrounding weight  to fail good die
    mEdgeDieWeighting = GEX_TPAT_GPAT_EDGE_SCALE;	// Weighting rule: Edge die handling
    mEdgeDieWeightingScale	= 1.1;					// Scaling factor over computed weight
    mEdgeDieType = GEX_TPAT_EDGE_DIE_BOTH;          // Edge die type : all edge dies (default)
    mSoftBin = -1;
    mHardBin = -1;
    mFailBinColor = QColor(255,202,181);
}

CGDBN_Rule::~CGDBN_Rule()
{
    if(mBadBinList != NULL)
    {
        delete mBadBinList;
        mBadBinList = NULL;
    }
}

CGDBN_Rule::CGDBN_Rule(const CGDBN_Rule &rule)
    : QObject(rule.parent()), mBadBinList(NULL)
{
    *this = rule;
}

CGDBN_Rule& CGDBN_Rule::operator=(const CGDBN_Rule& rule)
{
    if (this != &rule)
    {
        if (rule.mBadBinList)
        {
            if (mBadBinList)
                delete mBadBinList;
            mBadBinList = new GS::QtLib::Range(rule.mBadBinList->GetRangeList());
        }
        else
        {
            if (mBadBinList)
                delete mBadBinList;
            mBadBinList = new GS::QtLib::Range((char*) "0;2-65535");
        }

    }

    mIsEnabled = rule.mIsEnabled;
    mRuleName = rule.mRuleName;
    mMaskName = rule.mMaskName;
    mYieldThreshold = rule.mYieldThreshold;
    mWafermapSource = rule.mWafermapSource;
    mAlgorithm = rule.mAlgorithm;
    mFailWaferEdges = rule.mFailWaferEdges;
    mClusterSize = rule.mClusterSize;
    mFailCount = rule.mFailCount;
    mAdjWeightLst = rule.mAdjWeightLst;
    mDiagWeightLst = rule.mDiagWeightLst;
    mMinimumWeighting = rule.mMinimumWeighting;
    mEdgeDieType = rule.mEdgeDieType;
    mEdgeDieWeighting = rule.mEdgeDieWeighting;
    mEdgeDieWeightingScale = rule.mEdgeDieWeightingScale;
    mSoftBin = rule.mSoftBin;
    mHardBin = rule.mHardBin;
    mFailBinColor = rule.mFailBinColor;

    return *this;
}


CMask_Rule::CMask_Rule()
{
    mIsEnabled = true;					// Default: rule is enabled!
    mRuleName = "Mask...";
    mWorkingArea = 0;	// Outer ring
    mRadius = 10;		// Ring radius
}

CMask_Rule::~CMask_Rule()
{
}
