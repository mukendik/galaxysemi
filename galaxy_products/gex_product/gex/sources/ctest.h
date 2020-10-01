#if !defined(GEX_CTEST_H__INCLUDED_)
#define GEX_CTEST_H__INCLUDED_

//////////////////////////////////////////////////////
// Test cell
//////////////////////////////////////////////////////
#include <qlist.h>
#include <qmap.h>
#include <qstring.h>

#include "ctest_chart_options.h"
#include "test_marker.h"
#include "test_result.h"
#include "test_defines.h"
#include "multi_limit_item.h"
#include "histogramdata.h"
#include "gstdl_type.h"
#include "subset_limits.h"


//typedef unsigned char	BYTE;
//typedef unsigned short	WORD;
//typedef unsigned long	DWORD;
//typedef float			FLOAT;
//typedef int				BOOL;

class CPinmap;
class GexXBarRDataset;

class	CGageR_R
{
public:
    CGageR_R();		// Constructor

    double	lfEV;	// Equipment Variation
    double	lfAV;	// Appraiser Variation
    double	lfRR;	// Gage R&R
    double	lfRR_percent;	// Gage R&R% (% of limits space)
    double	lfGB;	// Guard Banding = %RNR*tolerance
    double	lfPV;	// Part Variation
    double	lfTV;	// Total Variation
    double	lfP_T;	// Ratio P/T
};


class GexVectorFailureInfo
{
public:

    GexVectorFailureInfo(uint uiCycleCount, uint uiRelativeVAddress)
        : m_uiVectorCycleCount(uiCycleCount), m_uiRelativeVectorAddress(uiRelativeVAddress)
    {}

    GexVectorFailureInfo(const GexVectorFailureInfo& other)
    {
        *this = other;
    }

    GexVectorFailureInfo& operator=(const GexVectorFailureInfo& other)
    {
        if (this != &other)
        {
            m_uiVectorCycleCount		= other.m_uiVectorCycleCount;
            m_uiRelativeVectorAddress	= other.m_uiRelativeVectorAddress;
            m_lstPinmap					= other.m_lstPinmap;
        }

        return *this;
    }

    void			addPinmap(CPinmap * pPinmap)		{ m_lstPinmap.append(pPinmap); }

    uint			vectorCycleCount() const			{ return m_uiVectorCycleCount; }
    uint			relativeVectorAddress() const		{ return m_uiRelativeVectorAddress; }
    int				pinmapCount() const					{ return m_lstPinmap.count(); }
    CPinmap *		pinmapAt(uint uiIndex) const		{ return m_lstPinmap[uiIndex]; }

private:

    uint			m_uiVectorCycleCount;
    uint			m_uiRelativeVectorAddress;
    QList<CPinmap*>	m_lstPinmap;
};

class	CFunctionalTest
{
public:
    CFunctionalTest();		// Constructor

    QString						strVectorName;		// Vector name
    long						lFails;				// Total failures in vector
    long						lExecs;				// Total execution of this vector

    QList<GexVectorFailureInfo>	m_lstVectorInfo;	// Detailed information about failure on vector
};

struct TestShift
{
    TestShift()
    {
        mRefGroup = mComparedGroup = "";
        mSigmaShiftPercent = mSigmaShiftValue = mMeanShiftValue = 0;
        mIsValid=false;
    }

    void SetGroups(const QString&refGroupName, const QString& comparedGroupName)
    {
        mRefGroup = refGroupName;
        mComparedGroup = comparedGroupName;
    }

    GS::Core::MLShift GetMlShift(GS::Core::MultiLimitItem* mlItem)
    {
        for (int lIdx = 0; lIdx < mMLShifts.size(); ++ lIdx)
        {
            if (mMLShifts.at(lIdx).first == mlItem)
                return mMLShifts.at(lIdx).second;
        }
        GS::Core::MLShift lMLimitShift;
        mMLShifts.append(QPair<GS::Core::MultiLimitItem*, GS::Core::MLShift>
                                                (mlItem, lMLimitShift));
        return lMLimitShift;
    }

    void SetMlShift(GS::Core::MultiLimitItem* mlItem, const GS::Core::MLShift& mlShift)
    {
        for (int lIdx = 0; lIdx < mMLShifts.size(); ++ lIdx)
        {
            if (mMLShifts.at(lIdx).first == mlItem)
            {
                mMLShifts[lIdx].second = mlShift;
                return;
            }
        }

        mMLShifts.append(QPair<GS::Core::MultiLimitItem*, GS::Core::MLShift>
                                                (mlItem, mlShift));
    }

    ~TestShift()
    {
        mMLShifts.clear();
    }

    void SetValidity(bool aValue)  {mIsValid = aValue;}
    bool IsValid() const  {return mIsValid;}

    QString mRefGroup;
    QString mComparedGroup;
    double	mSigmaShiftValue;
    double	mMeanShiftValue;
    double  mSigmaShiftPercent;
    bool    mIsValid;
    QList< QPair<GS::Core::MultiLimitItem*, GS::Core::MLShift> > mMLShifts;
};

/*!
 \class CTest
 \brief Structure to hold test's info & results
*/
class CTest : public QObject
{
    Q_OBJECT
public slots:
    /*!
     * \fn GetTestName
     */
    QString GetTestName() const { return strTestName; }
    /*!
     * \fn GetTestNumber
     */
    QString GetTestNumber() const { return QString("%1").arg(lTestNumber); }
    /*!
     * \fn GetPinName
     */
    QString GetPinIndex() const { return QString("%1").arg(lPinmapIndex); }
    /*!
     * \fn GetPinPhysicName
     */
    QString GetPinPhysicName() const { return mPinPhysicName; }
    /*!
     * \fn GetPinLogicName
     */
    QString GetPinLogicName() const { return mPinLogicName; }
    /*!
     * \fn GetChannelName
     */
    QString GetPinChannelName() const { return mPinChannelName; }

public:
    CTest();
    virtual ~CTest();

    enum TestOrigin
    {
        OriginStdf,				// Based on stdf file
        OriginCustom			// Based on formula or calculation
    };

    // Add a count for this site to determine if we handle multi result
    void	addMultiResultCount(unsigned short site);

    // Clear the count by site: Must be done at the end of eacg part read
    void	clearMultiResultCount();
    bool	hasMultiResult() const				{ return m_cMaxCount > 1; }

    // Description	:	Scale test result to the most appropriate value
    void	executeSmartScaling();

    // Name			:	void findoutSmartScalingFactor()
    // Description	:	define the most appropriate scaling factor from min and max
    int		findoutSmartScalingFactor();		// define from min and max the most appropriate scaling factor

    void	uniformizePercentUnits();
    QMap<QString, QMap<uint,int> >  buildFunctionalHistoData(int iHistoType, QString strPattern);

    /*!
        \fn void QString CTest::GetScaledUnits(double* ptLfCustomScaleFactor);

        Returns the scaled units string
    */
    QString GetScaledUnits(double* pCustomScaleFactor, const QString & scalingOption);

    // Return the Nth valid result (skipping NaN results) from data array
    // Offset = 0 for 1st sample value, etc...
    // return -1 on failure, 1 on succces
    int		FindValidResult(long lIndex, double &lfValue);

    // Return array offset to Nth valid result (skipping NaN results)
    // Return array offset to Nth valid result (0....n) (skipping NaN results)
    long FindValidResultOffset(long lValidValueOccurante);

    // CTest: Check if value is PASS or FAIL
    bool isFailingValue(double lfValue, CTestResult::PassFailStatus ePassFailStatus, bool *ptLowFail=NULL, bool *ptHighFail=NULL);

    /*!
      * \fn IsMprMasterTest
      **/
    bool    IsMprMasterTest() const;

    // CTest: Return begin and end offset of a sublot into the test result array
    void	findSublotOffset(int& nBegin, int& nEnd, int nSublot) const;
    QString     generateTestKey(int lTestMergeRule);
    /*!
     * \fn SetUserTestName
     */
    void SetUserTestName(const QString& s) { mUserTestName = s; }
    /*!
     * \fn SetUserTestNumber
     */
    void SetUserTestNumber(const QString& s) { mUserTestNumber = s; }
    /*!
     * \fn SetUserPinName
     */
    void SetUserPinName(const QString& s) { mUserPinName = s; }
    /*!
     * \fn GetUserTestName
     */
    QString GetUserTestName() const { return mUserTestName; }
    /*!
     * \fn GetUserTestNumber
     */
    QString GetUserTestNumber() const { return mUserTestNumber; }
    /*!
     * \fn GetUserPinName
     */
    QString GetUserPinName() const { return mUserPinName; }
    /*!
     * \fn SetPinPhysicName
     */
    void SetPinPhysicName(const QString& s) { mPinPhysicName = s; }
    /*!
     * \fn SetPinLogicName
     */
    void SetPinLogicName(const QString& s) { mPinLogicName = s; }
    /*!
     * \fn SetPinChannelName
     */
    void SetPinChannelName(const QString& s) { mPinChannelName = s; }
    /*!
     * \fn GetDistribution
     */
    int GetDistribution() const;
    /*!
     * \fn SetDistribution
     */
    void SetDistribution(int distrib);
    /*!
     * \fn GetBimodalSplitValue
     */
    double GetBimodalSplitValue() const;
    /*!
     * \fn SetBimodalSplitValue
     */
    void SetBimodalSplitValue(double distrib);
    void    UpdateSampleFails(int site);
    const QMap<int, int>& GetTotalFailPerSite() const { return mTotalFailPerSite;}
public:
    /*!
     * \var lTestNumber
     */
    unsigned int lTestNumber;	// TestNumber
    int		lPinmapIndex;		// =GEX_PTEST (-1) for standard Parametric test
                                // =GEX_MPTEST (-2) for parametric multi-result master test
                                // >=0 for MultiParametric results test, tells result pinmap index
    int		lTestID;			// Test position in the test list (0,1,2,....).
    int		lTestFlowID;		// Test position in the testing flow (order of testing in test flow: 1,2,....).
    int		lResultArraySize;	// 0 if parametric test, >= 1 for master test record for parametric multi-result
    WORD*	ptResultArrayIndexes; // MPR: points to the list of pinmap indexes.
    WORD*	ptFirstResultArrayIndexes; // MPR: points to the list of pinmap indexes. first occurence
    bool	bTestInList;		// true if this test cell is par of the test list selected (with RAW data collected for specific HTML charting).
    long	lPartIDsArraySize;	// Keeps track of memory size allocated to store the PartIds.
    long	lFirstPartResult;	// First part# collected for Advanced charting
    long	lLastPartResult;	// Last part# collected for Advanced charting
    bool	bMergedTsrExists;	// Set to 'true' in pass1 if a merged TSR exsists for this test
    bool	bTestNameMapped;	// Set to 'true' if test name mapped through ascii mapping file
    double	lfTotal;			// Sum of X
    double	lfTotalSquare;		// Sum of X*X
    double	lfSamplesTotal;		// Sum of X (from samples)
    double	lfSamplesTotalSquare;// Sum of X*X (from samples)
    int		ldSamplesValidExecs;// Number of test valid results (only values to consider in statistics and charts)
    int		ldSamplesExecs;		// Number of test results (including invalid runs)
    QList<int> pSamplesInSublots;// List of Samples offset starting point for each sublot: used to draw Sublots markers in trend charts.
    int		ldCurrentSample;	// OBSOLETE MEMBER Not used anymore,since we hold the runIndex in the CPartBinning class except with MASA plugins - In pass2, keeps track of current test sample#.
    int		ldExecs;			// Number of executions
    bool	bOutlierLimitsSet;	// true once outlier limits are computer.
    double	lfMean;				// Mean Built after pass2...
    double	lfSigma;			// Sigma Built after pass2...
    QMap<QString, TestShift> mTestShifts; // Stores Test shift linked with Ref Group Name
    //QMap<QString, TestShift> mTestShifts;
    double	lfRange;			// Range Built after pass2...
    double	lfMaxRange;			// MAx range (between datasets) Built after pass2...
    double	lfMin;				// Min. of tested values :max(PTR,TSR)
    double	lfSamplesMin;		// Min. of values sampled (PTR) + limits if requested
    long	lSamplesMinCell;	// Offset to the cell that holds the 'Min' value of the samples.
    double	lfMax;				// Max. of tested values :max(PTR,TSR)
    double	lfSamplesMax;		// Max. of values sampled (PTR) + limits if requested
    long	lSamplesMaxCell;	// Offset to the cell that holds the 'Min' value of the samplles.
    double	lfStartIn;			// Multiparamteric: starting input value condition (for shmoo)
    double	lfIncrementIn;		// Multiparametric: increment of input condition (for shmoo)
    double	lfSamplesSkew;		// Skew (degree of asymmetry of a distribution around the mean)
    double	lfSamplesSkewWithoutNoise;		// Skew computed on data samples without the first X% and last X% (the noise) as defined with 'lfSamplesStartAfterNoise'
    double	lfSamplesKurt;		// Kurtosis or relative peakedness or flatness of a distribution compared with the normal distribution
    double	lfSamplesStartAfterNoise;	// Starting point after the first X% (norlmally 1%) if data, aliminated few samples of noise ! (used in PAT-Man
    double	lfSamplesEndBeforeNoise;	// Starting point after the first X% (norlmally 1%) if data, aliminated few samples of noise ! (used in PAT-Man
    double	lfSamplesP0_5;		// Data at percentile 0.5%
    double	lfSamplesP2_5;		// Data at percentile 2.5%
    double	lfSamplesP10;		// Data at percentile 10%
    double	lfSamplesQuartile1;	// Data at end of 25% of sorted data.
    double	lfSamplesQuartile2;	// Median: Data at end of 50%
    double	lfSamplesQuartile3;	// Data at end of 75% of sorted data.
    double	lfSamplesP90;		// Data at percentile 90%
    double	lfSamplesP97_5;		// Data at percentile 97.5%
    double	lfSamplesP99_5;		// Data at percentile 99.5%
    double	lfSamplesSigmaInterQuartiles;// Sigma of values from quartile 1 to Quartile 3 values.
    long	lDifferentValues;	// Holds the number of different values available in the dataset (used to detect distributions with only few distinct values)
    bool	bIntegerValues;		// 'true' if samples ar only integer values.
    double	lfSamplesDecatiles[10]; // Holds the 10 values of the 10 percentiles of the data.

    TestOrigin m_eOrigin;
    // Functional test
    QMap<QString,CFunctionalTest>	mVectors;	// Holds list of failing vectors & fail count.

    // Gage R&R (if computed)
    CGageR_R	*pGage;

    // temporary buffer for min/max values. updated at PRR time if part matches filter
    double	lfTmpSamplesTotal;	// Sum of X (including outliers)
    double	lfTmpSamplesTotalSquare; // Sum of X*X (including outliers)
    double	lfTmpHistogramMin;
    double	lfTmpSamplesMin;
    double	lfTmpHistogramMax;
    double	lfTmpSamplesMax;
    int		ldTmpSamplesExecs;
    int		iFailBin;		// Soft Bin associated with this test when failing.
    int		iAlarm;				// Bit Alarm flags: GEX_ALARM_MEANSHIFT, GEX_ALARM_SIGMASHIFT,...
    int		iHtmlStatsPage;		// Tells in which HTML page is the test Statistics
    int		iHtmlHistoPage;		// Tells in which HTML page is the test Histogram
    int		iHtmlWafermapPage;	// Tells in which HTML page is the test wafermap
    int		iHtmlAdvancedPage;	// Tells in which HTML page is the test Advanced chart (Trend, etc...)
    int		res_scal;			// Scale factor on test results
    BOOL		bTsrIncorrectScale; // true if SumX, and Sum(X*X) of TSR has wrong scaling...
    BYTE		bTestType;			// P=Parametric,F=Functionnal,M=Multiple-result parametric.
    BOOL		bStatsFromSamples;	// True if test stats coming from Samples, False if from summary.
    bool		bTestExecuted;		// Flag telling if part was in the run flow.
    QString strTestName;			// Test Name
    char	szTestUnits[GEX_UNITS];		// Test Units.
    char	szTestUnitsIn[GEX_UNITS];	// Input condition Test Units (Multi-parametric test)
    int	lHistogram[TEST_HISTOSIZE];	// To compute histogram data.
    // Arrays to store displayed info...avoids to rebuild them for each report page
    char	szTestLabel[GEX_TEST_LABEL];
    char	szLowSpecL[GEX_LIMIT_LABEL];
    char	szHighSpecL[GEX_LIMIT_LABEL];
    CGexTestChartingOptions *ptChartOptions;	// Allows to define custom charting options per test
    double	lfLowSpecLimit;		// Test Low Spec Limit
    double	lfHighSpecLimit;	// Test High Spec Limit
    int		llm_scal;			// Scale factor on LowLimit
    int		hlm_scal;			// Scale factor on HighLimit

    // Custom markers list
    QList<TestMarker*>	ptCustomMarkers;


    // Test result access method
    CTestResult		m_testResult;

    // Holds calculation for XBar-R data
    GexXBarRDataset *	m_pXbarRDataset;

    GS::Gex::HistogramData mHistogramData;

    static int GetNumberOfInstances() { return s_nInstances; }

    int     m_iHistoClasses;
    bool    m_bUseCustomBarNumber;
    QMap <QString, bool> m_oPatMarkerEnable;
    // update all properties from cache
    bool UpdateProperties();
    // Map to contain all simple members in order to access easily and in JS
    // to do : move mean, median, .....
    QMap <QString, QVariant > mProperties;

    double getT_Test();
    void   setT_Test(double dVal);
    double getP_Value() ;
    void   setP_Value(double dVal);
    // Check which Cpk to use...CpkH, CpkL, or min(CpkH,CpkL)
    // Depends if outliers on one side to ignore (in such case, ignore limit of that side too!)
//    double GetRelevantCpk(int lOutliersToKeep);

    /// \brief Get the list of rolling limits
    /// \return Map of subset limits
    inline GS::Gex::SubsetLimits& GetRollingLimits() {return mSubsetLimits;}

    /// \brief Get the number of pins in ptResultArrayIndexes
    /// \return The number of pins in ptResultArrayIndexes
    inline int getPinCount() const { return mPinCount;}
    inline void setPinCount(int pinCount) {mPinCount = pinCount;}


    void AddLimitItem(GS::Core::MultiLimitItem* limitItem);
    /// \brief Add the multilimits item to mMultiLimits if not already exists
    bool AddMultiLimitItem(GS::Core::MultiLimitItem *limitItem);
    /// \brief return multi limit item count
    int MultiLimitItemCount() const;
    /// \brief return multi limit item at index
    GS::Core::MultiLimitItem *GetMultiLimitItem(int index);

    GS::Core::MultiLimitItem *GetCurrentLimitItem() const;

    /// \brief add invalid limit for test that doesn't have the same number of test
    void CompleteWithInvalideLimit(int nbMultiLimits);


    void    SetNextTest     (CTest* test)    {ptNextTest = test;}
    CTest*  GetNextTest     ()               {return ptNextTest;}

    void    SetPreviousTest(CTest* test)    {ptPreviousTest = test;}
    CTest*  GetPreviousTest()    {return ptPreviousTest;}

    BOOL getBStatsFromSamples() const;
    void setBStatsFromSamples(const BOOL &value);

    void setCurrentLimitItem(GS::Core::MultiLimitItem *currentLimitItem);

    bool setCurrentLimitItem(int index);

// HTH: This doesnt look useful anymore since GCORE-10297
//    static void deleteInvalideLimitItem() {
//        delete mInvalidLimitItem;
//        mInvalidLimitItem = 0;
//    }

protected:
    double                          lfT_Test;			///< Student's T-Test after pass2...
    double                          lfP_Value;

private:
    Q_DISABLE_COPY(CTest)
    std::map< unsigned short, BYTE > m_cSiteCount;  ///< Count the number of exec for this test for each part
    BYTE                            m_cMaxCount;        ///< Max result for one part one this test
    static int                      s_nInstances;       ///< Number of CTest instances in RAM
    GS::Gex::SubsetLimits           mSubsetLimits;      ///< Map contains all subset limits

// HTH: This doesnt look useful anymore since GCORE-10297
//    static GS::Core::MultiLimitItem *mInvalidLimitItem; ///< Used for test that does not have all the limit index

    ///< Hold a reference to the current limit item;
    GS::Core::MultiLimitItem        *mCurrentLimitItem ;
    QList<GS::Core::MultiLimitItem*> mMultiLimits;       ///< List of multi limit items
    bool                            mFirstMultiLimits;  ///< is true if the file has his first ML
    int                             mPinCount;          ///< MPR: The number of pins in ptResultArrayIndexes
    QString                         mUserTestName;      ///< mUserTestName
    QString                         mUserTestNumber;    ///< mUserTestNumber
    QString                         mUserPinName;       ///< mUserPinName
    QString                         mPinPhysicName;     ///< mPinPhysicName
    QString                         mPinLogicName;      ///< mPinLogicName
    QString                         mPinChannelName;       ///< mPinChannelName
    int                             mDistribution;      ///< Distribution shape
    double                          mBimodalSplitValue;

    QMap<int, int>                  mTotalFailPerSite;
    CTest*			ptNextTest;     // Pointer to CTest structure
    CTest*			ptPreviousTest;	// Pointer to CTest structure
};

#endif	// #ifdef GEX_CTEST_H__INCLUDED_
