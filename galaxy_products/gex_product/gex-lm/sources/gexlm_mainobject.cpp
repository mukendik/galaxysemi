///////////////////////////////////////////////////////////
// GEX-LM main object implementation file
///////////////////////////////////////////////////////////

#include <QTimer>
#include <QFile>
#include <gqtl_sysutils.h>

#include "gexlm.h"
#include "gexlm_mainobject.h"
#include "gexlm_server.h"


///////////////////////////////////////////////////////////
// Extern variables
///////////////////////////////////////////////////////////
// from gexlm_server.cpp
extern QList<ClientNode *>		pClientNodes;	// List of client nodes.
extern QString					strLocalFolder;

///////////////////////////////////////////////////////////
// Extern function
///////////////////////////////////////////////////////////
extern void g_event(QString strEvent,QString strMessage, QDateTime dateTimeStamp = QDateTime::currentDateTime());

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////
GexLicenseManager::GexLicenseManager(int nPortNumber)
{
	// Start Server socket listener.
	//TODO: check if server is really used
	SimpleServer *server = new SimpleServer(this, nPortNumber);
	if (server->isListening() == false) {
	}

	// Create timer used to check if nodes still alive.
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(timerDone()));
	timer->start(CHECK_ALIVE);		// Every 1 minute, check nodes still alive.
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////
GexLicenseManager::~GexLicenseManager()
{
	
}

///////////////////////////////////////////////////////////
// Timer event: periodically checks nodes still alive...
///////////////////////////////////////////////////////////
void GexLicenseManager::timerDone()
{
	int i;
	
	// Check timeout on all clients.
 	ClientNode *pClient;
	for(i=0; i<pClientNodes.size(); i++)
	{
		pClient = pClientNodes.at(i);
		QDateTime dtNow = QDateTime::currentDateTime();
		int iSec = pClient->LastHandshake.secsTo(dtNow);
		if(iSec > MAX_ALIVE_WAIT)
		{
			// Remove this node from the list...we consider it dead.
			pClient->OnDisconnectClient();

			// Exit the for loop
			break;
		}
	};	
	
	// write a time stamp in the config file with the curren date time
	writeTimeStamp(QDateTime::currentDateTime());
}

////////////////////////////////////////////////////////////////////////////
// Write a time stamp periodically (every 60 sec)...
// this value will be used when LicenseManager will restart after it crashes
////////////////////////////////////////////////////////////////////////////
void GexLicenseManager::writeTimeStamp(const QDateTime& dateTimeStamp)
{
	// Build path to configuration file
	QString strConfigFile = strLocalFolder + "/.gexlm_timestamp.txt";
	CGexSystemUtils::NormalizePath(strConfigFile);
	
	QFile file(strConfigFile); 
    
	// open the file
	if (file.open(QIODevice::WriteOnly) == true)
	{
		QTextStream hFile;

		// Assign file handle to data stream
		hFile.setDevice(&file);	

		// Write the text to the file
		hFile << "<gexlm_config>" << endl;

		QString strTimeStamp;

		if (dateTimeStamp.isValid())
			strTimeStamp = dateTimeStamp.toString(Qt::ISODate);
		
		hFile << "timeStamp=" << strTimeStamp << endl;
		file.close();
	}
}

////////////////////////////////////////////////////////////////////////////
// Read the time stamp from the configuration file 
////////////////////////////////////////////////////////////////////////////
QDateTime GexLicenseManager::readTimeStamp()
{
	QDateTime	dtTimeStamp;

	// Build path to configuration file
	QString		strConfigFile = strLocalFolder + "/.gexlm_timestamp.txt";
	CGexSystemUtils::NormalizePath(strConfigFile);
	
	QFile file(strConfigFile); 
    
	if (file.open(QIODevice::ReadOnly) == true)
	{
		QTextStream hFile;

		// Assign file handle to data stream
		hFile.setDevice(&file);	
		
		// Check if valid header...or empty!
		QString strString = hFile.readLine();
		
		if(strString == "<gexlm_config>")
		{
			do
			{
				// Read one line from file
				strString = hFile.readLine();

				if(strString.startsWith("timeStamp=") == true)
				{
					strString = strString.section('=',1);

					if (strString.isEmpty() == false)
						dtTimeStamp = QDateTime::fromString(strString, Qt::ISODate);
				}
			}
			while(hFile.atEnd() == false);
		}

		file.close();
	}

	return dtTimeStamp;
}

////////////////////////////////////////////////////////////////////////////
// Check the time stamp from the configuration file when starting
////////////////////////////////////////////////////////////////////////////
void GexLicenseManager::checkTimeStamp()
{
	// Read the time stamp from the configuration file
	QDateTime dateTimeStamp = GexLicenseManager::readTimeStamp();

	// If the time stamp is a valid date, the previous session was not correctly terminated
	// We need to add a g_close message in the log file
	if (dateTimeStamp.isValid())
	{	
		// Build path to configuration file
		QString		strLogFile = strLocalFolder  + "/gex-lm_" + dateTimeStamp.toString("yyyy_MM") + ".log";
		CGexSystemUtils::NormalizePath(strLogFile);
		QFile		file(strLogFile); 
		QDateTime	dateTimeLastEvent;
	
		// open the file
		if (file.open(QIODevice::ReadOnly) == true)
		{
			QTextStream hFile;

			// Assign file handle to data stream
			hFile.setDevice(&file);	

			QString strString;
			
			do
			{
				// Read one line from file
				strString = hFile.readLine();
			}
			while(hFile.atEnd() == false);

			file.close();
			
			// Get the date/time of the last event logged
			dateTimeLastEvent = QDateTime::fromString(strString.section(' ', 1,1), "dd-MM-yyyy_hh:mm:ss");
		}

		// Check that the last event is older than the time stamp. Use the nearest datetime
		if (dateTimeLastEvent > dateTimeStamp)
			dateTimeStamp = dateTimeLastEvent;

		// log the abnormal event
		g_event("g_stop_abnormal", "GEX-LM was not correctly closed at the previous session", dateTimeStamp);
	}

	// Write the new time stamp in the config file
	GexLicenseManager::writeTimeStamp(QDateTime::currentDateTime());
}
