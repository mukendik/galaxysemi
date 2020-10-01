#ifndef GEX_CHART_DIR_H
#define GEX_CHART_DIR_H

///////////////////////////////////////////////////////////
// GEX Includes
///////////////////////////////////////////////////////////
#include <chartdir.h>

///////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////
#include <QString>
#include <QList>

// Define the best font that works under Windows or Unix for text labels, scales.
#if defined unix || __MACH__
	#define	GEXCHART_FONTNAME "Helvetica"		// Under Unix
#else
	#define	GEXCHART_FONTNAME "Times New Roman"	// Under Windows
#endif


struct MarkOrdered
{
    Mark*  mMarker;
    double mValue;
};


///////////////////////////////////////////////////////////
// ChartDirector drawing object: Class definition
///////////////////////////////////////////////////////////
class CChartDirector
{
public:

	CChartDirector();
	virtual ~CChartDirector();

	XYChart *		xyChart() const				{ return m_pXYChart; }

	bool			isEnabled()	const			{ return m_bEnabled; }

	bool			allocateXYChart(int nWidth, int nHeight, int nBackgroundColor);		// allocate the chart struct
	void			clearXYChart();														// clear the chart struct

    // draw the chart into a file
    virtual bool	drawChart(const QString& strFile, const QString& strCopyright);

    // Description	:	Add a marker to the chart
    Mark *			addMarker(Axis * pAxis, double lfValue, int nColor, const QString& strLabel, int nWidth = 1, Chart::Alignment alignment = Chart::TopLeft, bool smartAlignement = true);
    // draw a line : coords are relative to the current axis (V,mV, A, uA,...) (not pixel)
    void            addLine(double x1, double y1, double x2, double y2);

protected:

	XYChart *		m_pXYChart;					// chart structure, provided by chart director dll

private:

	bool			m_bEnabled;					// Set to 'false' if chart director lib is disabled (eg: Computer OS too old, etc...)
	int				m_nBackGroundColor;			// holds the background color of the chart




    QList<MarkOrdered>    marks;
    MarkOrdered          mLastMarkOrdered;
    void sortMarkersAndUpdate();

};
#endif	// #ifdef GEX_CHART_DIR_H
