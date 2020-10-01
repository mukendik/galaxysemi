#if !defined(TEST_MARKER_H__INCLUDED_)
#define TEST_MARKER_H__INCLUDED_

//////////////////////////////////////////////////////
// Test marker
//////////////////////////////////////////////////////
#include <QString>
#include <QColor>

//////////////////////////////////////////////////////
// Test custom marker definition (thrugh scripting interface)
//////////////////////////////////////////////////////
/*! \class  TestMarker
    \brief  The TestMarker class holds information about a marker that can be displayed on charts.
*/
class	TestMarker
{
public:
    enum TestMarkerType
    {
        PatNearLimits = 0x0001,         // Near limits ("-N", "+N")
        PatMediumLimits = 0x0002,       // Medium limits ("-M", "+M")
        PatFarLimits = 0x0004,          // Far limits ("-F", "+F")
        PatRelaxedLimits = 0x0008,      // Relaxed limits ("xxx relaxed")
        PatLimits = PatNearLimits | PatMediumLimits | PatFarLimits | PatRelaxedLimits
    };

    TestMarker()
	{
		iLine		= 0;				// Line Width (0 if not marker set)
		iLayer		= 0;				// Default: layer#0 (1st group/Dataset analysed)
		lRunID		= -1;				// Run# in Samples array
		bForceColor = false;			// USe layer color if in multi-layer mode...
	}	

    /*! \brief Check if the marker is a PAT N,M,F marker.
        Returns true if the marker is a PAT N,M,F marker, false else.
    */
    bool    IsMarkerOfType(const TestMarkerType MarkerType);

	// Fields for custom markers
    QString	strLabel;			// Marker label (text)
	QColor	cColor;				// Marker color
	int		iLine;				// Line Width (0 if not marker set)
	double	lfPos;				// Marker position
	long	lRunID;				// Run#
	int		iLayer;				// Makes marker sticking to a given layer (0....n)
	bool	bForceColor;		// =true if must use its own color, even if multy-layer mode (where default is markers inherit of layer color)
};

#endif	// #ifdef TEST_MARKER_H__INCLUDED_
