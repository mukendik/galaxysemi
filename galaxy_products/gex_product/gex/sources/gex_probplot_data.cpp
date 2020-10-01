#include "interactive_charts.h"
#include "test_defines.h" // for C_INFINITE
#include <gqtl_global.h>
#include <qmath.h>

///////////////////////////////////////////////////////////
// Constructor: Probability plot data
///////////////////////////////////////////////////////////
CGexProbabilityPlotData::CGexProbabilityPlotData()
    : mLocalScaleFactor(0),
      mReferenceScaleFactor(0),
      mLineToTheoreticalNormalQQPlotInter(0),
      mLineToTheoreticalNormalQQPlotSlope(0)
{
    m_lTestNumber	= -1;
    m_lSize			= -1;
    m_dMinY			= C_INFINITE;
    m_dMaxY			= -C_INFINITE;

    m_pXData		= NULL;
    m_pYData		= NULL;
}

///////////////////////////////////////////////////////////
// Copy Constructor: Probability plot data
///////////////////////////////////////////////////////////
CGexProbabilityPlotData::CGexProbabilityPlotData(const CGexProbabilityPlotData& data)
{
    m_lTestNumber           = -1;
    m_lSize                 = -1;
    mLocalScaleFactor       = 0;
    mReferenceScaleFactor   = 0;
    m_dMinY                 = C_INFINITE;
    m_dMaxY                 = -C_INFINITE;

    m_pXData                = NULL;
    m_pYData                = NULL;

    *this = data;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexProbabilityPlotData::~CGexProbabilityPlotData()
{
    clean();
}

///////////////////////////////////////////////////////////
// Initialize the internal array
///////////////////////////////////////////////////////////
void CGexProbabilityPlotData::init(long lSize, long lTestNumber, int localScaleFactor)
{
    if (lSize > 0)
    {
        m_lSize                 = lSize;
        m_lTestNumber           = lTestNumber;
        mLocalScaleFactor       = localScaleFactor;
        mReferenceScaleFactor   = localScaleFactor;

        m_pXData                = new double[lSize];
        m_pYData                = new double[lSize];
    }
}

///////////////////////////////////////////////////////////
// Clean the internal array
///////////////////////////////////////////////////////////
void CGexProbabilityPlotData::clean()
{
    m_lTestNumber           = -1;
    m_lSize                 = -1;
    m_dMinY                 = C_INFINITE;
    m_dMaxY                 = -C_INFINITE;
    mLocalScaleFactor       = 0;
    mReferenceScaleFactor   = 0;

    if (m_pXData)
    {
        delete [] m_pXData;
        m_pXData = NULL;
    }

    if (m_pYData)
    {
        delete [] m_pYData;
        m_pYData = NULL;
    }

    mLineToTheoreticalNormalQQPlotInter = 0;
    mLineToTheoreticalNormalQQPlotSlope = 0;
}

///////////////////////////////////////////////////////////
// Rescale the dataset with a new scaling factor
///////////////////////////////////////////////////////////
void CGexProbabilityPlotData::rescale(int newScaleFator)
{
    int     customScaleFactor   = (mReferenceScaleFactor - mLocalScaleFactor) + (mLocalScaleFactor - newScaleFator);
    double  scaleFactor         = 1 / GS_POW(10.0,customScaleFactor);

    for (int idx = 0; idx < m_lSize; ++idx)
    {
        m_pXData[idx] *= scaleFactor;
    }

    mReferenceScaleFactor = newScaleFator;
}

///////////////////////////////////////////////////////////
// Test if data have already been computer for this test
///////////////////////////////////////////////////////////
bool CGexProbabilityPlotData::isReady(long lTestNumber) const
{
    return (m_lSize != -1 && lTestNumber == m_lTestNumber);
}

///////////////////////////////////////////////////////////
// Override the copy operator
///////////////////////////////////////////////////////////
CGexProbabilityPlotData& CGexProbabilityPlotData::operator=(const CGexProbabilityPlotData& data)
{
    if (this != &data)
    {
        m_lTestNumber           = data.m_lTestNumber;
        m_lSize                 = data.size();
        m_dMinY                 = data.minY();
        m_dMaxY                 = data.maxY();
        mLocalScaleFactor       = data.mLocalScaleFactor;
        mReferenceScaleFactor   = data.mReferenceScaleFactor;

        if (m_pXData)
        {
            delete [] m_pXData;
            m_pXData = NULL;
        }

        if (m_pYData)
        {
            delete [] m_pYData;
            m_pYData = NULL;
        }

        if (m_lSize > 0)
        {
            m_pXData = new double[m_lSize];
            memcpy(m_pXData, data.dataX(), m_lSize * sizeof(double));

            m_pYData = new double[m_lSize];
            memcpy(m_pYData, data.dataY(), m_lSize * sizeof(double));
        }

        mLineToTheoreticalNormalQQPlotInter =
            data.mLineToTheoreticalNormalQQPlotInter;
        mLineToTheoreticalNormalQQPlotSlope =
            data.mLineToTheoreticalNormalQQPlotSlope;
    }

    return *this;
}

///////////////////////////////////////////////////////////
// Add data in the array
///////////////////////////////////////////////////////////
void CGexProbabilityPlotData::addData(long lIndex, double dXData, double dYData)
{
    if (m_lSize > 0 && lIndex >= 0 && lIndex < m_lSize)
    {
        m_pXData[lIndex] = dXData;
        m_pYData[lIndex] = dYData;

        m_dMinY = qMin(m_dMinY, dYData);
        m_dMaxY = qMax(m_dMaxY, dYData);
    }
}
