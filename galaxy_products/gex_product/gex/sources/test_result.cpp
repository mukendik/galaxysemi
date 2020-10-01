///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "test_result.h"
#include "gex_constants.h"
#include "browser_dialog.h"
#include "csl/csl_engine.h"
#include "report_options.h"
#include <gqtl_log.h>
#include <gqtl_sysutils.h>
#include "message.h"
#include "ctest.h"
#include "pat_defines.h"

#include <QString>


///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
// External objects
///////////////////////////////////////////////////////////////////////////////////
extern CReportOptions	ReportOptions;

///////////////////////////////////////////////////////////////////////////////////
// Static members
///////////////////////////////////////////////////////////////////////////////////
double CTestResult::m_dNanResult = GEX_C_DOUBLE_NAN;

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CTestResult
//
// Description	:	Class holding results for a test
//
///////////////////////////////////////////////////////////////////////////////////

long CTestResult::s_lNumInstances=0;
long long CTestResult::sTotalAllocated=0;
double** CTestResult::sResults=0;

CTestResult::CTestResult(CTest *parent)
    : m_pResult(NULL), m_pFlag(NULL), m_pMultiResult(NULL), m_lCount(0),mParent(parent)
{
    mID=s_lNumInstances;
    s_lNumInstances++;
}

CTestResult::~CTestResult()
{
    QString lTestResultsAlloc=ReportOptions.GetOption("dataprocessing", "test_results_allocation").toString();
    if (lTestResultsAlloc!="one_for_all")
    {
        if (m_pResult)
        {
            delete [] m_pResult;
            m_pResult = NULL;
            sTotalAllocated -= m_lCount*sizeof(double);
        }
    }

    if (m_pFlag)
    {
        delete [] m_pFlag;
        m_pFlag = NULL;
        sTotalAllocated -= m_lCount*sizeof(BYTE);
    }

    if (m_pMultiResult)
    {
        for (int lIt = 0; lIt < m_lCount; ++lIt)
            delete m_pMultiResult[lIt];
        delete [] m_pMultiResult;
        m_pMultiResult = NULL;
        sTotalAllocated -= m_lCount*sizeof(CTestMultiResultItem);
    }
    s_lNumInstances--;
    //GSLOG(SYSLOG_SEV_DEBUG, QString("TestResult destructor : total allocated (%1 instances): %2 Mo").arg(
      //     s_lNumInstances,
        //   sTotalAllocated/1024/1024);
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CTestResult::isMultiResultAt(int nRunIndex) const
//
// Description	:	Checks that the test handles multi results
//
///////////////////////////////////////////////////////////////////////////////////
bool CTestResult::isMultiResultAt(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** bool CTestResult::isMultiResultAt(int nRunIndex) const : m_pFlag == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** bool CTestResult::isMultiResultAt(int nRunIndex) const : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    return (m_pMultiResult != NULL && !(m_pFlag[nRunIndex] & GEX::forced));
}

bool CTestResult::isValidResultAt(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");

    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pResult == NULL");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_ERROR,
              QString("nRunIndex < 0 || nRunIndex >= m_lCount : RunIndex=%1").
              arg(nRunIndex).toLatin1().constData());
#endif

    return ((m_pFlag[nRunIndex] & GEX::valid) && (m_pResult[nRunIndex] != m_dNanResult) && isDeletedResultAt(nRunIndex) == false);
}

bool CTestResult::isValidIndex(const int nRunIndex) const
{
    return ((nRunIndex>=0)&&(nRunIndex<m_lCount));
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CTestResult::isDeletedResultAt(int nRunIndex) const
//
// Description	:	Checks if the result is deleted for this index
//
///////////////////////////////////////////////////////////////////////////////////
bool CTestResult::isDeletedResultAt(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** bool CTestResult::isDeletedResultAt(int nRunIndex) const : m_pFlag == NULL ...");

    if (m_pResult == NULL)
        qDebug("**** SANITY CHECK **** bool CTestResult::isDeletedResultAt(int nRunIndex) const : m_pResult == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** bool CTestResult::isDeletedResultAt(int nRunIndex) const : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    return ((m_pFlag[nRunIndex] & GEX::deleted));
}

CTestResult::PassFailStatus CTestResult::passFailStatus(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_EMERGENCY, "PassFailStatus: m_pFlag == NULL ");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_EMERGENCY, QString("RunIndex out of range: 0 < %1 < %2").arg(nRunIndex).arg(m_lCount)
              .toLatin1().data() );
#endif

    if ((m_pFlag[nRunIndex] & GEX::forced) || (m_pFlag[nRunIndex] & GEX::valid) == 0x00)
        return CTestResult::statusUndefined;

    if ((m_pFlag[nRunIndex] & (GEX::passed | GEX::failed)) == 0x00 || (m_pFlag[nRunIndex] & (GEX::passed | GEX::failed)) == 0x18)
        return CTestResult::statusUndefined;

    if ((m_pFlag[nRunIndex] & GEX::passed))
        return CTestResult::statusPass;

    if ((m_pFlag[nRunIndex] & GEX::failed))
        return CTestResult::statusFail;

    return CTestResult::statusUndefined;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const double& CTestResult::resultAt(int nRunIndex) const
//
// Description	:	Returns a reference on results for the given run index
//
// to use this method :
// user has to check that desired index is valid (use count() method)
// before calling resultAt() method, use isValidResult() to check that the returned result will be correct
///////////////////////////////////////////////////////////////////////////////////
const double& CTestResult::resultAt(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");
    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pResult == NULL");
    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_ERROR, "nRunIndex < 0 || nRunIndex >= m_lCount");
#endif

    if (m_pFlag[nRunIndex] & GEX::valid)
        return m_pResult[nRunIndex];
    else
        return m_dNanResult;
}

double CTestResult::scaledResultAt(int nRunIndex, int res_scal) const
{
    #ifdef QT_DEBUG
        if (m_pFlag == NULL)
         GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");
        if (m_pResult == NULL)
         GSLOG(SYSLOG_SEV_ERROR, "m_pResult == NULL");
        if (nRunIndex < 0 || nRunIndex >= m_lCount)
         GSLOG(SYSLOG_SEV_ERROR, "nRunIndex < 0 || nRunIndex >= m_lCount");
    #endif

    if (isValidResultAt(nRunIndex))
    {
        return m_pResult[nRunIndex] * ScalingPower(res_scal);
    }
    else
        return m_dNanResult;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestResult::pushResultAt(int nRunIndex, const double& dValue)
//
// Description	:	Push a new result at the given run index
//
///////////////////////////////////////////////////////////////////////////////////
void CTestResult::pushResultAt(int nRunIndex, const double& dValue, bool lUpdate)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_WARNING, "TestResult push Result : m_pResult == NULL");
    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_WARNING, "TestResult push Result : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    // Not yet initialized
    if ((m_pFlag[nRunIndex] & GEX::initialized) == 0)
    {
        m_pResult[nRunIndex] = dValue;
        m_pFlag[nRunIndex]	|= GEX::valid | GEX::initialized;
    }
    else if (lUpdate)
    {
        m_pResult[nRunIndex] = dValue;
        m_pFlag[nRunIndex]	|= GEX::valid;
    }

    if (m_pMultiResult)
        m_pMultiResult[nRunIndex]->pushResult(dValue);

    // reset distribution
    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestResult::forceResultAt(int nRunIndex, const double& dValue)
//
// Description	:	Force the result at the given run index
//
///////////////////////////////////////////////////////////////////////////////////
void CTestResult::forceResultAt(int nRunIndex, const double& dValue)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::forceResultAt(int nRunIndex, const double& dValue) : m_pResult == NULL ...");

    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::forceResultAt(int nRunIndex, const double& dValue) : m_pFlag == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** void CTestResult::forceResultAt(int nRunIndex, const double& dValue) : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    m_pFlag[nRunIndex]	|= GEX::forced;
    m_pResult[nRunIndex] = dValue;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestResult::invalidateResultAt(int nRunIndex)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_ERROR, QString("m_pResult == NULL at index %1").
              arg(nRunIndex).toLatin1().constData());
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, QString("m_pFlag == NULL at index %1").
              arg(nRunIndex).toLatin1().constData());
    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_ERROR,
              QString("nRunIndex < 0 || nRunIndex >= m_lCount for index %1").
              arg(nRunIndex).toLatin1().constData());
#endif

    m_pFlag[nRunIndex] &= ~GEX::valid;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestResult::validateResultAt(int nRunIndex){

#ifdef QT_DEBUG
    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_ERROR, QString("m_pResult == NULL at index %1").
              arg(nRunIndex).toLatin1().constData());
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, QString("m_pFlag == NULL at index %1").
              arg(nRunIndex).toLatin1().constData());
    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_ERROR,
              QString("nRunIndex < 0 || nRunIndex >= m_lCount for index %1").
              arg(nRunIndex).toLatin1().constData());
#endif
    m_pFlag[nRunIndex] |= GEX::valid;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestResult::deleteResultAt(int nRunIndex)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : m_pResult == NULL ...");

    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : m_pFlag == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    m_pFlag[nRunIndex] |= GEX::deleted;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestResult::undeleteResultAt(int nRunIndex)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : m_pResult == NULL ...");

    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : m_pFlag == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** void CTestResult::deleteResultAt(int nRunIndex) : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    m_pFlag[nRunIndex] &= ~GEX::deleted;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestResult::setPassFailStatusAt(int nRunIndex, CTestResult::PassFailStatus ePassFailStatus)
//
// Description	:	Flag the result at the given run index as failed
//
///////////////////////////////////////////////////////////////////////////////////
void CTestResult::setPassFailStatusAt(int nRunIndex, CTestResult::PassFailStatus ePassFailStatus)
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        qDebug("**** SANITY CHECK **** void CTestResult::setPassFailStatusAt(int nRunIndex, CTestResult::PassFailStatus ePassFailStatus) : m_pFlag == NULL ...");

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        qDebug("**** SANITY CHECK **** void CTestResult::setPassFailStatusAt(int nRunIndex, CTestResult::PassFailStatus ePassFailStatus) : nRunIndex < 0 || nRunIndex >= m_lCount...");
#endif

    if (ePassFailStatus == CTestResult::statusFail)
        m_pFlag[nRunIndex] |= GEX::failed;
    else if (ePassFailStatus == CTestResult::statusPass)
        m_pFlag[nRunIndex] |= GEX::passed;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CTestResult::swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex)
//
// Description	:	Swap results
//
///////////////////////////////////////////////////////////////////////////////////
bool CTestResult::swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex)
{
    if (m_pResult == NULL)
    {
        GSLOG(SYSLOG_SEV_WARNING, "swapResults : m_pResult == NULL ...");
        return false;
    }

    if (m_pFlag == NULL)
    {
        qDebug("**** SANITY CHECK **** bool CTestResult::swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex) : m_pFlag == NULL ...");
        return false;
    }

    if (nOldRunIndex + nOffset >= m_lCount)
    {
        qDebug("**** SANITY CHECK **** bool CTestResult::swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex) : uiOldRunIndex + uiOffset >= m_lCount...");
        return false;
    }

    if (nNewRunIndex + nOffset >= m_lCount)
    {
        qDebug("**** SANITY CHECK **** bool CTestResult::swapResults(int nOffset, int nOldRunIndex, int nNewRunIndex) : uiOldRunIndex + uiOffset >= m_lCount...");
        return false;
    }

    double dTempValue = m_pResult[nOffset + nOldRunIndex];
    m_pResult[nOffset + nOldRunIndex] = m_pResult[nOffset + nNewRunIndex];
    m_pResult[nOffset + nNewRunIndex]	= dTempValue;

    BYTE cTempValue = m_pFlag[nOffset + nOldRunIndex];
    m_pFlag[nOffset + nOldRunIndex] = m_pFlag[nOffset + nNewRunIndex];
    m_pFlag[nOffset + nNewRunIndex] = cTempValue;

    if (m_pMultiResult)
    {
        CTestMultiResultItem* multiResult = m_pMultiResult[nOffset + nOldRunIndex];
        m_pMultiResult[nOffset + nOldRunIndex] = m_pMultiResult[nOffset + nNewRunIndex];
        m_pMultiResult[nOffset + nNewRunIndex] = multiResult;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CTestResult::cleanDeletedResults()
//
// Description	:	clean sample results, move deleted data to the end of the array
//
///////////////////////////////////////////////////////////////////////////////////
bool CTestResult::cleanDeletedResults()
{
    long lTmpCount = m_lCount;

    for(int nIndex = m_lCount-1; nIndex >= 0; nIndex--)
    {
        if (isDeletedResultAt(nIndex))
        {
            if (nIndex + 1 < lTmpCount)
            {
                memmove((m_pResult+nIndex), (m_pResult+nIndex+1), (lTmpCount - nIndex - 1) * sizeof(double));
                memmove((m_pFlag+nIndex), (m_pFlag+nIndex+1), (lTmpCount - nIndex - 1) * sizeof(BYTE));

                if (m_pMultiResult)
                {
                    CTestMultiResultItem* multiResult = m_pMultiResult[nIndex];

                    for(int nIndexMulti = nIndex; nIndexMulti < lTmpCount-1; nIndexMulti++)
                        m_pMultiResult[nIndexMulti] = m_pMultiResult[nIndexMulti+1];

                    m_pMultiResult[lTmpCount-1] = multiResult;
                }
            }

            --lTmpCount;
        }
    }

    m_lCount = lTmpCount;

    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CTestResultItem& CTestResult::at(int nRunIndex) const
//
// Description	:	Returns a reference on results for the given run index
//
///////////////////////////////////////////////////////////////////////////////////
const CTestMultiResultItem * CTestResult::at(int nRunIndex) const
{
#ifdef QT_DEBUG
    if (m_pMultiResult == NULL)
        GSLOG(SYSLOG_SEV_ERROR, QString("m_pMultiResult == NULL at index %1 ").
              arg(nRunIndex).toLatin1().constData());

    if (nRunIndex < 0 || nRunIndex >= m_lCount)
        GSLOG(SYSLOG_SEV_ERROR,
              QString("nRunIndex < 0 || nRunIndex >= m_lCount for index %1").
              arg(nRunIndex).toLatin1().constData());
#endif
    return m_pMultiResult[nRunIndex];
}

CTestMultiResultItem*	CTestResult::at(int nRunIndex)
{
    return m_pMultiResult[nRunIndex];
}

QString CTestResult::createResultTable(long lCount, bool bMultiResult /*= false*/)
{
    if (m_pResult || m_pFlag || m_pMultiResult)
    {
        GSLOG(SYSLOG_SEV_WARNING, "Memory already allocated");
        GSLOG(SYSLOG_SEV_DEBUG, "m_pResult or m_pFlag or m_pMultiResult already exists...");
        return "error : memory already allocated"; // was return false
    }

    if (lCount <= 0)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "create result table : array size must be > 0");
        return "error : array size must be upper than 0"; // was return false
    }

    void* lDoublesAndBytes=0;
    void* lMRI=0;
    void* lSecurityBuff=0;

    // case 6419 : let's try to alloc the needed space + few megs
    // to check the system has enough mem to survive
    // todo 7.1 : Use CGexSystemUtils::GetMemoryInfo(...) to get mem status if confirmed in 7.0
    //QMap<QString, QVariant> lMemInfo=CGexSystemUtils::GetMemoryInfo(false, false);

    char* lGexMemCheck=getenv("GEX_MEMCHECK_SIZE");
    bool ok=false;
    int lMC=QString(lGexMemCheck).toInt(&ok);
    if (!lGexMemCheck || !ok || lMC<1)
        lMC=20;

    lDoublesAndBytes=malloc(lCount*sizeof(double) + lCount*sizeof(BYTE));
    if (!lDoublesAndBytes)
        goto OOM;

    if (bMultiResult)
    {
        lMRI=malloc( lCount*sizeof(CTestMultiResultItem) );
        if (!lMRI)
            goto OOM;
    }
    lSecurityBuff=malloc( lMC*1024*1024 );
    if (!lSecurityBuff)
        goto OOM;
    goto OK;

    OOM: // Out of Mem
        if (lDoublesAndBytes)
            free(lDoublesAndBytes);
        if (lMRI)
            free(lMRI);
        if (lSecurityBuff)
            free(lSecurityBuff);
        GSLOG(SYSLOG_SEV_ERROR,
              QString("Out of memory at memcheck step (%1Mo) : failed to alloc %2 results").arg(lMC).arg(lCount)
              .toLatin1().constData());
        return "error : out of memory";

    OK:
        if (lDoublesAndBytes)
            free(lDoublesAndBytes);
        if (lMRI)
            free(lMRI);
        if (lSecurityBuff)
            free(lSecurityBuff);

    try
    {
        QString lTestResultsAlloc=ReportOptions.GetOption("dataprocessing", "test_results_allocation").toString();
        if (lTestResultsAlloc=="one_for_all")
        {
            // No more allocation needed: let s point to the right location in the static table
            // mID is going from 0 to (num of tests)...
            //m_pResult = &CTestResult::sResults[0][mID];
            //m_pResult = &CTestResult::sResults[mID*lCount][0];
            //m_pResult = CTestResult::sResults[0]+mID*lCount; // crash
            m_pResult = ((double*)CTestResult::sResults)+mID*lCount;
        }
        else
            m_pResult	= new double[lCount];

        m_pFlag		= new BYTE[lCount];

        if (bMultiResult)
        {
            m_pMultiResult	= new CTestMultiResultItem*[lCount];
            for (int lIt = 0; lIt < lCount; ++lIt)
                m_pMultiResult[lIt] = new CTestMultiResultItem(mParent);
        }
    }
    catch(const std::bad_alloc& e)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL,
              QString("create result table : exception caught : %1").
              arg(e.what() ? e.what() : "?").toLatin1().constData());
        // System is out of memory
        GSLOG(SYSLOG_SEV_ERROR, "System out of memory, the process will be aborted");
        // Display a message box to alert the customer then abort the running script
        GS::Gex::Message::critical(
            "Alert",
            QString(
                "Program out of memory:\n"
                "The OS refuses to give more memory space to the program \n"
                "in order for it to store more test results (already %1Mo "
                "of test results).\n"
                "The process will be aborted!").
            arg(GetTotalAllocated()/1024/1024) );

        GS::Gex::CSLEngine::GetInstance().AbortScript(); // was pGexMainWindow->OnStop();

        // Free allocated memory
        if (m_pResult)
        {
            delete m_pResult;
            m_pResult = NULL;
        }

        if (m_pFlag)
        {
            delete m_pFlag;
            m_pFlag = NULL;
        }

        if (m_pMultiResult)
        {
            for (int lIt = 0; lIt < m_lCount; ++lIt)
                delete m_pMultiResult[lIt];
            delete [] m_pMultiResult;
            m_pMultiResult = NULL;
        }

        return "error : out of memory";
    }

    sTotalAllocated+= lCount*sizeof(double);
    sTotalAllocated+= lCount*sizeof(BYTE);
    if (m_pMultiResult)
        sTotalAllocated+= lCount*sizeof(CTestMultiResultItem);

    /*
    GSLOG(SYSLOG_SEV_DEBUG,
      "Create result table : allocated %d results, total allocated (%d instances): %d Mo",
      lCount, s_lNumInstances,
      sTotalAllocated/1024/1024);
    */

    if (m_pResult && m_pFlag)
    {
        m_lCount = lCount;

        // Initialize the Flag array to invalid for all run
        memset(m_pFlag, 0, sizeof(BYTE) * lCount);

        // Initialize the sample result to GEX_C_DOUBLE_NAN
        for (int nIndex = 0; nIndex < lCount; ++nIndex)
            m_pResult[nIndex] = GEX_C_DOUBLE_NAN;

        return "ok";
    }
    else
        return "error : cannot allocate mem";
}

QString CTestResult::resizeResultTable(long lCount)
{
    if (lCount <= 0)
    {
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Cannot resize result table : array size must be > 0");
        return "error : array size must be greater than 0"; // was return false
    }

    if (lCount != m_lCount)
    {
        double *                        pTmpResult      = NULL;
        BYTE *                          pTmpFlag        = NULL;
        CTestMultiResultItem** pTmpMultiResult = NULL;

        try
        {
            pTmpResult	= new double[lCount];
            pTmpFlag	= new BYTE[lCount];

            if (m_pMultiResult)
            {
                pTmpMultiResult = new CTestMultiResultItem*[lCount];
                // if new array is bigger -> instanciate missing items
                for (int lIt = 0; lIt < lCount; ++lIt)
                {
                    if (lIt >= m_lCount)
                        pTmpMultiResult[lIt] = new CTestMultiResultItem(mParent);
                    else
                        pTmpMultiResult[lIt] = NULL;
                }
            }

        }
        catch(const std::bad_alloc& e)
        {
            // Free allocated memory
            if (pTmpResult)
            {
                delete pTmpResult;
                pTmpResult = NULL;
            }

            if (pTmpFlag)
            {
                delete pTmpFlag;
                pTmpFlag = NULL;
            }

            if (pTmpMultiResult)
            {
                if (lCount > m_lCount) // if new table was bigger -> clean additional items
                {
                    for (int lIt = lCount - m_lCount; lIt < lCount; ++ lIt)
                        delete pTmpMultiResult[lIt];
                }
                delete [] pTmpMultiResult;
                pTmpMultiResult = NULL;
            }

            return "error : out of memory";
        }

        // Reallocation succeed
        if (pTmpResult && pTmpFlag)
        {
            // Min of the two size
            int minCount = qMin(lCount, m_lCount);

            // Copy data from old buffer to the new one
            for (int idx = 0; idx < minCount; ++idx)
            {
                pTmpResult[idx] = m_pResult[idx];
                pTmpFlag[idx]   = m_pFlag[idx];

                if (pTmpMultiResult)
                    pTmpMultiResult[idx] = m_pMultiResult[idx];
            }

            // Initialize new extra space allocated
            for(int idx = minCount; idx < lCount; ++idx)
            {
                pTmpResult[idx] = GEX_C_DOUBLE_NAN;
                pTmpFlag[idx]   = 0;
            }

            // Delete old result buffer
            if (m_pResult)
                delete m_pResult;

            // Get the new address of the result buffer
            m_pResult       = pTmpResult;

            // Delete old flag buffer
            if (m_pFlag)
                delete m_pFlag;

            // Get the new address of the flag buffer
            m_pFlag         = pTmpFlag;

            // Delete old multi-result buffer
            if (m_pMultiResult)
            {
                // new table is smaller -> clean additional items
                if (m_lCount > lCount)
                {
                    for (int lIt = m_lCount - lCount; lIt < m_lCount; ++ lIt)
                        delete pTmpMultiResult[lIt];
                }
                delete [] m_pMultiResult;
            }

            // Keep the new buffer size
            m_lCount = lCount;

            // Get the new address of the multiresult buffer
            m_pMultiResult  = pTmpMultiResult;

            // Keep amount of memory allocated
            sTotalAllocated = m_lCount * sizeof(double);
            sTotalAllocated = m_lCount * sizeof(BYTE);

            if (m_pMultiResult)
                sTotalAllocated+= m_lCount * sizeof(CTestMultiResultItem);

        }
        else
            return "error : cannot allocate mem";
    }

    return "ok";
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestResult::processMultiResult(BYTE cMultiResultValue, bool bAlwaysKeepOne)
//
// Description	:	Process the multi-result steps
//
///////////////////////////////////////////////////////////////////////////////////
void CTestResult::processMultiResult(BYTE cMultiResultValue, bool bAlwaysKeepOne)
{
    if (m_pMultiResult)
    {
        CTestMultiResultItem::MultiResultValue eMultiResultValue;

        switch (cMultiResultValue)
        {
            default							:
            case GEX_MULTIRESULT_USE_FIRST	:	eMultiResultValue = CTestMultiResultItem::firstMultiResult;
                                                break;

            case GEX_MULTIRESULT_USE_LAST	:	eMultiResultValue = CTestMultiResultItem::lastMultiResult;
                                                break;

            case GEX_MULTIRESULT_USE_MIN	:	eMultiResultValue = CTestMultiResultItem::minMultiResult;
                                                break;

            case GEX_MULTIRESULT_USE_MAX	:	eMultiResultValue = CTestMultiResultItem::maxMultiResult;
                                                break;

            case GEX_MULTIRESULT_USE_MEDIAN	:	eMultiResultValue = CTestMultiResultItem::medianMultiResult;
                                                break;

            case GEX_MULTIRESULT_USE_MEAN	:	eMultiResultValue = CTestMultiResultItem::meanMultiResult;
                                                break;
        }

        for(int nIndex = 0; nIndex < m_lCount; nIndex++)
        {
            if (isValidResultAt(nIndex) && isMultiResultAt(nIndex))
                m_pResult[nIndex] = m_pMultiResult[nIndex]->processMultiResult(eMultiResultValue);
        }

        if (bAlwaysKeepOne)
        {
            delete [] m_pMultiResult;
            m_pMultiResult = NULL;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool CTestResult::rescaleResults(int nOldScale, int nNewScale)
//
// Description	:	Rescale result
//
///////////////////////////////////////////////////////////////////////////////////
void CTestResult::rescaleResults(int nOldScale, int nNewScale)
{
#ifdef QT_DEBUG
    if (m_pResult == NULL)
        GSLOG(SYSLOG_SEV_WARNING, "rescale Results : m_pResult == NULL");
#endif

    // Mono result step.
    for (int nCount = 0; nCount < m_lCount; nCount++)
    {
        if(isValidResultAt(nCount)){
        m_pResult[nCount] *= ScalingPower(nOldScale);
        m_pResult[nCount] /= ScalingPower(nNewScale);

        if (m_pMultiResult)
            m_pMultiResult[nCount]->rescale(nOldScale, nNewScale);
        }
    }
}

#ifdef QT_DEBUG
void CTestResult::dumpResultsFlagsMaps(int aTestCellNumber, int aMaxSamples, QString& aOFilePath) const
{
    int i = 0;
    QString tmpStr = QString::number(aTestCellNumber);
    QString lOFilePath(aOFilePath);
    lOFilePath += "_";
    lOFilePath += tmpStr;
    lOFilePath += ".txt";
    std::ofstream outputfile;
    bool lB = false;
    outputfile.open(lOFilePath.toStdString().c_str(), std::ios_base::out);
    outputfile << "\nm_pResults content:\n";
    for(i = 0; i < aMaxSamples; i++)
        outputfile << i << ": " << QString::number(m_pResult[i]).toStdString() << "\n";
    outputfile << "\nm_pFlags content:\n";
    for(i = 0; i < aMaxSamples; i++)
    {
        lB = m_pFlag[i] & GEX::valid;
        outputfile << i << ": " << lB << "\n";
    }
    outputfile.close();
}
#endif
