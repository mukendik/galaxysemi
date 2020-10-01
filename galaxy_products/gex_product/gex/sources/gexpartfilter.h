#ifndef GEXPARTFILTER_H
#define GEXPARTFILTER_H

#include <stdio.h>

class CGexPartFilterPrivate;

// Name			:	CGexPartFilter
// Description	:	Class holding the filtering for parts

class CGexPartFilter
{

public:

	enum filterType
	{
		filterOver= 0,
		filterUnder,
		filterInside,
		filterOutside
	};

	CGexPartFilter();
	~CGexPartFilter();

	const	CGexPartFilterPrivate *		partFilterPrivate() const		{ return m_pPartFilterPrivate;	}

	bool	isFiltered(double dValue) const;
	bool	isActivated() const											{ return m_pPartFilterPrivate != NULL; }

	void	addFilter(long lTestNumber, double dLowValue, double dHighValue, filterType eType);

private:

	CGexPartFilterPrivate *	m_pPartFilterPrivate;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////
class CGexPartFilterPrivate
{

public:

	CGexPartFilterPrivate(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eType);
	virtual ~CGexPartFilterPrivate();

	CGexPartFilter::filterType 			type() const				{ return m_eType; }
	long								testNumber() const			{ return m_lTestNumber; }
	double								lowValue() const			{ return m_dLowValue; }
	double								highValue() const			{ return m_dHighValue; }

	virtual bool						contains(double dValue) const = 0;

	static CGexPartFilterPrivate *		create(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eFilter);

protected:

	long							m_lTestNumber;
	double							m_dLowValue;
	double							m_dHighValue;
	CGexPartFilter::filterType		m_eType;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterOverPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////
class CGexPartFilterOverPrivate : public CGexPartFilterPrivate
{

public:

	CGexPartFilterOverPrivate(long lTestNumber, double dValue);
	~CGexPartFilterOverPrivate();

	bool	contains(double dValue) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterUnderPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////
class CGexPartFilterUnderPrivate : public CGexPartFilterPrivate
{

public:

	CGexPartFilterUnderPrivate(long lTestNumber, double dValue);
	~CGexPartFilterUnderPrivate();

	bool	contains(double dValue) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterUnderPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////
class CGexPartFilterInsidePrivate : public CGexPartFilterPrivate
{

public:

        CGexPartFilterInsidePrivate(long lTestNumber, double dLowValue, double dHighValue);
        ~CGexPartFilterInsidePrivate();

        bool	contains(double dValue) const;
};

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterUnderPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////
class CGexPartFilterOutsidePrivate : public CGexPartFilterPrivate
{

public:

        CGexPartFilterOutsidePrivate(long lTestNumber, double dLowValue, double dHighValue);
        ~CGexPartFilterOutsidePrivate();

        bool	contains(double dValue) const;
};


#endif // GEXPARTFILTER_H
