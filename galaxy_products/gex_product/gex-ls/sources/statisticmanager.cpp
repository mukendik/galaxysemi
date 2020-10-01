#include "statisticmanager.h"


///////////////////////////////////////////////////////////////////////////////////
// Class CStatisticManager - class which manages the statistic computed by GEX-LM
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CStatisticManager::CStatisticManager()
{
	m_lLicenseNumber		= -1;
	m_lMaxLicenseUsed		= 0;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CStatisticManager::~CStatisticManager()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void init(const QDate& dateFrom, const QDate& dateTo)
//
// Description	:	Initialize the start date and the end date.
//
// Param		:	dateFrom	[in]	Start date/time
//					dateTo		[in]	End date/time
///////////////////////////////////////////////////////////////////////////////////
void CStatisticManager::init(const QDate& dateFrom, const QDate& dateTo)
{
	QDateTime dateCurrent =  QDateTime::currentDateTime();

	m_lLicenseNumber		= -1;
	m_lMaxLicenseUsed		= 0;
	m_dtReferenceBegin.setDate(dateFrom);
	m_dtReferenceBegin.setTime(QTime(0, 0, 0));
	m_dtReferenceEnd.setDate(dateTo);
	m_dtReferenceEnd.setTime(QTime(23, 59, 59));

	// Initialize some value
	if (dateCurrent < m_dtReferenceEnd)
		m_dtReferenceEnd = dateCurrent;

	m_dtReferenceLast = m_dtReferenceBegin;
	
	clear();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void clear()
//
// Description	:	Clear the statistics stables already computed.
//
///////////////////////////////////////////////////////////////////////////////////
void CStatisticManager::clear()
{
	m_mapPareto.clear();
	m_mapConnectionGroupComputer.clear();
	m_mapConnectionGroupUser.clear();
	
	m_connectionGroupAll.clear();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	long period() const
//
// Description	:	Calculate the time period used for the statistic in seconds				
//
// Return		:	long	- 	time period in seconds
//
///////////////////////////////////////////////////////////////////////////////////
long CStatisticManager::period() const 
{
	long lPeriod = m_dtReferenceBegin.secsTo(m_dtReferenceEnd);

	if (lPeriod < 0)
		lPeriod = 0;

	return lPeriod;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void computeEventClient(CEventLMClient * pEventLMClient)
//
// Description	:	Compute statistic of pareto for a client event
//
// Param		:	pEventLMClient	[in]	Client event
//
///////////////////////////////////////////////////////////////////////////////////
void CStatisticManager::computeEventClient(CEventLMClient * pEventLMClient)
{
	// Compute the first event, we will look for the license number in use
	if (m_lLicenseNumber == -1)
	{
		// if we are computing a client disconnection, the previous license number in use
		// was the max between the current license number plus 1 and the number of licenses allowed
		// if we are computing a client connection, the previous license number in use
		// was current license number minus 1.
		if (pEventLMClient->type() == CEventLM::eventLMClientStop)
		{
			m_lLicenseNumber = qMax(pEventLMClient->licenseUsed()+1, pEventLMClient->licenseAllowed());
		}
		else if (pEventLMClient->type() == CEventLM::eventLMClientStart)
			m_lLicenseNumber = pEventLMClient->licenseUsed() - 1;
	}

	// Calculate the elapsed time
	long lElapsedTime = m_dtReferenceLast.secsTo(pEventLMClient->dateTime());

	// Look for a previous time for this license number
	it_mapParetoLicenseMap itMapNodeTime = m_mapPareto.find(m_lLicenseNumber);

	// Increase the elapsed time for this license number
	if (itMapNodeTime != m_mapPareto.end())
		lElapsedTime += itMapNodeTime.value(); 
	
	m_mapPareto.insert(m_lLicenseNumber, lElapsedTime);
	
	// Reference changes
	m_dtReferenceLast = pEventLMClient->dateTime();

	// License number in use changes
	// In order to manage old log files, we check if the event license number is 
	// equal to 0 and license number is greater than 1. 
	// In this case, the next license number will be licensenumber - 1;
	if (pEventLMClient->licenseUsed() == 0 && m_lLicenseNumber > 1)
		m_lLicenseNumber -= 1;
	else
		m_lLicenseNumber  = pEventLMClient->licenseUsed();

	// Check if the license number in used is greater than the max
	if (m_lMaxLicenseUsed < m_lLicenseNumber)
		m_lMaxLicenseUsed = m_lLicenseNumber;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void computeConnection(const CConnectionLM& connectionLM)
//
// Description	:	Compute statistic of connection for a client connection
//
// Param		:	connectionLM	[in]	Client connection
//
///////////////////////////////////////////////////////////////////////////////////
void CStatisticManager::computeConnection(const CConnectionLM& connectionLM)
{
	// Compute information for all connection
	m_connectionGroupAll.addConnection(connectionLM);
	
	// Compute information by computer
	mapConnectionGroup::iterator itConnectionGroup = m_mapConnectionGroupComputer.find(connectionLM.computer());

	// Computer group already exists, update it
	if (itConnectionGroup != m_mapConnectionGroupComputer.end())
		itConnectionGroup.value().addConnection(connectionLM);
	else
	{
		// Create a new connection group for this computer
		CConnectionGroup connectionGroup;
		connectionGroup.addConnection(connectionLM);

		m_mapConnectionGroupComputer.insert(connectionLM.computer(), connectionGroup);
	}

	// Compute information by user
	itConnectionGroup = m_mapConnectionGroupUser.find(connectionLM.user());

	// User group already exists, update it
	if (itConnectionGroup != m_mapConnectionGroupUser.end())
		itConnectionGroup.value().addConnection(connectionLM);
	else
	{
		// Create a new connection group for this user
		CConnectionGroup connectionGroup;
		connectionGroup.addConnection(connectionLM);

		m_mapConnectionGroupUser.insert(connectionLM.user(), connectionGroup);
	}
}
