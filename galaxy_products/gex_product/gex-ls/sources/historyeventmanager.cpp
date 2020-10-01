#include "historyeventmanager.h"
#include "gex-ls.h"


extern char *szAppFullName;

///////////////////////////////////////////////////////////////////////////////////
// Class CHistoryEventManager - class which manages the events processed by GEX-LM
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CHistoryEventManager::CHistoryEventManager()
{
	m_pStatisticManager = NULL;
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CHistoryEventManager::~CHistoryEventManager()
{
	clean();
}

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::setStatisticManager(CStatisticManager * pStatisticManager)
{
	m_pStatisticManager = pStatisticManager;
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void clean()
//
// Description	:	Clean the lists of events and connections.
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::clean()
{
	while(!m_eventLMHistory.isEmpty())
		delete m_eventLMHistory.takeFirst();

	while(!m_connectionLMHistory.isEmpty())
		delete m_connectionLMHistory.takeFirst();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	const CEventLM * pushEvent(const QString& strEvent)
//
// Description	:	Push a new event in the histrory manager and parse it.
//
// Param		:	strEvent	[in]	Event to parse
//
// Return		:	CEventLM*	-	The event parsed, NULL if the format is wrong
//
///////////////////////////////////////////////////////////////////////////////////
const CEventLM * CHistoryEventManager::pushEvent(const QString& strEvent)
{
	// Create the event
	CEventLM * pEventLM = CEventLM::CreateEventLM(strEvent);

	// Process the event
	if (pEventLM)
		processEvent(pEventLM);

	return pEventLM;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void pushEndEvent()
//
// Description	:	Push an event to finish the computation.
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::pushEndEvent()
{
	// if no statistic manager has been initiliazed, we don't need to simulate an end event.
	if (m_pStatisticManager)
		pushCloseEvent(m_pStatisticManager->dateTimeTo());
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void pushCloseEvent(const QDateTime& dateTimeClose)
//
// Description	:	Simulate a close event from the server
//
// Param		:	dateTimeClose	[in]	Date/time of the close event
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::pushCloseEvent(const QDateTime& dateTimeClose)
{
	// Create a string in order to simulate a close event. Used to finish the computation
	QString strEvent = HISTORY_STOP_EVENT;
	strEvent += " ";
	strEvent += dateTimeClose.toString("dd-MM-yyyy_hh:mm:ss");

	// Push the event in the history list
	pushEvent(strEvent);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void processEvent(CEventLM * pEventLM)
//
// Description	:	Process the event 
//
// Param		:	pEventLM	[in]	Event to process
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::processEvent(CEventLM * pEventLM)
{
	// Add the event in the list except the internal event
	if (pEventLM->type() != CEventLM::eventLMInternalStop)
		m_eventLMHistory.push_back(pEventLM);

	CEventLMClient * pEventLMClient = dynamic_cast<CEventLMClient*>(pEventLM);

	// If the event is a client event, try to create a connection
	if (pEventLMClient)
	{
		// Start event are handled until the close event is processed
		if (pEventLMClient->type() == CEventLM::eventLMClientStart)
			m_eventClientStartTmp.push_back(pEventLMClient);
		// if close event, try to retrieve the right start event and create a connection
		else if (pEventLMClient->type() == CEventLM::eventLMClientStop)
		{
			CEventLMClient * pEventLMStart = NULL;

			for (int nPos = 0; nPos < m_eventClientStartTmp.count() && !pEventLMStart; nPos++)
			{
				// the start event and the close event matche 
				if (pEventLMClient->isRelatedTo(*m_eventClientStartTmp.at(nPos)))
					pEventLMStart = m_eventClientStartTmp.takeAt(nPos);
			}

			// Create a new connection and process it 
			if (pEventLMStart)
				pushConnection(*pEventLMStart, *pEventLMClient);
		}
		// if internal event, close all opened connection
		else if (pEventLMClient->type() == CEventLM::eventLMInternalStop)
		{
			CEventLMClient * pEventLMStart = NULL;

			while (m_eventClientStartTmp.isEmpty() == false)
			{
				pEventLMStart = m_eventClientStartTmp.takeFirst();
					
				if (pEventLMStart)
					pushConnection(*pEventLMStart, *pEventLMClient);
			}
		}

		// Compute statistic for this event
		if (m_pStatisticManager)
			m_pStatisticManager->computeEventClient(pEventLMClient);
	}
	// If some connection are not closed when we are processing either a server close event or server start event. We have to close all these connections
	else if ((pEventLM->type() == CEventLM::eventLMServerStart || pEventLM->type() == CEventLM::eventLMServerStop) && m_eventClientStartTmp.isEmpty() == false)
		pushCloseEvent(pEventLM->dateTime());
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void pushConnection(const CEventLMClient& eventLMLogin, const CEventLMClient& eventLMLogout)
//
// Description	:	Create a new connection and push it in the histrory manager.
//
// Param		:	eventLMLogin	[in]	Event for starting the connection
//					eventLMLogout	[in]	Event for stopping the connection
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::pushConnection(const CEventLMClient& eventLMLogin, const CEventLMClient& eventLMLogout)
{
	// Create the connection object
	CConnectionLM * pConnectionLM = new CConnectionLM(eventLMLogin, eventLMLogout);

	// Process the connection
	if (pConnectionLM)
		processConnection(pConnectionLM);
}	

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void processConnection(CConnectionLM * pConnectionLM)
//
// Description	:	Process the connection 
//
// Param		:	pConnectionLM	[in]	Connection to process
//
///////////////////////////////////////////////////////////////////////////////////
void CHistoryEventManager::processConnection(CConnectionLM * pConnectionLM)
{
	// Add the connection to the list
	m_connectionLMHistory.push_back(pConnectionLM);

	// Compute statistic for this connection.
	if (m_pStatisticManager)
		m_pStatisticManager->computeConnection(*pConnectionLM);
}
