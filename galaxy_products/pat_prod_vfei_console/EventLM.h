#ifndef GEX_EVENT_LM_H
#define GEX_EVENT_LM_H

///////////////////////////////////////////////////////////////////////////////////
// QT Includes
///////////////////////////////////////////////////////////////////////////////////
#include <QStringList>
#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////////
// System Includes
///////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
    #include <windows.h>
#endif

#define HISTORY_STOP_EVENT				"g_event_stop"

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CEventLM
//
// Description	:	Base class which represents an event processed by GX-LM.
//					This class is not instanciable
//
///////////////////////////////////////////////////////////////////////////////////
class CEventLM
{
			
public:

	enum eventLMType
	{
		eventLMUnknown = -1,
		eventLMServerStart = 0,
		eventLMServerStop,
		eventLMServerMessage,
		eventLMClientStart,
		eventLMClientStop,
		eventLMInternalStop,
		eventLMClientRejected
	};
	
	CEventLM();
	~CEventLM();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	QString				eventName() const		{ return m_strEventName; }
	QDateTime			dateTime() const		{ return m_dateTime; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	void				init(const QString& strString);				// Initialize the event data by parsing the log string		
	virtual void		rawData(QStringList& strRawData) const;		// Put in a string list the raw datas		
	virtual eventLMType	type() const = 0;							// Get the type of the event							
	static CEventLM *	CreateEventLM(const QString& strString);	// Create the right event according the event type 	

protected:

	virtual void		parseEventLM(const QString& strString) = 0;	// Parse the log line from GEX-LM	 

private:
	
	QString				m_strEventName;								// event name								
	QDateTime			m_dateTime;									// date and time of the event
	
};


///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CEventLMServer
//
// Description	:	Base class which represents a server event processed by GX-LM.
//
///////////////////////////////////////////////////////////////////////////////////
class CEventLMServer : public CEventLM
{

public:
	
	CEventLMServer();
	~CEventLMServer();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	QString				message() const			{ return m_strMessage; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	void				rawData(QStringList& strRawData) const;				// Put in a string list the raw datas
	eventLMType			type() const;										// Get the type of event

protected:
	
	void				parseEventLM(const QString& strString);				// Parse the log line from GEX-LM

private:
	
	QString				m_strMessage;										// message displayed for server

};

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CEventLMClient
//
// Description	:	Base class which represents a client event processed by GX-LM.
//
///////////////////////////////////////////////////////////////////////////////////
class CEventLMClient : public CEventLM
{

public:

	CEventLMClient();
	~CEventLMClient();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	QString				computer() const		{ return m_strComputer; }
	QString				user() const			{ return m_strUser;	}
	long				licenseUsed() const		{ return m_lLicenseUsed; }
	long				licenseAllowed() const	{ return m_lLicenseAllowed;	}
	long				peerPort() const		{ return m_lPeerPort; }
	long				productID() const		{ return m_lProductID; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	void				rawData(QStringList& strRawData) const;						// Put in a string list the raw datas
	virtual eventLMType	type() const;												// Get the type of the event
	virtual bool		isRelatedTo(const CEventLMClient& eventLMClient) const;		// Check if both events are related to the same connection
	
protected:
	
	virtual void		parseEventLM(const QString& strString);						// Parse the log line from GEX-LM

protected:
	
	QString				m_strComputer;										// computer name
	QString				m_strUser;											// user name 
	long				m_lLicenseUsed;										// Number of license used when event is processed
	long				m_lLicenseAllowed;									// Max number of license allowed by GEX-LM
	long				m_lPeerPort;										// Port used by GEX-LM to identify the client
	long				m_lProductID;										// ID of the product used on the client station

};

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CEventLMClient
//
// Description	:	Base class which represents a client event rejected by GX-LM.
//
///////////////////////////////////////////////////////////////////////////////////
class CEventLMClientRejected : public CEventLMClient
{

public:

	CEventLMClientRejected();
	~CEventLMClientRejected();

///////////////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////////////

	QString				licenseMessage() const	{ return m_strLicenseMessage; }

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	void				rawData(QStringList& strRawData) const;					// Put in a string list the raw datas
	eventLMType			type() const;											// Get the type of the event
	bool				isRelatedTo(const CEventLMClient& eventLMClient) const;	// Check if both events are related to the same connection

protected:
		
	void				parseEventLM(const QString& strString);					// Parse the log line from GEX-LM

private:
	
	QString				m_strLicenseMessage;									// Message error for rejected event
};

///////////////////////////////////////////////////////////////////////////////////
// 
// Name			:	CEventLMClient
//
// Description	:	Base class which represents a internal event sent by GEX-LS.
//
///////////////////////////////////////////////////////////////////////////////////
class CEventLMClientInternal : public CEventLMClient
{

public:

	CEventLMClientInternal();
	~CEventLMClientInternal();

///////////////////////////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////////////////////////

	eventLMType			type() const;											// Get the type of the event										
	bool				isRelatedTo(const CEventLMClient& eventLMClient) const;	// Check if both events are related to the same connection

protected:
	
	void				parseEventLM(const QString& strString);					// Parse the log line from GEX-LM
};

#endif // GEX_EVENT_LM_H