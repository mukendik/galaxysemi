#ifndef GEX_GROUP_OF_FILES_H
#define GEX_GROUP_OF_FILES_H

#include <QString>
#include <QMap>
#include "db_gexdatabasequery.h"	//GexDatabaseQuery
#include "gex_file_in_group.h"
#include "cmerged_results.h"
#include "gex_site_limits.h"
#include "cstats.h"
#include "wafermap.h"

class CTest;
class CReportOptions;
class CGexFileInGroup;

// Structures to hold last failing test in flow (keep track of multi-site#)
typedef QHash<QString, CGexSiteLimits>              tdHashSiteLimits;
typedef QMap<CTest*, tdHashSiteLimits>				tdMapTestSiteLimits;

///////////////////////////////////////////////////////////
// This class holds information about a group of files
///////////////////////////////////////////////////////////
class	CGexGroupOfFiles
{
public:

    /*!
     * \brief The Limits enum
     */
    enum LimitUsed { specLimitIfAny, standardLimitOny, specLimitOnly, multiLimitIfAny };

    CGexGroupOfFiles(QString GroupName, int lGroupId);
	~CGexGroupOfFiles();

    void Init(CGexGroupOfFiles* pGroup, CReportOptions* ptReportOptions,  FILE*hAdvFile, int iPass);
    //
	QString	GetBinName(bool bSoftBin,int iBin);
    void	UpdateBinSummary(bool bSoftBin,int iBin, BYTE bPass,QString strBinName);	// Add(create if needed) a bin entry
	void	UpdateBinTables(bool bSoftBin);											// Update the soft bins or hard bins table (using Samples array results)
	void	UpdateBinTables_Wafermap(bool bSoftBin);									// Update the soft bins or hard bins table (using Wafermap array table)       
    void	GetSampleBinCount(bool bSoftBin, QMap<int, int>& mapBinCount);			// Return total bin records for a given bin# from sample data
	void	GetWaferBinCount(bool bSoftBin, QMap<int, int>& mapBinCount);			// Return total bin records for a given bin# from wafermap data
    // computes yield level for a given die location
    bool	GetDieYieldValue(int iDieX,int iDieY,double &lfValue);
    // return the total for ALL files in this group of MirData->PartCount
    int GetTotalMirDataPartCount();

	// OBSOLETE FUNCTIONS, PLEASE DON'T USE THEM//////////////////////////////////////
	long	GetSampleBinCount(bool bSoftBin, int iBin);								// Return total bin records for a given bin# from sample data
	long	GetWaferBinCount(bool bSoftBin, int iBin);								// Return total bin records for a given bin# from wafermap data
	//////////////////////////////////////////////////////////////////////////////////

	long	GetTotalValidFiles(void);												// Returns total logical files in group that hold valild data
	bool	isPassingBin(bool bSoftBin,int iBin);									// Returns true if given bin is of 'Pass' type.

	void	postLoadProcessing();													// Post load processing : apply test creation, test filtering, part filtering
	void	removeParts(QList<long> lstParts);										// Remove a list of parts

    void    UpdateTestLimits();                                                     // Update the tests limits according to the option selected
    void    SwitchLimitIndex();

    int GetGroupId() const;

	// Group name
	QString	strGroupName;

	// Query parameters (in case this group was generated from a Examinator Query)
	GexDatabaseQuery cQuery;

    const QList<CGexFileInGroup*>& GetFilesList() const {return pFilesList;}
	// List of files to merge in the group
	QList<CGexFileInGroup*>	pFilesList;
    //	CGexFileInGroupList pFilesList;

	// Group of files merge all tests + binnings
	CMergedResults cMergedData;				// Includes data that are merged (Tests stats, Binning, Samples count,...)

    QMultiMap<int, CTest*>& GetMapTestsByNumber() { return mGroupMapTestsByNumber;}
    void SetMapTestsByNumber(QMultiMap<int, CTest*>& testsHashByNumber) {mGroupMapTestsByNumber = testsHashByNumber;}


    //
    //QList<double> GetTestResults(test_number, partID);
    // return the test result for given CTest and PartInfo
    //double GetTestResults(CTest*, CPartInfo*);
    // return the testresult
    // CTest mTestResult resutlAt(partId_index)
    //CTest* GetTest(test_number, test_name, pinmap);

    // return a CTest with given number, name and PinmapIndex
    // return Null if unfindable
    // uses defaut params of FindTestCell of GexFileInGroup
    CTest* FindTestCell(unsigned int lTestNumber, QString name, int lPinmapIndex);

    // Return the part id corresponding to the given X and Y for that dataset
    // Return "?" if not found
    bool FindPartId(int coordX, int coordY, QString &partId);
    
	// Stacked wafermaps
	void	BuildStackedWaferMap(CReportOptions *,QString strBinList="",int iWafermapType=-1);		// function to merge (stack) all wafermap data in this group.

	CStackedWaferMap cStackedWaferMapData;															// Stacked wafermap for all wafermaps in the group.
    CStackedWaferMap cDieMismatchWaferMapData;														// Die mismatch wafermap

    void IncrementFlowId() {++mFlowId;}
    int GetFlowId() const {return mFlowId;}

    bool    addTestSiteLimits(CTest * ptTestCell, int iProcessSite, BYTE bSite,
                              double dLowLimit, double dHighLimit, BYTE optFlag, time_t limitTimeStamp);
    bool	getTestSiteLimits(CTest * ptTestCell, int iProcessSite, BYTE bSite, double& dLowLimit, double& dHighLimit);

    void resetGroupRunOffset() { m_nGroupRunOffset = 0;}
     int m_nGroupRunOffset;
protected:

    bool    updateTestSiteLimits(double dLowLimit, double dHighLimit, BYTE optFlag,
                                 time_t limitTimeStamp, CGexSiteLimits &lSiteLimits);
    void    createTestSiteLimits(double dLowLimit, double dHighLimit, BYTE optFlag,
                                 time_t limitTimeStamp, CGexSiteLimits &lSiteLimits);

    /*!
     * \var mMapTestsByNumber
     * \brief Contains the list of tests in the current group sorted by test number
     */
    QMultiMap<int, CTest*> mGroupMapTestsByNumber;

	// Statistics engine.
	CGexStats		m_cStats;

	// Site limits
	tdMapTestSiteLimits		m_mapTestSiteLimits;

private:
    int mFlowId;
    Q_DISABLE_COPY(CGexGroupOfFiles)        // please manage copy constructor and = operator to remove this

public:
    static bool lessThan(const CGexGroupOfFiles * pItem1, const CGexGroupOfFiles * pItem2)		{ return (*pItem1) < (*pItem2); }
    bool operator <(const CGexGroupOfFiles& other) const;

private:
    /*!
     * \enum LimitSelectionCriteria
     */
    enum LimitSelectionCriteria { LargestLimit = 0, FirstLimit, LastLimit};
    enum LimitScope { LimitOverAllSite = 0, LimitPerSite};

    QString m_strDatasetSorting ;
    bool m_bIgnoreMRRWarning;
    static bool m_sbIgnoreAllMRRWarning;
    int mGroupId;                       ///\param The group id

    /*!
     * \var m_eLimitSelection
     */
    LimitSelectionCriteria  mLimitSelection;
    LimitScope              mLimitScope;
    LimitUsed               mLimitUsed;
    int                     mLimitSetId;

    /*!
     * \fn UpdateOptions
     * \brief Update options cache
     */
    bool UpdateOptions(CReportOptions& options);

public:

    void resetIgnoreMRR()
    {
       m_bIgnoreMRRWarning = false;
    }

    static void resetIgnoreAllMRR(){
        m_sbIgnoreAllMRRWarning = false;
    }


    QVariant GetTestConditionsValue(const QString &strTestConditionName) ; //returns the value for the given condition name in a variant object
    void AddTestConditions(const QString &strTestConditionName, const QString &strTestConditionValue) ;//add a new Test conditions name and value to the group. If test conditions name already exists, erase the old value with the new one.
    void RemoveTestConditions(const QString &strTestConditionName);//remove a test condition from the group
protected:
    QMap<QString, QVariant> m_oConditionsMaps;
public:
    CGexStats &getStats(){
        return m_cStats;
    }


    void setLimitSetId(int limitSetId);
    int getLimitSetId() const;
    LimitUsed getLimitUsed() const;
};

#endif // GEX_GROUP_OF_FILES_H
