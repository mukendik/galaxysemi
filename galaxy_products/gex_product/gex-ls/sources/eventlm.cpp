#include "eventlm.h"


///////////////////////////////////////////////////////////////////////////
// Class CEventLM - Base class which describes an event processed by GEX-LM
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CEventLM::CEventLM()
{
	m_dateTime = QDateTime::currentDateTime();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CEventLM::~CEventLM()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void init(const QString& strString)
//
// Description	:	Initialize the event data by parsing the log string
//
// Param		:	strString	[in]	String of the event to log
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLM::init(const QString& strString)
{
	parseEventLM(strString);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void rawData(QStringList& strRawData) const
//
// Description	:	Put in a string list the raw datas
//
// Param		:	strRawData	[out]	String list of the raw datas
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLM::rawData(QStringList& strRawData) const
{
	strRawData << m_strEventName;

	strRawData << m_dateTime.date().toString(Qt::ISODate);

	strRawData << m_dateTime.time().toString(Qt::ISODate);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	static CEventLM * CreateEventLM(const QString& strString)
//
// Description	:	Create the right event according the event type 
//
// Param		:	strString	[out]	String log
//
// Return 		:	CEventLM *	-	A pointer on the event created, NULL if log string
//									is in a bad format
//
///////////////////////////////////////////////////////////////////////////////////
CEventLM * CEventLM::CreateEventLM(const QString& strString)
{
	CEventLM * pEventLM = NULL;

	if (strString.startsWith("g_start") || strString.startsWith("g_stop") || strString.startsWith("g_message") || strString.startsWith("g_stop_abnormal"))
		pEventLM = new CEventLMServer;
	else if (strString.startsWith("g_client_start") || strString.startsWith("g_client_stop"))
		pEventLM = new CEventLMClient;
	else if (strString.startsWith(HISTORY_STOP_EVENT))
		pEventLM = new CEventLMClientInternal;
	else if (strString.startsWith("g_client_rejected"))
		pEventLM = new CEventLMClientRejected;

	// Parse the string and initialize the even object
	if (pEventLM)
		pEventLM->init(strString);

	return pEventLM;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void parseEventLM(const QString& strString) = 0
//
// Description	:	Parse the log line from GEX-LM
//					virtual pure function : must be implemented in each subclass
//
// Param 		:	strString	[in]	String to parse
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLM::parseEventLM(const QString& strString)
{
	// Get the event type
	m_strEventName = strString.section(' ', 0, 0);

	// Get the date/time part
	QString strDateTime = strString.section(' ', 1, 1);

	// Get the date
	m_dateTime = QDateTime::fromString(strDateTime, "dd-MM-yyyy_hh:mm:ss");
}

///////////////////////////////////////////////////////////////////////////////////
// Class CEventLMServer - class which describes a server event processed by GEX-LM
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CEventLMServer::CEventLMServer() : CEventLM()
{
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CEventLMServer::~CEventLMServer()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	eventLMType	type() const
//
// Description	:	Get the type of the event 
//
// Return 		:	eventLMType	-	type of the event
//					
///////////////////////////////////////////////////////////////////////////////////
CEventLM::eventLMType CEventLMServer::type() const
{
	eventLMType evtType = eventLMUnknown;

	if (eventName() == "g_start")
		evtType = eventLMServerStart;
	else if (eventName() == "g_stop"  || eventName() == "g_stop_abnormal")
		evtType = eventLMServerStop;
	else if (eventName() == "g_message")
		evtType = eventLMServerMessage;

	return evtType;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void rawData(QStringList& strRawData) const
//
// Description	:	Put in a string list the raw datas
//
// Param		:	strRawData	[out]	String list of the raw datas
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMServer::rawData(QStringList& strRawData) const
{
	CEventLM::rawData(strRawData);

	strRawData << " " << " " << " " << " " << m_strMessage;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void parseEventLM(const QString& strString)
//
// Description	:	Parse the log line from GEX-LM
//					virtual pure function : must be implemented in each subclass
//
// Param 		:	strString	[in]	String to parse
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMServer::parseEventLM(const QString& strString)
{
	// Get the common part
	CEventLM::parseEventLM(strString);
	
	// Get the message
	m_strMessage = strString.section(' ', 2);
}

///////////////////////////////////////////////////////////////////////////////////
// Class CEventLMClient - class which describes a client event processed by GEX-LM
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CEventLMClient::CEventLMClient() : CEventLM()
{
	m_lPeerPort			= 0;
	m_lProductID		= -1;
	m_lLicenseUsed		= 0;
	m_lLicenseAllowed	= 0;
}	

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CEventLMClient::~CEventLMClient()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual eventLMType	type() const
//
// Description	:	Get the type of the event 
//
// Return 		:	eventLMType	-	type of the event
//					
///////////////////////////////////////////////////////////////////////////////////
CEventLM::eventLMType CEventLMClient::type() const
{
	eventLMType evtType = eventLMUnknown;

	if (eventName() == "g_client_start")
		evtType = eventLMClientStart;
	else if (eventName() == "g_client_stop")
		evtType = eventLMClientStop;

	return evtType;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void rawData(QStringList& strRawData) const
//
// Description	:	Put in a string list the raw datas
//
// Param		:	strRawData	[out]	String list of the raw datas
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMClient::rawData(QStringList& strRawData) const
{
	CEventLM::rawData(strRawData);

	// data format is : <event> <date> <time> <computer> <user> <license_used> <license_allowed>
	strRawData << m_strComputer << m_strUser << QString::number(m_lLicenseUsed) << QString::number(m_lLicenseAllowed);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void parseEventLM(const QString& strString)
//
// Description	:	Parse the log line from GEX-LM
//					virtual pure function : must be implemented in each subclass
//
// Param 		:	strString	[in]	String to parse
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMClient::parseEventLM(const QString& strString)
{
	// Get the common part
	CEventLM::parseEventLM(strString);

	// Get the computer name
	m_strComputer = strString.section(' ', 2, 2);

	// Get the user name
	m_strUser = strString.section(' ', 3, 3);

	// Get the license# used
	m_lLicenseUsed = strString.section(' ', 4, 4).toLong();

	// Get the license# allowed
	m_lLicenseAllowed = strString.section(' ', 5, 5).toLong();

	// Get the peer port used
	m_lPeerPort = strString.section(' ', 6, 6).toLong();

	// Get the product ID
	m_lProductID = strString.section(' ', 7, 7).toLong();
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual bool isRelatedTo(const CEventLMClient& eventLMClient) const
//
// Description	:	Check if both events are related to the same connection
//
// Param		:	eventLMClient	[in]	Event to compare
//					
// Return		:	bool	-	True if both event are related to same connection
//								False otherwise
///////////////////////////////////////////////////////////////////////////////////
bool CEventLMClient::isRelatedTo(const CEventLMClient& eventLMClient) const
{
	bool bIsRelated = false;

	// If one peerport is equal to one, we have an old version of log file, we will compare
	// the events with computer name and user name
	if (peerPort() == 0 || eventLMClient.peerPort() == 0)
	{
		if (computer() == eventLMClient.computer() && user() == eventLMClient.user())
			bIsRelated = true;
	}
	else if (peerPort() == eventLMClient.peerPort())
		bIsRelated = true;

	return bIsRelated;
}

///////////////////////////////////////////////////////////////////////////////////
// Class CEventLMClientInternal - class which describes a internal client event
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CEventLMClientInternal::CEventLMClientInternal()
{
}	

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CEventLMClientInternal::~CEventLMClientInternal()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	eventLMType	type() const
//
// Description	:	Get the type of the event 
//
// Return 		:	eventLMType	-	type of the event
//					
///////////////////////////////////////////////////////////////////////////////////
CEventLM::eventLMType CEventLMClientInternal::type() const
{
	eventLMType evtType = eventLMUnknown;

	if (eventName() == HISTORY_STOP_EVENT)
		evtType = eventLMInternalStop;
	
	return evtType;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void parseEventLM(const QString& strString)
//
// Description	:	Parse the log line from GEX-LM
//
// Param 		:	strString	[in]	String to parse
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMClientInternal::parseEventLM(const QString& strString)
{
	// Get the common part
	CEventLM::parseEventLM(strString);
}
	
///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	bool isRelatedTo(const CEventLMClient& eventLMClient) const
//
// Description	:	Check if both events are related to the same connection
//
// Param		:	eventLMClient	[in]	Event to compare
//					
// Return		:	bool	-	True if both event are related to same connection
//								False otherwise
///////////////////////////////////////////////////////////////////////////////////
bool
CEventLMClientInternal::
isRelatedTo(const CEventLMClient& /*eventLMClient*/) const
{
	// internal event is only used to close all connections which are still opened, so
	// it isn't related to a connection
	return false;
}

///////////////////////////////////////////////////////////////////////////////////
// Class CEventLMClientRejected - class which describes a rejected client event
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
CEventLMClientRejected::CEventLMClientRejected()
{
}	

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
CEventLMClientRejected::~CEventLMClientRejected()
{
}

///////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	eventLMType	type() const
//
// Description	:	Get the type of the event 
//
// Return 		:	eventLMType	-	type of the event
//					
///////////////////////////////////////////////////////////////////////////////////
CEventLM::eventLMType CEventLMClientRejected::type() const
{
	eventLMType evtType = eventLMUnknown;

	if (eventName() == "g_client_rejected")
		evtType = eventLMClientRejected;
	
	return evtType;
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	void rawData(QStringList& strRawData) const
//
// Description	:	Put in a string list the raw datas
//
// Param		:	strRawData	[out]	String list of the raw datas
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMClientRejected::rawData(QStringList& strRawData) const
{
	CEventLM::rawData(strRawData);

	// data format is : <event> <date> <time> <computer> <user> <license_used> <license_allowed>
	strRawData << computer() << user() << m_strLicenseMessage << QString::number(licenseAllowed());
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	virtual void parseEventLM(const QString& strString)
//
// Description	:	Parse the log line from GEX-LM
//					virtual pure function : must be implemented in each subclass
//
// Param 		:	strString	[in]	String to parse
//					
///////////////////////////////////////////////////////////////////////////////////
void CEventLMClientRejected::parseEventLM(const QString& strString)
{
	// Get the common part
	CEventLMClient::parseEventLM(strString);

	// Get the rejection message in the field license used
	m_strLicenseMessage = strString.section(' ', 4, 4);
}

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	bool isRelatedTo(const CEventLMClient& eventLMClient) const
//
// Description	:	Check if both events are related to the same connection
//
// Param		:	eventLMClient	[in]	Event to compare
//					
// Return		:	bool	-	True if both event are related to same connection
//								False otherwise
///////////////////////////////////////////////////////////////////////////////////
bool
CEventLMClientRejected::
isRelatedTo(const CEventLMClient& /*eventLMClient*/) const
{
	// rejected event isn't related to a connection, it is a single event
	return false;
}
