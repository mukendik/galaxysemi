#ifndef PAT_OPTIONS_H
#define PAT_OPTIONS_H

#include "pat_defines.h"
#include "pat_global.h"
#include "pat_option_reticle.h"


#include <QObject>
#include <QColor>
#include <QMap>

#include <pat_rules.h>
#include <gqtl_utils.h>

namespace GS
{
namespace QtLib
{
    class Range;
}
namespace Gex
{
    class PATRecipeIO;
}
}

//class	CNNR_Rule;
class	CIDDQ_Delta_Rule;
class	CGDBN_Rule;
class	CClusterPotatoRule;
class	CMask_Rule;

// Global Options (section [Outlier_Options] in Outlier config file)
class COptionsPat : public QObject
{
    Q_OBJECT

public:
    //! \brief Constructor
    COptionsPat(QObject *parent);
    //! \brief Destructor
    virtual ~COptionsPat();

    GS::Gex::PAT::RecipeType    GetRecipeType() const;
    QString                     GetRecipeVersion() const;
    int                         GetPATSoftBinFailType(int lBin) const;
    int                         GetPATHardBinFailType(int lBin) const;
    void                        SetRecipeType(GS::Gex::PAT::RecipeType lType);
    void                        SetRecipeVersion(const QString &);

    enum GexDefaultRuleSet { n_sigma, n_robust_sigma, q1_q3_idq, percent_of_limits, new_limits, range, smart_adaptive,
        disabled };

    // Todo : move me to properties
    // Product name
    QString	strProductName;				// Product to which the configuration file applies.
    // GUI globals.
    int	iRecipeVersion;				// Recipe version (-1 if disabled) the "X" in Version X.Y
    int	iRecipeBuild;				// Recipe Build (-1 if disabled) the "Y" in Version X.Y
    // false if disabled
    bool bStaticPAT;
    // false if disabled
    bool bDynamicPAT;
    // SoftBin to assign if fail static PAT limits...unless customized on a per test basis
    int	iFailStatic_SBin;
    // HardBin to assign if fail static PAT limits...unless customized on a per test basis
    int	iFailStatic_HBin;
    int	iFailDynamic_SBin;			// SoftBin to assign if fail static PAT limits...unless customized on a per test basis
    int	iFailDynamic_HBin;			// HardBin to assign if fail static PAT limits...unless customized on a per test basis
    bool bCustomPatLib;				// 'true' if custom PAT library/Dll enabled
    QString	strCustomPatLibName;		// Hold logical PAT lib to use.
    QString	strCustomPatLibFile;		// Hold physical PAT lib to use.
    QString	strCustomPatLibRevision;	// Hold PAT lib Build# selected by user
    GS::QtLib::Range	*pGoodSoftBinsList;		// List of good SOFT bins (eg: 1,99)
    GS::QtLib::Range	*pGoodHardBinsList;		// List of good HARD bins (eg: 1,99)
    int		mOptionsTestKey;					// Define how tests are identified: based on test#, test name, both, etc...
    int		mTestKey;                           // Define how tests are identified: based on test#, test name, both, etc...
    QString	strPatLimitsFromBin;		// List of bins from which samples are used to build the PAT limits.
    int		iPatLimitsFromBin;			// Tells if list of bins or except-bins...
    long	iMinimumSamples;			// Minimum samples required in a test to check for outliers.
    long	iMinimumOutliersPerPart;	// Minimum outliers to detect in a part to issue a Outlier binning
    bool	bIgnoreIQR0;				// 'true' if have to ignore PAT static limits for tests with Robust Sigma IQR = 0
    bool	bStopOnFirstFail;			// 'true' if stop identifying/counting PAt failures once first failure in test flow is found
    double	lfIgnoreHistoricalCpk;		// >=0 if ignore static PAT for tests with Cpk >= xxx
    bool	bStickWithinSpecLimits;		// 'true' if PAT limits forced to remain within spec. limits
    bool	bIgnoreHistoricalCategories;// 'true' if ignore static PAT for tests with historical distribution shape = Categories
    bool	bScanGoodPartsOnly;	// 'true' if search for outliers in Good parts only (Bin1)

    // 'Category' distribution options
    bool	bAssumeIntegerCategory;		// true = Assume integer-only distributions to be categories
    int		iCategoryValueCount;		// 'N' classes or less makes a distribution a 'Category'

    // WAFER SORT Options
    QStringList strFlowNames;	// List of flow names (eg: EWS1,EWS2,...VI,...)
    QStringList strFlowComments;		// List of flow name comments
    QStringList strFlowKillSTDF_SBIN;	// Kill STDF SBINs (SBIN dies to remove from map)
    QStringList strFlowKillSTDF_HBIN;	// Kill STDF HBINs (HBIN dies to remove from map)
    QStringList strFlowKillMAP_BIN;		// Kill MAP BINs (BIN dies to remove from map)
    QStringList strRulePrecedence;		// List in which order rules to be executed

//    CRecipeFlow	cWorkingFlow;			// PAT processing allows running multiple flows, but only one flow at a time. Here we store the flow details
    int iUpdateMap_FlowNameID;			// Flow name for UpdateMap

    // IDDQ Delta: Identify outliers in the IDDQ Delta (postStress-PreStress) dataset
    bool	mIsIDDQ_Delta_enabled;		// true if IDDQ Delta enabled
    int		mIDDQ_Delta_SBin;			// SoftBin to assign to good parts forced to fail because of IDDQ Delta algorithm
    int		mIDDQ_Delta_HBin;			// HardBin to assign to good parts forced to fail because of IDDQ Delta algorithm
    QColor	mIDDQ_Delta_Color;			// Color for the fail bin due to IDDQ-Delta
    QList <CIDDQ_Delta_Rule> mIDDQ_Delta_Rules;	// IDDQ-Delta rules

    // Geographic PAT must include PPAT bins
    bool	mGPAT_IgnorePPatBins;		// Default: true
    // GDBN: Bad neighbors/clustering analysis
    bool	mIsGDBNEnabled;			// false if disabled
    int		mGDBNPatSBin;			// SoftBin to assign to good parts forced to fail because of bad clustere (too many bad dies around)
    int		mGDBNPatHBin;			// HardBin to assign to good parts forced to fail because of bad clustere (too many bad dies around)
    QColor	mGDBNColor;			// Color for the fail bin due to Bad cluster
    bool	mGDBNCustomLib;		// 'true' if custom GDBN PAT lib enabled.
    QList <CGDBN_Rule> mGDBNRules;	// Clustering rules

    // Clustering PotatoColor
    bool	mClusteringPotato;						// True if Clustering Potato enabled
    int		mClusteringPotatoSBin;					// SoftBin to assign to good parts on the edge of the potato bad cluster
    int		mClusteringPotatoHBin;					// HardBin to assign to good parts on the edge of the potato bad cluster
    QColor	mClusteringPotatoColor;					// Color for the fail bin due to Bad cluster Potato
    QList <CClusterPotatoRule> mClusterPotatoRules;	// Clustering rules
    // Mask rules
    QList <CMask_Rule *> mMaskRules;					// Mask rules

    ///////////////////////////////////////////
    // ZPAT: COMPOSITE LOT-WAFER Options
    ///////////////////////////////////////////
    GS::QtLib::Range	*pBadBinsZPAT_List;					// List of Bad bins to consider when computing ZPAT yield.
    QColor	cZpatColor;								// Color used for Z-Pat bins
    bool	bZPAT_SoftBin;							// =true if ZPAT working over SoftBin; false if over HardBin
    double	lfCompositeExclusionZoneYieldThreshold;	// Yield threshold under which Good bins are failed
    int		iCompositeZone_SBin;						// Soft: Failing bin associated.
    int		iCompositeZone_HBin;						// Hard: Failing bin associated.
    bool	b3DNeighbourhoodEnabled;				// True if 3D-bad neighbors enabled.
    bool	bMergeEtestStdf;						// True if merge Etest & STDF data (overload STDF data).
    int		iCompositeEtestStdf_SBin;				// SBin: Failing bin associated with GoodSTDF bin that are rejects in E-test file.
    int		iCompositeEtestStdf_HBin;				// HBin: Failing bin associated with GoodSTDF bin that are rejects in E-test file.
    bool	bZPAT_GDBN_Enabled;						// True GDBN rules to be executed over the composite exclusion map.
    bool	bZPAT_Reticle_Enabled;					// True Reticle rules to be executed over the composite exclusion map.
    bool	bZPAT_Clustering_Enabled;				// True Clustering rules to be executed over the composite exclusion map.

    // FINAL TEST Options
    int		mFT_BaseLine;				// Total parts in base line
    int     mFT_SubsequentBL;           // Total parts required to execute the subsequent baseline per site
    int     mFT_BaseLineAlgo;           // Type of algorithm to use to build the baseline
    int     mFT_MinSamplesPerSite;      // Minimum samples per site to build the baseline when using merged site algorithm
    int		mFT_RunsPerPacket;			// Number of test program runs per packet sent by tester to GTM server
    int		mFT_Traceability;			// Traceability mode: Disabled, STDF (save PAT limits in STDF file), or ASCII
    int		mFT_BaseLineMaxOutliers;	// Alarm level on outliers detected in baseline.
    bool    mFT_TuningIsEnabled;        // Enable / Disable Dynamic PAT limits tuning
    int		mFT_Tuning;					// Dynamic PAT limits tuning frequency (0 = Never tune)
    int		mFT_TuningType;				// Tuning type: (0) = Every N parts, (1) = Every N outliers detected
    int     mFT_TuningSamples;          // Nb or samples to keep
    int		mFT_MaxOutlierParts;		// Maximum outliers allowed in a lot
    int		mFT_YieldLevel;				// Minimum good yield expected
    int		mFT_CriticalBinsYield;		// Maximum yield loss on critical Bins
    GS::QtLib::Range	*mFT_CriticalBinsList;	// List of critical bins
    QString m_FT_AlarmType;

    // Final test: Alarm timeouts
    // 7103 moved in properties:  AlarmSeverityOutliers, ...
    // int iFT_Alarm_Timeout_Outliers;	// timeout if Too many outliers in base line
    // warning : these define will be the one to be in the recipe !!!
    #define ALARM_SEVERITY_TOO_MUCH_OUTLIERS_IN_BASELINE  "ft_alarm_severity_outliers"
    // int iFT_Alarm_Timeout_Shapes;	// timeout if Distribution shape changed between Baseline & Historical data
    #define ALARM_SEVERITY_DISTRIB_SHAPE_CHANGED "ft_alarm_severity_shapes"
    // int iFT_Alarm_Timeout_PatDrift;	// timeout if Dynamic Vs Static PAT limits drift too high
    #define ALARM_SEVERITY_PAT_DRIFT "ft_alarm_severity_patdrift"
    //int iFT_Alarm_Timeout_LowGoodYield;// timeout if good bin yield too low
    #define ALARM_SEVERITY_LOW_GOOD_YIELD "ft_alarm_severity_lowgoodyield"
    // int	iFT_Alarm_Timeout_YieldLoss;// timeout if Too much yield loss on critical bins
    #define ALARM_SEVERITY_YIELD_LOSS "ft_alarm_severity_yieldloss"
    // int iFT_Alarm_Timeout_ParamDrift;// timeout if parameter drift
    #define ALARM_SEVERITY_PARAM_DRIFT "ft_alarm_severity_paramdrift"
    // int iFT_Alarm_Timeout_ParamCpk;	// timeout if parameter Cpk too low
    #define ALARM_SEVERITY_PARAM_CPK "ft_alarm_severity_paramcpk"

    // Final test: Alarm emails
    int		iFT_Alarm_Email_Outliers;	// =1 if send email if Too many outliers in base line
    int		iFT_Alarm_Email_Shapes;		// 1 if send email if Distribution shape changed between Baseline & Historical data
    int		iFT_Alarm_Email_PatDrift;	// 1 if send email if Dynamic Vs Static PAT limits drift too high
    int		iFT_Alarm_Email_LowGoodYield;// 1 if send email if good bin yield too low
    int		iFT_Alarm_Email_YieldLoss;	// 1 if send email if Too much yield loss on critical bins
    int		iFT_Alarm_Email_ParamDrift;	// 1 if send email if parameter drift
    int		iFT_Alarm_Email_ParamCpk;	// 1 if send email if parameter Cpk too low
    // Report Options
    bool	bReport_Stats;					// PAT report include Stats for failing tests.
    bool	bReport_Histo;					// PAT report include Histogram for failing tests.
    // PAT-71: PAT report include Histogram tests with no outliers
    bool	bReport_Histo_NoOutliers;
    bool	bReport_Wafermap;				// PAT report include Wafermap
    int		iReport_WafermapType;			// PAT report: Soft-Bin or HardBin wafermap
    bool	bReport_Pareto;					// PAT report include Pareto
    bool	bReport_Binning;				// PAT report include Binning
    bool	bReport_SPAT_Limits;			// PAT report includes SPAT limits computed
    bool	bReport_DPAT_Limits_Outliers;	// PAT report lists DPAT limits for tests with outliers
    bool	bReport_DPAT_Limits_NoOutliers;	// PAT report lists DPAT limits for tests without outlier

    // STDF Output (allows enabling/disabling STDF generation)
    int		iSTDF_Output;					// (GEX_PAT_STDF_OUTPUT_xxx) : -1 = Use default, 0=Disabled, 1=Wafermap only, 2=Full STDF

    // Emails
    QString	strFT_MailingList;			// List of emails / mailing list for alarm notifications.
    int		mFT_EmailFormat;			// 0 if emails to be sent in HTML format, 1 if TXT format

    // Colors
    QColor	cStaticFailColor;	// Color for the Static PAT bin failure
    QColor	cDynamicFailColor;	// Color for the Static PAT bin failure
    // Smart mode options
    double	lfSmart_IgnoreHighCpk;												// Ignore test if its Cpk >= to this value. If 'lfIgnoreHighCpk' < 0, option is disabled.
    bool	mSmart_IgnoreHighCpkEnabled; // Enable/disable the Smart_IgnoreHighCpk parameter
    double	lfSmart_HeadGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for Gaussian distribution
    double	lfSmart_TailGaussian[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for Gaussian distribution
    bool	bPAT_Gaussian;															// 'false' if disabled PAT on gaussian distributions.
    int		iAlgo_Gaussian;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// -H*Sigma limit for Gaussian distribution+tailed
    double	lfSmart_TailGaussianTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// +T*Sigma limit for Gaussian distribution+tailed
    bool	bPAT_GaussianTailed;													// 'false' if disabled PAT on Tailed-gaussian distributions.
    int		iAlgo_GaussianTailed;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// -H*Sigma limit for Gaussian distribution+tailed
    double	lfSmart_TailGaussianDoubleTailed[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// +T*Sigma limit for Gaussian distribution+tailed
    bool	bPAT_GaussianDoubleTailed;											// 'false' if disabled PAT on double-tailed-gaussian distributions.
    int		iAlgo_GaussianDoubleTailed;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for LogNormal distribution
    double	lfSmart_TailLogNormal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for LogNormal distribution
    bool	bPAT_LogNormal;														// 'false' if disabled PAT on LogNormal distributions.
    int		iAlgo_LogNormal;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for Multi-Modal distribution
    double	lfSmart_TailMultiModal[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for Multi-Modal distribution
    bool	bPAT_MultiModal;													// 'false' if disabled PAT on MultiModal distributions.
    int		iAlgo_MultiModal;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for Clamped distribution
    double	lfSmart_TailClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for Clamped distribution
    bool	bPAT_Clamped;														// 'false' if disabled PAT on Clamped distributions.
    int		iAlgo_Clamped;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// -H*Sigma limit for Double Clamped distribution
    double	lfSmart_TailDoubleClamped[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];	// +T*Sigma limit for Double Clamped distribution
    bool	bPAT_DoubleClamped;													// 'false' if disabled PAT on DoubleClamped distributions.
    int		iAlgo_DoubleClamped;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for Category distribution
    double	lfSmart_TailCategory[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for Category distribution
    bool	bPAT_Category;														// 'false' if disabled PAT on Category distributions.
    int		iAlgo_Category;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    double	lfSmart_HeadUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// -H*Sigma limit for Unknown distribution
    double	lfSmart_TailUnknown[GEX_TPAT_OUTLIER_SEVERITY_LIMIT_CLASSES];		// +T*Sigma limit for Unknown distribution
    bool	bPAT_Unknown;														// 'false' if disabled PAT on Unknown distributions.
    int		iAlgo_Unknown;	// Type of algo for limits: Mean+N*Sigma; MEdian+N*Robust sigma, etc...
    int     mMinConfThreshold; // Below that threshold of confidence a shape is recognized as unknown

    void        SetDefaultDynamicRule(const GexDefaultRuleSet defaultDynamicRule);
    void        SetDefaultFactor(const double defaultFactor);
    void        SetExclusionZoneEnabled(const bool lEnable);
    void        SetMVPATEnabled(const bool lEnable);
    void        SetMVPATIgnorePPATBins(bool lIgnore);
    void        SetMVPATAutomaticGroupCreation(bool lAutomatic);
    void        SetMVPATGroupCorrelation(double lCorrelation);
    void        SetMVPATNormalShapeOnly(bool lNormalOnly);
    void        SetMVPATSoftBin(int lSoftBin);
    void        SetMVPATHardBin(int lHardBin);
    void        SetMVPATColor(const QColor& lColor);
    void        SetMVPATDistance(GS::Gex::PAT::OutlierDistance lDistance, double lValue);
    void        SetMVPATReportStdCharts(bool lEnabled);
    void        SetMVPATReportCorrCharts(bool lEnabled);
    void        SetMVPATReportPCAProjection(bool lEnabled);
    void        SetMVPATReportPairs(GS::Gex::PAT::MVPairs lMode);
    void        SetMVPATReportMaxCharts(int lMaxCharts);
    void        SetEnableYALimit(const bool enabled);
    void        SetOveralPatYALimit(const double YALimit);
    void        SetReticleSoftBin(int softBin);
    void        SetReticleHardBin(int hardBin);
    void        SetReticleColor(const QColor& color);
    void        SetReticleEnabled(bool isEnabled);
    void        SetReticleSizeX(int sizeX);
    void        SetReticleSizeY(int sizeY);
    void        SetReticleSizeSource(PATOptionReticle::ReticleSizeSource source);
    void        SetNNRSoftBin(int softBin);
    void        SetNNRHardBin(int hardBin);
    void        SetNNRColor(const QColor& color);
    void        SetNNREnabled(bool isEnabled);

    /*!
      @brief    Sets the property to merge or split the sites during PAT processing

      @param    lAllSitesMerged     True to force DPAT to be done on merged sites
      */
    void                    SetAllSitesMerged(bool lAllSitesMerged);

    GexDefaultRuleSet       GetDefaultDynamicRule() const;
    double                  GetDefaultFactor() const;
    bool                    GetExclusionZoneEnabled() const;
    bool                    GetMVPATEnabled() const;
    bool                    GetMVPATIgnorePPATBins() const;
    bool                    GetMVPATAutomaticGroupCreation() const;
    double                  GetMVPATGroupCorrelation() const;
    bool                    GetMVPATNormalShapeOnly() const;
    int                     GetMVPATSoftBin() const;
    int                     GetMVPATHardBin() const;
    const QColor&           GetMVPATColor() const;
    double                  GetMVPATDistance(GS::Gex::PAT::OutlierDistance lDistance) const;
    bool                    GetMVPATReportStdCharts() const;
    bool                    GetMVPATReportCorrCharts() const;
    bool                    GetMVPATReportPCAProjection() const;
    GS::Gex::PAT::MVPairs   GetMVPATReportPairs() const;
    int                     GetMVPATReportMaxCharts() const;
    bool                    GetEnableYALimit() const;
    double                  GetOveralPatYALimit() const;
    QList<PATOptionReticle>&        GetReticleRules()       { return mReticleRules;}
    const QList<PATOptionReticle>&  GetReticleRules() const { return mReticleRules;}
    int                             GetReticleSoftBin() const;
    int                             GetReticleHardBin() const;
    const QColor&                   GetReticleColor() const;
    bool                            GetReticleEnabled() const;
    int                                 GetReticleSizeX() const;
    int                                 GetReticleSizeY() const;
    PATOptionReticle::ReticleSizeSource GetReticleSizeSource() const;

    QList<CNNR_Rule>&       GetNNRRules()       { return mNNRRules;}
    const QList<CNNR_Rule>& GetNNRRules() const { return mNNRRules;}
    int                     FindNNRRuleByName(const QString& name) const;
    int                     GetNNRSoftBin() const;
    int                     GetNNRHardBin() const;
    const QColor&           GetNNRColor() const;
    bool                    IsNNREnabled() const;

    /*!
      @brief    Indicates whether all sites must be merged together or splitted

      @return   true if the DPAT processing must be done with all sites merged
      */
    bool                    GetAllSitesMerged() const;

    //! \brief Clear internal variables
    void                    clear(GS::Gex::PAT::RecipeType lRecipeType);
    //! \brief Resets Rules precedence/Ordering to default.
    void                    resetRulePrecedence(void);

private:

    friend class GS::Gex::PATRecipeIO;

    GS::Gex::PAT::RecipeType    mRecipeType;
    QMap<int, int>              mPATSoftBinFailType;    // Holds the outlier failure type for each PAT soft bin.
    QMap<int, int>              mPATHardBinFailType;    // Holds the outlier failure type for each PAT soft bin.
    QString                     mRecipeVersion;
    bool                        mExclusionZonesEnabled;
    bool                        mMVPATEnabled;
    bool                        mMVPATAutomaticGroupCreation;
    bool                        mMVPATIgnorePPATBins;
    double                      mMVPATGroupCorrelation;
    bool                        mMVPATNormalShapeOnly;
    int                         mMVPATSoftBin;
    int                         mMVPATHardBin;
    QColor                      mMVPATColor;
    bool                        mMVPATReportStdCharts;
    bool                        mMVPATReportCorrCharts;
    bool                        mMVPATReportPCAProjection;
    GS::Gex::PAT::MVPairs       mMVPATReportPairs;
    int                         mMVPATReportMaxCharts;
    QMap<GS::Gex::PAT::OutlierDistance, double>  mMVPATDistance;
    GexDefaultRuleSet           mDefaultDynamicRule;                     /// \param the default value for all dynamic rules
    double                      mDefaultFactor;

    bool mEnableYALimit;                ///\param true if the pat yield alarm is enabled
    double mOveralPatYALimit;           ///\param The value of overal limit used in the yield alarm

    bool                        mAllSitesMerged;
    bool                        mReticleEnabled;
    int                         mReticleSoftBin;
    int                         mReticleHardBin;
    QColor                      mReticleColor;
    QList<PATOptionReticle>     mReticleRules;      // Reticle rules
    int                                 mReticleSizeX;			///< Number of Dies in X on the reticle
    int                                 mReticleSizeY;			///< Number of Dies in Y on the reticle
    PATOptionReticle::ReticleSizeSource mReticleSizeSource;     ///< holds reticle size source

    // NNR: Nearest Neighbor Residual, identify parametric outliers within geographic clusters
    bool                        mNNRIsEnabled;			// true if NNR enabled
    int                         mNNRSBin;				// SoftBin to assign to good parts forced to fail because of NNR algorithm
    int                         mNNRHBin;				// HardBin to assign to good parts forced to fail because of NNR algorithm
    QColor                      mNNRColor;				// Color for the fail bin due to NNR
    QList <CNNR_Rule>           mNNRRules;              // NNR rules

};

#endif // PAT_OPTIONS_H
