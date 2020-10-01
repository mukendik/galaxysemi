#ifndef GEX_HISTORY_EVENT_MANAGER_H
#define GEX_HISTORY_EVENT_MANAGER_H

///////////////////////////////////////////////////////////////////////////////////
// GEX-LS Includes
///////////////////////////////////////////////////////////////////////////////////
#include "statisticmanager.h"

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QList>

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CHistoryEventManager
//
// Description	:	Parse and manage the event received from GEX-LM.
//
///////////////////////////////////////////////////////////////////////////////////
class CHistoryEventManager
{

public:
	
	CHistoryEventManager();
	~CHistoryEventManager();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	void					setStatisticManager(CStatisticManager * pStatisticManager);

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////
	
	void					clean();													// Clean the lists of events and connections
	const CEventLM *		pushEvent(const QString& strEvent);							// Push a new event in the histrory manager and parse it
	void					pushEndEvent();												// Push an event to finish the computation

private:
	
	void					pushCloseEvent(const QDateTime& dateTimeClose);				// simulate a close event from the server
	void					processEvent(CEventLM * pEventLM);							// Process the event
	void					processConnection(CConnectionLM * pConnectionLM);			// Process the connection	
	void					pushConnection(const CEventLMClient& eventLMLogin, const CEventLMClient& eventLMLogout);	// Create a new connection and push it in the histrory manager
				
	QList<CEventLM*>		m_eventLMHistory;				// List of event received from GEX-LM
	QList<CConnectionLM*>	m_connectionLMHistory;			// List of connection managed by GEX-LM
	QList<CEventLMClient*>	m_eventClientStartTmp;			// List of client start event without close event

	CStatisticManager *		m_pStatisticManager;			// Compute and manage the datas for statistic
};

#endif // GEX_HISTORY_EVENT_MANAGER_H
