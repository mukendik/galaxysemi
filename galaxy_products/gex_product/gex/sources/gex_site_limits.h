#ifndef GEX_SITE_LIMITS_H
#define GEX_SITE_LIMITS_H

#include "test_defines.h"
#include "time.h"

// Name			:	CGexSiteLimits
//
// Description	:	This class holds limits for a specific site

class CGexSiteLimits
{
public:

	CGexSiteLimits(double dLowLimit = -C_INFINITE, double dHighLimit = C_INFINITE);
	~CGexSiteLimits()	{ }

    time_t          limitTimeStamp() const          { return m_timeStamp; }
	const double&	lowLimit() const				{ return m_dLowLimit; }
	const double&	highLimit() const				{ return m_dHighLimit; }
	void			limits(double& dLowLimit, double& dHighLimit) const;

	void			setLowLimit(double dLowLimit);
	void			setHighLimit(double dHighLimit);
	void			setLimits(double dLowLimit, double dHighLimit);
    void            setLimitTimeStamp(time_t limitTimeStamp);

private:

	double	m_dLowLimit;
	double	m_dHighLimit;
    time_t  m_timeStamp;
};

#endif // GEX_SITE_LIMITS_H
