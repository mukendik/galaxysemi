/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_message.c																*/
/*                                                                                      */
/* GTL messages to screen, log...														*/
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_MESSAGE_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//#include <syslog.h>

#ifdef unix
    #include <sys/param.h>
#endif

#include <gstdl_utils_c.h>

#include "gtl_core.h"
#include "gtl_message.h"
#include "gtl_constants.h"
#include "gtl_output.h"

/*======================================================================================*/
/* Constants                                                                            */
/*======================================================================================*/
/* In case it is not already defined */
#ifndef MAXHOSTNAMELEN
    #define MAXHOSTNAMELEN 256
#endif

/*======================================================================================*/
/* Externs                                                                              */
/*======================================================================================*/
/* From gtl_main.c */
extern char	gtl_szApplicationFullName[]; /* Library name and version */
extern GTL_Station gtl_stStation; /* Tester station information */
extern GTL_GlobalInfo gtl_stGlobalInfo;	/* GTL Global information */
extern GTL_ProdInfo	gtl_stProdInfo;	/* Tester Production information */

/*======================================================================================*/
/* Private declarations                                                                 */
/*======================================================================================*/
int	gtl_msg_nLogFileNames_Initialized = 0;

// 1=info 0=warning -1=error
// return <0 on error
int gtl_push_back_new_message(int severity, char* message, int message_id)
{
    GTL_LOG(6, "%d %d %s", severity, message_id, message?message:"?");
    GTL_Message* lM=malloc(sizeof(GTL_Message));
    lM->mSeverity=severity;
    if (message)
        snprintf(lM->mMessage, GTL_MESSAGE_STRING_SIZE, message);
    else
        sprintf(lM->mMessage, (char*)"?");
    lM->mMessageID=message_id;
    int r=RingBufferPushBack(&gtl_stGlobalInfo.mMessages, lM);
    if (r<0)
        GTL_LOG(3, "RingBufferPushBack failed: %d : %s", r, message);

    GTL_LOG(7, "Stack has now %d messages", r);

    int lR=gtl_OutputNewMessage(lM);
    if (lR!=0)
        GTL_LOG(4, "Failed to output new message (code %d) '%s'", lR, message);

    return r;
}

/*======================================================================================*/
/* Public CODE                                                                          */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : create filenames for log files.                                       */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_InitLogfileNames()
{
    char lBaseName[GTC_MAX_PATH]="";

    /* Build part of the file name, but make sure no '?' used */
    strcpy(lBaseName, "jobname");
    if(strcmp(gtl_stStation.szTestJobName, "?") != 0)
        strcpy(lBaseName, gtl_stStation.szTestJobName);
    strcat(lBaseName, "_");
    if(strcmp(gtl_stStation.szNodeName, "?") != 0)
        strcat(lBaseName, gtl_stStation.szNodeName);
    else
        strcat(lBaseName, "nodename");

	/* Initialize file names */
	gtl_msg_nLogFileNames_Initialized = 1;
	return;
}

int gtl_get_number_messages_in_stack()
{
    return gtl_stGlobalInfo.mMessages.size;
}

int gtl_pop_last_message(int *severity, char* message, int* messageID)
{
    if (gtl_stGlobalInfo.mMessages.size<=0)
        return GTL_CORE_ERR_NO_MORE_MESSAGE;

    if (!severity || !message)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    void* m=0; // actually a GTL_Message*
    int r=RingBufferPopBack(&gtl_stGlobalInfo.mMessages, &m);
    if (r<0)
    {
        GTL_LOG(3, "RingBufferPopBack failed: %d", r);
        return GTL_CORE_ERR_MESSAGE_POPING_FAILED;
    }
    if (!m)
    {
        GTL_LOG(4, "GTL message null in ringbuffer. Strange.",0);
        return GTL_CORE_ERR_MESSAGE_POPING_FAILED;
    }
    *severity=((GTL_Message*)m)->mSeverity;
    if (((GTL_Message*)m)->mMessage)
        sprintf(message, ((GTL_Message*)m)->mMessage);
    else
        sprintf(message, "N/A");
    if (messageID)
        *messageID=((GTL_Message*)m)->mMessageID;

    free(m);

    return GTL_CORE_ERR_OKAY;
}

int gtl_pop_first_message(int *severity, char* message, int* messageID)
{
    if (gtl_stGlobalInfo.mMessages.size<=0)
        return GTL_CORE_ERR_NO_MORE_MESSAGE;

    if (!severity || !message)
        return GTL_CORE_ERR_INVALID_PARAM_POINTER;

    if (gtl_stGlobalInfo.mMessages.size<=0)
        return GTL_CORE_ERR_NO_MORE_MESSAGE;

    void* m=0;
    //GTL_Message *m;
    int r=RingBufferPopFront(&gtl_stGlobalInfo.mMessages, &m);
    if (r<0)
    {
        GTL_LOG(4, "RingBufferPopFront returned %d", r);
        return GTL_CORE_ERR_MESSAGE_POPING_FAILED;
    }
    if (!m)
        return GTL_CORE_ERR_MESSAGE_POPING_FAILED; //-1;
    *severity=((GTL_Message*)m)->mSeverity;
    if (((GTL_Message*)m)->mMessage)
        sprintf(message, ((GTL_Message*)m)->mMessage);
    else
        sprintf(message, "N/A");
    if (messageID)
        *messageID=((GTL_Message*)m)->mMessageID;

    free(m);

    return GTL_CORE_ERR_OKAY;
}

int gtl_clear_messages_stack()
{
    // will free all GTL messages if any
    return RingBufferClear(&gtl_stGlobalInfo.mMessages, 'y');
}

/*======================================================================================*/
/* Description  : print an information message to the screen.                           */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nLabel                                                                       */
/*         1   : message label should be used                                           */
/*         != 1: no message label                                                       */
/*                                                                                      */
/*     int nLineFeed                                                                    */
/*         1   : add LineFeed at the end of the message                                 */
/*         != 1: no LineFeed                                                            */
/*                                                                                      */
/*     int nInfoCode                                                                    */
/*         code specifying info message (see gtl_message.h)                             */
/*                                                                                      */
/*     char* szInfo                                                                     */
/*         info specific text                                                           */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_Info(int nLabel, int nLineFeed, int nInfoCode, char* szInfo)
{
    char szString[GTL_MSG_MAXSIZE], szTemp[GTL_MSG_MAXSIZE];

	if(gtl_stGlobalInfo.nUserOptions & GTL_USEROPTION_QUIETON)
        return;

	/* Build message string */
	if(nLabel)
		strcpy(szString, GTL_INFO_LABEL);
	else
		strcpy(szString, "");

	switch(nInfoCode)
	{
		case GTL_MSGINFO_STATE:
			/* Display GTL state */
			gtl_msg_GetGTLStateString(szTemp);
			strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_INIT:
			/* Init GTL */
			strcat(szString, "Initializing ");
			strcat(szString, gtl_szApplicationFullName);
			break;
		case GTL_MSGINFO_CONNECTING:
			/* Connecting to a production server */
            sprintf(szTemp, "Connecting to server %s ...", szInfo);
            strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_CONNECT_OK:
			/* Connecting to a production server successfull */
            sprintf(szTemp, " OK %s.", szInfo);
			strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_CONNECT_NOK:
			/* Connecting to a production server unsuccessfull */
            sprintf(szTemp, " FAILED %s.", szInfo);
			strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_VERSION:
			/* Display GTL version: */
			strcat(szString, gtl_szApplicationFullName);
			break;
		case GTL_MSGINFO_TEXT:
			/* Display the text in szInfo */
			strcat(szString, szInfo);
			break;
		case GTL_MSGINFO_RECONNECT:
			/* Trying to reconect to GTL server */
            sprintf(szTemp, "Trying to re-connect to GTM server %s ", szInfo);
			strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_CHECKSESSION:
			/* Checking if GTM session still valid */
			strcat(szString, "Checking if GTM session is valid ");
			break;
		case GTL_MSGINFO_SETSTATE_DISABLED:
			/* Set GTL state to DISABLED */
			strcat(szString, "GTL disabled.");
			break;
			break;
        case GTL_MSGINFO_SETSTATE_ENABLED:
            /* Set GTL state to ENABLED */
            strcat(szString, "GTL enabled.");
			break;
        case GTL_MSGINFO_SETSTATE_OFFLINE:
            /* Set GTL state to OFFLINE */
            strcat(szString, "GTL state set to GTL_STATE_OFFLINE. Testing will continue with current limits.");
            break;
        case GTL_MSGINFO_ENDLOT:
			/* New lot detected */
			sprintf(szTemp, "Endlot detected.");
			strcat(szString, szTemp);
			break;
        case GTL_MSGINFO_END_SPLITLOT:
            sprintf(szTemp, "End of splitlot.");
            strcat(szString, szTemp);
            break;
		case GTL_MSGINFO_RESTART_BASELINE:
			/* Baseline must be re-started */
            sprintf(szTemp, "Re-starting baseline for site %s...",szInfo);
			strcat(szString, szTemp);
			break;
		case GTL_MSGINFO_RESET:
			/* Reset GTL */
			sprintf(szTemp, "Resetting GTL library...");
			strcat(szString, szTemp);
			break;
        case GTL_MSGINFO_SETSTATE_NOT_INITIALIZED:
            /* Set GTL state to GTL_MSGINFO_SETSTATE_NOT_INITIALIZED */
            strcat(szString, "GTL state set to GTL_STATE_NOT_INITIALIZED.");
            break;
    }
  
    GTL_LOG(5, "Info message: '%s'", szString);

	if(nLineFeed)
		strcat(szString, "\n");

    // case 7103
    int r=gtl_push_back_new_message(1, szString, nInfoCode);
    if (r<0)
        GTL_LOG(3, "gtl_push_back_new_message failed: %d", r);

	return;
}

/*======================================================================================*/
/* Description  : emit an error message                                                 */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*     int nErrorCode                                                                   */
/*         code specifying error (see message.h)                                        */
/*     char* szInfo                                                                     */
/*         specifies some error dependant text                                          */
/*     int nMode                                                                        */
/*         specifies destination of the message                                         */
/*         GTL_MSG_TODISPLAY : send msg to display                                      */
/*         GTL_MSG_TOBUFFER  : send msg to buffer                                       */
/*     char *szDestBuffer                                                               */
/*         string to receive msg (if GTL_MSG_TOBUFFER)                                  */
/* Return Codes : none                                                                  */
/*======================================================================================*/
int gtl_msg_Error(nErrorCode, szInfo, nMode, szDestBuffer)
int   nErrorCode;
char  *szInfo;
int   nMode;
char  *szDestBuffer;
{
    char szMessage[GTL_MSG_MAXSIZE]="", szText[GTL_MSG_MAXSIZE]="";
	
	/* Build error string */
	sprintf(szMessage, GTL_ERR_LABEL);
	switch(nErrorCode)
	{
		case GTL_MSGERROR_MAIN_CONF:
			/* Couldn't read configuration file */
			sprintf(szText, "Error reading Configuration file (%s).", szInfo);
			break;
        case GTL_MSGERROR_MAIN_CRRF:
            /* Couldn't read recipe file */
            sprintf(szText, "Error reading Recipe file (%s).", szInfo);
            break;
        case GTL_MSGERROR_MAIN_GTM_INIT:
			/* Error initializing connection to GTM server */
			sprintf(szText, "Error initializing connection to GTM server.");
			break;
        case GTL_MSGERROR_MAIN_GTM_RECIPE:
			/* GTM server couldn't read recipe file */
            sprintf(szText, "GTM server couldn't read recipe file (%s).", szInfo);
			break;			
		case GTL_MSGERROR_MAIN_GTM_LICEXPIRED:
			/* License expired */
            sprintf(szText, "License expired (%s).", szInfo);
			break;			
        case GTL_MSGERROR_MAIN_GTM_NOLIC:
            /* No license */
            sprintf(szText, "No license.");
            break;
        case GTL_MSGERROR_MAIN_GTM_TESTLIST:
			/* Error retrieving testlist from GTM server */
			sprintf(szText, "Error retrieving testlist from GTM server.");
			break;
		case GTL_MSGERROR_MAIN_GTM_INITPAT:
			/* Error retrieving PAT configuration from GTM server */
			sprintf(szText, "Error retrieving PAT configuration from GTM server.");
			break;
		case GTL_MSGERROR_MAIN_GTM_COMM:
			/* Error during communication with GTM server */
			sprintf(szText, "Error during communication with GTM server.");
			break;
        case GTL_MSGERROR_MAIN_PROCESSMESSAGE:
            /* Error processing message received from GTM */
            sprintf(szText, "Error processing message received from GTM.");
            break;
        case GTL_MSGERROR_MAIN_GTM_UNKNOWN_STATUS:
            /* Unknown status received from GTM */
            sprintf(szText, "Unknown status received from GTM during initialization.");
            break;
        case GTL_MSGERROR_MAIN_UPDATE_SPAT:
            /* Error updating GTL with SPAT limits */
            sprintf(szText, "Error updating GTL with SPAT limits.");
            break;
        case GTL_MSGERROR_MAIN_UPDATE_DPAT:
            /* Error updating GTL with DPAT limits */
            sprintf(szText, "Error updating GTL with DPAT limits.");
            break;
        case GTL_MSGERROR_TESTLIST_MAF:
			/* Memory allocation failure */
			sprintf(szText, "Memory allocation failure (Testlist).");
			break;
		case GTL_MSGERROR_TESTLIST_NTL:
			/* Couldn't find TestList */
			sprintf(szText, "Couldn't update Testlist: no Testlist found.");
			break;
		case GTL_MSGERROR_TESTLIST_EOTL:
			/* End of TestList reached */
			sprintf(szText, "End of Testlist reached.");
			break;
		case GTL_MSGERROR_SOCKET_CREATE:
			/* Couldn't create socket */
			sprintf(szText, "Couldn't create client socket (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_BIND:
			/* Couldn't bind socket */
			sprintf(szText, "Couldn't bind client socket (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_READBACK:
			/* Couldn't read back socket informations */
			sprintf(szText, "Couldn't read back socket informations (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_HOSTNAME:
			/* Couldn't retrieve IP address for GTM server */
            sprintf(szText, "Couldn't retrieve IP address of GTM server %s.", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_OPTION:
			/* Couldn't set socket options */
			sprintf(szText, "Couldn't set socket options (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_CONNECT:
			/* Couldn't connect to server socket */
			sprintf(szText, "Error connecting to server.");
			break;
		case GTL_MSGERROR_SOCKET_INIT:
			/* Couldn't initialize socket system */
			sprintf(szText, "Couldn't initialize socket system (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_CLEANUP:
			/* Couldn't cleanup socket system */
			sprintf(szText, "Couldn't cleanup socket system (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_SEND:
			/* Couldn't send request to server */
			sprintf(szText, "Error sending request to GTM Server (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_REC:
			/* Couldn't receive request from server */
			sprintf(szText, "Error receiving reply from GTM Server (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_MAF:
			/* Memory allocation failure */
			sprintf(szText, "Memory allocation failure (Socket).");
			break;
		case GTL_MSGERROR_SOCKET_CBUF:
			/* Error creating network buffer */
			sprintf(szText, "Couldn't create network buffer.");
			break;
		case GTL_MSGERROR_SOCKET_RBUF:
			/* Error reading network buffer */
			sprintf(szText, "Error reading network buffer.");
			break;
		case GTL_MSGERROR_SOCKET_DOWN:
			/* Socket is not up */
			sprintf(szText, "Socket is down.");
			break;
		case GTL_MSGERROR_SOCKET_CONFIG:
			/* Error during socket configuration */
			sprintf(szText, "Error during socket configuration (%s).", szInfo);
			break;
		case GTL_MSGERROR_SOCKET_COMM:
			/* Error during socket communication */
			sprintf(szText, "Error during socket communication.");
			break;
		case GTL_MSGERROR_SOCKET_TRANSFER_INIT:
			/* Error during INIT */
			sprintf(szText, "Error during Init with GTM server.");
			break;
		case GTL_MSGERROR_SOCKET_TRANSFER_RECONNECT:
			/* Error during RFR request */
			sprintf(szText, "Error during Request For Reconnect.");
			break;
		case GTL_MSGERROR_SOCKET_TRANSFER_HBEAT:
			/* Error during HEARTBEAT */
			sprintf(szText, "Error during Heartbeat to GTM Server.");
			break;
		case GTL_MSGERROR_GNET_ERROR:
			/* Error in Gnet module: the error message is already formatted in szInfo */
			sprintf(szText, "%s", szInfo);
			break;

  }
  
	/* Complete Message */
	strcat(szMessage, szText);
  
	/* Send message to destination */
	if(nMode == GTL_MSG_TOBUFFER)
		strcpy(szDestBuffer, szMessage);
	else
	{
		strcat(szMessage, "\n");
        // 7103
        //tester_Printf(szMessage, 0);
        if (gtl_push_back_new_message(0, szMessage, nErrorCode)<0)
            GTL_LOG(3, "gtl_push_back_new_message failed !", 0)
        GTL_LOG(4, "Error message: '%s'", szMessage);
        if (szInfo)
            GTL_LOG(4, szInfo, 0);
	}
  
    return 0;
}

/*======================================================================================*/
/* Description  : print a Warning message to the screen.                                */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nWarningCode                                                                 */
/*         code specifying warning message (see gtl_message.h)                          */
/*                                                                                      */
/*     char* szCustomString                                                             */
/*         string to add to the Warning message                                         */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_Warning(nWarningCode, szCustomString)
int nWarningCode;
char *szCustomString;
{
    char szString[GTL_MSG_MAXSIZE]="", szText[GTL_MSG_MAXSIZE]="";

	/* Build warning string */
	strcpy(szString, GTL_WARN_LABEL);
	switch(nWarningCode)
	{
		case GTL_MSGWARN_MAIN_BADBIN:
			/* Bad Bin */
			sprintf(szText, "Device Failed %s.", szCustomString);
			strcat(szString, szText);
			break;
		case GTL_MSGWARN_MAIN_INVCMD:
			/* Invalid GTL command */
			sprintf(szText, "Invalid GTL command: %s.", szCustomString);
			strcat(szString, szText);
			break;
		case GTL_MSGWARN_TESTLIST_NOTEXEC:
			/* Test not executed during last run */
			sprintf(szText, "Test not executed %s.", szCustomString);
			strcat(szString, szText);
			break;
        case GTL_MSGWARN_SQLITE_CANT_CHECK_CONTEXT:
            /* Couldn't check context for resume/retest */
            sprintf(szText, "Couldn't check context for resume/retest. Let's consider the context is different.");
            strcat(szString, szText);
            break;
        default:
			break;
	}
	
	/* Print message	 */
	strcat(szString, "\n");
    // 7103
    //tester_Printf(szString, 0);
    if (gtl_push_back_new_message(0, szString, nWarningCode)<0)
        GTL_LOG(3, "gtl_push_back_new_message failed",0)
    GTL_LOG(4, szString, 0);
	
	return;
}

/*======================================================================================*/
/* Description  : print a debug string to screen.                                       */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nLabel                                                                       */
/*         1   : message label should be used                                           */
/*         != 1: no message label                                                       */
/*                                                                                      */
/*     int nLineFeed                                                                    */
/*         1   : add LineFeed at the end of the message                                 */
/*         != 1: no LineFeed                                                            */
/*                                                                                      */
/*     int nInfoCode                                                                    */
/*         code specifying info message (see gtl_message.h)                             */
/*                                                                                      */
/*     char* szDbgString                                                                */
/*         debug text                                                                   */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_Debug(nLabel, nLineFeed, szDbgString)
int		nLabel;
int		nLineFeed;
char	*szDbgString;
{
	char	szString[GTL_MSG_MAXSIZE];
	
	/* Check if Debug mode enabled  */
	if(gtl_nDebugMsg_ON == 0)
		return;

	/* Build debug string */
	if(nLabel)
		sprintf(szString, "%s%s", GTL_DEBUG_LABEL, szDbgString);
	else
		sprintf(szString, "%s", szDbgString);
	if(nLineFeed)
		strcat(szString, "\n");
	
    // 7103
    //tester_Printf(szString, 0);
    if (gtl_push_back_new_message(1, szString, -1)<0)
        GTL_LOG(3, "gtl_push_back_new_message failed", 0);
    GTL_LOG(7, szString, 0);

	return;
}

/*======================================================================================*/
/* Description  : write GTL state into a string.										*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     char* szString                                                                   */
/*         buffer to receive the state info                                             */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_GetGTLStateString(szString)
char *szString;
{
    switch(gtl_stGlobalInfo.nCurrentLibState)
	{
        case GTL_STATE_NOT_INITIALIZED:
            strcpy(szString, "GTL state is: Not initialized.");
			break;
        case GTL_STATE_ENABLED:
            strcpy(szString, "GTL state is: Enabled.");
            break;
        case GTL_STATE_DISABLED:
			sprintf(szString, "GTL state is: Disabled (%s).", gtl_stGlobalInfo.szDisabledRootcause);
			break;
        case GTL_STATE_OFFLINE:
            sprintf(szString, "GTL state is: Offline (no GTM connection).");
			break;
		default:
			strcpy(szString, "GTL state is: Unknown.");
			break;
	}
	
	return;
}

/*======================================================================================*/
/* Description  : display GTL help.														*/
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_msg_DisplayHelp()
{
    char *szBuffer = (char*)malloc((50*GTC_MAX_STRING)*sizeof(char));

	strcpy(szBuffer, "\n");
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "**** GTL help ******************************************************************\n");
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "Usage: run -args <gtl_option>\n");
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "Valid options:                Meanning:\n");
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "    -gtl_help                 Display this message\n");
	/* For all users */
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "    -gtl_info                 Show current GTL state and version\n");
#if 0 // Deprecated for now since GTL V3.6
    strcat(szBuffer, GTL_INFO_LABEL);
    strcat(szBuffer, "    -gtl_reset                Reset GTL\n");
    strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "    -gtl_disabled             Disable GTL\n");
#endif
    strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "    -gtl_quieton/off          Display/don't display GTL messages to screen\n");
	strcat(szBuffer, GTL_INFO_LABEL);
	strcat(szBuffer, "********************************************************************************\n\n");

    //tester_Printf(szBuffer, 0);
    // 7103
    // Bernard : Let s log (GTL_LOG) that big string. Do we now have to send a GTL message for that ?
    GTL_LOG(6, szBuffer, 0);
	
    free(szBuffer);
}

