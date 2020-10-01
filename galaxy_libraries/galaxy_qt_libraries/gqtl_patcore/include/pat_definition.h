#ifndef PAT_DEFINITION_H
#define PAT_DEFINITION_H

#include "pat_defines.h"
#include "pat_dynamic_limits.h"

#include <QMap>
#include <QMutex>
#include <QString>

class CTest;
class CPatInfo;
class COptionsPat;

class CPatDefinition
{
    static unsigned sTotalNumOfInstances;

    QMutex mMutex;

public:
    //! \brief Constructor : calls clear()
    CPatDefinition();
    //! \brief Copy constructor
    CPatDefinition(const CPatDefinition &);
    ~CPatDefinition() { sTotalNumOfInstances--; }

    static bool SortOnTestNumber(const CPatDefinition*, const CPatDefinition*);

    //! \brief Returns the current number of instances of this class
    static unsigned GetNumOfInstances() { return sTotalNumOfInstances; }
    //! \brief Clear internal variables
    void clear(void);
    //! \brief Returns 'true' if test is disables from all PAT & Monitoring options
    bool IsTestDisabled(void);
    //! \brief Retrieves dynamic test limits for a given testing site,
    //! copy into 'm_lfLowDynamicLimit1/2 & m_lfHighDynamicLimit1/2'
    GS::PAT::DynamicLimits	GetDynamicLimits(int iSite) const;
    //! \brief Commpute static PAT (SPAT) limits based on historical data.
    //! If provided (samples >= 33), the limits will be centered using mean/median from current dataset.
    bool ComputeStaticLimits(const int Samples=0, const double Mean=0.0, const double Median=0.0);
    //! \brief Check static PAT (SPAT) limits compared to test limits.
    //! If the option is set that SPAT limits cannot exceed test limits, make adjustements.
    bool CheckStaticLimits(const COptionsPat & PatOptions);

    //! \brief Saves dynamic test limits for a given testing site, copy into '
    void SetDynamicLimits(int lSite, GS::PAT::DynamicLimits &lDynLimits, const COptionsPat &lOptions,
                             unsigned char lLimitsFlag, double lLowLimit, double lHighLimit);

    //! \brief Compute Failure severity (based on mixing count of outliers & type)
    void computeSeverity(void);
    //! \brief Returns list of sites having for Dynamic Limits computed
    QList<int>  GetDynamicSites() const;

    //Test Type Enum
    enum TestType
    {
        ParametricTest,
        MultiParametricTest,
        FunctionalTest,
        UnknownTest,
        InvalidTest
    };
    //! \brief Getter
    TestType GetTestType() const;
    //! \brief Setter
    void SetTestType(const TestType &testType);
    //! \brief Getter that return mTestType attribute in the legacy mode returns 'P', 'F', 'M' ....
    char GetTestTypeLegacy() const;
    //! \brief Setter that set mTestType attribute in the legacy mode 'P', 'F', 'M' ....
    void SetTestTypeLegacy(const char &);
    //! \brief Pinmap setter
    void SetPinmapNumber(long );

    // Test sequence ID in execution flow (0 = 1st test executed in flow, 2= 2nd test in flow, etc...)
    long	m_lSequenceID;
    long	m_lFailStaticBin;
    long	m_lFailDynamicBin;
    unsigned long	m_lTestNumber;
    long    mPinIndex;
    QString	m_strTestName;
    QString	m_strUnits;
    double	m_lfLowLimit;
    double	m_lfHighLimit;
    int		m_llm_scal;		// Low limit display scale factor (used when exporting SPAT limits
    int		m_hlm_scal;		// High limit display scale factor (used when exporting SPAT limits
    double	m_lfMedian;
    double	m_lfRobustSigma;
    double	m_lfMean;
    double	m_lfSigma;
    double	m_lfRange;
    double	m_lfLowStaticLimit;
    double	m_lfHighStaticLimit;
    int		m_iTailDirection;	// holds tail direction (NONE, RIGHT or LEFT)
    int		m_iDistributionShape;	// Distribution shape: 0, 1, 2,...(eg: PATMAN_LIB_SHAPE_DOUBLECLAMPED, etc...)
    QMap<int, GS::PAT::DynamicLimits> mDynamicLimits;	// Two sets of dynamic limits per testing site.

    double m_lHistoricalCPK; // Historical Cpk

    // Tells if there are type of outliers to keep: none, low values or high values (GEX_TPAT_KEEPTYPE_XXX
    int		m_OutliersToKeep;
    // Tells if consider all data samples in distribution analysis, or ignore some data when computing PAT limits (eg: ignore negative values, etc...)
    int		m_SamplesToIgnore;
    int		m_iOutlierLimitsSet;// Tells the outlier limits set to use: 'near outliers & higher', 'medium & higher' 'far outliers only'
    int		m_SPATRuleType;	// GEX_SPAT_RULETYPE_XXX)
    double	m_lfSpatOutlierNFactor;	// SPAT: Outlier space (or Head limit). eg: 6 for +/- 6xSigma, etc...
    double	m_lfSpatOutlierTFactor;	// SPAT: Outlier Tail space: E.g: +9 Sigma.
    int		mOutlierRule;	// Holds outlier rule to apply - template: GEX_TPAT_RULETYPE_XXX
    int     mComputedOutlierRule;  // Holds rule computed with smart and adaptive - template: GEX_TPAT_RULETYPE_XXX
    int		mTailMngtRuleType;	// Tail management rule
    double	m_lfOutlierNFactor;	// Outlier space (or Head limit). eg: 6 for +/- 6xSigma, etc...
    double	m_lfOutlierTFactor;	// Outlier Tail space: E.g: +9 Sigma.
    // For ALL Parts: Holds statistics (number of failures from STATIC pats, and Dynamic pats)
    long	m_lStaticFailuresLow_AllParts;
    long	m_lStaticFailuresHigh_AllParts;
    long	m_lDynamicFailuresLow_AllParts[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    long	m_lDynamicFailuresHigh_AllParts[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    long	m_TotalFailures_AllParts;	// Cumul of all failures (low/high Static & dynamic limits)
    // For Good Parts: Holds statistics (number of failures from STATIC pats, and Dynamic pats)
    long	m_lStaticFailuresLow;
    long	m_lStaticFailuresHigh;
    long	m_lDynamicFailuresLow[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    long	m_lDynamicFailuresHigh[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];
    QMap<unsigned,unsigned> m_DynamicFailCountPerSite;	// Holds DPAT fail count per site.
    long	m_TotalFailures;	// Cumul of all failures (low/high Static & dynamic limits)
    long	m_lSeverityScore;
    QMap<unsigned,unsigned> m_NewPTR_PerSite;	// Flag used to know if PTR of new STDF file needs to include the new limits

    // NNR rule
    int		m_iNrrRule;		// NNR rule type (0=enabled,1=disabled)

    // SPC variables
    double	m_SPC_PatMedianDriftAlarm;		// Static Vs Dynamic PAT-Median drift limit ( positive value if valid, negative value if disabled)
    int		m_SPC_PatMedianDriftAlarmUnits;	// PAT-Median Drift units (% of limits, space, etc....)
    double	m_SPC_TestMeanDriftAlarm;		// Test Mean drift limit ( positive value if valid, negative value if disabled)
    int		m_SPC_TestMeanDriftAlarmUnits;	// Test Mean Drift units (% of limits, space, etc....)
    double	m_SPC_CpkAlarm;		// Cpk alarm threshold level
    QString mSPATNote,mDPATNote;
private:
    TestType mTestType;
    static QMap<TestType, char> sTestTypeLegacyMapping;
};

#endif // PAT_DEFINITION_H
