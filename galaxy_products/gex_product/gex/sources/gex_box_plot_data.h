#ifndef GEX_BOX_PLOT_DATA_H
#define GEX_BOX_PLOT_DATA_H

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QString>

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexBoxPlotData
//
// Description	:	Class used to store the result of data computed for
//					Box Plot chart
//
///////////////////////////////////////////////////////////////////////////////////
class CGexBoxPlotData
{

    Q_DISABLE_COPY(CGexBoxPlotData);

public:

    CGexBoxPlotData();
    ~CGexBoxPlotData();

///////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////

    char **		labels() const			{ return m_pLabels;	}
    double *	dataQ0() const			{ return m_pQ0Data;	}
    double *	dataQ1() const			{ return m_pQ1Data;	}
    double *	dataQ2() const			{ return m_pQ2Data;	}
    double *	dataQ3() const			{ return m_pQ3Data;	}
    double *	dataQ4() const			{ return m_pQ4Data;	}
    double *	dataX() const			{ return m_pXData;	}
    int *		colors() const			{ return m_pColors; }

    double      minData() const         { return m_dMinData; }
    double      maxData() const         { return m_dMaxData; }

    int			size() const			{ return m_nSize;	}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

    void			init(int nSize);																		// initialize class and create internal array
    void			clean();																				// Clean data and delete internal array

    void			addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
                            double dQ3Data, double dQ4Data, int nColors, const QString& strLabel);			// Add a data at the index

    void			addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data,
                            double dQ3Data, double dQ4Data, double dXData);                                 // Add a data at the index

private:

    void            addData(long lIndex, double dQ0Data, double dQ1Data, double dQ2Data, double dQ3Data, double dQ4Data);

    int             m_nSize;				// Internal array size

    char **         m_pLabels;				// Array of labels
    double  *       m_pXData;               // Array of X data
    double *        m_pQ0Data;				// Array of Q0 data
    double *        m_pQ1Data;				// Array of Q1 data
    double *        m_pQ2Data;				// Array of Q2 data
    double *        m_pQ3Data;				// Array of Q3 data
    double *        m_pQ4Data;				// Array of Q4 data
    int *           m_pColors;				// Array of colors

    double          m_dMinData;
    double          m_dMaxData;
};

#endif // GEX_BOX_PLOT_DATA_H
