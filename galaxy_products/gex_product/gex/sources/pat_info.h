#ifndef PATINFO_H
#define PATINFO_H

#include "pat_options.h"
#include "pat_outliers.h"
#include "pat_mv_rule.h"
#include "pat_recipe.h"
#include "wafermap.h"
#include "classes.h"

#include <QMap>
#include <QHash>

class CBinning;
class CPatInfo;
class CPatDefinition;

#ifdef WS_PAT_PERFORMANCE
#include <QElapsedTimer>

class PATPerfBench
{
public:

    PATPerfBench(const QString& lKey);
    ~PATPerfBench();

private:

    QString         mKey;
    QElapsedTimer   mTimer;
};

class PATPerfLogger
{
public:

    PATPerfLogger();
    ~PATPerfLogger();

    void            Start();
    void            Dump();
    static void     AddBench(const QString& lKey, qint64 lElapsedTime);

private:

    static QHash<QString, QPair<int, qint64> > mLogs;
    static int                                 mSession;
};

#define PAT_PERF_RUN        PATPerfLogger   lPerfLogger;
#define PAT_PERF_BENCH      PATPerfBench    lPerfBench(QString(Q_FUNC_INFO));

#else

#define PAT_PERF_RUN
#define PAT_PERF_BENCH

#endif

// Class used to tell if a die coordinate failed PAT, and which algorithms it failed)
class CPatFailureDeviceDetails
{
public:

    CPatFailureDeviceDetails();
    ~CPatFailureDeviceDetails();

    void	clear();		// Init. variables

    int		iDieX;			// DieX
    int		iDieY;			// DieY
    int		iSite;			// Testing site
    int		iPatSBin;		// PAT SoftBin if part failed PAT; or -1 otherwise
    int		iPatHBin;		// PAT HardBin if part failed PAT; or -1 otherwise
    int		iPatRules;		// PAT algorithms that failed on that part.
};

// Class used to keep track of PatBin created and the number of parts failing such bin#
class CPatBin
{
public:
    int             iBin;			// Pat Bin#
    int             iBinCount;		// Number of failures into this Pat Bin
    unsigned int    bFailType;		// Flag to define if this bin is a Static or Dynamic pat failure
};

// Class used to keep track of Good dies that need to be failed (because of PAT rule)
class CPatDieCoordinates
{
public:

    CPatDieCoordinates():
        mDieX(GEX_WAFMAP_INVALID_COORD),
        mDieY(GEX_WAFMAP_INVALID_COORD),
        mSite(0),
        mPatHBin(-1),
        mPatSBin(-1),
        mOrigHBin(-1),
        mOrigSBin(-1),
        mFailType(0)
    {}


    int		mDieX;			// DieX
    int		mDieY;			// DieY
    int		mSite;			// Site#
    int     mPatHBin;       // Pat Hard Bin
    int     mPatSBin;       // Pat Soft Bin
    int     mOrigHBin;      // Original Hard Bin
    int     mOrigSBin;      // Original Soft Bin
    int		mFailType;		// PAT rule type that failed this die. (eg: GEX_TPAT_BINTYPE_BADCLUSTER)
    QString	mRuleName;      // PAT rule name that failed this die.
    QString mPartId;        // Part ID
};

typedef QList<CPatOutlierPart*>                         tdListPatOutlierParts;
typedef QListIterator<CPatOutlierPart*>                 tdIterPatOutlierParts;
typedef QHash<QString, CPatDefinition*>                 tdPATDefinitions;
typedef QHashIterator<QString, CPatDefinition*>         tdIterPATDefinitions;
typedef QHash<QString, CPatDieCoordinates>              tdGPATOutliers;
typedef QHashIterator<QString, CPatDieCoordinates>      tdIterGPATOutliers;
typedef QHash<QString, GS::Gex::PATMVOutlier>           tdMVPATOutliers;
typedef QHashIterator<QString, GS::Gex::PATMVOutlier>   tdIterMVPATOutliers;
typedef QMap<int,CPatBin>                               tdPATBins;

// Structure to hold global information about the PAT processing task executed
class CPatInfo : public QObject
{
    Q_OBJECT

public:
    CPatInfo(QObject* parent=0);
    virtual ~CPatInfo();

    void	clear();	// Frees memory, empties all lists
    void	clearOutlierCount(bool bAll);			// reset the outlier count for good parts 'true' , or all parts 'false'
    // Check if given die location failed PPAT rules.
    int		isDieFailure_PPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin);
    // Check if given die location failed GPAT rules (NNR,IDDQ,GDBN,Cluster...)
    int		isDieFailure_GPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin);
    // Check if given die location failed GPAT rules (NNR,IDDQ,GDBN,Cluster...)
    int		isDieFailure_MVPAT(int iDieX, int iDieY,int &iPatSBin,int &iPatHBin);
    // From PAT-Bin, tells which Outlier type it is (DPAT, NNR, GDBN, etc...: type enum OutlierType)
    int		getOutlierType(int iBin);
    int		getOutliersCount(int iOutlierType,int iSite=-1,bool bSoftBin=true,int iBin=-1, int iBinPat=-1);	// Returns total outliers detected for a given PAT lagorithm and on a given testing site.
    int		GetOriginalBin(bool bSoftBin,int iDieX,int iDieY); // Return original bin# (before PAT) for given die location
    bool	isNNR_Die(int iDieX, int iDieY, unsigned uTestNumber,int iPinmapIndex);	// Check if given Test at die location is a NNR
    void	logPatWarning(QString strType,int iTestNumer,int iPinmap,QString strMessage);
    QString	strGetPatBinList(int iSite=-1);	// Returns list of PAT binnings detected in data files

    QString				strDataFile;			// Holds name of test data file analyzed
    QStringList			strDataSources;			// Holds list of files merged to create file to analyze (if only one file, then no merge).

    // Holds the list of Good dies (x,y) with too many bad neighbours and must be failed.
    tdGPATOutliers      mGDBNOutliers;
    // Holds list of dies to exclude detected by 'Reticle rule'
    tdGPATOutliers      mReticleOutliers;
    // Holds the list of Good dies (x,y) with NNR (geographic parametric outiers)
    tdGPATOutliers      mNNROutliers;
    // Holds the list of Good dies (x,y) with IDDQ-Delta
    tdGPATOutliers      mIDDQOutliers;
    // Holds list of dies to exclude detected by 'Clustering rule'
    tdGPATOutliers      mClusteringOutliers;
    // Holds list of dies to reject due to Z-PAT composite map.
    tdGPATOutliers      mZPATOutliers;

    long				lTotalGoodAfterPAT_OutputMap;		// Holds total good parts after PAT processing in OUTPUT MAP (updated by .log file generation).
    long				lPatShapeMismatch;		// Holds total number of distributions that mismatch from historical shapes.
    long				lPartsWithoutDatalog;	// Used to compute total parts without datalog.

    // DPAT Outlier parts identified with all details (failing tests, PartId,  Die XY loc, ...)
    tdListPatOutlierParts		m_lstOutlierParts;
    QList <CPatOutlierNNR *>  pNNR_OutlierTests;      // NNR Outlier parts identified with all details (failing tests, PartId,  Die XY loc, ...)
    QList <CPatOutlierIDDQ_Delta*> pIDDQ_Delta_OutlierTests;	// IDDQ-Delta Outlier parts identified with all details (failing tests, PartId,  Die XY loc, ...)

    //! \brief Holds full Probermap...optional but may be used as input map for all geographic rules.
    CWaferMap m_ProberMap;
    //! \brief Holds STDF full SoftBin wafermap of all sites combined. Map created before per-site PAT analysis.
    CWaferMap m_AllSitesMap_Sbin;
    //! \brief Original STDF HardBin Wafermap WITHOUT PAT bins.
    CWaferMap m_Stdf_HbinMap;
    //! \brief Holds STDF full HardBin wafermap of all sites combined. Map created before per-site PAT analysis.
    CWaferMap m_AllSitesMap_Hbin;
    //! \brief Original STDF SoftBin Wafermap WITHOUT PAT bins.
    CWaferMap m_Stdf_SbinMap;

    QMap<int,int>   cMapTotalGoodParts;		// Holds Total good parts per testing site: Built from SBRs
    QMap<int,int>   cMapTotalFailPatParts;	// Holds Total good parts that FAIL PAT per testing site: Built from SBRs
    QMap<int,int>   cMapTotalFailParts;		// Holds Total Fail parts (no PAT applied) per testing site: Built from SBRs
    QStringList     m_strLogWarnings;		// Holds all warnings identified durin PAT processing

    // Outlier types
    enum OutlierType
    {
        Outlier_DPAT,
        Outlier_NNR,
        Outlier_IDDQ_Delta,
        Outlier_GDBN,
        Outlier_Reticle,
        Outlier_Clustering,
        Outlier_ZPAT,
        Outlier_MVPAT
    };

    void    AddMVPATRuleChartPath(const QString& lRuleName, const QString& lChartPath);
    const QMap<QString, QStringList>&   GetMVPATChartsPath() const;
    const QStringList                   GetMVPATRuleChartsPath(const QString& lRuleName) const;

    /*!
      @brief    Append a MV PAT outlier to the MV outlier list.

      @param    GS::Gex::PATMVOutlier   MV PAT outlier to add

      @return   True if the outlier is added, false if it already exists.
      */
    bool                        AddMVOutlier(const GS::Gex::PATMVOutlier& lMVOutlier);

    /*!
      @brief    Append a MV PAT failing rule to an outlier

      @param    GS::Gex::WaferCoordinate    Coordinates of the die failing the MV rule
      @param    GS::Gex::PATMVFailingRule   MV PAT failing rule details

      @return   True if the failing rule is added, false if it fails.
      */
    bool                        AddMVFailingRule(const GS::Gex::WaferCoordinate& lCoord,
                                                 const GS::Gex::PATMVFailingRule& lMVFailingRule);

    /*!
      @brief    Increment the count of PAT hard bins for bin \a lHBin and
                overload the failure type with \a lFailType. If the bin doesn't
                exist, it creates a new entry with a count set to 1.

      @param    lHBin       PAT Hard bin number to increment
      @param    lFailType   Failure type of the PAT Hard bin
      */
    void                        IncrementPATHardBins(int lHBin,
                                                     unsigned int lFailType);

    /*!
      @brief    Increment the count of PAT soft bins for bin \a lBin and
                overload the failure type with \a lFailType. If the bin doesn't
                exist, it creates a new entry with a count set to 1.

      @param    lSBin       PAT SOFT bin number to increment
      @param    lFailType   Failure type of the PAT Soft bin
      */
    void                        IncrementPATSoftBins(int lSBin,
                                                     unsigned int lFailType);

    /*!
      @brief    Consolidate some statistics about the PAT processing

      @return   True if the consolidation succeed, otherwise false
      */
    bool                        ConsolidatePATStatistics();


    /*!
      @brief    Overload both Soft and Hard bin reference map with \a lBin PAT bin at the
                coordinate \a lXCoord and \a lYCoord.

      @param    lXCoord     The X Coordinate of the overloaded die
      @param    lYCoord     The Y Coordinate of the overloaded die
      @param    lSoftBin    The soft bin to assign to the targeted die
      @param    lHardBin    The hard bin to assign to the targeted die
      */
    void                        OverloadRefWafermap(int lXCoord, int lYCoord, int lSoftBin, int lHardBin);

    /*!
      @brief    Retrieve a PPAT outlier part from its die coordinate.

      @param    lXCoord     The X Coordinate of the die
      @param    lYCoord     The Y Coordinate of the die

      @return   A pointer on the PPAT outlier part details. NULL if no PPAT outlier for the die.
      */
    CPatOutlierPart *           FindPPATOutlierPart(int lXCoord, int lYCoord);

    /*!
      @brief    Overload reference map with PPAT bins.
      */
    bool                        OverloadRefMapWithPPATBins();

    /*!
      @brief    Erase outliers detected for this die (used each time reaching a PASS die,
                allows ignoring outliers in previous test instances for this die)

      @param    lXCoord     the X coordinates of the die
      @param    lYCoord     the Y coordinates of the die
      @param    lLastPass

      @return   True when a previous outlier detected has been revmoved
      */
    bool                        CleanPATDieAlarms(int lXCoord, int lYCoord, bool lLastPass);

    /*!
      @brief    Retuns true if the /a bin belongs to the PAT binnings.

      @param    bin     bin number to check
      */
    bool                        IsPATBinning(int bin) const;

    /*!
      @brief    Retuns true if die at the given coordinate is a MV outlier

      @param    coord   Die coordinate
      */
    bool                        IsMVOutlier(const GS::Gex::WaferCoordinate& coord) const;

    /*!
      @brief    Maps the coordinates in the current coordinate system into the
                coordinates in the original coordinate system for the external map

      @param    coord   Coordinates in the current coordinate system

      @return   Coordinates mapped to the original coordinate system
      */
    GS::Gex::WaferCoordinate    GetOriginalExternalCoordinate(const GS::Gex::WaferCoordinate& coord);

    /*!
      @brief    Returns the transformation applied to the external map
      */
    GS::Gex::WaferTransform     GetExternalTransformation() const;

    /*!
      @brief    Returns the STDF soft bins list WITHOUT PAT bins
      */
    CBinning *                  GetSTDFSoftBins() const;

    /*!
      @brief    Returns the STDF hard bins list WITHOUT PAT bins
      */
    CBinning *                  GetSTDFHardBins() const;

    /*!
      @brief    Returns the extenal map bins list
      */
    CBinning *                  GetExternalBins() const;

    /*!
      @brief    Returns the soft bins list with PAT bins
      */
    CBinning *                  GetSoftBins() const;

    /*!
      @brief    Returns the hard bins list with PAT bins
      */
    CBinning *                  GetHardBins() const;

    /*!
      @brief    Returns the SOFT PatBins created
      */
    const tdPATBins&            GetPATSoftBins() const;

    /*!
      @brief    Returns the Hard PatBins created
      */
    const tdPATBins&            GetPATHardBins() const;

    /*!
      @brief    Returns the Recipe file name
      */
    const QString&              GetRecipeFilename() const;

    /*!
      @brief    Returns the STDF file name to analyze
      */
    const QString&              GetSTDFFilename() const;

    /*!
      @brief    Returns the Reticle Location file name
      */
    const QString&              GetReticleLocation() const;

    const CWaferMap* GetReticleStepInformation() const;

    /*!
      @brief    Returns the STDF file name generated
      */
    const QString&              GetOutputDataFilename() const;

    /*!
      @brief    Returns the STDF total dies
      */
    long                        GetSTDFTotalDies() const;

    /*!
      @brief    Returns the total good parts after PAT processing
      */
    int                         GetTotalGoodPartsPostPAT() const;

    /*!
      @brief    Returns the total good parts before PAT processing
      */
    int                         GetTotalGoodPartsPrePAT() const;

    /*!
      @brief    Returns the total PAT failing parts
      */
    int                         GetTotalPATFailingParts() const;

    /*!
      @brief    Returns the Parametric PAT failing parts (DPAT and SPAT)
      */
    int                         GetTotalPPATFailingParts() const;

     /*!
      @brief    Returns the NNR PAT failing parts
     */
     int                         GetTotalNNRPATFailingParts() const;

    /*!
      @brief    Returns the PAT definition of the test using the \a lTestNumber, \a lPinmapIndex
                and \a lTestName.

       @param   lTestNumber     The number of the test seeked
       @param   lPinmapIndex    The pinmap of the test seeked
       @param   lTestName       The name of the test seeked

       @return  Pointer on the CPATDefinition found, NULL if no PAT definition found
      */
    CPatDefinition *            GetPatDefinition(long lTestNumber, long lPinmapIndex,
                                                 const QString& lTestName);

    /*!
      @brief    Retrieve the name of a test defined in the PAT test definitions using the given
                \a lTestNumber and \a lPinmapIndex.

       @param   lTestNumber     The number of the test seeked
       @param   lPinmapIndex    The pinmap of the test seeked

       @return  The name of the targeted test if exists, otherwise returns an empty string.
      */
    QString                     GetTestName(long lTestNumber,long lPinmapIndex);

    /*!
      @brief    Returns Total PPAT part failures (SPAT + DPAT)
      */
    int                         GetPPATPartCount() const;

    /*!
      @brief    Returns Total SPAT part failures
      */
    int                         GetSPATPartCount() const;

    /*!
      @brief    Returns Total DPAT part failures
      */
    int                         GetDPATPartCount() const;

    /*!
      @brief    Returns Total MVPAT part failures
      */
    int                         GetMVPATPartCount() const;

    /*!
      @brief    Returns Total GPAT failures (geographic: GDBN+NNR+IDDQ...)

      */
    int                         GetGPATPartCount() const;

    /*!
      @brief    Retrieve PAT failure details for the device at the given \a lX and \a lY coordinates.

       @param   lX      X coordinate of the targeted device
       @param   lY      Y coordinate of the targeted device
       @param   lDevice Reference to the object holding device failure details
      */
    void                        GetPatFailureDeviceDetails(int lX, int lY,
                                                           CPatFailureDeviceDetails& lDevice);

    /*!
      @brief    Return the MV outliers list
      */
    const tdMVPATOutliers &     GetMVOutliers() const;

    /*!
      @brief    Return the list of sites number being processed
      */
    const QList<int> &          GetSiteList() const;

    /*!
      @brief    Return the results of the reticle for the given rule name

      @param    ruleName    Name of the rule
      */
    QJsonObject GetReticleResults(const QString& ruleName) const;

    /*!
      @brief    Sets the transformation applied to the external map

      @param    transform   Transformation to applied
      */
    void                        SetExternalTransformation(const GS::Gex::WaferTransform& transform);

    /*!
      @brief    Sets the original hard binnings list for STDF file

      @param    binnings   Hard binnings list
      */
    void                        SetSTDFHardBins(const CBinning *binnings);

    /*!
      @brief    Sets the original hard binnings list for STDF file

      @param    binnings   Soft binnings list
      */
    void                        SetSTDFSoftBins(const CBinning *binnings);

    /*!
      @brief    Sets the binnings list for the external map

      @param    binnings   Binnings list
      */
    void                        SetExternalBins(CBinning * binnings);

    /*!
      @brief    Sets the \a fileName of the Recipe.
      */
    void                        SetRecipeFilename(const QString& fileName);

    /*!
      @brief    Sets the \a fileName of the STDF to analyze.
      */
    void                        SetSTDFFilename(const QString& fileName);

    /*!
      @brief    Sets the \a fileName of the Retcile Step Information.
      */
    void                        SetReticleStepInfo(const QString& reticleStepInfo);
    /*!
      @brief    Sets the \a fileName of the STDF generated by PAT processing.
      */
    void                        SetOutputDataFilename(const QString& fileName);

    /*!
      @brief    Sets the list of sites to process
      */
    void                        SetSiteList(const QList<int>& siteList);

    /*!
      @brief    Sets the results for reticle for the given rule name

      @param    ruleName        Name of the rule
      @param    reticleResults  Reticle results for the given rule name
      */
    void                        SetReticleResults(const QString& ruleName, const QJsonObject& results);

      /*!
      @brief    Tells if die location already identified as outlier (by one of the algprithms: DPAT, NNR, GDBN, Clustering, Reticle, etc...)

      @param    iDieX   coordinate along the x-axis
      @param    iDieY   coordinate along the y-axis
      @param    iBin    Pat binning associated with the coordinate. -1 means the die is not an outlier.

      @return   True if the die is an outlier, otherwise false
      */
    bool                        isDieOutlier(int iDieX,int iDieY,int &iBin);

    /*!
      @brief    Returns the mask definition with the given name

      @param    maskName    Name of the mask

      @return   A pointer on the mask definition, NULL if no definition found for the given mask name
      */
    CMask_Rule *                GetMaskDefinition(QString maskName);

    /*!
      @brief    Returns the recipe data
      */
    GS::Gex::PATRecipe&         GetRecipe();
    const GS::Gex::PATRecipe*   GetRecipe() const;

    /*!
      @brief    Returns options details for the recipe
      */
    COptionsPat&                GetRecipeOptions();

    /*!
      @brief    Returns options details for the recipe
      */
    const COptionsPat&          GetRecipeOptions() const;

    /*!
      @brief    Returns hash table of the univariate rules
      */
    tdPATDefinitions&           GetUnivariateRules();

    /*!
      @brief    Returns list of the multivariate rules
      */
    const QList<GS::Gex::PATMultiVariateRule>& GetMultiVariateRules() const;

    /*!
      @brief    Returns list of the multivariate rules
      */
    QList<GS::Gex::PATMultiVariateRule>& GetMultiVariateRules();

    /*!
      @brief    Find PAT algorithm where a die is tested under SPAT, PPAT and/or NNR for a given test
      */
    int         GetPPATAlgoTested(int lCoordX,
                                  int lCoordY,
                                  unsigned long lTestNumber,
                                  int lPinmapIndex,
                                  QString lTestName,
                                  int &lFailAlgo);

    /*!
      @brief    test if the test passed in parameter has been test in PAT
      */
    bool        IsPATTestExecutedWithPass(int lCoordX,
                                          int lCoordY,
                                          unsigned long lTestNumber,
                                          int lPinmapIndex,
                                          QString lTestName);

    void        addToGeneratedFilesList(const QString &file);
    void        CleanupGeneratedFileList(bool bDelete);

    void SetSoftBins(const CBinning *binnings);
    void SetHardBins(const CBinning *binnings);
private:

    GS::Gex::WaferTransform     mExternalTransform;
    CBinning *                  mExternalBins;          // Hold the binnings for the external map.
    CBinning *                  mSTDFHardBins;          // Hold the original hard bins list from the STDF file.
    CBinning *                  mSTDFSoftBins;          // Hold the original soft bins list from the STDF file
    CBinning *                  mHardBins;              // Hold the hard bins list updated with PAT Bins.
    CBinning *                  mSoftBins;              // Hold the soft bins list updated with PAT Bins.
    tdPATBins                   mPATSoftBins;           // Holds the list of SOFT PatBins created (and the fail count)
    tdPATBins                   mPATHardBins;           // Holds the list of HARD PatBins created (and the fail count)
    QString                     mRecipeFile;            // Holds the name of the recipe file
    QString                     mSTDFFile;              // Holds the STDF file name created if input file is not in STDF format
    QString                     mReticleStepInfo;       // Holds the Reticle Step Information
    QString                     mOuputDataFile;         // Holds the STDF file name generated during PAT processing
    long                        mSTDFTotalDies;         // Holds the total number of dies in the STDF
    QStringList                 mGeneratedFilesList;
    tdMVPATOutliers             mMVOutliers;            // Hold list of dies to exclude due to MV-PAT algorithm
    QMap<QString, QStringList>  mMVPATCharts;           // Holds path to MV PAT Charts generated
    int                         mTotalGoodAfterPAT;		// Holds total good STDF parts after PAT processing
    int                         mPatFailingParts;       // Holds number of parts failing the Outlier limits (allows to compute yield loss)
    QHash<QString, QJsonObject> mReticleResults;        // Holds the reticle results per rule

    GS::Gex::PATRecipe          mPATRecipe;
    QList<int>                  mSiteList;              // Holds the list of site numbers being processed.
};

#endif // PATINFO_H
