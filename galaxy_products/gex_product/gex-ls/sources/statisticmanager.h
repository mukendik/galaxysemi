#ifndef GEX_STATISTIC_MANAGER_H
#define GEX_STATISTIC_MANAGER_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-LS Includes
///////////////////////////////////////////////////////////////////////////////////
#include "eventlm.h"
#include "connectionlm.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QMap>

typedef QMap<long, long>									mapParetoLicense;
typedef QMap<long, long>::const_iterator					it_mapParetoLicenseMap;

typedef QMap<QString, CConnectionGroup>						mapConnectionGroup;
typedef QMap<QString, CConnectionGroup>::const_iterator		it_mapConnectionGroup;

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CStatisticManager
//
// Description	:	Compute and manage the datas statistics.
//
///////////////////////////////////////////////////////////////////////////////////
class CStatisticManager
{

public:

	CStatisticManager();
	~CStatisticManager();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
	
	const mapParetoLicense&		paretoLicense() const				{ return m_mapPareto; }
	const mapConnectionGroup&	connectionGroupComputer() const		{ return m_mapConnectionGroupComputer; }
	const mapConnectionGroup&	connectionGroupUser() const			{ return m_mapConnectionGroupUser; }

	const CConnectionGroup&		connectionGroupAll() const			{ return m_connectionGroupAll; }
	const QDateTime&			dateTimeFrom() const				{ return m_dtReferenceBegin; }
	const QDateTime&			dateTimeTo() const					{ return m_dtReferenceEnd; }

	long						maxLicenseUsed() const				{ return m_lMaxLicenseUsed;	}

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////
	
	void						init(const QDate& dateFrom, const QDate& dateTo);		// Initialize the start date and the end date
	void						clear();												// Clear the statistics stables already computed
	void						computeEventClient(CEventLMClient * pEventLMClient);	// Compute statistic of pareto for a client event
	void						computeConnection(const CConnectionLM& connectionLM);	// Compute statistic of connection for a client connection
	long						period() const;											// Calculate the time period used for the statistic in seconds

private:

	mapParetoLicense			m_mapPareto;						// pareto of license used
	mapConnectionGroup			m_mapConnectionGroupComputer;		// connections statistics by computer
	mapConnectionGroup			m_mapConnectionGroupUser;			// connections statistics by user
	CConnectionGroup			m_connectionGroupAll;				// connections statistics for all

	long						m_lMaxLicenseUsed;					// Maximum number of license used at a moment
	
	QDateTime					m_dtReferenceBegin;					// Start date/time of statistics
	QDateTime					m_dtReferenceEnd;					// end date/time of statistics
	
	long						m_lLicenseNumber;					// Last license number computed
	QDateTime					m_dtReferenceLast;					// date/time of the last event computed
};

#endif // GEX_STATISTIC_MANAGER_H
