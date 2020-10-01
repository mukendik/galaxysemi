#include "gex_box_plot_data.h"
#include "test_defines.h"
#include "gex_constants.h"

///////////////////////////////////////////////////////////
// Class GexBoxPlotData
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor: Box plot data
///////////////////////////////////////////////////////////
CGexBoxPlotData::CGexBoxPlotData()
{
    m_nSize			= -1;

    m_pLabels		= NULL;
    m_pXData        = NULL;
    m_pQ0Data		= NULL;
    m_pQ1Data		= NULL;
    m_pQ2Data		= NULL;
    m_pQ3Data		= NULL;
    m_pQ4Data		= NULL;
    m_pColors		= NULL;

    m_dMinData      = C_INFINITE;
    m_dMaxData      = -C_INFINITE;
}

///////////////////////////////////////////////////////////
// Destructor: Box plot data
///////////////////////////////////////////////////////////
CGexBoxPlotData::~CGexBoxPlotData()
{
    clean();
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void init(long lSize)
//
// Description	:	Initialize the internal array
//
///////////////////////////////////////////////////////////////////////////////////
void CGexBoxPlotData::init(int nSize)
{
    if (nSize > 0)
    {
        m_nSize			= nSize;

        m_pQ0Data		= new double[nSize];
        m_pQ1Data		= new double[nSize];
        m_pQ2Data		= new double[nSize];
        m_pQ3Data		= new double[nSize];
        m_pQ4Data		= new double[nSize];
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void clean()
//
// Description	:	Clean the internal array
//
///////////////////////////////////////////////////////////////////////////////////
void CGexBoxPlotData::clean()
{
    m_nSize			= -1;
    m_dMinData      = C_INFINITE;
    m_dMaxData      = -C_INFINITE;

    if (m_pXData)
    {
        delete [] m_pXData;
        m_pXData = NULL;
    }

    if (m_pQ0Data)
    {
        delete [] m_pQ0Data;
        m_pQ0Data = NULL;
    }

    if (m_pQ1Data)
    {
        delete [] m_pQ1Data;
        m_pQ1Data = NULL;
    }

    if (m_pQ2Data)
    {
        delete [] m_pQ2Data;
        m_pQ2Data = NULL;
    }

    if (m_pQ3Data)
    {
        delete [] m_pQ3Data;
        m_pQ3Data = NULL;
    }

    if (m_pQ4Data)
    {
        delete [] m_pQ4Data;
        m_pQ4Data = NULL;
    }

    if (m_pColors)
    {
        delete [] m_pColors;
        m_pColors = NULL;
    }

    if (m_pLabels)
    {
        for (int nLabelIndex = 0; nLabelIndex < m_nSize; nLabelIndex++)
        {
            if (m_pLabels[nLabelIndex])
            {
                delete m_pLabels[nLabelIndex];
                m_pLabels[nLabelIndex] = NULL;
            }
        }

        delete [] m_pLabels;
        m_pLabels = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
//								double dQ3Data, double dQ4Data, int nColors, const QString& strLabel)
//
// Description	:	Add a data at the index
//
///////////////////////////////////////////////////////////////////////////////////
void	CGexBoxPlotData::addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
                                    double dQ3Data, double dQ4Data, int nColors, const QString& strLabel)
{
    if (m_pColors == NULL)
        m_pColors	= new int[m_nSize];

    if (m_pLabels == NULL)
    {
        m_pLabels		= new char*[m_nSize];

        memset(m_pLabels, 0, m_nSize * sizeof(char*));
    }

    if (m_nSize > 0 && lIndex >= 0 && lIndex < m_nSize)
    {
       addData(lIndex, dQ0Data, dQ1Data, dQ2Data, dQ3Data, dQ4Data);

        m_pColors[lIndex]	= nColors;

        m_pLabels[lIndex]	= new char[strLabel.size()+1];

        strcpy(m_pLabels[lIndex], strLabel.toLatin1().constData());
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
//								double dQ3Data, double dQ4Data, double dXData)
//
// Description	:	Add a data at the index
//
///////////////////////////////////////////////////////////////////////////////////
void	CGexBoxPlotData::addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
                                    double dQ3Data, double dQ4Data, double dXData)
{
    if (m_pXData == NULL)
        m_pXData	= new double[m_nSize];

    if (m_nSize > 0 && lIndex >= 0 && lIndex < m_nSize)
    {
        m_pXData[lIndex]	= dXData;

        addData(lIndex, dQ0Data, dQ1Data, dQ2Data, dQ3Data, dQ4Data);
    }
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
//								double dQ3Data, double dQ4Data)
//
// Description	:	Add a data at the index
//
///////////////////////////////////////////////////////////////////////////////////
void CGexBoxPlotData::addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data, double dQ3Data, double dQ4Data)
{
    m_pQ0Data[lIndex]	= dQ0Data;
    m_pQ1Data[lIndex]	= dQ1Data;
    m_pQ2Data[lIndex]	= dQ2Data;
    m_pQ3Data[lIndex]	= dQ3Data;
    m_pQ4Data[lIndex]	= dQ4Data;

    if (dQ0Data != GEX_C_DOUBLE_NAN)
        m_dMinData = qMin(m_dMinData, dQ0Data);

    if (dQ4Data != GEX_C_DOUBLE_NAN)
        m_dMaxData = qMax(m_dMaxData, dQ4Data);
}
