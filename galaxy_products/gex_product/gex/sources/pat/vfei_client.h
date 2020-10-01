#ifndef VFEI_CLIENT_H
#define VFEI_CLIENT_H

#include "vfei_config.h"

#include <QList>
#include <QTimer>
#include <QStringList>

class QTcpSocket;
class CPROD_Recipe;
class CRecipeFlow;

namespace GS
{
namespace Gex
{

class PATRecipe;

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

/*! \class  VFEIClient
    \brief  The VFEIClient class provides a socket that is connected with a client.
            For every client that connects to the server, the server creates a new
            instance of this class.
*/

class VFEIClient : public QObject
{
    Q_OBJECT

public:

    /*!
      @brief    Constructs a VFEI Client object connected to the VFEI server

      @param    lTcpSocket  Pointer to connected socket
      @param    lConfig     The VFEI configuration.
      @param    parent      Pointer to the parent object
      */
    VFEIClient(QTcpSocket * lTcpSocket, const VFEIConfig &lConfig,
               QObject * parent = 0);


    /*!
      @brief    Destroys the VFEIClient object.
    */
    ~VFEIClient();

    /*!
      @brief    Disconnect the VFEIClient from the host (\l{VFEIServer}).
    */
    void            DisconnectFromHost();

private:

    Q_DISABLE_COPY(VFEIClient)

    QTcpSocket	*   mTcpSocket;                     // Ptr to client socket
    VFEIConfig  	mConfig;                        // Config file details.
    bool            mBusy;                          // Used to lock code sections
    bool            mCloseClient;					// =true when socket must be closed
    bool            mProcessingLot;                 // =true when currently busy processing a lot
    QTimer          mTimer;
    QString         mSocketMessage;                 // Holds message received thru TCP/IP
    int             mPatTaskId;                    // Task ID (for managing Pause/Run capability from PAT-Server GUI)
    QList <CVFEI_JOB_QUEUE>	mJobQueue;              // Holds the list of CAM lots queued for PAT processing.

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
#ifdef GCORE15334

    void	process_START(QString &strClientMessage,QString &strReplyMessage);
#endif
    void	process_ABORT(QString &strClientMessage,QString &strReplyMessage);
    void	process_STATUS(QString &strClientMessage,QString &strReplyMessage);
    void	create_ReplyString(QString &strReplyMessage,QString strCMD,QString strMID,QString strMTY, int iTID=-1,int iErrorCode=0,QString strDetailedErrorInfo="");
    bool	getReleasedPROD_Recipe(QString &strRecipeFile,QString &strOverloadRecipeFile,CPROD_Recipe &cProdRecipe,QString strRecipeName,QString strVersion,QString &strErrorMessage);
    QString getSTDF_WaferFile(QString strFolder,QString strLot, int iWafer,QString strErrorMsg);
    bool	buildOutputSTDF_From_OLI(QString strOLI_MapSource,QString &strStdfOutputFileName,CVFEI_JOB_START cJob_Start, const CRecipeFlow& cWorkingFlow);
    bool    ReadRecipe(PATRecipe& lPATRecipe, const QString& lRecipeFile,
                       const QString& lProdRecipe, QString &lErrorMessage);

public slots:

    void	OnReadyRead();							// Receiving query from client...push message in queue
    void	OnProcessMessage();						// Called (on timer event) to process TCP/IP commands received
    void	OnDisconnect();							// Client closed connection...

signals:

    void	readMoreLine(); // Emit signal when additional line can be read from socket.
};

}   // namespace Gex
}   // namespace GS

#endif // VFEI_CLIENT_H
