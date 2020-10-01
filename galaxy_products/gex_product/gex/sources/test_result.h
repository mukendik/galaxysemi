#ifndef TEST_RESULT_H
#define TEST_RESULT_H

///////////////////////////////////////////////////////////
// GEX includes
///////////////////////////////////////////////////////////
#include "test_result_item.h"

class CTest;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CTestResult
//
// Description	:	Class holding results for a test
//
///////////////////////////////////////////////////////////////////////////////////
class CTestResult
{

public :

    enum PassFailStatus
    {
        statusUndefined	= 0x00,
        statusPass		= 0x01,
        statusFail		= 0x02
    };

    //! \brief Constructor
    CTestResult(CTest* parent);
    //! \brief Destructor
    ~CTestResult();

    // Name			:	bool CTestResult::createResultTable(long lCount, bool bMultiResult /*= false*/)
    // Description	:	Create an array of results
    //					Return "ok" if the resize succeed otherwise "error : ..."
    QString	createResultTable(long lCount, bool bMultiResult = false);
    // Name			:	bool CTestResult::resizeResultTable(long lCount)
    // Description	:	resize the array of results
    //					Return "ok" if the resize succeed otherwise "error : ..."
    QString	resizeResultTable(long lCount);

    void	rescaleResults(int nOldScale, int nNewScale);

    long						count() const
    { return m_lCount;
    }

    // Results methods
    bool						isMultiResultAt(int nRunIndex) const;

    // Description	:	Checks that the result is valid for this index
    // Warning      : Could make the soft crash if index illegal
    bool						isValidResultAt(int nRunIndex) const;
    //! \brief Checks that index is between 0 and test result list size (exclude)
    bool                        isValidIndex(const int nRunIndex) const;

    bool						isDeletedResultAt(int nRunIndex) const;
    //! \brief Returns the Pass/Fail Status for this run index
    CTestResult::PassFailStatus passFailStatus(int nRunIndex) const;
    const double&				resultAt(int nRunIndex) const;
    // Return the scaled result (raw unit : V, A, Ohm, ...)
    // res_scal is the scaling factor (10^n)
    double                      scaledResultAt(int nRunIndex, int res_scal) const;

    // Push a new result at the given run index
    void						pushResultAt(int nRunIndex, const double& dValue, bool lUpdate = false);
    void						forceResultAt(int nRunIndex, const double& dValue);
    // Description	:	Invalidate the result at the given run index
    void						invalidateResultAt(int nRunIndex);
    void						validateResultAt(int nRunIndex);
    // Description	:	Mark the result as deleted at the given run index
    void						deleteResultAt(int nRunIndex);
    void						undeleteResultAt(int nRunIndex);
    void						setPassFailStatusAt(int nRunIndex,
                                    CTestResult::PassFailStatus ePassFailStatus);
    bool						swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex);
    bool                        cleanDeletedResults();

    // Multi Results methods
    const CTestMultiResultItem*	at(int nRunIndex) const;
    CTestMultiResultItem*       at(int nRunIndex);
    void						processMultiResult(BYTE cMultiResultValue, bool bAlwaysKeepOne);

    // return the total allocatd on octet for all test results of all tests
    static long long GetTotalAllocated() { return sTotalAllocated; }
    static long GetNumberOfInstances() { return s_lNumInstances; }

    //! \brief static global 2 dim table storing ALL results for ALL tests. Used only if option on.
    static double**             sResults;

#ifdef QT_DEBUG
	//! /brief dumping content of pFlag and pResult lists, for purposes of debug at any moment
    void			dumpResultsFlagsMaps(int aTestCellNumber, int aMaxSamples, QString& aOFilePath) const;
#endif

private:
    Q_DISABLE_COPY(CTestResult)

    // Total allocated (in octet) by all the instances of this class
    static long long sTotalAllocated;
    // hold measurements values for all runs
    double*                     m_pResult;
    // Holds flags values for all runs
    BYTE*						m_pFlag;
    // result table
    CTestMultiResultItem**      m_pMultiResult;
    // Number of run
    long						m_lCount;
    //! ID (starting with 0, following number of instances:0,1,2,3,...)
    unsigned mID;
    // set to GEX_C_DOUBLE_NAN
    static double               m_dNanResult;
    // number of instances
    static long                 s_lNumInstances;
    // Do not delete this member
    CTest*                      mParent;
};

#endif // TEST_RESULT_H
