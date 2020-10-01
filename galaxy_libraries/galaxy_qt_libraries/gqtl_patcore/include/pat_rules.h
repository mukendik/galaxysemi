#ifndef PAT_RULES_H
#define PAT_RULES_H

#include <QList>
#include <QString>
#include <QColor>
#include <QObject>

namespace GS
{
namespace QtLib
{
    class Range;
}
}

// Holds one rule of NNR
class	CNNR_Rule
{
public:
    CNNR_Rule();					// Constructor

    void SetIsEnabled(bool IsEnabled);
    void SetRuleName(const QString &GetRuleName);
    void SetAlgorithm(int GetAlgorithm);
    void SetNFactor(double GetNFactor);
    void SetClusterSize(int GetClusterSize);
    void SetLA(double GetLA);
    void SetSoftBin(int softBin);
    void SetHardBin(int hardBin);
    void SetFailBinColor(const QColor &GetFailBinColor);

    bool IsEnabled() const;
    QString GetRuleName() const;
    int GetAlgorithm() const;
    double GetNFactor() const;
    int GetClusterSize() const;
    double GetLA() const;
    int GetSoftBin() const;
    int GetHardBin() const;
    QColor GetFailBinColor() const;

private:
    bool		mIsEnabled;			///< Rule enabled/disabled
    QString		mRuleName;			///< Holds rule name
    int			mAlgorithm;			///< Algorithm
    double		mN_Factor;			///< N factor
    int			mClusterSize;		///< Cluster size
    double		mLA;				///< Location Averaging: Top X% best dies to average
    int         mSoftBin;           ///< Holds Soft bin
    int         mHardBin;           ///< Holds Hard bin
    QColor      mFailBinColor;      ///< Holds color
};

// Holds one rule of IDDQ-Delta
class	CIDDQ_Delta_Rule
{
public:
    CIDDQ_Delta_Rule();					// Constructor

    void SetIsEnabled(bool IsEnabled);
    void SetRuleName(const QString &GetRuleName);
    void SetPreStress(const QString &GetPreStress);
    void SetPostStress(const QString &GetPostStress);
    void SetCaseSensitive(bool GetCaseSensitive);
    void SetAlgorithm(int GetAlgorithm);
    void SetNFactor(double GetNFactor);
    void SetSoftBin(int softBin);
    void SetHardBin(int hardBin);
    void SetFailBinColor(const QColor &GetFailBinColor);

    bool IsEnabled() const;
    QString GetRuleName() const;
    QString GetPreStress() const;
    QString GetPostStress() const;
    bool GetCaseSensitive() const;
    int GetAlgorithm() const;
    double GetNFactor() const;
    int GetSoftBin() const;
    int GetHardBin() const;
    QColor GetFailBinColor() const;


private:
    bool		mIsEnabled;				// Rule enabled/disabled
    QString		mRuleName;				// Holds rule name
    QString		mPreStress;       // PreStress sub-string in test name
    QString		mPostStress;      // PostStress sub-string in test name
    bool		mCaseSensitive;	// true if case-sensitive checks over test names.
    int			mAlgorithm;		// IDDQ-Delta Algorithm (Mean+/-N*Sigma, IQR,...)
    double		mN_Factor;		// N factor (eg: 6 for 6*Sigma)
    int         mSoftBin;               // Holds soft bin
    int         mHardBin;               // Holds hard bin
    QColor      mFailBinColor;          // Holds color
};

//! \class Holds one rule of Clutering potato:
//! \brief defines minimum number of bad dies in a potato cluster to flag it as a bad cluster to outline
class	CClusterPotatoRule : public QObject
{
    Q_OBJECT
public:
    CClusterPotatoRule();				// Constructor
    ~CClusterPotatoRule();				// Destructor
    CClusterPotatoRule(const CClusterPotatoRule& rule);
    CClusterPotatoRule&  operator=(const CClusterPotatoRule& rule);

    bool		mIsEnabled;				// Rule enabled/disabled
    QString		mRuleName;              // Holds clustering rule name
    QString		mMaskName;              // Mask to use
    GS::QtLib::Range	*mBadBinIdentifyList;// List of Bad bins to consider in a potato bad cluster
    GS::QtLib::Range	*mBadBinInkingList;	// List of Bad bins to ink in a potato bad cluster
    int			mWaferSource;			// Wafermap source: STDF Soft/Hard bin, Prober map
    //! \brief Positive number to define minimum number of dies to flag an area as bad cluster,
    // if negative numbern defines a percentage of dies over wafer size instead
    float		mClusterSize;
    int			mOutlineWidth;			// Number of good-die rings to remove around the cluster (default is 1)
    bool		mIsLightOutlineEnabled;	// true if light outline enabled (weight each die surround).
    int			mOutlineMatrixSize;		// Outline matrix size: 0=3x"; 1=5x5; 2=7x7
    QList <int>	mAdjWeightLst;             // Ring1...N: Adjacent weight for bad dies
    QList <int>	mDiagWeightLst;			// Ring1...N: Diagonal weight for bad dies
    int			mFailWeight;			// Fail weight threshold of bad dies.
    bool		mIgnoreScratchLines;	// True if ignore one-die scratch lines
    bool		mIgnoreScratchRows;		// True if ignore one-die scratch rows
    bool		mIgnoreDiagonalBadDies;	// True if ignore diagonal bad dies when doing cluster identification

    // Edge dies
    int			mEdgeDieType;           // Edge die type to analyze
    int			mEdgeDieWeighting;		// Weighting rule: Edge die handling
    double		mEdgeDieWeightingScale; // Scaling factor over computed weight
    int         mSoftBin;               // Holds soft bin
    int         mHardBin;               // Holds hard bin
    QColor      mFailBinColor;          // Holds color
};

//! \class Holds one rule of Clutering potato: defines minimum number of bad dies in a potato cluster
//! // to flag it as a bad cluster to outline
class	CGDBN_Rule : public QObject
{
    Q_OBJECT
public:
    CGDBN_Rule();						// Constructor
    ~CGDBN_Rule();						// Destructor
    CGDBN_Rule(const CGDBN_Rule& rule);
    CGDBN_Rule&  operator=(const CGDBN_Rule& rule);

    bool		mIsEnabled;				// Rule enabled/disabled
    QString		mRuleName;              // Holds GDBN rule name
    QString		mMaskName;              // Mask to use
    double      mYieldThreshold;		// Enable good bin screening if yield is below this threshold
    GS::QtLib::Range	*mBadBinList;		// List of Bad bins to consider around
    int         mWafermapSource;		// GDBN wafermap source: STDF (soft/hard bin), prober map
    int         mAlgorithm;             // GDBN algorithm(Squeeze , Weighting)
    bool        mFailWaferEdges;		// 'true' if to fail wafer edges with bad neighbors.
    // Squeeze algorithm
    int         mClusterSize;			// Cluster size: 3, 5 or 7
    int         mFailCount;             // Minimum number of failing neighbors to fail a good part

    // Weighting algorithm
    QList <int>	mAdjWeightLst;          // Ring1...N: Adjacent weight for bad dies
    QList <int>	mDiagWeightLst;         // Ring1...N: Diagonal weight for bad dies
    int         mMinimumWeighting;		// Minimum surrounding weight  to fail good die

    // Edge dies
    int         mEdgeDieType;			// Edge die type to analyze
    int         mEdgeDieWeighting;		// Weighting rule: Edge die handling
    double      mEdgeDieWeightingScale;	// Scaling factor over computed weight
    int         mSoftBin;               // Holds soft bin
    int         mHardBin;               // Holds hard bin
    QColor      mFailBinColor;          // Holds color
};

// Holds one Mask rule potato: defines area over which rules apply
class	CMask_Rule
{
public:
    CMask_Rule();						// Constructor
    ~CMask_Rule();						// Destructor

    bool		mIsEnabled;				// Rule enabled/disabled
    QString		mRuleName;              // Holds Mask name
    int			mWorkingArea;			// 0=Outer Ring, 1 = Inner Ring
    int			mRadius;				// Ring radius width
};

#endif // PAT_RULES_H
