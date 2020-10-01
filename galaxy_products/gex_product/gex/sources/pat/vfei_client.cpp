#include "vfei_client.h"
#include "engine.h"
#include "read_system_info.h"
#include "tasks_manager_engine.h"
#include "task.h"
#include "pat_info.h"
#include "gqtl_log.h"
#include "gqtl_archivefile.h"
#include "stdfparse.h"
#include "../../../pat_prod_admin/prod_recipe_file.h"
#include "product_info.h"
#include "pat_recipe_io.h"
#include "pat_recipe.h"
#include "pat_definition.h"
#include "gex_pat_processing.h"
#include "gex_report.h"
#include <QApplication>

//#include "patman_lib.h"
#include "scheduler_engine.h"
#include "pat_engine.h"

#include <QTcpSocket>
#include <QDateTime>

extern CGexReport *     gexReport;

// Define VFEI Communication Errors (10000-29999)
#define	VFEI_COMM_ERR_MID		10000	// Error in the format or syntax of a VFEI message: Invalid/Missing MID
#define	VFEI_COMM_ERR_MTY		10001	// Error in the format or syntax of a VFEI message: Invalid/Missing MTY
#define	VFEI_COMM_ERR_TID		10002	// Error in the format or syntax of a VFEI message: Invalid/Missing TID

// Define PAT-Man errors (50000-59999)
#define	VFEI_ARG_ERR_CAM_LOT	50000	// CAM_LOT field invalid/missing
#define	VFEI_ARG_ERR_OCR_LOT	50001	// OCR_LOT field invalid/missing
#define	VFEI_ARG_ERR_OPERATION	50002	// OPERATION field invalid/missing
#define	VFEI_ARG_ERR_WAFCNT		50003	// Wafer count invalid/missing
#define	VFEI_ARG_ERR_WAFLST		50004	// Wafer list invalid/missing
#define	VFEI_ARG_ERR_RCP_NME	50005	// Recipe name invalid/missing
#define	VFEI_ARG_ERR_RCP_VER	50006	// Recipe version invalid/missing
#define	VFEI_ARG_ERR_DATE		50007	// DATE field missing/invalid
#define	VFEI_ERR_STATUS_CREATE	50008	// Invalid status(stage) to accept this command: not in JOB_CREATE mode
#define	VFEI_JOB_CREATE			50009	// Error during JOB_CREATE sequence (eg: error loading recipe, etc)
#define	VFEI_JOB_START			50010	// Error during JOB_START sequence (eg: error executing recipe, etc)
#define	VFEI_ABORT_ERR			50011	// ABORT rejected (typically: recipe name/version mismatch
#define VFEI_JOB_ANALYSIS		50012	// Error during PAT processing
#define VFEI_JOB_BUSY			50013	// Error PAT processing ALREADY running, can't launch two PAT processing concurrently!

namespace GS
{
namespace Gex
{

///////////////////////////////////////////////////////////
//  The VFEIClient class provides a socket that is connected with a client.
//  For every client that connects to the server, the server creates a new
//  instance of this class.
///////////////////////////////////////////////////////////
VFEIClient::VFEIClient(QTcpSocket* lTcpSocket, const VFEIConfig& lConfig,
                       QObject *parent)
    : QObject(parent), mTcpSocket(lTcpSocket), mConfig(lConfig)
{
    // Init variables
    LockCodeSection(false);
    mCloseClient    = false;		// set to 'true' when client must be closed
    mProcessingLot  = false;	// To process only one lot at a time

    // Ensure we disable the TEXT flag (so CR+LF and CR handles the same way)
    mTcpSocket->setTextModeEnabled(false);

    // Signal/Slot connections
    connect(mTcpSocket,     SIGNAL(disconnected()), this, SLOT(OnDisconnect()));
    connect(mTcpSocket,     SIGNAL(readyRead()),    this, SLOT(OnReadyRead()));
    connect(this,           SIGNAL(readMoreLine()), this, SLOT(OnReadyRead()));

    // Timer based event processing (as TCP/IP readReady is not re-entrant)
    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(OnProcessMessage()));

    // Debug notification
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "VFEI client connection initiated");
}

VFEIClient::~VFEIClient()
{
    // Ensure we do a clean shutdown...

}

void VFEIClient::DisconnectFromHost()
{
    if (mTcpSocket)
        mTcpSocket->disconnectFromHost();
}

///////////////////////////////////////////////////////////
// Prevents application from closing within critical code
///////////////////////////////////////////////////////////
void VFEIClient::LockCodeSection(bool bBusy)
{
    mBusy = bBusy;
}

///////////////////////////////////////////////////////////
// Returns true if software busy running critical code
///////////////////////////////////////////////////////////
bool VFEIClient::isLockedCodeSection(void)
{
    return mBusy;
}
///////////////////////////////////////////////////////////
// Send string to client
///////////////////////////////////////////////////////////
void VFEIClient::WriteLineToSocket(QString strMessage)
{
    QTextStream os(mTcpSocket);

    // Send command line server
    os << strMessage << "\n";
}

///////////////////////////////////////////////////////////
// Read + decrypt string received from client
///////////////////////////////////////////////////////////
QString VFEIClient::ReadLineFromSocket(void)
{
    QString		strMessage;

    // Clean incoming string (sent by WMR: remove any non-ASCII character)
    strMessage = mTcpSocket->readLine();		// Read socket buffer received
    strMessage = strMessage.trimmed();			// Remove leading '\n' and other spaces
    strMessage = strMessage.replace("\r","");	// Remove '\r' if any
    strMessage = strMessage.replace(QRegExp("[\\0000-\\0037]"), "");	// Remove non-printable characters

    return strMessage;
}

///////////////////////////////////////////////////////////
// Socket message received: store into FIFO buffer
///////////////////////////////////////////////////////////
void VFEIClient::OnReadyRead()
{
    QString strString;

    // Read one line received from client
    mSocketMessage = ReadLineFromSocket();

    // If empty line, quietly ignore it!
    if(mSocketMessage.isEmpty())
        goto exit_read;

    // DEBUG purpose
    strString = "WMR > " + mSocketMessage;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().constData());

exit_read:
    // Sets to call VFEI command parser in few ms.
    mTimer.start(100);
}

///////////////////////////////////////////////////////////
// Process received messages
///////////////////////////////////////////////////////////
void VFEIClient::OnProcessMessage()
{
    // DEBUG purpose
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "TCP/IP: OnProcessMessage()");

    // Check if 'close' event received.
    if(checkDeleteClient(true))
        return;

    // Check if any message in queue
    if(mSocketMessage.isEmpty())
        return;

    // Get first message in queue
    QString	strReply;
    QString strClientMessage = mSocketMessage;

    // Clear WMR command buffer received
    mSocketMessage.clear();

    if(strClientMessage.startsWith("CMD/A=\"JOB_CREATE\""))
    {
        process_JOB_CREATE(strClientMessage,strReply);
        goto send_reply;
    }

    if(strClientMessage.startsWith("CMD/A=\"START\""))
    {
#ifdef GCORE15334
        process_START(strClientMessage,strReply);
#endif
        goto send_reply;
    }

    if(strClientMessage.startsWith("CMD/A=\"ABORT\""))
    {
        process_ABORT(strClientMessage,strReply);
        goto send_reply;
    }

    if(strClientMessage.startsWith("CMD/A=\"STATUS\""))
    {
        process_STATUS(strClientMessage,strReply);
        goto send_reply;
    }

    // Unknown command.
    strReply = "ERROR_UNKOWN_COMMAND: '" + strClientMessage + "'";

send_reply:
    // Check if 'close' event received.
    if(checkDeleteClient(true))
        return;

    // Send back string to client
    WriteLineToSocket(strReply);

    // DEBUG purpose
    QString strString = "WMR < " + strReply;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().constData());

    // Check if other pending messages, if so signal it
    if(mTcpSocket->canReadLine())
        emit readMoreLine();
}

///////////////////////////////////////////////////////////
// Client closed connection...flag to close socket
///////////////////////////////////////////////////////////
void VFEIClient::OnDisconnect()
{
    // DEBUG purpose
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "TCP/IP Client-server connection closed");
    mCloseClient = true;

    checkDeleteClient(false);
}

///////////////////////////////////////////////////////////
// Delete client if possible
///////////////////////////////////////////////////////////
bool VFEIClient::checkDeleteClient(bool bAllowDelete)
{
    if(mCloseClient == false)
        return false;

    if(isLockedCodeSection() == true)
        return false;

    // Tells application must close socket; but don't do it yet!
    if(bAllowDelete == false)
        return true;

    // Delete ALL temp. files created, delete all pending FVEI requests
    deleteJobInQueue();

    // Delete socket
    mTcpSocket->deleteLater();
    delete this;	// To Do : "delete this" is forbidden in ISO. Remove me !

    return true;
}

///////////////////////////////////////////////////////////
// Process request: JOB_CREATE
///////////////////////////////////////////////////////////
void VFEIClient::create_ReplyString(QString &strReplyMessage,QString strCMD,QString strMID,QString strMTY, int iTID/*=0*/,int iErrorCode/*=-1*/,QString strDetailedErrorInfo/*=""*/)
{
    strReplyMessage =  "CMD/A=\"" + strCMD + "\" ";
    strReplyMessage += "MID/A=\"" + strMID + "\" ";
    strReplyMessage += "MTY/A=\"" + strMTY + "\" ";
    strReplyMessage += "TID/U4=" + QString::number(iTID) + " ";


    QString strErrorDetails;
    switch(iErrorCode)
    {
        case 0:
            strErrorDetails = "NO ERROR";	// No error
            strDetailedErrorInfo="";		// No error
            break;

        case VFEI_COMM_ERR_MID:
            strErrorDetails = "Error in the format or syntax of VFEI message: Invalid/Missing MID value.";
            break;
        case VFEI_COMM_ERR_MTY:
            strErrorDetails = "Error in the format or syntax of VFEI message: Invalid/Missing MTY value.";
            break;
        case VFEI_COMM_ERR_TID:
            strErrorDetails = "Error in the format or syntax of VFEI message: Invalid/Missing TID value.";
            break;

        case VFEI_ARG_ERR_CAM_LOT:
            strErrorDetails = "Error in VFEI message: Invalid/Missing CAM_LOT value.";
            break;
        case VFEI_ARG_ERR_OCR_LOT:
            strErrorDetails = "Error in VFEI message: Invalid/Missing OCR_LOT value.";
            break;
        case VFEI_ARG_ERR_OPERATION:
            strErrorDetails = "Error in VFEI message: Invalid/Missing OPERATION value.";
            break;
        case VFEI_ARG_ERR_WAFCNT:
            strErrorDetails = "Error in VFEI message: Invalid/Missing WAF_CNT value.";
            break;
        case VFEI_ARG_ERR_WAFLST:
            strErrorDetails = "Error in VFEI message: Invalid/Missing WAF_LST value.";
            break;
        case VFEI_ARG_ERR_RCP_NME:
            strErrorDetails = "Error in VFEI message: Invalid/Missing RCP_NME value.";
            break;
        case VFEI_ARG_ERR_RCP_VER:
            strErrorDetails = "Error in VFEI message: Invalid/Missing RCP_VER value.";
            break;
        case VFEI_ARG_ERR_DATE:
            strErrorDetails = "Error in VFEI message: Invalid/Missing DATE value.";
            break;
        case VFEI_ERR_STATUS_CREATE:
            strErrorDetails = "Error: JOB_CREATE command needs to be called first";
            break;
        case VFEI_JOB_CREATE:
            strErrorDetails = "Error during JOB_CREATE sequence.";
            break;
        case VFEI_JOB_START:
            strErrorDetails = "Error during JOB_START sequence.";
            break;
        case VFEI_ABORT_ERR:
            strErrorDetails = "ABORT command not allowed.";
            break;
        case VFEI_JOB_ANALYSIS:
            strErrorDetails = "PAT processing error";
            break;
        case VFEI_JOB_BUSY:
            strErrorDetails = "PAT processing busy (ongoing analysis)";
            break;
    }

    // Build error details reply
    strReplyMessage += "ECD/U4=" + QString::number(iErrorCode) + " ";	// Error code
    strReplyMessage += "ETX/A=\"" + strErrorDetails;					// Error Text
    if(strDetailedErrorInfo.isEmpty() == false)
    {
        // If Error message, then append details to message string.
        strDetailedErrorInfo.replace("\n","");
        strDetailedErrorInfo.replace("\"","'");
        strReplyMessage += " [ " + strDetailedErrorInfo + " ]";

        // Report Error
        GSLOG(SYSLOG_SEV_ERROR, strDetailedErrorInfo.toLatin1().constData());

        // Full debug details
        GSLOG(SYSLOG_SEV_DEBUG, strReplyMessage.toLatin1().constData());
    }
    strReplyMessage+= "\" ";

}

///////////////////////////////////////////////////////////
// Extract long integer from VFEI message
///////////////////////////////////////////////////////////
bool VFEIClient::vfei_extract_Msg_Long(QString strClientMessage,int iOffset,long &lValue)
{
    QString strString = strClientMessage.mid(iOffset);
    strString = strString.section("=",1);
    if(strString.isEmpty())
        return false;
    if (sscanf(strString.toLatin1().constData(), "%ld", &lValue) == 1)
        return true;
    else
        return false;
}

///////////////////////////////////////////////////////////
// Extract LIST of numbers from VFEI message
///////////////////////////////////////////////////////////
bool VFEIClient::vfei_extract_Msg_LongList(QString strClientMessage,int iOffset,QList <long> &cLongList)
{
    QString strNumber;
    int iWaferListSize;
    bool bOk;

    // Clears list of numbers
    cLongList.clear();

    // Extract Wafer list size defined
    QString strString = strClientMessage.mid(iOffset);
    strNumber = strString.section("[",1);
    if(strNumber.isEmpty())
        return false;
    strNumber = strNumber.section("]",0,0);
    if(strNumber.isEmpty())
        return false;
    iWaferListSize = strNumber.toInt(&bOk);
    if(!bOk)
        return false; // Failed extracting array size.

    // Seek to '=[' string
    strString = strString.section("=[",1);
    if(strString.isEmpty())
        return false;
    strString = strString.section("]",0,0);
    if(strString.isEmpty())
        return false;

    // Trim white spaces from beginning and end
    strString = strString.trimmed();

    // Extract each number in string
    long    lValue;
    do
    {
        // Extract firt number in string
        strNumber = strString.section(" ",0,0);

        // Remove all spaces
        strNumber = strNumber.trimmed();
        if (sscanf(strString.toLatin1().constData(), "%ld", &lValue) != 1)
            return false;

        // Save extracted number
        cLongList += lValue;

        // Move to next number in string
        strString = strString.section(" ",1);
    }
    while(strString.isEmpty() == false);

    // Check if item list matches array size specified
    if(iWaferListSize != cLongList.count())
        return false;

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Process request: JOB_CREATE
///////////////////////////////////////////////////////////
void VFEIClient::process_JOB_CREATE(QString &strClientMessage,QString &strReplyMessage)
{
    QString strCMD = "JOB_CREATE";
    QString	strItem;
    CVFEI_JOB_CREATE	cJob_Create;
    int			iOffset;

    // Parse MID argument
    iOffset = strClientMessage.indexOf("MID/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }
    cJob_Create.m_strMID = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty MID
    if(cJob_Create.m_strMID.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }

    // Parse MTY argument
    iOffset = strClientMessage.indexOf("MTY/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }
    cJob_Create.m_strMTY = strClientMessage.mid(iOffset).section("\"",1,1);
    // MTY must always be be 'C'
    if(cJob_Create.m_strMTY != "C")
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }

    // Parse TID argument
    iOffset = strClientMessage.indexOf("TID/U4=");
    if(iOffset <0)
    {
        // TID missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }
    if(vfei_extract_Msg_Long(strClientMessage,iOffset,cJob_Create.m_lTID) == false)
    {
        // Invalid/Corrupted TID
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }

    // Parse CAM_LOT lot name argument
    iOffset = strClientMessage.indexOf("CAM_LOT/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }
    cJob_Create.m_strCAM_LOT = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty CAM Lot
    if(cJob_Create.m_strCAM_LOT.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }

    // Parse Recipe name
    iOffset = strClientMessage.indexOf("RCP_NME/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_RCP_NME);
        return;
    }
    cJob_Create.m_strRCP_NME = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty recipe
    if(cJob_Create.m_strRCP_NME.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_RCP_NME);
        return;
    }

    // Parse Recipe version
    iOffset = strClientMessage.indexOf("RCP_VER/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_RCP_VER);
        return;
    }
    cJob_Create.m_strRCP_VER = strClientMessage.mid(iOffset).section("\"",1,1);

    // Parse Date lot name argument
    iOffset = strClientMessage.indexOf("DATE/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }
    cJob_Create.m_strDate = strClientMessage.mid(iOffset).section("\"",1,1);
    // Check if valid date
    QDateTime cDateTimeInfo;
    cDateTimeInfo = QDateTime::fromString(cJob_Create.m_strDate,"yyyyMMddHHmmss");
    if(cDateTimeInfo.isValid() == false)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }

    // Check if Job already in queue...reject this duplicated request!
    QString         strPatmanErrorMessage;
    CPROD_Recipe    cProdRecipe;
    PATRecipe       lPATRecipe;
    bool            bSuccess    = true;
    bool            bFlushQueue = true;	// Remove job from que if exception detected

    if(isJobInQueue(cJob_Create.m_strCAM_LOT) >= 0)
    {
        strPatmanErrorMessage = "LOT already in queue";
        bFlushQueue = false;    // Job already in queue, do not remove its current instance!
        bSuccess = false;	// Flag error
    }
    else
    {
        // Add Job to the queue
        CVFEI_JOB_QUEUE cNewJob;
        cNewJob.m_strCAM_LOT = cJob_Create.m_strCAM_LOT;	// CAM lot name
        cNewJob.m_strRCP_NME = cJob_Create.m_strRCP_NME;	// Recipe name
        cNewJob.m_strRCP_VER = cJob_Create.m_strRCP_VER;	// Recipe version
        cNewJob.m_strStatus = "SETUP";
        mJobQueue += cNewJob;

        // JOB_CREATE arguments loaded, so now load recipe!
        QString	strRecipeFile;
        QString strRecipeOverloadFile;
        bSuccess = getReleasedPROD_Recipe(strRecipeFile,strRecipeOverloadFile,cProdRecipe,cJob_Create.m_strRCP_NME,cJob_Create.m_strRCP_VER,strPatmanErrorMessage);

        if(bSuccess)
            bSuccess = ReadRecipe(lPATRecipe, strRecipeFile, strRecipeOverloadFile, strPatmanErrorMessage);
    }

    // Check if PAT-Server GUI is in PAUSE mode
    if(Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
    {
        strPatmanErrorMessage = "PAT-Server in PAUSE mode. Can't process any lot for now.";
        bSuccess = false;	// Flag error (as we're in PAUSE mode)
    }

    if(bSuccess == false)
    {
        // JOB_CREATE Failed
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_JOB_CREATE,strPatmanErrorMessage);
    }
    else
    {
        // JOB_CREATE Successful
        create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID);
    }

    // Add reply details
    strReplyMessage += "CAM_LOT/A=\"" + cJob_Create.m_strCAM_LOT + "\" ";
    strReplyMessage += "RCP_NME/A=\"" + cJob_Create.m_strRCP_NME + "\" ";
    strReplyMessage += "RCP_VER/A=\"" + cJob_Create.m_strRCP_VER + "\" ";

    // If loading failure
    if(bSuccess == false)
    {
        strReplyMessage += "STDF_MAP/A[1]=[\"NONE\"] OLI_MAP/A[1]=[\"NONE\"] ";
    }
    else
    {
        // Loading success, return STDF & Maps flows used
        QStringList strStdfList;
        if(cProdRecipe.getFlowList(FLOW_SOURCE_STDF,strStdfList) == false)
        {
            // NO STDF in PROD Recipe, check if this fine with ENG recipe rules.
            if(lPATRecipe.IsUsingParametric())
            {
                // JOB_CREATE Failed: Recipe conflict
                strPatmanErrorMessage = "*ERROR*: STDF (Parametric data) required by ENG Recipe";
                create_ReplyString(strReplyMessage,strCMD,cJob_Create.m_strMID,"R",cJob_Create.m_lTID,VFEI_JOB_CREATE,strPatmanErrorMessage);
                bSuccess = false;
            }
        }

        int nIndex;
        strReplyMessage += "STDF_MAP/A[" + QString::number(strStdfList.count()) + "]=[";
        for(nIndex=0; nIndex < strStdfList.count(); nIndex++)
        {
            // Add each STDF flow name
            strReplyMessage += "\"" + strStdfList.at(nIndex) + "\"";

            // If more entries left, insert space separator
            if(nIndex < strStdfList.count()-1)
                strReplyMessage += " ";
        }
        strReplyMessage += "] ";

        // Add external maps used (if any)
        QStringList strMapList;
        cProdRecipe.getFlowList(FLOW_SOURCE_MAP,strMapList);
        strReplyMessage += "OLI_MAP/A[" + QString::number(strMapList.count()) + "]=[";
        for(nIndex=0; nIndex < strMapList.count(); nIndex++)
        {
            // Add each MAP flow name
            strReplyMessage += "\"" + strMapList.at(nIndex) + "\"";

            // If more entries left, insert space separator
            if(nIndex < strMapList.count()-1)
                strReplyMessage += " ";
        }
        strReplyMessage += "] ";
    }
    strReplyMessage += "DATE/A=\"" + cJob_Create.m_strDate + "\"";

    // Exit
    if(bSuccess == false)
    {
        // JOB_CREATE loading failure: remove job from queue...unless the exception comes from job already in queue (in such case, keep existing job instance)
        if(bFlushQueue)
            deleteJobInQueue(cJob_Create.m_strCAM_LOT);
    }
    else
    {
        // JOB_CREATE successful
        setJobStatus(cJob_Create.m_strCAM_LOT,"READY");
    }
}

///////////////////////////////////////////////////////////
// Checks if given CAM lot is present in the queue (pending processing)
///////////////////////////////////////////////////////////
int VFEIClient::isJobInQueue(QString strCAM_LOT)
{
    CVFEI_JOB_QUEUE cJob;
    int	iIndex;

    for(iIndex = 0; iIndex < mJobQueue.count(); iIndex++)
    {
        cJob = mJobQueue[iIndex];
        if(cJob.m_strCAM_LOT == strCAM_LOT)
            return iIndex;    // Found job in queue
    }
    return -1;   // CAM lot not in queue.
}

///////////////////////////////////////////////////////////
// Remove CAM lot if present in the queue (pending processing)
///////////////////////////////////////////////////////////
bool VFEIClient::deleteJobInQueue(QString strCAM_LOT/*="*"*/, bool bError/*=false*/, QString strRecipeName/*="*"*/, QString strRecipeVersion/*="*"*/)
{
    CVFEI_JOB_QUEUE cJob;
    QString	strFile;
    QDir	cDir;
    int		iIndex;

    for(iIndex = 0; iIndex < mJobQueue.count(); iIndex++)
    {
        cJob = mJobQueue[iIndex];
        if((cJob.m_strCAM_LOT == strCAM_LOT) || (cJob.m_strCAM_LOT == "*"))
        {
            if(strRecipeName=="*")
                strRecipeName = cJob.m_strRCP_NME;
            if(strRecipeVersion=="*")
                strRecipeVersion = cJob.m_strRCP_VER;

            // Check recipe name & version match! (ignored if Empty recipe names & version).
            if((strRecipeName.isEmpty() == false) && (strRecipeVersion.isEmpty() == false))
            {
                if((strRecipeName != cJob.m_strRCP_NME) || (strRecipeVersion != cJob.m_strRCP_VER))
                    return false;	// Mismatch: reject delete!
            }

            // Matching entry: remove input file
            int iInputFileID;
            for(iInputFileID=0;iInputFileID<cJob.m_strInputFiles.count();iInputFileID++)
            {
                strFile = cJob.m_strInputFiles[iInputFileID];
                // Check if 'delete source' enabled...
                if(mConfig.GetDeleteInput())
                    cDir.remove(strFile);
            }

            // If Error during PAT process, move ouput files created to the temp. folder
            if(bError)
            {
                QString		strOutputFile;
                QFileInfo	cFileInfo;
                for(iInputFileID=0;iInputFileID<cJob.m_strOutputFiles.count();iInputFileID++)
                {
                    strFile = cJob.m_strOutputFiles[iInputFileID];
                    cFileInfo.setFile(strFile);
                    strOutputFile = mConfig.GetErrorFolder();
                    if(strOutputFile.endsWith("/") == false && strOutputFile.endsWith("\\") == false)
                        strOutputFile += "/";
                    strOutputFile += cFileInfo.fileName();
                    cDir.rename(strFile,strOutputFile);
                }
            }

            // remove job from queue
            mJobQueue.removeAt(iIndex);
            return true;
        }
    }

    // Job not in queue: quietly ignore if queue is empty
    if(mJobQueue.count() == 0)
        return true; // Queue is empty, so any Abort is accepeted!
    else
        return false; // Queue not empty, and Abort requested over a lot NOT in queue: signal error
}


///////////////////////////////////////////////////////////
// Set queue status for given CAM lot
///////////////////////////////////////////////////////////
bool VFEIClient::setJobStatus(QString strCAM_LOT,QString strStatus)
{
    int	iIndex;

    for(iIndex = 0; iIndex < mJobQueue.count(); iIndex++)
    {
        if(mJobQueue[iIndex].m_strCAM_LOT == strCAM_LOT)
        {
            mJobQueue[iIndex].m_strStatus = strStatus;
            return true;
        }
    }

    // Job not in queue
    return false;
}

///////////////////////////////////////////////////////////
// Returns total number of active jobs (with PAT task launched)
///////////////////////////////////////////////////////////
int VFEIClient::iTotalPatTasksRunning(void)
{
    CVFEI_JOB_QUEUE cJob;
    int	iIndex;
    int	iTotal=0;

    for(iIndex = 0; iIndex < mJobQueue.count(); iIndex++)
    {
        cJob = mJobQueue[iIndex];
        if(cJob.m_strStatus.isEmpty() == false)
            iTotal++;
    }
    return iTotal;   // Total jobs currently running PAT
}

///////////////////////////////////////////////////////////
// Find STDF file holding specific WaferID
///////////////////////////////////////////////////////////
QString VFEIClient::getSTDF_WaferFile(QString strFolder,QString strLot, int iWafer,QString strErrorMsg)
{
    int			iIndex,iStatus,nRecordType;
    QString		strTemp;
    QDir		cDir;
    QStringList	strFileFilter;
    QStringList	strFileList;

    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Entering : getSTDF_WaferFile");
    strTemp = " strFolder = " + strFolder + " Lot=" + strLot + " WaferID=" + QString::number(iWafer);
    GSLOG(SYSLOG_SEV_DEBUG, strTemp.toLatin1().constData());

    // 1- Identify file '*_<LotId>_<WaferID>_*'
    cDir.setPath(strFolder);
    strFileFilter.clear();
    strFileFilter << "*_" + strLot + QString("_") + strTemp.sprintf("%02d",iWafer) + QString("_*");
    cDir.setNameFilters(strFileFilter);
    strFileList = cDir.entryList(QDir::Files);

    // 2- Identify file '*_<LotId>-<WaferID>*'
    cDir.setPath(strFolder);
    strFileFilter.clear();
    strFileFilter << "*_" + strLot + QString("-") + strTemp.sprintf("%02d",iWafer) + QString("*");
    cDir.setNameFilters(strFileFilter);
    strFileList += cDir.entryList(QDir::Files);

    // 3- Generic serch (not involving Wafer# in name...
    strFileFilter.clear();
    strFileFilter << "*_" + strLot + "*";
    cDir.setNameFilters(strFileFilter);
    strFileList += cDir.entryList(QDir::Files);


    // No File for this lot...
    if(strFileList.count() == 0)
    {
        strErrorMsg = " Error: no Input STDF file found for Lot:" + strLot;
        GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
        return "";
    }

    strTemp = "Total files found: " + QString::number(strFileList.count());
    GSLOG(SYSLOG_SEV_DEBUG, strTemp.toLatin1().constData());

    GQTL_STDF::StdfParse clStdfParse;	// STDF V4 parser
    GQTL_STDF::Stdf_MIR_V4	clStdfMIR;
    bool			bOk;
    QString			strFileName,strFilePath;
    QStringList		strUncompressedFiles;
    CArchiveFile	clZip(strFolder);
    for(iIndex=0;iIndex < strFileList.count();iIndex++)
    {
        // Read MIR and check sub-lot is matching the WaferID we're looking for.
        strFileName = strFileList.at(iIndex);
        strFilePath = strFolder + "/" + strFileName;

        strTemp = "Try: " + strFilePath;
        GSLOG(SYSLOG_SEV_DEBUG, strTemp.toLatin1().constData());

        // If Compressed file, need to unzip first!
        if(clZip.IsCompressedFile(strFilePath))
        {
            strTemp = "File needs Unzip: " + strFilePath;
            GSLOG(SYSLOG_SEV_DEBUG, strTemp.toLatin1().constData());

            if(clZip.Uncompress(strFilePath, strUncompressedFiles) == false)
            {
                strErrorMsg = " Error: Failed opening STDF file:" + strFilePath;
                GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
                return "";
            }
            strTemp = "Unzipped: " + strFilePath + " to " + strUncompressedFiles.first();
            GSLOG(SYSLOG_SEV_DEBUG, strTemp.toLatin1().constData());

            // Remove source file
            //cDir.remove(strFilePath);

            // New STDF file is the uncompressed one
            strFileName = strUncompressedFiles.first();
            strFilePath = strFolder + "/" + strFileName;
        }

        // Read file + extract MIR
        if(clStdfParse.Open((char *)strFilePath.toLatin1().constData()) == false)
        {
            strErrorMsg = " Error: Failed opening STDF file:" + strFilePath;
            GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
            return "";
        }

        // Read one record from STDF file.
        iStatus = clStdfParse.LoadNextRecord(&nRecordType);
        while(iStatus == GQTL_STDF::StdfParse::NoError)
        {
            switch(nRecordType)
            {
                case GQTL_STDF::Stdf_Record::Rec_MIR:
                if(clStdfParse.ReadRecord((GQTL_STDF::Stdf_Record *)&clStdfMIR) == false)
                {
                    strErrorMsg = " Error: Failed reading STDF MIR record:" + strFilePath;
                    GSLOG(SYSLOG_SEV_ERROR, strErrorMsg.toLatin1().constData());
                    return "";
                }
                if(clStdfMIR.m_cnSBLOT_ID.toInt(&bOk) == iWafer && bOk)
                    return strFileName;
                break;
            }

            iStatus = clStdfParse.LoadNextRecord(&nRecordType);
        };
        clStdfParse.Close();	// Close STDF file
    }

    // No match...
    return "";
}

#ifdef GCORE15334

///////////////////////////////////////////////////////////
// Process request: START
///////////////////////////////////////////////////////////
void VFEIClient::process_START(QString &strClientMessage,QString &strReplyMessage)
{
    strReplyMessage = "CMD/A=\"START\" MID/A=\"PATMAN_SW\" MTY/A=\"R\"";

    QString strCMD = "START";
    QString				strString;
    QString				strItem;
    CVFEI_JOB_START	    cJob_Start;
    int					iOffset;

    // Parse MID argument
    iOffset = strClientMessage.indexOf("MID/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }
    cJob_Start.m_strMID = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty MID
    if(cJob_Start.m_strMID.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }

    // Parse MTY argument
    iOffset = strClientMessage.indexOf("MTY/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }
    cJob_Start.m_strMTY = strClientMessage.mid(iOffset).section("\"",1,1);
    // MTY must always be be 'C'
    if(cJob_Start.m_strMTY != "C")
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }

    // Parse TID argument
    iOffset = strClientMessage.indexOf("TID/U4=");
    if(iOffset <0)
    {
        // TID missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }
    if(vfei_extract_Msg_Long(strClientMessage,iOffset,cJob_Start.m_lTID) == false)
    {
        // Invalid/Corrupted TID
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }

    // If already processing one lot, reject further processing
    if(mProcessingLot)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_JOB_BUSY);
        return;
    }

    // Parse CAM_LOT lot name argument
    iOffset = strClientMessage.indexOf("CAM_LOT/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }
    cJob_Start.m_strCAM_LOT = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty CAM Lot
    if(cJob_Start.m_strCAM_LOT.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }

    // Parse OCR_LOT lot name argument
    iOffset = strClientMessage.indexOf("OCR_LOT/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_OCR_LOT);
        return;
    }
    cJob_Start.m_strOCR_LOT = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty OCR Lot
    if(cJob_Start.m_strOCR_LOT.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_OCR_LOT);
        return;
    }

    // Parse OPERATION lot name argument
    iOffset = strClientMessage.indexOf("OPERATION/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_OPERATION);
        return;
    }
    cJob_Start.m_strOPERATION = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty OPERATION
    if(cJob_Start.m_strOPERATION.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_OPERATION);
        return;
    }

    iOffset = strClientMessage.indexOf("WAF_CNT/U2=");
    if(iOffset <0)
    {
        // WAF_CNT missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_ARG_ERR_WAFCNT);
        return;
    }
    if(vfei_extract_Msg_Long(strClientMessage,iOffset,cJob_Start.m_lWaferCount) == false)
    {
        // Invalid/Corrupted Wafer count
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_ARG_ERR_WAFCNT);
        return;
    }

    iOffset = strClientMessage.indexOf("WAF_LST/U2[");
    if(iOffset <0)
    {
        // WAF_LST missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_ARG_ERR_WAFLST);
        return;
    }
    // Extract list of wafer#
    if(vfei_extract_Msg_LongList(strClientMessage,iOffset,cJob_Start.cWaferIdList) == false)
    {
        // Invalid/Corrupted Wafer count
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_ARG_ERR_WAFLST);
        return;
    }
    // Check Waferlist matches WaferCount
    if(cJob_Start.m_lWaferCount != cJob_Start.cWaferIdList.count())
    {
        // Invalid/Corrupted Wafer count
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",0,VFEI_ARG_ERR_WAFLST);
        return;
    }

    // Parse Recipe name lot name argument
    iOffset = strClientMessage.indexOf("RCP_NME/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_RCP_NME);
        return;
    }
    cJob_Start.m_strRCP_NME = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty recipe
    if(cJob_Start.m_strRCP_NME.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_RCP_NME);
        return;
    }

    // Parse Recipe version
    iOffset = strClientMessage.indexOf("RCP_VER/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_RCP_VER);
        return;
    }
    cJob_Start.m_strRCP_VER = strClientMessage.mid(iOffset).section("\"",1,1);

    // Parse Date lot name argument
    iOffset = strClientMessage.indexOf("DATE/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }
    cJob_Start.m_strDate = strClientMessage.mid(iOffset).section("\"",1,1);
    // Check if valid date
    QDateTime cDateTimeInfo;
    cDateTimeInfo = QDateTime::fromString(cJob_Start.m_strDate,"yyyyMMddHHmmss");
    if(cDateTimeInfo.isValid() == false)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }

    // JOB_START arguments loaded: start PAT!
    mProcessingLot = true;	// Ensures no other lot to be PAT procssed now

    // so now load recipe!
    QString		strPatmanErrorMessage;

    // EXECUTE recipe
    bool bSuccess = false;
    int iJobIndex = isJobInQueue(cJob_Start.m_strCAM_LOT);
    if(iJobIndex >= 0)
    {
        // Check if Recipe details matching JOB_CREATE commande
        if((mJobQueue[iJobIndex].m_strRCP_NME != cJob_Start.m_strRCP_NME) ||
           (mJobQueue[iJobIndex].m_strRCP_VER != cJob_Start.m_strRCP_VER))
        {
            // Recipe name / version Mismatch...
            strPatmanErrorMessage = "Recipe name/version mismatch between JOB_CREATE & START commands";
            bSuccess = false;
        }
        else
        if(mJobQueue[iJobIndex].m_strStatus != "READY")
        {
            // Job in queue but not in correct stage!
            strPatmanErrorMessage = "LOT not in READY stage";
            bSuccess = false;
        }
        else
        {
            // Launch PAT-Man processing & set status
            setJobStatus(cJob_Start.m_strCAM_LOT,"ANALYSIS");

            // Flag Success
            bSuccess = true;
        }
    }

    if(bSuccess == false)
    {
        // JOB_START error (either status not okay, or processing error)
        if(iJobIndex < 0)
            create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,
                               VFEI_ERR_STATUS_CREATE);
        else
            create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID,
                               VFEI_JOB_START,strPatmanErrorMessage);
    }
    else
    {
        // JOB_START Successful
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"R",cJob_Start.m_lTID);
    }

    // Add reply details
    strReplyMessage += "CAM_LOT/A=\"" + cJob_Start.m_strCAM_LOT + "\" ";
    strReplyMessage += "OCR_LOT/A=\"" + cJob_Start.m_strOCR_LOT + "\" ";
    strReplyMessage += "OPERATION/A=\"" + cJob_Start.m_strOPERATION + "\" ";
    strReplyMessage += "WAF_CNT/U2=" + QString::number(cJob_Start.m_lWaferCount) + " ";
    strReplyMessage += "WAF_LST/U2[" + QString::number(cJob_Start.m_lWaferCount) + "]=[";
    QString strWaferNumber;
    int	iIndex;
    for(iIndex=0;iIndex < cJob_Start.m_lWaferCount; iIndex++)
    {
        strReplyMessage +=
            strWaferNumber.sprintf("%02ld", cJob_Start.cWaferIdList[iIndex]);
        // Insert space between wafer#
        if(iIndex +1 < cJob_Start.m_lWaferCount)
        strReplyMessage += " ";
    }
    // Close wafer list
    strReplyMessage += "] ";

    strReplyMessage += "RCP_NME/A=\"" + cJob_Start.m_strRCP_NME + "\" ";
    strReplyMessage += "RCP_VER/A=\"" + cJob_Start.m_strRCP_VER + "\" ";
    strReplyMessage += "DATE/A=\"" + cJob_Start.m_strDate + "\"";

    // Send back string to client
    WriteLineToSocket(strReplyMessage);

    // Remove redundant spaces in message to keep it short.
    strReplyMessage = strReplyMessage.simplified();

    // DEBUG purpose
    strString = "WMR < " + strReplyMessage;
    GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().constData());

    // IF status is okay, Launch PAT processing (on all wafers)
    QString				strRecipeFile;
    QString				strRecipeOverloadFile;
    QString				strWaferStdfFile;
    QString				strOLI_MapSource;
    int					iJobInQueue=0;
    CVFEI_JOB_QUEUE		*ptJobQueue=0;
    PATProcessing       cFields(this);
    CPROD_Recipe		cProdRecipe;
    QString				strLogFilePath;
    QString				strFolder;
    QString				strStdfOutputFileName;
    QString				strGoodDieCountList="";	// To include list of GoodBins per wafer (for VFEI reply)
    QStringList			strListOfInputSources;	// List of Source files (used if delete after processing enabled)
    CRecipeFlow			cWorkingFlow;
    QDir				cDir;
    QFile				cFile;
    GQTL_STDF::StdfParse cStdfV4;
    int					iLocalIndex;
    bool				bOLI_Source_Only;
    PATRecipe           lRecipe;

    if(bSuccess == false)
        goto end_analysis;

    // Enter locked section (prevents application from accepting close-event)
    LockCodeSection(true);

    // Notify PAT-Server entering in bloc that can't be stopped.
    if(Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        mPatTaskId = Engine::GetInstance().GetSchedulerEngine().onStartTask(Task::OutlierRemoval);

    // Trace
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Load PROD recipe");

    // Load recipe file.
    if(getReleasedPROD_Recipe(strRecipeFile,strRecipeOverloadFile,cProdRecipe,
                              cJob_Start.m_strRCP_NME,cJob_Start.m_strRCP_VER,strPatmanErrorMessage) == false)
    {
        bSuccess = false;	// Error reloading PAT recipe!
        goto end_analysis;
    }

    // Trace
    GSLOG(SYSLOG_SEV_INFORMATIONAL, "Load ENG recipe");

    if (ReadRecipe(lRecipe, strRecipeFile, strRecipeOverloadFile, strPatmanErrorMessage) == false)
    {
        bSuccess = false;	// Error reloading PAT recipe!
        goto end_analysis;
    }

    // Check if 'close' event received.
    if(checkDeleteClient(false))
        return;

    // Exit locked code section
    LockCodeSection(false);

    // Build folder for output report: <output_report>/<LOT-X>/file.rpt
    for(iLocalIndex = 1; iLocalIndex <= 99; iLocalIndex++)
    {
        strLogFilePath = mConfig.GetOutputReportFolder() + "/" + cJob_Start.m_strCAM_LOT + "-";
        strLogFilePath += strString.sprintf("%02d",iLocalIndex);
        if(cDir.exists(strLogFilePath) == false)
            break;	// We've found the first folder not used yeat...so use it now!
    }
    if(cDir.mkpath(strLogFilePath) == false)
    {
        // Failed to created report output folder
        bSuccess = false;	// PAT error!
        strPatmanErrorMessage = "Failed to created report folder";
        goto end_analysis;
    }

    // Load settings
    cFields.strOutputReportFormat = "interactive";		// Output report disabled, could be 'pdf'
    cFields.strRecipeFile = strRecipeFile;
    cFields.mOutputDatalogFormat = GEX_TPAT_DLG_OUTPUT_STDF;
    cFields.mOutputFormat.clear();                      // No Output map (as updating OptionalSource map)
    cFields.bOutputMapIsSoftBin = false;                // Hardin map
    cFields.bDeleteDataSource = false;                  // Delete input files after processing
    cFields.bReportStdfOriginalLimits = true;           // Include original test limits in STDF file generated.
    cFields.mAlarmType = 0;                             // 0=check Yield loss (in %)
    cFields.mYieldAlarmLoss = 101.0;                   // Disable yield alarm (PAT yield alarm if yield > 101%!)
    cFields.strOperation = cJob_Start.m_strOPERATION;
    cFields.bUpdateTimeFields = true;               // MIR/WIR timestamp is new current time
    cFields.strProd_RecipeName = cJob_Start.m_strRCP_NME;
    cFields.strProd_RecipeVersion = cJob_Start.m_strRCP_VER;
    cFields.bEmbedStdfReport=true;		// Include PAT HTML report in STDF file generated
    getReleasedPROD_Recipe(strRecipeFile,strRecipeOverloadFile,cProdRecipe,
                           cJob_Start.m_strRCP_NME,cJob_Start.m_strRCP_VER,strPatmanErrorMessage);
    cFields.strRecipeOverloadFile = strRecipeOverloadFile;
    //    cWorkingFlow = lRecipe.GetOptions().cWorkingFlow;

    // Check if ZPAT enabled in recipe?
    if(lRecipe.GetOptions().GetExclusionZoneEnabled() && lRecipe.GetOptions().lfCompositeExclusionZoneYieldThreshold > 0)
    {
        QStringList strStdfFileList;

        // Trace
        GSLOG(SYSLOG_SEV_INFORMATIONAL, "Create Z-PAT mask");

        // Get list of STDF files to process so to build Z-PAT mask.
        for(iIndex=0;iIndex < cJob_Start.m_lWaferCount; iIndex++)
        {
            // Find STDF & MAP file for given wafer#.
            strFolder = mConfig.GetInputSTDFFolder();
            if(strFolder.endsWith("/") == false && strFolder.endsWith("\\") == false)
                strFolder += "/";
            // Note: For now, only support ONE flow!
            strFolder += cWorkingFlow.m_strFlow;

            // Get STDF file name
            cFields.strSources.clear();
            strWaferStdfFile = getSTDF_WaferFile(strFolder,
                                  cJob_Start.m_strOCR_LOT,cJob_Start.cWaferIdList[iIndex],strPatmanErrorMessage);
            if(cWorkingFlow.m_iDataSources == FLOW_SOURCE_MAP)
            {
                // Ensure we make a copy of the MAP as STDF input. Path to OLI map is one level above the STDF folder.
                // So need to reset the folder path.
                strFolder = mConfig.GetInputSTDFFolder();
                strWaferStdfFile = strOLI_MapSource;
            }
            strString = strFolder + "/" + strWaferStdfFile;

            // Keep track of STDF input files (full path to each file)
            strStdfFileList << strString;
        }

        // With list of all Data files to process, generate Z-PAT map!
        cFields.strCompositeFile = mConfig.GetOutputSTDFFolder() + "/" + cJob_Start.m_strOCR_LOT + "_composite_pat.txt";

        if (PATEngine::GetInstance().CreateZPATCompositeFile(cFields.strRecipeFile,
                                                                         strStdfFileList,
                                                                         cFields.strCompositeFile) == false)
        {
            // Failed creating Z-PAT file!
            bSuccess                = false;	// PAT error!
            strPatmanErrorMessage   = "Failed creating Z-PAT map:"
                    + PATEngine::GetInstance().GetErrorMessage();
            goto end_analysis;
        }
    }

    // Process each individual file until completion or error
    strListOfInputSources.clear();
    for(iIndex=0;iIndex < cJob_Start.m_lWaferCount; iIndex++)
    {
        // Find STDF & MAP file for given wafer#.
        strFolder = mConfig.GetInputSTDFFolder();
        if(strFolder.endsWith("/") == false && strFolder.endsWith("\\") == false)
            strFolder += "/";
        // Note: For now, only support ONE flow!
        strFolder += cWorkingFlow.m_strFlow;

        // Update wafermap file
        // 1: Define Source map path : <Map_Folder>.Lot.flow.xml
        strOLI_MapSource = cJob_Start.m_strOCR_LOT;
        strOLI_MapSource += "-" +
            strWaferNumber.sprintf("%03ld", cJob_Start.cWaferIdList[iIndex]);
        strOLI_MapSource += "." + cWorkingFlow.m_strFlow;
        strOLI_MapSource += ".XML";
        cFields.strOptionalSource = mConfig.GetInputMapFolder() + "/" + strOLI_MapSource;

        // Keep track of source files to delete: OLI file
        strListOfInputSources += cFields.strOptionalSource;

        // Check if OLI MAP required but missing
        if((cWorkingFlow.m_iDataSources != FLOW_SOURCE_STDF) && (QFile::exists(cFields.strOptionalSource)==false))
        {
            // Missing input MAP file
            bSuccess = false;	// PAT error!
            strPatmanErrorMessage = "Missing input OLI (ENG vs PROD recipe conflict): " + cFields.strOptionalSource;
            break; // Exit loop
        }

        // 2: Define Input STDF file: find specific STDF file...if exists
        // Folder: <stdf_folder>/<flow#>/<STDF file>
        cFields.strSources.clear();
        strWaferStdfFile = getSTDF_WaferFile(strFolder,cJob_Start.m_strOCR_LOT,
                                             cJob_Start.cWaferIdList[iIndex],strPatmanErrorMessage);

        // If NO STDF file, while PROD recipe says to use one, throw exception!
        bOLI_Source_Only = false;
        if(cWorkingFlow.m_iDataSources == FLOW_SOURCE_MAP)
        {
            // Ensure we make a copy of the MAP as STDF input. Path to OLI map is one level above the STDF folder.
            // So need to reset the folder path.
            strFolder = mConfig.GetInputSTDFFolder();
            strWaferStdfFile = strOLI_MapSource;
            bOLI_Source_Only= true;	// We only have a OLI file as data source (no STDF file)
        }
        else
        if(strWaferStdfFile.isEmpty())
        {
            // Missing STDF while mandatory: throw exception!
            strPatmanErrorMessage = "Missing input STDF (ENG vs PROD recipe conflict)";
            bSuccess = false;	// PAT error!
            break; // Exit loop
        }
        // Set STDF input file
        strString = strFolder + "/" + strWaferStdfFile;
        cFields.strSources << strString;

        // Keep track of source files to delete: STDF file
        strListOfInputSources += cFields.strSources;

        // 3: Output STDF file (for Yieldman); ensure to drop the .Z extension if any (basically, drop what's after the .std)
        if(bOLI_Source_Only)
        {
            // No STDF source file available, so we need to read info within the OLI map to
            // load fields needed to build output STDF name. Will build name like:
            // eg: R7105DDK-01_L301100301B4_R7105XXL_p_PAT2_patman_20100622-183700.std
            // Note: on error, returns 'false' so to fall into next case.
            bOLI_Source_Only = buildOutputSTDF_From_OLI(strString,strStdfOutputFileName,cJob_Start,cWorkingFlow);
        }

        // Need to build STDF output name from Input STDF file.
        if(bOLI_Source_Only == false)
        {
            // Input file includes STDF data file: use it to build output STDF file name
            strString = strWaferStdfFile;
            strString = strString.section(".std",0,0);
            // eg: R7105DDK-01_L301100301B4_R7105XXL_p_PAT2_patman_20100622-183700.stdf
            // Now change some fields in the name (to mark it as PAT step)
            cDateTimeInfo = QDateTime::currentDateTime();
            strStdfOutputFileName = strString.section('_',0,1).toUpper() + "_";
            strStdfOutputFileName += cJob_Start.m_strRCP_NME + "_";
            strStdfOutputFileName += strString.section('_',3,3) + "_PAT" + cWorkingFlow.m_strFlow.right(1) + "_patman_";
            strStdfOutputFileName += cDateTimeInfo.toString("yyyyMMdd-hhmmss");
            strStdfOutputFileName += ".stdf";
        }

        cFields.mOutputFolderSTDF = mConfig.GetOutputSTDFFolder() + "/yieldman";
        cDir.mkdir(cFields.mOutputFolderSTDF);
        cFields.mOutputFolderSTDF += "/" + strStdfOutputFileName;
        // Create z-pat sub-folder for Yield-Man to insert SINF maps
        strString = mConfig.GetOutputSTDFFolder() + "/yieldman/zmap";
        cDir.mkdir(strString);

        // 4: Define OUTPUT map path : <path>.Lot.xml
        cFields.strOptionalSource += " " + mConfig.GetOutputMapFolder() + "/" + cJob_Start.m_strOCR_LOT;
        cFields.strOptionalSource += "-" +
            strWaferNumber.sprintf("%03ld", cJob_Start.cWaferIdList[iIndex]);
        cFields.strOptionalSource += ".XML";

        // Build path to output report file (.rpt) to create
        cFields.strLogFilePath = strLogFilePath + "/";
        cFields.strLogFilePath += cJob_Start.m_strOCR_LOT + "-" +
            strWaferNumber.sprintf("%03ld", cJob_Start.cWaferIdList[iIndex]);
        cFields.strLogFilePath += "." + cWorkingFlow.m_strFlow + ".rpt";

        // Call PAT processing
        if(GS::LPPlugin::ProductInfo::getInstance()->isMonitoring())
        {
            // Keep track of files created & used (for cleanup purpose)
            iJobInQueue = isJobInQueue(cJob_Start.m_strCAM_LOT);
            if(iJobInQueue < 0)
            {
                // A ABORT probably occured...erase all files created.
                bSuccess = false;
                break;
            }

            // Enter locked section (prevents application from axcepting close-event)
            LockCodeSection(true);

            // Handle to job queue (so to update list of files created & used)
            ptJobQueue = &mJobQueue[iJobInQueue];

            ptJobQueue->m_strInputFiles += strListOfInputSources;		// Input OLI+STDF, in case need to erase them!
            ptJobQueue->m_strOutputFiles.clear();
            ptJobQueue->m_strOutputFiles += cFields.mOutputFolderSTDF;	// Output STDF
            ptJobQueue->m_strOutputFiles += cFields.strLogFilePath;			// Output Report

            // Trace
            strString = "Process PAT, File: " + cFields.strSources.first();
            GSLOG(SYSLOG_SEV_INFORMATIONAL, strString.toLatin1().constData());

            // Apply PAT to file...
            // No PATPump so just send a null pointer
            Engine::GetInstance().GetSchedulerEngine().ExecutePatProcessing(cFields,strPatmanErrorMessage, 0);

            // Exit locked code section
            LockCodeSection(false);

            // Check if 'close' event received.
            if(checkDeleteClient(false))
                return;

            if(strPatmanErrorMessage.isEmpty() == false)
            {
                bSuccess = false;	// PAT error!
                break;
            }

            // Update list of GoodParts per wafer
            strGoodDieCountList += cFields.Get("TotalGoodAfterPAT").toInt();
            if(iIndex < cJob_Start.m_lWaferCount-1)
                strGoodDieCountList += " ";	// Add space unless last item in list

            // STDF output file wanted by ST WMR?
            // Path to STDF file to create for ST purpose (can be light STDF)
            strString = mConfig.GetOutputSTDFFolder() + "/" + strStdfOutputFileName;
            switch(cProdRecipe.m_iStdfOutput)
            {
                case GEX_PAT_STDF_OUTPUT_DISABLED:
                    // Do NOT create a STDF output file for ST purpose (only keep the one created for Yieldman)
                    break;

                case GEX_PAT_STDF_OUTPUT_FULL:
                    // Get FULL STDF output:simply copy PAT-Man STDF file created in yieldman folder!
                    cFile.copy(cFields.mOutputFolderSTDF,strString);
                    break;

                case GEX_PAT_STDF_OUTPUT_WAFERMAP:
                    // Extract wafermap from STDF...create 'Light STDF'
                    cStdfV4.toolExtractWafermap(cFields.mOutputFolderSTDF,strString);
                    break;
            }
        }

        // Make sure screen is refreshed & purge temporary files
        QCoreApplication::processEvents();
    }

end_analysis:
    // Exit locked code section
    LockCodeSection(false);

    // Notify PAT-Server exiting bloc that couldn't be stopped.
    if(Engine::GetInstance().GetSchedulerEngine().isSchedulerStopped())
        Engine::GetInstance().GetTaskManager().OnStoppedTask(mPatTaskId);

    // if Z-MAP enabled in recipe, remove intermediate Z-Map file created
    if(QFile::exists(cFields.strCompositeFile))
    {
        // Export this Z-PAT map as a .SINF map (so Yield-Man can insert it in the database
        strString = mConfig.GetOutputSTDFFolder() + "/yieldman/zmap/" + cJob_Start.m_strCAM_LOT + "_zmap.sinf";

        // Get Product name...to know it, simply read MIR info of last STDF processed.
        CGexGroupOfFiles *pGroup;
        CGexFileInGroup *pFile;
        QString strDeviceName = "na";
        if(gexReport->getGroupsList().count()>0)
        {
            pGroup = gexReport->getGroupsList().first();
            if(pGroup)
            {
                pFile = pGroup->pFilesList.first();
                if(pFile)
                    strDeviceName = pFile->getMirDatas().szPartType; // Product
            }
        }

        // Device string is used to hold ZPAT-Mask + total count of wafers stacked in mask.
        QString strWaferName = "ZPAT-Mask/" + QString::number(cJob_Start.m_lWaferCount);;
        if(bSuccess)
        {
            if (PATEngine::GetInstance().ExportZPATCompositeFileToSINF(cFields.strCompositeFile,
                                                                                   strString,
                                                                                   strDeviceName,
                                                                                   cJob_Start.m_strCAM_LOT,
                                                                                   strWaferName) == false)
            {
                bSuccess = false;
                strPatmanErrorMessage = PATEngine::GetInstance().GetErrorMessage();
            }
        }

        // Delete Z-PAT map.
        cDir.remove(cFields.strCompositeFile);
    }

    // Write back LOT_END event message
    strCMD = "LOT_END";
    cJob_Start.m_lTID++;	// Ensures this is a packet ID never used before
    if(bSuccess == false)
    {
        // Error during PAT processing
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"E",cJob_Start.m_lTID,VFEI_JOB_ANALYSIS,strPatmanErrorMessage);
    }
    else
    {
        // Successful PAT processing
        create_ReplyString(strReplyMessage,strCMD,cJob_Start.m_strMID,"E",cJob_Start.m_lTID);
    }
    // Add reply details
    strReplyMessage += "CAM_LOT/A=\"" + cJob_Start.m_strCAM_LOT + "\" ";
    strReplyMessage += "OCR_LOT/A=\"" + cJob_Start.m_strOCR_LOT + "\" ";
    strReplyMessage += "OPERATION/A=\"" + cJob_Start.m_strOPERATION + "\" ";
    strReplyMessage += "RCP_NME/A=\"" + cJob_Start.m_strRCP_NME + "\" ";
    strReplyMessage += "RCP_VER/A=\"" + cJob_Start.m_strRCP_VER + "\" ";
    strReplyMessage += "WAF_CNT/U2=" + QString::number(cJob_Start.m_lWaferCount) + " ";
    strReplyMessage += "WAF_LST/U2[" + QString::number(cJob_Start.m_lWaferCount) + "]=[";
    for(iIndex=0;iIndex < cJob_Start.m_lWaferCount; iIndex++)
    {
        strReplyMessage +=
            strWaferNumber.sprintf("%02ld", cJob_Start.cWaferIdList[iIndex]);

        // Insert space between wafer#
        if(iIndex +1 < cJob_Start.m_lWaferCount)
        strReplyMessage += " ";
    }
    // Close wafer list + open GoodDie list
    strReplyMessage += "] WAF_GOOD/U4[";

    // On error, do not return the GOOD Die list
    if(bSuccess == false)
    {
        // Some PAT exception occured on this lot...
        strReplyMessage += "0]=[";
    }
    else
    {
        // PAT sucessfull on this lot.
        strReplyMessage += QString::number(cJob_Start.m_lWaferCount) + "]=[" + strGoodDieCountList;
    }

    // Close wafer GoodDie list
    strReplyMessage += "] ";
    strReplyMessage += "DATE/A=\"" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + "\"";

    // Job completed: delete input files, and on failure move output files to 'error' folder
    bool	bError = (bSuccess) ? false:true;
    deleteJobInQueue(cJob_Start.m_strCAM_LOT,bError);

    // Accept other LOT to be processed now (exit from single-thread type processing)
    mProcessingLot = false;
}
#endif
///////////////////////////////////////////////////////////
// Read OLI contents to extract fields needed to build STDF
// output file name
///////////////////////////////////////////////////////////
bool VFEIClient::buildOutputSTDF_From_OLI(QString strOLI_MapSource,
                                          QString &strStdfOutputFileName,
                                          CVFEI_JOB_START cJob_Start,
                                          const CRecipeFlow& cWorkingFlow)
{
    // If original file doesn't exists, fail processing
    if(QFile::exists(strOLI_MapSource) == false)
        return false;

    // Check the input file is a SEMI-E142 compliant file.
    QFile f(strOLI_MapSource);
    if(!f.open(QIODevice::ReadOnly))
        return false;

    // Assign file I/O stream
    QTextStream hE142File(&f);

    int		iIndex;
    QString strString = hE142File.readLine();
    if(strString.contains("<?xml", Qt::CaseInsensitive) == false)
        return false;	// This file is not a E142
    strString = hE142File.readLine();
    if(strString.contains("urn:semi-org:xsd.E142", Qt::CaseInsensitive) == false)
        return false;	// This file is not a E142

    // need:
    // r7105xcl-01_q915gjs-01e2_wi7105xxx_p_ews2_ta93k53_20090905-094342.stdf.3506.1_F.5506.1
    // R7105DDK-01_L301100301B4_R7105XXL_p_PAT2_patman_20100622-183700.stdf
    //
    QString	strProductID="";
    QString strLotWaferID="";

    do
    {
        // Read next line from file
        strString = hE142File.readLine();

        // Extract ProductID
        iIndex = strString.indexOf("<ProductId>",0,Qt::CaseInsensitive);
        if(iIndex >= 0)
        {
            // Extract Product (eg: R7105XCL-01)
            strProductID = strString.replace("<ProductId>","");
            strProductID = strProductID.replace("</ProductId>","");
            strProductID = strProductID.trimmed();
            goto next_line;
        }

        // Extract Lot+Wafer# (eg: Q915GJS-01E2)
        iIndex = strString.indexOf("SubstrateId=\"",0,Qt::CaseInsensitive);
        if(iIndex >= 0)
        {
            // Extract Lot+Wafer
            strLotWaferID = strString.section("SubstrateId=\"",1);
            strLotWaferID = strLotWaferID.section("\"",0,0);
            goto next_line;
        }

        // next line
        next_line:;
    }
    while(hE142File.atEnd() == false);

    f.close();

    // Build STDF output file name
    // eg: R7105DDK-01_L301100301B4_R7105XXL_p_PAT2_RSD4042_20100622-183700.stdf
    QDateTime cDateTimeInfo = QDateTime::currentDateTime();
    strStdfOutputFileName = strProductID + "_" + strLotWaferID + "_";
    strStdfOutputFileName += cJob_Start.m_strRCP_NME;
    strStdfOutputFileName += "_p_PAT" + cWorkingFlow.m_strFlow.right(1);

    // Server running PAT-Server software
    strStdfOutputFileName += "_" + Engine::GetInstance().GetSystemInfo().strHostName + "_";
    strStdfOutputFileName += cDateTimeInfo.toString("yyyyMMdd-hhmmss");
    strStdfOutputFileName += ".stdf";

    // Success
    return true;
}

bool VFEIClient::ReadRecipe(PATRecipe &lPATRecipe, const QString &lRecipeFile,
                            const QString &lProdRecipe, QString& lErrorMessage)
{
    QSharedPointer<PATRecipeIO> lRecipeIO(PATRecipeIO::CreateRecipeIo(lRecipeFile));

    if (lRecipeIO.isNull())
    {
        lErrorMessage = "Bad recipe: Cannot find a matching RecipeIO for this recipe";
        return false;
    }

    if(lRecipeIO->Read(lPATRecipe) == false)
    {
        lErrorMessage = lRecipeIO->GetErrorMessage();
        return false;
    }

    // If no Overload recipe file (production recipe) defined, nicely return!
//    lPATRecipe.GetOptions().cWorkingFlow.clear();	// Reset variables

    if(lProdRecipe.isEmpty())
        return true;

    // Load overload recipe
    CPROD_Recipe cOverloadRecipe;
    if(cOverloadRecipe.loadRecipe(lProdRecipe, lErrorMessage) == false)
        return false;

    // Overload Parametric Bin (SPAT, DPAT,NNR,IDDQ-Delta)
    long lValue = cOverloadRecipe.m_iPPAT_Bin;
    if(lValue)
    {
        lPATRecipe.GetOptions().iFailStatic_HBin    = lValue;
        lPATRecipe.GetOptions().iFailDynamic_HBin   = lValue;
        lPATRecipe.GetOptions().SetNNRHardBin(lValue);
        lPATRecipe.GetOptions().mIDDQ_Delta_HBin    = lValue;

        // Overload binning for each Parametric test with a custom PAT setting
        QHash<QString, CPatDefinition*>::iterator   itPATDefinifion;
        CPatDefinition	*                           ptPatDef = NULL;

        for(itPATDefinifion = lPATRecipe.GetUniVariateRules().begin();
            itPATDefinifion != lPATRecipe.GetUniVariateRules().end(); ++itPATDefinifion)
        {
            ptPatDef = *itPATDefinifion;

            // If test is SPAT enabled, overload its binning!
            if(ptPatDef->m_lFailStaticBin >= 0)
                ptPatDef->m_lFailStaticBin = lValue;

            // If test is DPAT enabled, overload its binning!
            if(ptPatDef->m_lFailDynamicBin >= 0)
                ptPatDef->m_lFailDynamicBin = lValue;
        }
    }

    // Overload Geographic Bin (GDBN,Clustering,Reticle)
    lValue = cOverloadRecipe.m_iGPAT_Bin;
    if(lValue >= 0)
    {
        lPATRecipe.GetOptions().mGDBNPatHBin = lValue;
        lPATRecipe.GetOptions().SetReticleHardBin(lValue);
        lPATRecipe.GetOptions().mClusteringPotatoHBin = lValue;
    }

    // Overload Z-PAT Bin
    lValue = cOverloadRecipe.m_iZPAT_Bin;
    if(lValue >= 0)
    {
        // Overload Bin only for rules enabled!

        // Zpat enabled
        if(lPATRecipe.GetOptions().GetExclusionZoneEnabled() && lPATRecipe.GetOptions().lfCompositeExclusionZoneYieldThreshold > 0)
            lPATRecipe.GetOptions().iCompositeZone_HBin  = lValue;
        if(lPATRecipe.GetOptions().bMergeEtestStdf)
            lPATRecipe.GetOptions().iCompositeEtestStdf_HBin = lValue;
    }

    // Flow: for now ONLY consider first flow defined.
//    CRecipeFlow cFirstFlow = lPATRecipe.GetOptions().cWorkingFlow = cOverloadRecipe.m_cFlows.first();
    CRecipeFlow cFirstFlow = cOverloadRecipe.m_cFlows.first();
    switch(cFirstFlow.m_iDataSources)
    {
        case FLOW_SOURCE_MAP:
            // This recipe only involves SEMI maps...
            break;
        case FLOW_SOURCE_STDF:
            // This recipe only works on STDF
            break;
        case FLOW_SOURCE_MAP_STDF:
            // This recipe used MAP + STDF files
            break;
    }

    // STDF Flow: For now, support single flow only.
    lPATRecipe.GetOptions().iUpdateMap_FlowNameID = 0;

    // Check Flows used in GDBN rules
    QList <CGDBN_Rule>::iterator lGDBNIter;
    CGDBN_Rule lGdbnRule;
    int iRuleID=0;
    for(lGDBNIter = lPATRecipe.GetOptions().mGDBNRules.begin();
        lGDBNIter != lPATRecipe.GetOptions().mGDBNRules.end(); ++lGDBNIter, iRuleID++)
    {
        lGdbnRule = *lGDBNIter;
        lGdbnRule.mWafermapSource = lGdbnRule.mWafermapSource % 3;
    }

    // Check Flows used in Cluster rules
    QList <CClusterPotatoRule>::iterator lItClusterPotato;
    CClusterPotatoRule lClusteringRule;
    iRuleID=0;
    for(lItClusterPotato = lPATRecipe.GetOptions().mClusterPotatoRules.begin();
        lItClusterPotato != lPATRecipe.GetOptions().mClusterPotatoRules.end(); ++lItClusterPotato, iRuleID++)
    {
        lClusteringRule = *lItClusterPotato;
        lClusteringRule.mWaferSource = lClusteringRule.mWaferSource % 3;
    }

    // Check Flows used in Reticle
    QList <PATOptionReticle>::iterator itReticle;
    iRuleID=0;
    for(itReticle = lPATRecipe.GetOptions().GetReticleRules().begin();
        itReticle != lPATRecipe.GetOptions().GetReticleRules().end(); ++itReticle, iRuleID++)
    {
        (*itReticle).SetReticle_WafermapSource((*itReticle).GetReticle_WafermapSource() % 3);
    }

    // Success
    return true;
}

///////////////////////////////////////////////////////////
// Process request: ABORT
///////////////////////////////////////////////////////////
void VFEIClient::process_ABORT(QString &strClientMessage,QString &strReplyMessage)
{
    strReplyMessage = "CMD/A=\"ABORT\" MID/A=\"PATMAN_SW\" MTY/A=\"R\"";

    QString strCMD = "ABORT";
    QString	strItem;
    CVFEI_ABORT		cJob_Abort;
    int			iOffset;

    // Parse MID argument
    iOffset = strClientMessage.indexOf("MID/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }
    cJob_Abort.m_strMID = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty MID
    if(cJob_Abort.m_strMID.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }

    // Parse MTY argument
    iOffset = strClientMessage.indexOf("MTY/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }
    cJob_Abort.m_strMTY = strClientMessage.mid(iOffset).section("\"",1,1);
    // MTY must always be be 'C'
    if(cJob_Abort.m_strMTY != "C")
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }

    // Parse TID argument
    iOffset = strClientMessage.indexOf("TID/U4=");
    if(iOffset <0)
    {
        // TID missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }
    if(vfei_extract_Msg_Long(strClientMessage,iOffset,cJob_Abort.m_lTID) == false)
    {
        // Invalid/Corrupted TID
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }

    // Parse CAM_LOT lot name argument
    iOffset = strClientMessage.indexOf("CAM_LOT/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }
    cJob_Abort.m_strCAM_LOT = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty CAM Lot
    if(cJob_Abort.m_strCAM_LOT.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_CAM_LOT);
        return;
    }

    // Parse Recipe name lot name argument
    iOffset = strClientMessage.indexOf("RCP_NME/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_RCP_NME);
        return;
    }
    cJob_Abort.m_strRCP_NME = strClientMessage.mid(iOffset).section("\"",1,1);

    // Parse Recipe version
    iOffset = strClientMessage.indexOf("RCP_VER/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_RCP_VER);
        return;
    }
    cJob_Abort.m_strRCP_VER = strClientMessage.mid(iOffset).section("\"",1,1);

    // Parse Date lot name argument
    iOffset = strClientMessage.indexOf("DATE/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }
    cJob_Abort.m_strDate = strClientMessage.mid(iOffset).section("\"",1,1);
    // Check if valid date
    QDateTime cDateTimeInfo;
    cDateTimeInfo = QDateTime::fromString(cJob_Abort.m_strDate,"yyyyMMddHHmmss");
    if(cDateTimeInfo.isValid() == false)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }

    // Remove job from queue.
    if(deleteJobInQueue(cJob_Abort.m_strCAM_LOT,false,cJob_Abort.m_strRCP_NME,cJob_Abort.m_strRCP_VER) == false)
    {
        // Abort failed: Recipe name/version mismatch!
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID,VFEI_ABORT_ERR);
    }
    else
    {
        // Abort successful
        create_ReplyString(strReplyMessage,strCMD,cJob_Abort.m_strMID,"R",cJob_Abort.m_lTID);
    }

    strReplyMessage += "CAM_LOT/A=\"" + cJob_Abort.m_strCAM_LOT + "\" ";
    strReplyMessage += "RCP_NME/A=\"" + cJob_Abort.m_strRCP_NME + "\" ";
    strReplyMessage += "RCP_VER/A=\"" + cJob_Abort.m_strRCP_VER + "\" ";
    strReplyMessage += "DATE/A=\"" + cJob_Abort.m_strDate + "\"";
}

///////////////////////////////////////////////////////////
// Process request: STATUS
///////////////////////////////////////////////////////////
void VFEIClient::process_STATUS(QString &strClientMessage,QString &strReplyMessage)
{
    // eg: strReplyMessage = "CMD/A=\"STATUS\" MID/A=\"PATMAN_SW\" MTY/A=\"R\"";

    QString strCMD = "STATUS";
    QString	strItem;
    CVFEI_STATUS		cJob_Status;
    int			iOffset;

    // Parse MID argument
    iOffset = strClientMessage.indexOf("MID/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }
    cJob_Status.m_strMID = strClientMessage.mid(iOffset).section("\"",1,1);
    // Error if empty MID
    if(cJob_Status.m_strMID.isEmpty())
    {
        create_ReplyString(strReplyMessage,strCMD,"?","R",0,VFEI_COMM_ERR_MID);
        return;
    }

    // Parse MTY argument
    iOffset = strClientMessage.indexOf("MTY/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }
    cJob_Status.m_strMTY = strClientMessage.mid(iOffset).section("\"",1,1);
    // MTY must always be be 'C'
    if(cJob_Status.m_strMTY != "C")
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",0,VFEI_COMM_ERR_MTY);
        return;
    }

    // Parse TID argument
    iOffset = strClientMessage.indexOf("TID/U4=");
    if(iOffset <0)
    {
        // TID missing
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }
    if(vfei_extract_Msg_Long(strClientMessage,iOffset,cJob_Status.m_lTID) == false)
    {
        // Invalid/Corrupted TID
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",0,VFEI_COMM_ERR_TID);
        return;
    }

    // Parse Date lot name argument
    iOffset = strClientMessage.indexOf("DATE/A=");
    if(iOffset <0)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",cJob_Status.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }
    cJob_Status.m_strDate = strClientMessage.mid(iOffset).section("\"",1,1);
    // Check if valid date
    QDateTime cDateTimeInfo;
    cDateTimeInfo = QDateTime::fromString(cJob_Status.m_strDate,"yyyyMMddHHmmss");
    if(cDateTimeInfo.isValid() == false)
    {
        create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",cJob_Status.m_lTID,VFEI_ARG_ERR_DATE);
        return;
    }

    // Return STATUS reply
    create_ReplyString(strReplyMessage,strCMD,cJob_Status.m_strMID,"R",cJob_Status.m_lTID);

    // Check if work in progress/queue or not!
    if(iTotalPatTasksRunning() == 0)
    {
        // Empty queue!
        strReplyMessage +=  "LOT/A[0]=[] LOT_STATUS/A[0]=[] ";	// PAT-Man is Idle!
    }
    else
    {
        QString	strCAM_LOTS="";
        QString	strCAM_STATUS="";
        CVFEI_JOB_QUEUE cJob;
        int	iIndex;
        int	iTotalActives=0;

        for(iIndex = 0; iIndex < mJobQueue.count(); iIndex++)
        {
            cJob = mJobQueue[iIndex];

            // For PAT tasks running...
            if(cJob.m_strStatus.isEmpty() == false)
            {
                // Update CAM_LOTS
                strCAM_LOTS += "\"" + cJob.m_strCAM_LOT + "\"";
                // Add space between CAM lots
                if(iIndex < mJobQueue.count()-1)
                strCAM_LOTS += " ";

                // Update CAM_STATUS
                strCAM_STATUS += "\"" + cJob.m_strStatus + "\"";
                // Add space between CAM lots
                if(iIndex < mJobQueue.count()-1)
                strCAM_STATUS += " ";

                // Keep track of total actives PAT processes
                iTotalActives++;
            }
        }

        // Build full reply status line
        strReplyMessage += "LOT/A[" + QString::number(iTotalActives) + "]=[";
        strReplyMessage += strCAM_LOTS + "] ";
        strReplyMessage += "LOT_STATUS/A[" + QString::number(iTotalActives) + "]=[";
        strReplyMessage += strCAM_STATUS + "] ";
    }

    strReplyMessage += "DATE/A=\"" + cJob_Status.m_strDate + "\"";
}

///////////////////////////////////////////////////////////
// Get file names for release PROduction recipe
///////////////////////////////////////////////////////////
bool VFEIClient::getReleasedPROD_Recipe(QString &strRecipeFile,QString &strOverloadRecipeFile,CPROD_Recipe &cProdRecipe,QString strRecipeName,QString strVersion,QString &strErrorMessage)
{
    // Build recipe name <prod_folder>/released/<recipe>.<version>.csv
    strOverloadRecipeFile = mConfig.GetProdRecipeFolder() + "/released/";
    strOverloadRecipeFile += strRecipeName + "." + strVersion + ".csv";

    // Overload recipe missing...?
    if(QFile::exists(strOverloadRecipeFile) == false)
    {
        strErrorMessage = "PROD recipe not found: " + strOverloadRecipeFile;
        return false;
    }

    // Read overload recipe to see which new recipe to use...
    if(cProdRecipe.loadRecipe(strOverloadRecipeFile,strErrorMessage) == false)
        return false;	// Loading error...

    // For now, only FIRST flow supported...
    CRecipeFlow cOneFlow = cProdRecipe.m_cFlows.first();
    QString strEngRecipePath = mConfig.GetEngRecipeFolder() + "/" + cOneFlow.m_strENG_RecipeFile;
    if(QFile::exists(strEngRecipePath) == false)
    {
        // ENG recipe specified in overload receipe doesn't exist...
        strErrorMessage = "ENG recipe not found: " + strEngRecipePath;
        return false;
    }

    // PROD recipe fine, use new ENG recipe as per Flow#1!
    strRecipeFile = strEngRecipePath;
    return true; // Overload recipe exists
}

}   // namespace Gex
}   // namespace GS
