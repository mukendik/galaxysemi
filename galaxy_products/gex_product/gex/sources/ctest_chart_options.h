#if !defined(GEX_CTEST_CHARTOPTIONS_H__INCLUDED_)
#define GEX_CTEST_CHARTOPTIONS_H__INCLUDED_

//////////////////////////////////////////////////////
// Test charting options
//////////////////////////////////////////////////////
#include <qstring.h>

class CGexTestChartingOptions
{
public:
	
	CGexTestChartingOptions()
	{
		m_bLotMarker				= true;
		m_bCustomViewPortY			= false;
		m_bCustomViewPortX			= false;
	}

// PROPERTIES

	bool	lotMarker() const									{ return m_bLotMarker; }
	bool	customViewportX() const								{ return m_bCustomViewPortX; }
	bool	customViewportY() const								{ return m_bCustomViewPortY; }
	double	lowY() const										{ return m_lfLowY; }
	double	lowX() const										{ return m_lfLowX; }
	double	highY() const										{ return m_lfHighY; }
	double	highX() const										{ return m_lfHighX; }

	void	setLotMarker(bool bLotMarker)						{ m_bLotMarker = bLotMarker; }
	void	setCustomViewportX(bool bCustomViewport)			{ m_bCustomViewPortX = bCustomViewport; }
	void	setCustomViewportY(bool bCustomViewport)			{ m_bCustomViewPortY = bCustomViewport; }
	void	setLowX(double dLowX)								{ m_lfLowX = dLowX; }
	void	setLowY(double dLowY)								{ m_lfLowY = dLowY; }
	void	setHighX(double dHighX)								{ m_lfHighX = dHighX; }
	void	setHighY(double dHighY)								{ m_lfHighY = dHighY; }

private:

	bool		m_bLotMarker;				// Show/hide lots markers.
	bool		m_bCustomViewPortY;			// =true if charting custom viewport in Y (defined herefater)
	double		m_lfLowY;				
	double		m_lfHighY;
	bool		m_bCustomViewPortX;			// =true if charting custom viewport in Y (defined herefater)
	double		m_lfLowX;
	double		m_lfHighX;
};

#endif	// #ifdef GEX_CTEST_CHARTOPTIONS_H__INCLUDED_
