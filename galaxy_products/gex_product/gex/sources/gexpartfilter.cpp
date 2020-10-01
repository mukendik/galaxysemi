#include "gexpartfilter.h"
#include "ctest.h"
#include "gex_constants.h"

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilter
//
// Description	:	Class holding the filter for parts
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexPartFilter::CGexPartFilter()
{
	m_pPartFilterPrivate = NULL;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexPartFilter::~CGexPartFilter()
{
	if (m_pPartFilterPrivate)
	{
		delete m_pPartFilterPrivate;
		m_pPartFilterPrivate = NULL;
	}
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool isFiltered(double dValue) const
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexPartFilter::isFiltered(double dValue) const
{
	if (m_pPartFilterPrivate)
		return m_pPartFilterPrivate->contains(dValue);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	void addFilter(long lTestNumber, double dLowValue, double dHighValue, filterType eType)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
void CGexPartFilter::addFilter(long lTestNumber, double dLowValue, double dHighValue, filterType eType)
{
	if (m_pPartFilterPrivate)
	{
		delete m_pPartFilterPrivate;
		m_pPartFilterPrivate = NULL;
	}

	m_pPartFilterPrivate = CGexPartFilterPrivate::create(lTestNumber, dLowValue, dHighValue, eType);
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexPartFilterPrivate::CGexPartFilterPrivate(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eType)
{
	m_lTestNumber	= lTestNumber;
	m_dLowValue		= dLowValue;
	m_dHighValue	= dHighValue;
	m_eType			= eType;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexPartFilterPrivate::~CGexPartFilterPrivate()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterPrivate * create(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eType)
//
// Description	:
//
///////////////////////////////////////////////////////////////////////////////////
CGexPartFilterPrivate * CGexPartFilterPrivate::create(long lTestNumber, double dLowValue, double dHighValue, CGexPartFilter::filterType eType)
{
	CGexPartFilterPrivate * pFilter = NULL;

	switch (eType)
	{
		case CGexPartFilter::filterOver		:	pFilter = new CGexPartFilterOverPrivate(lTestNumber, dLowValue);
                                                                break;
		case CGexPartFilter::filterUnder	:	pFilter = new CGexPartFilterUnderPrivate(lTestNumber, dHighValue);
                                                                break;
                case CGexPartFilter::filterInside       :       pFilter = new CGexPartFilterInsidePrivate(lTestNumber,dLowValue ,dHighValue);
                                                                break;
                case CGexPartFilter::filterOutside       :       pFilter = new CGexPartFilterOutsidePrivate(lTestNumber,dLowValue ,dHighValue);
                                                                break;
                default					:	break;
	}

	return pFilter;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterOverPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexPartFilterOverPrivate::CGexPartFilterOverPrivate(long lTestNumber, double dValue) : CGexPartFilterPrivate(lTestNumber, dValue, 0, CGexPartFilter::filterOver)
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexPartFilterOverPrivate::~CGexPartFilterOverPrivate()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool contains(double dValue) const
//
// Description	:	check if filter contains part value
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexPartFilterOverPrivate::contains(double dValue) const
{
	if (dValue != GEX_C_DOUBLE_NAN)
		return (dValue >= m_dLowValue);

	return false;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	CGexPartFilterUnderPrivate
//
// Description	:	Class holding the rule to filter a part
//
///////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CGexPartFilterUnderPrivate::CGexPartFilterUnderPrivate(long lTestNumber, double dValue) : CGexPartFilterPrivate(lTestNumber, 0, dValue, CGexPartFilter::filterUnder)
{
}

//////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CGexPartFilterUnderPrivate::~CGexPartFilterUnderPrivate()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//
// Name			:	bool contains(double dValue) const
//
// Description	:	check if filter contains part value
//
///////////////////////////////////////////////////////////////////////////////////
bool CGexPartFilterUnderPrivate::contains(double dValue) const
{
	if (dValue != GEX_C_DOUBLE_NAN)
		return (dValue < m_dHighValue);

	return false;
}

CGexPartFilterInsidePrivate::CGexPartFilterInsidePrivate(long lTestNumber, double dLowValue, double dHighValue)
    : CGexPartFilterPrivate(lTestNumber, dLowValue, dHighValue, CGexPartFilter::filterInside)
{

}

CGexPartFilterInsidePrivate::~CGexPartFilterInsidePrivate(){

}

bool	CGexPartFilterInsidePrivate::contains(double dValue) const{
    if (dValue != GEX_C_DOUBLE_NAN)
            return ( m_dLowValue<= dValue  && dValue <= m_dHighValue );

    return false;
}

CGexPartFilterOutsidePrivate::CGexPartFilterOutsidePrivate(long lTestNumber, double dLowValue, double dHighValue)
    : CGexPartFilterPrivate(lTestNumber, dLowValue, dHighValue, CGexPartFilter::filterOutside)
{

}

CGexPartFilterOutsidePrivate::~CGexPartFilterOutsidePrivate(){

}

bool	CGexPartFilterOutsidePrivate::contains(double dValue) const{
    if (dValue != GEX_C_DOUBLE_NAN)
            return ( m_dLowValue>= dValue  || dValue >= m_dHighValue );

    return false;
}

