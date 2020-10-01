///////////////////////////////////////////////////////////
// VFEI Socket  server header file
///////////////////////////////////////////////////////////
#ifndef TCPIP_VFEI_H
#define TCPIP_VFEI_H

#include <QStringList>
#include <QDateTime>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>

#include "cstdf.h"
#include "cstdfparse_v4.h"
#include "../../../pat_prod_admin/prod_recipe_file.h"	// PAT-Man Production recipe (eg: ST users)

class VfeiClient;	// Declaration follows!

// Configuration loaded from file 'patman_vfei.conf' in application folder
class VfeiConfigFile
{
public:
	VfeiConfigFile();	// Constructor

	int			m_SocketPort;			// Socket port#
	int			m_iMaxInstances;		// Maximum concurrent instances allowed
	bool		m_bDeleteInput;			// Delete input files after processing.
	QString		m_strPROD_RecipeFolder;	// Production recipe location
	QString		m_strENG_RecipeFolder;	// Engineering recipe location
	QString		m_strInputStdfFolder;	// STDF input folder
	QString		m_strInputMapFolder;	// MAP input folder
	QString		m_strOutputStdfFolder;	// Output STDF folder
	QString		m_strOutputMapFolder;	// Output MAP folder
	QString		m_strOutputReportFolder;// Output REPORT folder
	QString		m_strErrorFolder;		// Output ERROR folder (where files created moved on error situation)
};

///////////////////////////////////////////////////////////
//  The TcpServer class handles new connections to the server. When the
//  client connects, it creates a new VfeiClient -- that instance is now
//  responsible for the communication with that client.
///////////////////////////////////////////////////////////
class VfeiServer : public QTcpServer
{
	Q_OBJECT

public:
    VfeiServer(QObject* parent=0, VfeiConfigFile *pVfeiConfig=NULL);

private slots:
	void	OnNewConnection();
};

class CVFEI_JOB_CREATE
{
public:
	QString	m_strMID;	    // Machine ID
	QString	m_strMTY;	    // Message type. eg: 'C' for Create, 'R' for Reply
	long	m_lTID;		    // Transaction ID.
	QString	m_strCAM_LOT;	// CAM lot name
	QString	m_strRCP_NME;	// Recipe name
	QString	m_strRCP_VER;	// Recipe version
	QString	m_strDate;	    // Date of message request
};

class CVFEI_JOB_START
{
public:
	QString	m_strMID;			// Machine ID
	QString	m_strMTY;			// Message type. eg: 'C' for Create, 'R' for Reply
	long	m_lTID;				// Transaction ID.
	QString	m_strCAM_LOT;	    // CAM lot name
	QString	m_strOCR_LOT;	    // OCR lot name
	QString m_strOPERATION;	    // Operation name
	long	m_lWaferCount;	    // Wafer count (total wafers to process)
	QList <long> cWaferIdList;  // Holds list of wafer # to process
	QString	m_strRCP_NME;	    // Recipe name
	QString	m_strRCP_VER;	    // Recipe version
	QString	m_strDate;			// Date of message request
};

class CVFEI_ABORT
{
public:
	QString	m_strMID;	    // Machine ID
	QString	m_strMTY;	    // Message type. eg: 'C' for Create, 'R' for Reply
	long	m_lTID;		    // Transaction ID.
	QString	m_strCAM_LOT;	// CAM lot name
	QString	m_strRCP_NME;	// Recipe name
	QString	m_strRCP_VER;	// Recipe version
	QString	m_strDate;	    // Date of message request
};

class CVFEI_STATUS
{
public:
	QString	m_strMID;	    // Machine ID
	QString	m_strMTY;	    // Message type. eg: 'C' for Create, 'R' for Reply
	long	m_lTID;		    // Transaction ID.
	QString	m_strDate;	    // Date of message request
};

class CVFEI_JOB_QUEUE
{
public:
	QString	m_strCAM_LOT;	    // CAM lot name
	QString	m_strOCR_LOT;	    // OCR lot name
	QString m_strOPERATION;	    // Operation name
	QList <long> cWaferIdList;  // Holds list of wafer # to process
	QString	m_strRCP_NME;	    // Recipe name
	QString	m_strRCP_VER;	    // Recipe version
	QString	m_strStatus;	    // PAT status

	// List of files used & created
	QStringList m_strInputFiles;	// List of input files (so to delete on successful exit)
	QStringList m_strOutputFiles;	// List of output files to delete on exception
};

///////////////////////////////////////////////////////////
//  The VfeiClient class provides a socket that is connected with a client.
//  For every client that connects to the server, the server creates a new
//  instance of this class.
///////////////////////////////////////////////////////////
class VfeiClient : public QObject
{
	Q_OBJECT

public:
	VfeiClient(QTcpSocket *pTcpSocket, QObject *parent=0);
	~VfeiClient();

	quint16		tcpSocketPeerPort() const				{ return m_pTcpSocket->peerPort(); }

private:

	QTcpSocket	*m_pTcpSocket;						// Ptr to client socket
	VfeiConfigFile	cVfeiConfig;					// Config file details.
	bool		m_bBusy;							// Used to lock code sections
	bool		m_CloseClient;						// =true when socket must be closed
	bool		m_bProcessingLot;					// =true when currently busy processing a lot
	QTimer		m_Timer;
	QString		m_strSocketMessage;					// Holds message received thru TCP/IP
	
	void	LockCodeSection(bool bBusy);			// Locks code section
	bool	isLockedCodeSection(void);				// Code section locked?
	bool	checkDeleteClient(bool bAllowDelete);	// Checks if socket to be closed
	void	WriteLineToSocket(QString strMessage);	// Send string to client (encrypt string)
	QString	ReadLineFromSocket(void);				// Read + decrypt string received from client
	bool	vfei_extract_Msg_Long(QString strClientMessage,int iOffset,long &lValue);
	bool	vfei_extract_Msg_LongList(QString strClientMessage,int iOffset,QList <long> &cLongList);
	int		isJobInQueue(QString strCAM_LOT);
	bool	deleteJobInQueue(QString strCAM_LOT="*",bool bError=false, QString strRecipeName="*", QString strRecipeVersion="*");
	bool	setJobStatus(QString strCAM_LOT,QString strStatus);
	int		iTotalPatTasksRunning(void);

	void	process_JOB_CREATE(QString &strClientMessage,QString &strReplyMessage);
	void	process_START(QString &strClientMessage,QString &strReplyMessage);
	void	process_ABORT(QString &strClientMessage,QString &strReplyMessage);
	void	process_STATUS(QString &strClientMessage,QString &strReplyMessage);
	void	create_ReplyString(QString &strReplyMessage,QString strCMD,QString strMID,QString strMTY, int iTID=-1,int iErrorCode=0,QString strDetailedErrorInfo="");
	bool	getReleasedPROD_Recipe(QString &strRecipeFile,QString &strOverloadRecipeFile,CPROD_Recipe &cProdRecipe,QString strRecipeName,QString strVersion,QString &strErrorMessage);
	QString getSTDF_WaferFile(QString strFolder,QString strLot, int iWafer,QString strErrorMsg);
	bool	buildOutputSTDF_From_OLI(QString strOLI_MapSource,QString &strStdfOutputFileName,CVFEI_JOB_START cJob_Start,CRecipeFlow cWorkingFlow);

	// Holds the list of CAM lots queued for PAT processing.
	QList <CVFEI_JOB_QUEUE>	cJobQueue;
	int		m_PatTaskId;	// Task ID (for managing Pause/Run capability from PAT-Server GUI)

public slots:
	void	OnReadyRead();							// Receiving query from client...push message in queue
	void	OnProcessMessage();						// Called (on timer event) to process TCP/IP commands received
	void	OnDisconnect();							// Client closed connection...

signals:
	void	readMoreLine(); // Emit signal when additional line can be read from socket.
};


///////////////////////////////////////////////////////////
// VFEI protocol handling
///////////////////////////////////////////////////////////
class GexVfeiService : public QObject
{
public:
	GexVfeiService();	// Constructor
	~GexVfeiService();	// Destructor
	
	VfeiServer		*hVfeiServer;
	VfeiClient		*hVfeiClient;		// Handle to socket instance created.

private:
	VfeiConfigFile	cVfeiConfig;
};


#endif // ifdef TCPIP_VFEI_H
