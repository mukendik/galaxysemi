///////////////////////////////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////////////////////////////
#include "test_result_item.h"
#include "gex_algorithms.h"
#include "gex_constants.h"
#include <gqtl_log.h>
#include "pat_defines.h"
#include "ctest.h"

#include <QVector>

///////////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////////
extern double	ScalingPower(int iPower);

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CTestMultiResultItem
//
// Description	:	Class holding results for a sample
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CTestMultiResultItem::CTestMultiResultItem(CTest* parent, WORD wSize /* = 2 */)
    : m_wSize(wSize), m_wValueCount(0), m_pValue(NULL), m_pFlag(NULL), mParent(parent)
{
    if (m_wSize <= 1)
        m_wSize = 2;
}

CTestMultiResultItem::CTestMultiResultItem(const CTestMultiResultItem &other, CTest *parent)
    : m_wSize(2), m_wValueCount(0), m_pValue(NULL), m_pFlag(NULL)
{
    this->clone(other, parent);
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CTestMultiResultItem::~CTestMultiResultItem()
{
    if (m_pValue)
    {
        free(m_pValue);
        m_pValue = NULL;
    }

    if (m_pFlag)
    {
        free(m_pFlag);
        m_pFlag = NULL;
    }
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	int CTestMultiResultItem::count()const
//
// Description	:	Returns the count of value for this run
//
///////////////////////////////////////////////////////////////////////////////////
int CTestMultiResultItem::count()const
{
    return (int) m_wValueCount;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double CTestMultiResultItem::processMultiResult(MultiResultValue eValue)
//
// Description	:	Returns the result value for this item
//
///////////////////////////////////////////////////////////////////////////////////
double CTestMultiResultItem::processMultiResult(MultiResultValue eValue)
{
    if (m_pValue)
    {
        // Optimize the memory space by reducing the allocated space to the exatc need
        if (m_wValueCount < m_wSize)
        {
            m_pValue	= (double *) realloc(m_pValue, m_wValueCount * sizeof(double));
            m_pFlag	= (BYTE *) realloc(m_pFlag, m_wValueCount * sizeof(BYTE));
            m_wSize		= m_wValueCount;
        }

        switch(eValue)
        {
            default					:
            case firstMultiResult	: return m_pValue[0];

            case lastMultiResult	: return m_pValue[m_wValueCount-1];

            case minMultiResult		: return minValue();

            case maxMultiResult		: return maxValue();

            case meanMultiResult	: return meanValue();

            case medianMultiResult	: return medianValue();
        }
    }
    else
        return GEX_C_DOUBLE_NAN;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	const double& CTestMultiResultItem::multiResultAt(WORD wIndex) const
//
// Description	:	Returns the multi result value for this item at the given index
//
///////////////////////////////////////////////////////////////////////////////////
const double& CTestMultiResultItem::multiResultAt(WORD wIndex) const
{
    if (wIndex >= m_wValueCount)
        GSLOG(SYSLOG_SEV_ERROR, "out of range request : wIndex > m_wValueCount ...");
    //if (m_pFlag[wIndex] & GEX::valid)
        return m_pValue[wIndex];
//    else
//        return double(GEX_C_DOUBLE_NAN);
}

void CTestMultiResultItem::invalidateResultAt(WORD wIndex)
{
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");

    if (m_pValue == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pValue == NULL");

    if ( wIndex >= m_wValueCount)
        GSLOG(SYSLOG_SEV_ERROR, QString("wIndex >= m_wValueCount : wIndex=%1").arg( wIndex).toLatin1().constData());
#endif
    m_pFlag[wIndex] &= ~GEX::valid;
    //m_pValue[wIndex] = GEX_C_DOUBLE_NAN;

    // reset distribution
    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestMultiResultItem::validateResultAt(WORD wIndex){
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");

    if (m_pValue == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pValue == NULL");

    if ( wIndex >= m_wValueCount)
        GSLOG(SYSLOG_SEV_ERROR, QString("wIndex >= m_wValueCount : wIndex=%1").arg( wIndex).toLatin1().constData());
#endif
    m_pFlag[wIndex] |= GEX::valid;

    // reset distribution
    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

bool CTestMultiResultItem::isValidResultAt(WORD wIndex) const
{
    //return  m_pValue[wIndex] != GEX_C_DOUBLE_NAN;
#ifdef QT_DEBUG
    if (m_pFlag == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pFlag == NULL");

    if (m_pValue == NULL)
        GSLOG(SYSLOG_SEV_ERROR, "m_pValue == NULL");

    if ( wIndex >= m_wValueCount)
        GSLOG(SYSLOG_SEV_ERROR, QString("wIndex >= m_wValueCount : wIndex=%1").arg( wIndex).toLatin1().constData());
#endif

    return ((m_pFlag[wIndex] & GEX::valid) && (m_pValue[wIndex] != GEX_C_DOUBLE_NAN));
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestMultiResultItem::pushResult(const double& dValue)
//
// Description	:	Push a new result value into the vector
//
///////////////////////////////////////////////////////////////////////////////////
void CTestMultiResultItem::pushResult(const double& dValue)
{
    if (dValue == GEX_C_DOUBLE_NAN)
        GSLOG(SYSLOG_SEV_WARNING, "dValue == GEX_C_DOUBLE_NAN ");

    // If m_pValue is not null, it means that we already have push more than 1 result.
    if (m_pValue)
    {
        // Reallocate more memory if need be
        if (m_wValueCount >= m_wSize)
        {
            if (m_wValueCount == 65535)
                GSLOG(SYSLOG_SEV_WARNING, " m_wValueCount == 65535 ");

            m_wSize		= m_wValueCount + 1;
            m_pValue	= (double *) realloc(m_pValue, m_wSize * sizeof(double));
            if (!m_pValue)
            {
                m_wValueCount=0;
                GSLOG(SYSLOG_SEV_ERROR, "realloc() failed");
                return;
            }
            m_pFlag	= (BYTE *) realloc(m_pFlag, m_wSize * sizeof(BYTE));
            if (!m_pFlag)
            {
                m_wValueCount=0;
                GSLOG(SYSLOG_SEV_ERROR, "realloc() failed");
                return;
            }
        }
    }
    else
    {
        // Second results pushed for this item, so create a table to manage them
        m_pValue	= (double*) malloc(m_wSize * sizeof(double));
        if (!m_pValue)
        {
            GSLOG(SYSLOG_SEV_ERROR, "malloc() failed");
            m_wValueCount=0;
            return;
        }
        m_pFlag	= (BYTE*) malloc(m_wSize * sizeof(BYTE));
        if (!m_pFlag)
        {
            GSLOG(SYSLOG_SEV_ERROR, "malloc() failed");
            m_wValueCount=0;
            return;
        }
    }

    m_pValue[m_wValueCount++]	= dValue;
    m_pFlag[m_wValueCount-1]	|= GEX::valid | GEX::initialized;

    // reset distribution
    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void CTestMultiResultItem::rescale(int nOldScale, int nNewScale)
//
// Description	:	Rescale the result
//
///////////////////////////////////////////////////////////////////////////////////
void CTestMultiResultItem::rescale(int nOldScale, int nNewScale)
{
    // Multi result step, rescale all values in the table
    if (m_pValue)
    {
        for(int nCount = 0; nCount < m_wValueCount; nCount++)
        {
            if(isValidResultAt(nCount)){
                m_pValue[nCount] *= ScalingPower(nOldScale);
                m_pValue[nCount] /= ScalingPower(nNewScale);
            }
        }
    }

    // reset distribution
    mParent->SetDistribution(PATMAN_LIB_SHAPE_UNSET);
}

void CTestMultiResultItem::clone(const CTestMultiResultItem &other, CTest *parent)
{
    if (this != &other)
    {
        m_wSize			= other.m_wSize;
        m_wValueCount	= other.m_wValueCount;
        mParent         = parent;

        if (m_pValue)
        {
            free(m_pValue);
            m_pValue = NULL;
        }

        if (m_pFlag)
        {
            free(m_pFlag);
            m_pFlag = NULL;
        }

        // If no value added to the multiresult item, don't allocate memory.
        if (m_wSize > 0)
        {
            m_pValue		= (double*) malloc(m_wSize * sizeof(double));
            m_pFlag		= (BYTE*) malloc(m_wSize * sizeof(BYTE));
            if (!m_pValue)
            {
                m_wValueCount=0;
                GSLOG(SYSLOG_SEV_ERROR, "malloc failed !");
            }
            else if(!m_pFlag){
                m_wValueCount=0;
                GSLOG(SYSLOG_SEV_ERROR, "malloc failed !");
            }
            else {
                if (m_wValueCount > 0 && other.m_pValue)
                    memcpy(m_pValue, other.m_pValue, m_wSize * sizeof(double));
                if (m_wValueCount > 0 && other.m_pFlag)
                    memcpy(m_pFlag, other.m_pFlag, m_wSize * sizeof(BYTE));
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double CTestMultiResultItem::minValue()
//
// Description	:	return the min value of the array
//
///////////////////////////////////////////////////////////////////////////////////
double CTestMultiResultItem::minValue()
{
    double dMinValue = m_pValue[0];

    for (int nCount = 1; nCount < m_wValueCount; ++nCount)
         if(isValidResultAt(nCount))
             dMinValue = qMin(dMinValue, m_pValue[nCount]);

    return dMinValue;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double CTestMultiResultItem::maxValue()
//
// Description	:	return the max value of the array
//
///////////////////////////////////////////////////////////////////////////////////
double CTestMultiResultItem::maxValue()
{
    double dMaxValue = m_pValue[0];

    for (int nCount = 1; nCount < m_wValueCount; ++nCount)
         if(isValidResultAt(nCount))
             dMaxValue = qMax(dMaxValue, m_pValue[nCount]);

    return dMaxValue;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double CTestMultiResultItem::meanValue()
//
// Description	:	return the mean value of the array
//
///////////////////////////////////////////////////////////////////////////////////
double CTestMultiResultItem::meanValue()
{
    double dMeanValue = 0.0;

    for (int nCount = 0; nCount < m_wValueCount; ++nCount)
         if(isValidResultAt(nCount))
             dMeanValue += m_pValue[nCount];

    return (dMeanValue / (double) m_wValueCount);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	double CTestMultiResultItem::medianValue()
//
// Description	:	return the median value of the array
//
///////////////////////////////////////////////////////////////////////////////////
double CTestMultiResultItem::medianValue()
{
    // Hervé: what about that ?
    if (m_wValueCount==0)
        return GEX_C_DOUBLE_NAN;

    QVector<double> vecDouble;
    vecDouble.reserve(m_wValueCount);

    for (int nCount = 0; nCount < m_wValueCount; ++nCount)
        if(isValidResultAt(nCount))
            vecDouble.append(m_pValue[nCount]);

    qSort(vecDouble.begin(), vecDouble.end());

    return algorithms::gexMedianValue(vecDouble);
}

