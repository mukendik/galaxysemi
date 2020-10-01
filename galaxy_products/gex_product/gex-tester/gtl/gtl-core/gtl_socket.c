/****************************************************************************************/
/* Copyright Galaxy                                                                     */
/* This computer program is protected by copyright law                                  */
/* and international treaties. Unauthorized reproduction or                             */
/* distribution of this program, or any portion of it,may                               */
/* result in severe civil and criminal penalties, and will be                           */
/* prosecuted to the maximum extent possible under the law.                             */
/****************************************************************************************/
/*                                                                                      */
/* Module: gtl_socket.c                                                                 */
/*                                                                                      */
/* Socket communication functions to communicate with GTM (Galaxy Tester Monitor).      */
/*                                                                                      */
/****************************************************************************************/
/*                                                                                      */
/* Notes:                                                                               */
/*                                                                                      */
/****************************************************************************************/

#define _GTL_SOCKET_MODULE_

/*======================================================================================*/
/* Includes                                                                             */
/*======================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined __APPLE__&__MACH__
    #include <malloc/malloc.h>
#else
    #include <malloc.h>
#endif

#if defined unix || __MACH__
    #include <sys/types.h>
    #include <sys/socket.h>
    //#include <sys/sockio.h>
    //#include <sys/filio.h>

    #if (defined __APPLE__&__MACH__ || (defined(__CYGWIN__) && !defined(__MINGW32__)))
    #include <sys/ioctl.h>
    #else
    #include <asm/ioctls.h>
    #endif
    #include <netinet/in.h>
    #include <netdb.h>
    #include <sys/time.h>
    #include <errno.h>
    #include <unistd.h>
#else
    #include <winsock.h>
    #include <sys/types.h>
    #include <process.h>
#endif

#include <gstdl_utils_c.h>
#include <gstdl_neterror_c.h>
#include <gstdl_netbuffer_c.h>
#include <gstdl_netmessage_c.h>
#include <gtc_netmessage.h>

#include "gtl_socket.h"
#include "gtl_message.h"
#include "gtl_constants.h"
#include "gtl_error.h"
#include "gtl_core.h"
#include "gtl_profile.h"

/*======================================================================================*/
/* Externs                                                                              */
/*======================================================================================*/
/* From gtl_main.c */
extern GTL_Server		gtl_stGTMServer;				/* GTM server settings */
extern GTL_GlobalInfo   gtl_stGlobalInfo;				/* GTL Global information */

/*======================================================================================*/
/* Public declarations                                                                  */
/*======================================================================================*/
/* See gtl_socket.h */
#define GNET_TRACE_MSG_SIZE		256

/*======================================================================================*/
/* Private declarations                                                                 */
/*======================================================================================*/
/* Variables */
SOCK_SOCKETTYPE		hSockHandle = 0;
int					nSocketIsBlocking = 1;
int					nSocketUP = 0;
int					nLastSocketError = 0;
PT_GTL_Server		pstCurrentServer = NULL;

/* Functions */
int		sock_Init();
int		sock_Cleanup();
int		sock_Create();
int		sock_Connect();
void	sock_FormatIPAddress();
int		sock_SetBlocking();
int		sock_SetNonBlocking();
int		sock_SetOptions();
void	sock_SetRecvTimeout();
void	sock_SetSendTimeout();
int		sock_ReceiveHeader();
int		sock_ReceiveData();
int		sock_SendBuffer();
int		sock_ReceiveBuffer();
int		sock_IsValidHeader();
int		sock_GetLastError();
void	sock_Trace();
int     gtl_InitSocketTrace();

/*======================================================================================*/
/* Public CODE                                                                          */
/*======================================================================================*/

/* Start socket system */
/* o Connection and communication modes are set through: */
/*      gtl_set("socket_connection_mode"): default is "blocking" */
/*      gtl_set("socket_communication_mode"): default is "non-blocking" */
/* o Timeouts are set through: */
/*      gtl_set("socket_connection_timeout"): default is 60sec */
/*      gtl_set("socket_receive_timeout"): default is 10sec */
/*      gtl_set("socket_send_timeout"): default is 10sec */
/* See "http://stackoverflow.com/questions/2597608/c-socket-connection-timeout" for detils on connection timeout */
int gtl_sock_Start(int nSilent, unsigned lNumOfServersToTry)
{
    int nConnected=0, nStatus=0;
    unsigned lNumOfServersTried=0;
    struct sockaddr_in stServerAddress;
    char szMessage[GNET_ERR_MSG_SIZE]="";
    char szInfo[GTL_MSG_MAXSIZE]="";
    PT_GTL_Server pstServer=0;

    GTL_LOG(6, "gtl_sock_Start trying %d servers (0=all)...", lNumOfServersToTry)

	/* Set hooks to read/write message data */
	gnm_SetReadBufferHook(gtcnm_ReadBufferData);
	gnm_SetAddDataToBufferHook(gtcnm_CreateBufferData);
	
	/* Initialize socket system */
    if(!sock_Init())
	{
		if(!nSilent)
		{
            gtl_sock_GetLastSocketError(szMessage);
            gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_INIT, 0, szMessage);
		}

		return GTL_SOCK_ERR_INIT;
	}
  
	/* Connect to one of the GTM servers */
	pstServer = &gtl_stGTMServer;
	nConnected = 0;

    while(!nConnected && pstServer)
	{
        if (lNumOfServersToTry!=0 && lNumOfServersTried>=lNumOfServersToTry)
            break;

        nStatus = GTC_STATUS_OK;

		/* Create socket */
		if(!sock_Create())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CREATE, 0, szMessage);
			}
			sock_Cleanup();

            return GTL_SOCK_ERR_CREATE;
		}
  
		/* Blocking/Non-blocking communications */
        if(gtl_stGlobalInfo.mSocketConnetionMode == GTL_SOCK_NON_BLOCKING)
		{
			/* Set socket to non-blocking mode */
			if(!sock_SetNonBlocking())
			{
				if(!nSilent)
				{
                    gtl_sock_GetLastSocketError(szMessage);
                    gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
				}
				sock_Cleanup();
				return GTL_SOCK_ERR_OPTION;
			}
		}
		else
		{
			/* Set socket to blocking mode */
			if(!sock_SetBlocking())
			{
				if(!nSilent)
				{
                    gtl_sock_GetLastSocketError(szMessage);
                    gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
				}
				sock_Cleanup();
				return GTL_SOCK_ERR_CONFIG;
			}

			/* Set socket options (timeouts) */
            if(!sock_SetOptions(gtl_stGlobalInfo.mSocketReceiveTimeout, gtl_stGlobalInfo.mSocketConnectionTimeout))
			{
				if(!nSilent)
				{
                    gtl_sock_GetLastSocketError(szMessage);
                    gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
				}
				sock_Cleanup();
				return GTL_SOCK_ERR_OPTION;
			}
		}

		/* Format IP Address */
        GTL_LOG(5, "Trying server %s...", pstServer->szServerIP);
		sock_FormatIPAddress(pstServer, &stServerAddress);

        /* Connect to GTM server socket */
		if(!nSilent)
        {
            sprintf(szInfo, "%s (IP=%s, port=%d)", pstServer->szServerName, pstServer->szServerIP,
                pstServer->uiServerPort);
            gtl_msg_Info(1, 0, GTL_MSGINFO_CONNECTING, szInfo);
        }
        if(sock_Connect((struct sockaddr *)&stServerAddress))
		{
            /* Needed, because under Solaris, socket connects even if daemon not running on the server ! */
            /* We also use this to check on the GTM side if max nb of allowed */
            /* connections is not reached. */
            nStatus = gtl_sock_CheckConnection();
            if(nStatus == GTC_STATUS_OK)
			{
                if(!nSilent)
                {
                    sprintf(szInfo, "(on server adress %s port %u)", pstServer->szServerIP, pstServer->uiServerPort);
                    gtl_msg_Info(0, 1, GTL_MSGINFO_CONNECT_OK, szInfo);
                }
				nConnected = 1;
			}
			else
            {
                if (shutdown(hSockHandle, SOCK_SHUTDOWN_MODE)!=0)
                    GTL_LOG(4, "Cant shutdown socket correctly : errno=%d", errno);
            }
		}

		if(!nConnected)
		{
			if(!nSilent)
            {
                if(nStatus == GTC_STATUS_MAXCONNECTIONS)
                    strcpy(szMessage, "Maximum number of GTM connections reached");
                else if(nStatus == GTC_STATUS_VERSIONMISMATCH)
                    strcpy(szMessage, "GTL/GTM version mismatch");
                else if(nStatus == GTC_STATUS_NO_GTM_LIC)
                    strcpy(szMessage, "No GTM License");
                else if(nStatus == GTC_STATUS_NO_GTL_LIC)
                    strcpy(szMessage, "No GTL License");
                else
                    gtl_sock_GetLastSocketError(szMessage);
                sprintf(szInfo, "(port %u): %s", pstServer->uiServerPort, szMessage);
                gtl_msg_Info(0, 1, GTL_MSGINFO_CONNECT_NOK, szInfo);
			}
            #if 0
                /* Shutdown function produces a timeout on unix if Socket not connected */
                shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
            #endif
			SOCK_CLOSE(hSockHandle);
			pstServer = pstServer->pstNextServer;
		}

        lNumOfServersTried++;
	}

	if(!nConnected)
	{
		if(!nSilent)
			gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONNECT, 0, "");
		sock_Cleanup();
        if (nStatus == GTC_STATUS_MAXCONNECTIONS)
            return GTL_SOCK_ERR_CONNECT_MAXCONN;
        else if(nStatus == GTC_STATUS_VERSIONMISMATCH)
            return GTL_SOCK_ERR_VERSIONMISMATCH;
        else if(nStatus == GTC_STATUS_NO_GTM_LIC)
            return GTL_SOCK_ERR_NO_GTM_LIC;
        else if(nStatus == GTC_STATUS_NO_GTL_LIC)
            return GTL_SOCK_ERR_NO_GTL_LIC;
		return GTL_SOCK_ERR_CONNECT;
	}

	/* Blocking/Non-blocking communications */
    if(gtl_stGlobalInfo.mSocketCommunicationMode == GTL_SOCK_NON_BLOCKING)
	{
        GTL_LOG(5, "Set socket to non blocking mode...", 0)
		/* Set socket to non-blocking mode */
		if(!sock_SetNonBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}
	else
	{
        GTL_LOG(5, "Set to blocking mode...", 0)
		/* Set socket to blocking mode */
		if(!sock_SetBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_CONFIG;
		}

		/* Set socket options (timeouts) */
        if(!sock_SetOptions(gtl_stGlobalInfo.mSocketReceiveTimeout, gtl_stGlobalInfo.mSocketSendTimeout))
        {
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}

	pstCurrentServer = pstServer;
	nSocketUP = 1;

    return GTL_SOCK_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : create/Re-connect client socket to communicate with GTM server.       */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nSilent                                                                      */
/*         1   : don't display connection messages                                      */
/*         != 1: display connection messages                                            */
/*                                                                                      */
/* Return Codes : GTL_SOCK_ERR_OKAY if successful, GTL_SOCK_ERR_XXX error code else     */
/*======================================================================================*/
/*======================================================================================*/
/* The difference with the sock_Start() function is that we don't scan through all GTM  */
/* servers. We only	use the current server (pstCurrentServer).							*/
/*======================================================================================*/
int gtl_sock_Restart(int nSilent)
{
    int 				nConnected, nStatus;
	struct sockaddr_in	stServerAddress;
    char				szMessage[GNET_ERR_MSG_SIZE]="";
    char                szInfo[GTL_MSG_MAXSIZE]="";

	/* Set hooks to read/write message data */
	gnm_SetReadBufferHook(gtcnm_ReadBufferData);
	gnm_SetAddDataToBufferHook(gtcnm_CreateBufferData);
	
	/* Initialize socket system */
	if(!sock_Init())
	{
		if(!nSilent)
		{
            gtl_sock_GetLastSocketError(szMessage);
            gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_INIT, 0, szMessage);
		}
		return GTL_SOCK_ERR_INIT;
	}
  
	/* Connect to current Galaxy Production server */
	nConnected = 0;
    nStatus = GTC_STATUS_OK;

	if(!pstCurrentServer)
	{
		if(!nSilent)
			gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONNECT, 0, "");
		return GTL_SOCK_ERR_CONNECT;
	}

	/* Create socket */
	if(!sock_Create())
	{
		if(!nSilent)
		{
            gtl_sock_GetLastSocketError(szMessage);
            gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CREATE, 0, szMessage);
		}
		sock_Cleanup();
		return GTL_SOCK_ERR_CREATE;
	}
  
	/* Blocking/Non-blocking communications */
    if(gtl_stGlobalInfo.mSocketConnetionMode == GTL_SOCK_NON_BLOCKING)
	{
		/* Set socket to non-blocking mode */
		if(!sock_SetNonBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}
	else
	{
		/* Set socket to blocking mode */
		if(!sock_SetBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_CONFIG;
		}

		/* Set socket options (timeouts) */
        if(!sock_SetOptions(gtl_stGlobalInfo.mSocketReceiveTimeout, gtl_stGlobalInfo.mSocketConnectionTimeout))
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}

	/* Format IP Address */
	sock_FormatIPAddress(pstCurrentServer, &stServerAddress);

	/* Connect to Galaxy server socket */
	if(!nSilent)
    {
        sprintf(szInfo, "%s (IP=%s, port=%d)",
        pstCurrentServer->szServerName, pstCurrentServer->szServerIP,
        pstCurrentServer->uiServerPort);
        gtl_msg_Info(1, 0, GTL_MSGINFO_CONNECTING, szInfo);
    }
	if(sock_Connect((struct sockaddr *)&stServerAddress))
	{
        nStatus = gtl_sock_CheckConnection();
        if(nStatus == GTC_STATUS_OK) /* Needed, because under Solaris, socket connects even if daemon not running on the server ! */
		{
			if(!nSilent)
            {
                sprintf(szInfo, "(port %u)", pstCurrentServer->uiServerPort);
                gtl_msg_Info(0, 1, GTL_MSGINFO_CONNECT_OK, szInfo);
            }
            nConnected = 1;
		}
		else
			shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
	}

	if(!nConnected)
	{
		if(!nSilent)
        {
            if(nStatus == GTC_STATUS_MAXCONNECTIONS)
                strcpy(szMessage, "Maximum number of GTM connections reached");
            else
                gtl_sock_GetLastSocketError(szMessage);
            gtl_sock_GetLastSocketError(szMessage);
            sprintf(szInfo, "(port %u): %s", pstCurrentServer->uiServerPort, szMessage);
            gtl_msg_Info(0, 1, GTL_MSGINFO_CONNECT_NOK, szInfo);
            gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONNECT, 0, "");
        }
#if 0
		/* Shutdown function produces a timeout on unix if Socket not connected */
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
#endif
		SOCK_CLOSE(hSockHandle);
		sock_Cleanup();
		return GTL_SOCK_ERR_CONNECT;
	}

	/* Blocking/Non-blocking communications */
    if(gtl_stGlobalInfo.mSocketCommunicationMode == GTL_SOCK_NON_BLOCKING)
	{
		/* Set socket to non-blocking mode */
		if(!sock_SetNonBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}
	else
	{
		/* Set socket to blocking mode */
		if(!sock_SetBlocking())
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_CONFIG;
		}

		/* Set socket options (timeouts) */
        if(!sock_SetOptions(gtl_stGlobalInfo.mSocketReceiveTimeout, gtl_stGlobalInfo.mSocketSendTimeout))
		{
			if(!nSilent)
			{
                gtl_sock_GetLastSocketError(szMessage);
                gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_CONFIG, 0, szMessage);
			}
			sock_Cleanup();
			return GTL_SOCK_ERR_OPTION;
		}
	}
	nSocketUP = 1;

	return GTL_SOCK_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : destroy client socket.                                                */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_sock_Stop()
{
	if(nSocketUP)
	{
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
		sock_Cleanup();
		nSocketUP = 0;
	}
}

/*======================================================================================*/
/* Description  : check connection with GTM server.                                     */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes : GTC_STATUS_OK on success, GTC_STATUS_XXX else                         */
/*======================================================================================*/
int gtl_sock_CheckConnection()
{
    char szTimestamp[UT_MAX_TIMESTAMP_LEN]="";
    int nStatus=0;
    GTL_SOCK_MSG stQueryMessage, stReplyMessage;
    GNM_Q_CFC stCFC_Query;
    PT_GNM_R_CFC pstCFC_Reply=0;

    GTL_LOG(7, "Checking socket connection...", 0);

	/* Fill request structure */
    stCFC_Query.mTimeStamp = ut_GetFullTextTimeStamp(szTimestamp);
    stCFC_Query.mGtlVersionMajor = GTL_VERSION_MAJOR;
    stCFC_Query.mGtlVersionMinor = GTL_VERSION_MINOR;

	/* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_Q_CFC;
    stQueryMessage.mMsgStruct = (void *)&stCFC_Query;

	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, &stReplyMessage, GNM_TYPE_R_CFC);
	if(nStatus != GNET_ERR_OKAY)
	{
        GTL_LOG(4, "Error checking socket connection.", 0);
        return GTC_STATUS_SOCKET;
	}

	/* Check reply */
    pstCFC_Reply = (PT_GNM_R_CFC)(stReplyMessage.mMsgStruct);
    nStatus = pstCFC_Reply->mStatus;

	/* Free stuff */
	gtcnm_FreeStruct(GNM_TYPE_R_CFC, pstCFC_Reply);
	free(pstCFC_Reply);

    return nStatus;
}

/*======================================================================================*/
/* Description  : send Heartbeat to Galaxy server.	                                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     char* szErrMessage				                                                */
/*         buffer to receive error message                                              */
/*                                                                                      */
/* Return Codes : 1 on success, 0 else                                                  */
/*======================================================================================*/
int gtl_sock_SendHeartbeat(int nSilent)
{
    int 			nStatus=0;
	GTL_SOCK_MSG	stQueryMessage;
	GNM_HEARTBEAT	stHBEAT_Query;

	/* Check if socket is UP */
	if(!nSocketUP)
	{
		if(!nSilent)
			gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_DOWN, 0, "");
		return GTL_SOCK_ERR_DOWN;
	}
	  
	/* Fill request structure */
    stHBEAT_Query.mTimeStamp = (long)ut_GetCompactTimeStamp(NULL);

	/* Socket message structure */
    stQueryMessage.mType = GNM_TYPE_HEARTBEAT;
    stQueryMessage.mMsgStruct = (void *)&stHBEAT_Query;

	/* Send request to server */
    nStatus = gtl_sock_SendMessage(&stQueryMessage, NULL, 0);
	if(nStatus != GNET_ERR_OKAY)
	{
		if(!nSilent)
		{
			gtl_err_Display(GTL_MODULE_GNET, nStatus, nLastSocketError, "");
			gtl_err_Display(GTL_MODULE_SOCKET, GTL_SOCK_ERR_TRANSFER_HBEAT, 0, "");
		}
		gtl_sock_Stop();
		return GTL_SOCK_ERR_COMM;
	}

	return GTL_SOCK_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : get error message for last socket error.	                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     char* szErrMessage				                                                */
/*         buffer to receive error message                                              */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void gtl_sock_GetLastSocketError(szErrMessage)
char *szErrMessage;
{
	gnet_GetLastSocketError(nLastSocketError, szErrMessage);
}

/*======================================================================================*/
/* Description  : send message to server.                                               */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     PT_GTL_SOCK_MSG ptMsgToSend                                                      */
/*         pointer on GTL_SOCK_MSG to send                                              */
/*                                                                                      */
/*     PT_GTL_SOCK_MSG ptMsgToReceive                                                   */
/*         pointer on SOCK_MSG to receive (NULL if nothing expected)                    */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int gtl_sock_SendMessage(PT_GTL_SOCK_MSG ptMsgToSend, PT_GTL_SOCK_MSG ptMsgToReceive, unsigned short ExpectedRcvMsgType)
{
    int lStatus=0;
    unsigned int lBufferSize=0;
    char *lBuffer=NULL;

    /* Should we request a GTM acknowledge? */
    ptMsgToSend->mAckRequested = 0;
    if(ptMsgToReceive == NULL)
    {
        switch(ptMsgToSend->mType)
        {
            case GNM_TYPE_PRODINFO:
            case GNM_TYPE_RESULTS:
                if(gtl_stGlobalInfo.mGtmCommunicationMode==GTL_GTMCOMM_SYNCHRONOUS)
                    ptMsgToSend->mAckRequested = 1;
                break;

            default:
                break;
        }
    }

    /* Prepare buffer with header and data */
    lStatus = gnm_CreateBufferHeaderData(ptMsgToSend->mType, ptMsgToSend->mAckRequested, ptMsgToSend->mMsgStruct,
                                         &lBuffer, &lBufferSize);
    if(lStatus != GNET_ERR_OKAY)
	{
        return lStatus;
	}
	
	/* Send Data */
    sock_Trace(GTL_TRACE_SEND_DATA, NULL, 0);
    lStatus = sock_SendBuffer(lBuffer, lBufferSize);
    if(lStatus != GNET_ERR_OKAY)
	{
        free(lBuffer);
        return lStatus;
	}

    free(lBuffer);

    /* Message sent */
    if((ptMsgToSend->mType != GNM_TYPE_RESULTS) && (ptMsgToSend->mType != GNM_TYPE_ACK))
        GTL_LOG(7, "Sent message (type %d, %s, %s) to socket...",
            ptMsgToSend->mType, ptMsgToSend->mAckRequested?"ack. requested":"no ack. requested",
            ptMsgToReceive?"reply message requested":"no reply message requested");

    /* Is a response message required ? */
    if(ptMsgToReceive != NULL)
	{
        do
        {
            lStatus = gtl_sock_ReceiveMessage(ptMsgToReceive);
            // Check status
            if(lStatus != GNET_ERR_OKAY)
                return lStatus;
            // Check if Ack
            if((ptMsgToReceive->mType) == GNM_TYPE_ACK)
                (gtl_stGlobalInfo.mWaitForGtmAck)--;
        }
        while((ptMsgToReceive->mType) == GNM_TYPE_ACK);

        // Check if received message is the expected one
        if(ptMsgToReceive->mType != ExpectedRcvMsgType)
        {
            GTL_LOG(3, "Received wrong reply: expected=%d, received=%d",
                    ExpectedRcvMsgType, ptMsgToReceive->mType);
            return GNET_ERR_RWR;
        }
	}
    /* If no response  required, check if we should receive an acknowledge */
    else if(ptMsgToSend->mAckRequested)
    {
        /* Set a flag to tell GTL to synchronize on next gtl_beginjob() */
        (gtl_stGlobalInfo.mWaitForGtmAck)++;
    }
	
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : check if message from the server available.                           */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     PT_GTL_SOCK_MSG ptMessage                                                        */
/*         pointer on GTL_SOCK_MSG to store message if any                              */
/*                                                                                      */
/*     int *pnMessageReceived                                                           */
/*         set to 0 if no message received, != 0  if a message was received             */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int gtl_sock_CheckForMessage(ptMessage, pnMessageReceived) 
PT_GTL_SOCK_MSG ptMessage;
int				*pnMessageReceived;
{
    fd_set fds_Check_Read;
    SOCK_TIMEVAL tvTimeout;
    int	nStatus;

	/* No message received yet */
	*pnMessageReceived = 0;

	/* Check if data available on socket */
	tvTimeout.tv_sec = 0L;
	tvTimeout.tv_usec = 0L;
	/* Check if socket ready for reading */
	FD_ZERO(&fds_Check_Read);
	FD_SET(hSockHandle, &fds_Check_Read);
    nStatus = select(hSockHandle+1, &fds_Check_Read, NULL, NULL, &tvTimeout);
    if(nStatus != 1)
        return GNET_ERR_OKAY;

	/* Data available, read the message */
    nStatus = gtl_sock_ReceiveMessage(ptMessage);
	if(nStatus == GNET_ERR_OKAY)
		*pnMessageReceived = 1;

	return nStatus;
}

/*======================================================================================*/
/* Description  : receive a message from the server.                                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     PT_GTL_SOCK_MSG ptMessage                                                        */
/*         pointer on GTL_SOCK_MSG to receive                                           */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int gtl_sock_ReceiveMessage(ptMessage) 
PT_GTL_SOCK_MSG ptMessage;
{
	int 		nStatus;
	GNM_HEADER	stHeader;
	void		*ptData=NULL;

	/* Receive Header */
	sock_Trace(GTL_TRACE_RECV_HEADER, NULL, 0);
	nStatus = sock_ReceiveHeader(&stHeader);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}

	/* Receive Data */
	sock_Trace(GTL_TRACE_RECV_DATA, NULL, 0);
	nStatus = sock_ReceiveData(&ptData, &stHeader);
	if(nStatus != GNET_ERR_OKAY)
	{
		return nStatus;
	}

	/* Fill Message structure */
    ptMessage->mType = stHeader.mMessageType;
    ptMessage->mMsgStruct = ptData;

    if((ptMessage->mType != GNM_TYPE_RESULTS) && (ptMessage->mType != GNM_TYPE_ACK))
        GTL_LOG(7, "Received message (type %d) from socket...", ptMessage->mType);

    return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Private CODE                                                                         */
/*======================================================================================*/
/*======================================================================================*/
/* Description  : initialize socket system if needed.		                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't initialize socket system											*/
/*======================================================================================*/
int sock_Init()
{
    #if !defined unix && !defined __MACH__
        WSADATA 	stWinSockData;
        WORD		wWinSockVersion;
    #endif

    GTL_LOG(7, "Initializing socket...", 0);

#if !defined unix && !defined __MACH__
	wWinSockVersion = MAKEWORD(1, 1); 

	if(WSAStartup(wWinSockVersion, &stWinSockData) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
        GTL_LOG(4, "Error initializing socket: error nb %d", nLastSocketError);
		return 0;
	}
#endif

    return 1;
}

/*======================================================================================*/
/* Description  : cleanup socket system if needed.			                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't cleanup socket system												*/
/*======================================================================================*/
int sock_Cleanup()
{
    #if !defined unix && !defined __MACH__
        if(WSACleanup() != 0)
            return 0;
    #endif

	return 1;
}

/*======================================================================================*/
/* Description  : set socket to non-blocking mode.			                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't set socket mode														*/
/*======================================================================================*/
int sock_SetNonBlocking()
{
	unsigned long ulParam = 1;

	//return 1;

    GTL_LOG(7, "Set socket to non-blocking mode...", 0);

	if(SOCK_IOCTL(hSockHandle, FIONBIO, &ulParam) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error setting socket to non-blocking mode: error nb %d", nLastSocketError);
		return 0;
	}

	nSocketIsBlocking = 0;
	return 1;
}

/*======================================================================================*/
/* Description  : set socket to blocking mode.				                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't set socket mode														*/
/*======================================================================================*/
int sock_SetBlocking()
{
	unsigned long ulParam = 0;

	//return 1;

    GTL_LOG(7, "Set socket to blocking mode...", 0);

	if(SOCK_IOCTL(hSockHandle, FIONBIO, &ulParam) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error setting socket to blocking mode: error nb %d", nLastSocketError);
        return 0;
	}

	nSocketIsBlocking = 1;
	return 1;
}

/*======================================================================================*/
/* Description  : set socket send/receive timeouts.			                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't set socket options													*/
/*======================================================================================*/
/*======================================================================================*/
/* NOTE: timeout options are not supported in winsock1, nor in Solaris					*/
/*======================================================================================*/
int sock_SetOptions(nRcvTimeout, nSndTimeout)
int nRcvTimeout, nSndTimeout;
{
    return 1;

    GTL_LOG(7, "Setting socket options...", 0);

    /* Set receive timeout */
	if(setsockopt(hSockHandle, SOL_SOCKET, SO_RCVTIMEO, (char *)&nRcvTimeout, sizeof(int)) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error setting socket options: error nb %d", nLastSocketError);
		return 0;
	}
  
	/* Set send timeout */
	if(setsockopt(hSockHandle, SOL_SOCKET, SO_SNDTIMEO, (char *)&nSndTimeout, sizeof(int)) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error setting socket options: error nb %d", nLastSocketError);
        return 0;
	}
  
	return 1;
}

/*======================================================================================*/
/* Description  : set socket receive timeout.				                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't set socket options													*/
/*======================================================================================*/
void sock_SetRecvTimeout()
{
	int nTimeout;

	/* Set receive timeout */
	nTimeout = GTL_SOCK_RCV_TIMEOUT;
	setsockopt(hSockHandle, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeout, sizeof(int));
	return;
}
  
/*======================================================================================*/
/* Description  : set socket send timeout.					                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't set socket options													*/
/*======================================================================================*/
void sock_SetSendTimeout()
{
	int nTimeout;

	/* Set send timeout */
    nTimeout = gtl_stGlobalInfo.mSocketSendTimeout;
	setsockopt(hSockHandle, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeout, sizeof(int));
	return;
}

/*======================================================================================*/
/* Description  : create/bind a socket.						                            */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't create/bind socket													*/
/*======================================================================================*/
int sock_Create()
{
    struct sockaddr_in	stSocketName;
    struct hostent		*hClient=0;
    char				szHostName[GTL_SOCK_MAX_HOSTNAME]="";
    int 				nOption=0;
#if 0
    unsigned long		ul_ClientAddress;
#endif

    GTL_LOG(7, "Creating socket...", 0);

	nSocketIsBlocking = 1;

	/* Create socket */
	if((hSockHandle = socket(GTL_CLIENT_SOCKET_FAMILY, GTL_CLIENT_SOCKET_TYPE, 0)) == (SOCK_SOCKETTYPE)INVALID_SOCKET)
	{
		nLastSocketError = sock_GetLastError(0);
        GTL_LOG(4, "Error creating socket (socket() function): error nb %d", nLastSocketError);
		return 0;
	}

	/* Set socket options */
    GTL_LOG(7, "Set socket options", 0);
    nOption = 1;
	if(setsockopt(hSockHandle, SOL_SOCKET, SO_REUSEADDR, (void *)&nOption, sizeof(int)) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error setting socket options: error nb %d", nLastSocketError);
		return 0;
	}

    /* Resolve host */
    memset((void *)&stSocketName, 0, sizeof(stSocketName));

    // BG 03/13/2012: modify structure initialization
#if 0
	gethostname(szHostName, GTL_SOCK_MAX_HOSTNAME);
	hClient = gethostbyname(szHostName);
    if (hClient == NULL)
    {
        if (h_errno == HOST_NOT_FOUND)
        {
            GTL_LOG(4, "gethostbyname: HOST_NOT_FOUND");
        }
        else
        {
            GTL_LOG(4, "gethostbyname error");
        }
    }
	memcpy(&(ul_ClientAddress), hClient->h_addr, hClient->h_length);
	stSocketName.sin_port = GTL_CLIENT_SOCKET_PORT;
	stSocketName.sin_family = GTL_CLIENT_SOCKET_FAMILY;
	stSocketName.sin_addr.s_addr = ul_ClientAddress;
	memset((void *)stSocketName.sin_zero, 0, 8);
#else

  gethostname(szHostName, GTL_SOCK_MAX_HOSTNAME);
  hClient = gethostbyname(szHostName);
  if(hClient == NULL)
  {
      shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
      SOCK_CLOSE(hSockHandle);
      GTL_LOG(4, "gethostbyname(%s) return NULL", szHostName)
      return 0;
  }
  stSocketName.sin_port = GTL_CLIENT_SOCKET_PORT;
  stSocketName.sin_family = GTL_CLIENT_SOCKET_FAMILY;
  #if defined unix || __MACH__
    stSocketName.sin_addr.s_addr = *(SOCK_IN_ADDR *) hClient->h_addr;
  #else
    stSocketName.sin_addr = *(SOCK_IN_ADDR *) hClient->h_addr;
  #endif
#endif

  /* Bind socket */
  /*
  GTL_LOG(7, "Binding socket")
  if(bind(hSockHandle, (struct sockaddr *)&stSocketName, sizeof(stSocketName)) != 0)
	{
		nLastSocketError = sock_GetLastError(0);
#if 0
		shutdown(hSockHandle, SOCK_SHUTDOWN_MODE);
#endif
		SOCK_CLOSE(hSockHandle);
        GTL_LOG(4, "Error creating socket (bind() function).")
		return 0;
	}
  */
	
	return 1;
}

/*======================================================================================*/
/* Description  : connect a socket to destination address.                              */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     struct sockaddr pstServerAddress                                                 */
/*         structure containing Internet address of server                              */
/*                                                                                      */
/* Return Codes :                                                                       */
/*     1 : no error 																	*/
/*     0 : couldn't connect the socket													*/
/*======================================================================================*/
int sock_Connect(pstServerAddress)
struct sockaddr *pstServerAddress;
{
	fd_set				fds_Check_Write;
	SOCK_TIMEVAL		tvTimeout;
    int 				nStatus=0;

    if(nSocketIsBlocking)
	{
        GTL_LOG(7, "Connecting socket to server (blocking mode)...", 0);

		/* Blocking socket */
		if(connect(hSockHandle, pstServerAddress, sizeof(*pstServerAddress)) != 0)
		{
			nLastSocketError = sock_GetLastError(0);
            GTL_LOG(4, "Error connecting socket to server (connect() function): error nb %d", nLastSocketError);
			return 0;
		}
		return 1;
	}
	else
	{
        GTL_LOG(7, "Connecting socket to server (non-blocking mode) ...", 0);

		/* Non blocking socket */
        tvTimeout.tv_sec = (long)gtl_stGlobalInfo.mSocketConnectionTimeout;
		tvTimeout.tv_usec = (long)0;

        if(connect(hSockHandle, pstServerAddress, sizeof(*pstServerAddress)) == 0)
            return 1;
		nLastSocketError = sock_GetLastError(0);
		if((nLastSocketError != WSAEWOULDBLOCK) && (nLastSocketError != WSAEINPROGRESS))
		{
            GTL_LOG(4, "Error connecting socket to server (connect() function): error nb %d", nLastSocketError);
			return 0;
		}

        GTL_LOG(7, "Checking connection status (select() function with timeout = %d sec) ...",
                gtl_stGlobalInfo.mSocketConnectionTimeout);

		/* Check if connection completed */
		FD_ZERO(&fds_Check_Write);
		FD_SET(hSockHandle, &fds_Check_Write);
		nStatus = select(hSockHandle+1, NULL, &fds_Check_Write, NULL, &tvTimeout);
		
		if(nStatus == 1)
		{
#if 0
#if defined unix && !defined __STDC__
			/* On unix systems, if physicall connection OK but server not responding, the select */
			/* function sees the socket as writeable, so check connection again to make sure		 */
			if(connect(hSockHandle, pstServerAddress, sizeof(*pstServerAddress)) == 0)
			{
				return 1;
			}
			nLastSocketError = sock_GetLastError(0);
			if((nLastSocketError == WSAEALREADY) || (nLastSocketError == WSAEISCONN))
			{
				return 1;
			}

            GTL_LOG(4, "Error connecting socket to server (connect() function): error nb %d", nLastSocketError);
			return 0;
#else
			return 1;
#endif
#else
			return 1;
#endif
		}
		
		nLastSocketError = WSAETIMEDOUT;
        GTL_LOG(4, "Error connecting socket to server (select() function): error nb %d", nLastSocketError);
		return 0;
	}
}

/*======================================================================================*/
/* Description  : format internet address.                                              */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     GTL_Server pstServer                                                             */
/*         ptr on server to use                                                         */
/*                                                                                      */
/*     struct sockaddr_in pstIPAddress                                                  */
/*         structure to receive Internet address                                        */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void sock_FormatIPAddress(pstServer, pstIPAddress)
GTL_Server			*pstServer;
struct sockaddr_in	*pstIPAddress;
{
    struct hostent	*hServer=0;
    unsigned long	ul_ServerAddress=0;

    // First try with server name
	hServer = gethostbyname(pstServer->szServerName);
	if(hServer)
		memcpy(&ul_ServerAddress, hServer->h_addr, hServer->h_length);
	else if(pstServer->ulServerIP != 0)
	{
		/* Take the address from the server configuration */
		ul_ServerAddress = htonl(pstServer->ulServerIP);
	}
	pstIPAddress->sin_addr.s_addr = ul_ServerAddress;
	pstIPAddress->sin_family = GTL_CLIENT_SOCKET_FAMILY;
	pstIPAddress->sin_port = htons((unsigned short)pstServer->uiServerPort);
    memset((void *)pstIPAddress->sin_zero, 0, 8);
}

/*======================================================================================*/
/* Description  : receive header from server.                                           */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     PT_GNM_HEADER pstHeader                                                          */
/*         ptr on Header structure                                                      */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int sock_ReceiveHeader(pstHeader)
PT_GNM_HEADER pstHeader;
{
	char	*ptBuffer=NULL;
	int		nStatus;
	
	/* Allocate memory to receive a header from the remote side */
    ptBuffer = (char*)malloc(GNM_HEADER_SIZE*sizeof(char));
	if(!ptBuffer)   return GNET_ERR_MAF;

	/* Read data from socket buffer */
	if(!sock_ReceiveBuffer((void *)ptBuffer, GNM_HEADER_SIZE))
	{
		free(ptBuffer);
		return GNET_ERR_REC_HDR;
	}

	/* Read header from received buffer */
	nStatus = gnm_ReadBufferHeader(pstHeader, (char *)ptBuffer);
	if(nStatus != GNET_ERR_OKAY)
	{
		free(ptBuffer);
		return nStatus;
	}

	if(!sock_IsValidHeader(pstHeader))
	{
		free(ptBuffer);
		return GNET_ERR_RIH;
	}

	/* No error */
	free(ptBuffer);
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : receive Data from server.                                             */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void **ptData                                                                    */
/*         ptr on pointer on void to store data structure received                      */
/*                                                                                      */
/*     PT_GNM_HEADER pstHeader                                                          */
/*         ptr on GNM_HEADER                                                            */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int sock_ReceiveData(ptptData, pstHeader)
void			**ptptData;
PT_GNM_HEADER	pstHeader;
{
	char	*ptBuffer=NULL;
	int		nStatus;

	/* Allocate memory to receive data from the remote side */
    ptBuffer = (char*)malloc(pstHeader->mMessageLength*sizeof(char));
    if(!ptBuffer)
        return GNET_ERR_MAF;

	/* Read data from socket buffer */
    if(!sock_ReceiveBuffer((void *)ptBuffer, pstHeader->mMessageLength))
	{
		free(ptBuffer);
        return GNET_ERR_REC_DATA;
	}

	/* Read data from received buffer */
    nStatus = gnm_ReadBufferData(pstHeader->mMessageType, ptptData, (char *)ptBuffer, pstHeader->mMessageLength);
	if(nStatus != GNET_ERR_OKAY)
	{
		free(ptBuffer);
		return nStatus;
	}

	/* No error */
	free(ptBuffer);
	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : send a buffer to the network.                                         */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptBuffer                                                                   */
/*         ptr on buffer to send                                                        */
/*                                                                                      */
/*     int   nBufferLength																*/
/*         nb of bytes to send											                */
/*                                                                                      */
/* Return Codes : GNET_ERR_OKAY if successful, GNET_ERR_XXX error code else             */
/*======================================================================================*/
int sock_SendBuffer(ptBuffer, nBufferLength)
void  *ptBuffer;
int   nBufferLength;
{
	int					nStatus, nBytesSent = 0;
	fd_set				fds_Check_Write;
	SOCK_TIMEVAL		tvTimeout;

	/* Trace function */
    sock_Trace(GTL_TRACE_SEND_ENTER, NULL, nBufferLength);
	
	/* Send the buffer */
	if(nSocketIsBlocking)
	{
		/* Blocking socket */
		nStatus = send(hSockHandle, ptBuffer,nBufferLength,0);
		if(nStatus == SOCKET_ERROR)
		{
			nLastSocketError = sock_GetLastError(0);
			/* Trace function */
			sock_Trace(GTL_TRACE_SEND_ERROR, NULL, 0); 
			return GNET_ERR_SND_DATA;
		}

		if(nStatus != nBufferLength)
			return GNET_ERR_SND_DATA;
	}
	else
	{
		/* Non blocking socket */
		while(nBytesSent != nBufferLength)
		{
			/* Check if socket ready for writing */
            tvTimeout.tv_sec = (long)gtl_stGlobalInfo.mSocketSendTimeout;
            tvTimeout.tv_usec = (long)0;
			FD_ZERO(&fds_Check_Write);
			FD_SET(hSockHandle, &fds_Check_Write);
			nStatus = select(hSockHandle+1, NULL, &fds_Check_Write, NULL, &tvTimeout);
			if(nStatus != 1)
			{
				nLastSocketError = sock_GetLastError(0);
				/* Trace function */
				sock_Trace(GTL_TRACE_SEND_ERROR, NULL, 0); 
				return GNET_ERR_SND_DATA;
			}

			nStatus = send(hSockHandle, ptBuffer,nBufferLength,0);
			if(nStatus == SOCKET_ERROR)
			{
				nLastSocketError = sock_GetLastError(0);
				if((nLastSocketError != WSAEWOULDBLOCK) && (nLastSocketError != WSAEINPROGRESS))
				{
					sock_Trace(GTL_TRACE_SEND_ERROR, NULL, 0); 
					return GNET_ERR_SND_DATA;
				}
			}
			else
				nBytesSent += nStatus;
		}
	}

	/* Trace function */
	sock_Trace(GTL_TRACE_SENT_BUFFER, (char *)ptBuffer, nBufferLength);

	return GNET_ERR_OKAY;
}

/*======================================================================================*/
/* Description  : receive a buffer from the network.                                    */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     void* ptBuffer                                                                   */
/*         ptr on buffer to receive the data                                            */
/*                                                                                      */
/*     int nBytesToReceive																*/
/*         nb of bytes to receive														*/
/*                                                                                      */
/* Return Codes : 1 if successful, 0 else                                               */
/*======================================================================================*/
int sock_ReceiveBuffer(ptBuffer, nBytesToReceive)
void  *ptBuffer;
int   nBytesToReceive;
{
	int					nStatus, nReadBytes, nBufferLength;
	char*				pRecBuffer;
	time_t				StartTime, CurTime;
	fd_set				fds_Check_Read;
	SOCK_TIMEVAL		tvTimeout;

	/* Trace function */
    sock_Trace(GTL_TRACE_RECV_ENTER, NULL, nBytesToReceive);

	/* Check pointer on buffer */
	if(!ptBuffer)
		return GNET_ERR_MAF;
  
	/* Receive the buffer */
	if(nSocketIsBlocking)
	{
		/* Blocking socket */
		nReadBytes = 0;
        while(nReadBytes != nBytesToReceive)
        {
			pRecBuffer = (char *)ptBuffer + nReadBytes;
			nBufferLength = nBytesToReceive - nReadBytes;
			nStatus = recv(hSockHandle, pRecBuffer,nBufferLength,0);
			if(nStatus == SOCKET_ERROR)
			{
				nLastSocketError = sock_GetLastError(0);
				/* Trace function */
				sock_Trace(GTL_TRACE_RECV_ERROR, NULL, 0);
				return 0;
			}
            else if(nStatus == 0)
            {
                nLastSocketError = WSAECONNRESET;
                /* Trace function */
                sock_Trace(GTL_TRACE_RECV_ERROR, NULL, 0);
                return 0;
            }
			nReadBytes += nStatus;
		}
	}
	else
	{
		/* Non blocking socket */
		time(&StartTime);
		nReadBytes = 0;
		do
		{
			/* Try to read more bytes */
			pRecBuffer = (char *)ptBuffer + nReadBytes;
			nBufferLength = nBytesToReceive - nReadBytes;
			nStatus = recv(hSockHandle,pRecBuffer,nBufferLength,0);
			if(nStatus == SOCKET_ERROR)
			{
				nLastSocketError = sock_GetLastError(0);
				if((nLastSocketError != WSAEWOULDBLOCK) && (nLastSocketError != WSAEINPROGRESS))
				{
					/* Trace function */
					sock_Trace(GTL_TRACE_RECV_ERROR, NULL, 0);
					return 0;
				}
			}
			else
				nReadBytes += nStatus;

			/* Check if all expected bytes read */
			if(nReadBytes != nBytesToReceive)
			{
				time(&CurTime);
				tvTimeout.tv_sec = ((long)GTL_SOCK_RCV_TIMEOUT - (long)(CurTime-StartTime)) < 0 ? 0 : ((long)GTL_SOCK_RCV_TIMEOUT - (long)(CurTime-StartTime));
				tvTimeout.tv_usec = (long)0;
				/* Check if socket ready for reading */
				FD_ZERO(&fds_Check_Read);
				FD_SET(hSockHandle, &fds_Check_Read);
				if(select(hSockHandle+1, &fds_Check_Read, NULL, NULL, &tvTimeout) != 1)
				{
					nLastSocketError = WSAETIMEDOUT;
					/* Trace function */
					sock_Trace(GTL_TRACE_RECV_ERROR, NULL, 0);
					return 0;
				}
				else if((CurTime - StartTime) > (time_t)GTL_SOCK_RCV_TIMEOUT)  
				{
					nLastSocketError = WSAETIMEDOUT;
					/* Trace function */
					sock_Trace(GTL_TRACE_RECV_ERROR, NULL, 0);
					return 0;
				}
			}
		}
		while(nReadBytes != nBytesToReceive);
	}
  
	/* Trace function */
	sock_Trace(GTL_TRACE_RECEIVED_BUFFER, (char *)ptBuffer, nReadBytes);

	return 1;
}

/*======================================================================================*/
/* Description  : check if header is valid.                                             */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     PT_GNM_HEADER pHeader                                                            */
/*         pointer on header structure                                                  */
/*                                                                                      */
/* Return Codes : 1 if header is valid, 0 else                                          */
/*======================================================================================*/
int sock_IsValidHeader(pHeader)
PT_GNM_HEADER pHeader;
{
	if(pHeader == NULL)
		return 0;

	/*Control message length */
    if(pHeader->mMessageLength == 0)
		return 0;

	/* Control message type  */
    if((pHeader->mMessageType < GNM_TYPE_BEGIN_NUMBER) ||
       (pHeader->mMessageType > GNM_TYPE_END_NUMBER))
	   return 0;

	return 1;
}

/*======================================================================================*/
/* Description  : get last network error from system.                                   */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nSelect                                                                      */
/*         used under unix to select between errno and h_errno                          */
/*                                                                                      */
/* Return Codes : last network error code                                               */
/*======================================================================================*/
int sock_GetLastError(nSelect)
int nSelect;
{
#if defined unix || __MACH__
	switch(nSelect)
	{
		case 0:
			return errno;
		case 1:
			return h_errno;
		default:
			return errno;
	}
#else
    /* To avoid warning on unused parameter */
    nSelect = 0;

    return WSAGetLastError();
#endif
}

/*======================================================================================*/
/* Description  : socket trace function.                                                */
/*                                                                                      */
/* Argument(s)  :                                                                       */
/*                                                                                      */
/*     int nTraceCode                                                                   */
/*         trace message ID                                                             */
/*                                                                                      */
/*     char *ptBuffer                                                                   */
/*         network buffer                                                               */
/*                                                                                      */
/*     unsigned int uiBufferSize                                                        */
/*         size of the network buffer                                                   */
/*                                                                                      */
/* Return Codes : none                                                                  */
/*======================================================================================*/
void sock_Trace(nTraceCode, ptBuffer, uiBufferSize)
int				nTraceCode;
char			*ptBuffer;
unsigned int	uiBufferSize;
{
    FILE        *fpTraceFile;
    char        szMessage[GNET_TRACE_MSG_SIZE]="";
    char        szTemp[GNET_TRACE_MSG_SIZE]="";
    int         i;
    char        cChar;
    time_t      rawtime;
    struct tm   *timeinfo;

#if defined unix || __MACH__
	char	*szEnv;
#endif
	
	/* Check if trace mode validated */
    if(gtl_stGlobalInfo.mSocketTrace)
	{
        /* Make sure file name has been set, else set it */
        if(strcmp(gtl_stGlobalInfo.mSocketTraceFileName, "") == 0)
            gtl_InitSocketTrace();

		/* Open trace file */
        fpTraceFile = fopen(gtl_stGlobalInfo.mSocketTraceFileName,"a+");
		if(fpTraceFile)
		{
            time(&rawtime);
            timeinfo = localtime (&rawtime);
            strftime(szTemp,GNET_TRACE_MSG_SIZE,"%H:%M:%S", timeinfo);

            /* Client or Server ? */
#if defined unix || __MACH__
            sprintf(szMessage, "[CLT:%lu] %s: ", (unsigned long)getpid(), szTemp);
#else
            sprintf(szMessage, "[CLT:%lu] %s: ", (unsigned long)_getpid(), szTemp);
#endif

			/* Check trace code */
			switch(nTraceCode)
			{
#if 0
				case GTL_TRACE_SENT_BUFFER:
					sprintf(szTemp, "Buffer [%d] :", uiBufferSize-8);
					strcpy(szMessage, szTemp);
					for(i=0 ; i<10 && i<(int)uiBufferSize-8 ; i++)
					{
						cChar = *(ptBuffer+i+8);
						sprintf(szTemp, " %2X", cChar & 0xff);
						strcat(szMessage, szTemp);
					}
					strcat(szMessage, " ...");
					break;
#endif

				case GTL_TRACE_SENT_BUFFER:
					sprintf(szTemp, "Sent Buffer [%d] :", uiBufferSize);
					strcat(szMessage, szTemp);
					for(i=0 ; i<12 && i<(int)uiBufferSize ; i++)
					{
						cChar = *(ptBuffer+i);
						sprintf(szTemp, " %2X", cChar & 0xff);
						strcat(szMessage, szTemp);
					}
					strcat(szMessage, " ...");
					break;

				case GTL_TRACE_RECEIVED_BUFFER:
					sprintf(szTemp, "Received Buffer [%d] :", uiBufferSize);
					strcat(szMessage, szTemp);
					for(i=0 ; i<12 && i<(int)uiBufferSize ; i++)
					{
						cChar = *(ptBuffer+i);
						sprintf(szTemp, " %2X", cChar & 0xff);
						strcat(szMessage, szTemp);
					}
					strcat(szMessage, " ...");
					break;

				case GTL_TRACE_ONRECEIVE:
					strcat(szMessage, "Data Ready.");
					break;

				case GTL_TRACE_ONRECEIVE_DIS:
					strcat(szMessage, "Received OnReceive notification while OnReceive disabled.");
					break;

				case GTL_TRACE_RECV_ENTER:
                    sprintf(szTemp, "Enter a receive function (%d bytes requested).", uiBufferSize);
                    strcat(szMessage, szTemp);
					break;

				case GTL_TRACE_SEND_ENTER:
                    sprintf(szTemp, "Enter a send function (%d bytes to be sent).", uiBufferSize);
                    strcat(szMessage, szTemp);
					break;


				case GTL_TRACE_RECV_ERROR:
                    sprintf(szTemp, "Error receiving a buffer (last error=%d).", nLastSocketError);
                    strcat(szMessage, szTemp);
					break;

				case GTL_TRACE_SEND_ERROR:
                    sprintf(szTemp, "Error sending a buffer (last error=%d).", nLastSocketError);
                    strcat(szMessage, szTemp);
					break;

				case GTL_TRACE_CANCEL_BLOCKING:
					strcat(szMessage, "Blocking call cancelled on timeout.");
					break;

				case GTL_TRACE_RECV_HEADER:
					strcat(szMessage, "Receive HADER.");
					break;

				case GTL_TRACE_RECV_DATA:
					strcat(szMessage, "Receive DATA.");
					break;

				case GTL_TRACE_SEND_ACK_HEADER:
					strcat(szMessage, "Send ACK for HADER.");
					break;

				case GTL_TRACE_SEND_ACK_DATA:
					strcat(szMessage, "Send ACK for DATA.");
					break;

				case GTL_TRACE_SEND_HEADER:
					strcat(szMessage, "Send HEADER.");
					break;

				case GTL_TRACE_SEND_DATA:
					strcat(szMessage, "Send DATA.");
					break;

				case GTL_TRACE_RECV_ACK_HEADER:
					strcat(szMessage, "Receive ACK for HADER.");
					break;

				case GTL_TRACE_RECV_ACK_DATA:
					strcat(szMessage, "Receive ACK for DATA.");
					break;
				default:
					strcat(szMessage, "Unknown trace code.");
			}

			/* Write message to trace file */
			fprintf(fpTraceFile, "%s\n", szMessage);

			/* Close trace file */
			fclose(fpTraceFile);
		}
	}
	return;
}

  /*======================================================================================*/
  /* PRIVATE Functions                                                                    */
  /* Description  : check if socket trace file can be written.                            */
  /* Argument(s)  :                                                                       */
  /* Return Codes : GTL_CORE_ERR_OKAY if initialization OK, GTL_CORE_ERR_XXX error code   */
  /*                else.                                                                 */
  /*======================================================================================*/
  int gtl_InitSocketTrace()
  {
      char lFileName[GTC_MAX_PATH]="";
      char lTimeStamp[UT_MAX_TIMESTAMP_LEN]="";
      char lPID[100]="";

      if(strcmp(gtl_stGlobalInfo.mOutputFolder,"") == 0)
          return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;

      /* Get TimeStamp string */
      strcpy(lTimeStamp, "");
      ut_GetCompactTimeStamp(lTimeStamp);

      /* Get PID */
      strcpy(lPID, "");
      ut_GetPID(lPID);

      /* Set full socket trace file name */
      sprintf(lFileName, "%s/gtl_socket_trace_%s_%s.log", gtl_stGlobalInfo.mOutputFolder, lTimeStamp, lPID);
      ut_NormalizePath(lFileName);
      FILE* f=fopen(lFileName, "a");

      /* Try to open log file in append mode */
      if(!f)
          return GTL_CORE_ERR_INVALID_OUTPUT_FOLDER;

      /* Success, save log file name */
      fclose(f);
      strcpy(gtl_stGlobalInfo.mSocketTraceFileName, lFileName);

      return GTL_CORE_ERR_OKAY;
  }

